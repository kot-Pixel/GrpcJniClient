package com.kotlinx.grpcjniclient.net

import android.annotation.SuppressLint
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.wifi.SoftApConfiguration
import android.net.wifi.SoftApConfiguration.SECURITY_TYPE_WPA3_SAE
import android.net.wifi.SoftApConfiguration.SECURITY_TYPE_WPA3_SAE_TRANSITION
import android.net.wifi.WifiManager
import android.os.Binder
import android.os.IBinder
import android.util.Log
import android.util.SparseIntArray
import com.kotlinx.grpcjniclient.net.event.NetEvent
import com.kotlinx.grpcjniclient.net.module.HostapdInfo
import com.kotlinx.grpcjniclient.net.module.HostapdSecurityType
import com.kotlinx.grpcjniclient.net.module.HostapdState
import com.kotlinx.grpcjniclient.net.module.HostapdState.WIFI_AP_STATE_FAILED
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.filter
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import java.lang.reflect.Proxy
import java.net.Inet6Address
import java.net.NetworkInterface
import java.nio.charset.Charset
import java.security.SecureRandom
import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlin.time.DurationUnit
import kotlin.time.measureTime

class HostapdService : NetService() {

    companion object {
        private const val WIFI_AP_STATE_CHANGED_ACTION_EXT =
            "android.net.wifi.WIFI_AP_STATE_CHANGED"
        private const val EXTRA_WIFI_AP_STATE_EXT = "wifi_state"
        private const val HOSTAPD_NET_INTERFACE_ADDRESS_V6 = "wlan2"
        private const val TAG = "HostapdService"
    }

    private val mSoftApConfigurationStateFlow: MutableStateFlow<HostapdInfo?> =
        MutableStateFlow(null)

    private var mHostapdIpv6Job: Job? = null

    private var mWifiManager: WifiManager? = null

    private var mHostapdLocalBinder: HostapdLocalBinder = HostapdLocalBinder()

    private val mBtIntentFilter: IntentFilter = IntentFilter(WIFI_AP_STATE_CHANGED_ACTION_EXT)

    private val mBluetoothReceiver: BroadcastReceiver = object : BroadcastReceiver() {

        @SuppressLint("NewApi")
        override fun onReceive(context: Context, intent: Intent) {
            val hostapdState = intent.getIntExtra(EXTRA_WIFI_AP_STATE_EXT, WIFI_AP_STATE_FAILED)
            if (hostapdState == HostapdState.WIFI_AP_STATE_ENABLED) {
                val configuration = getSoftApConfigViaReflection()

                if (configuration != null) {
                    val bandAndChannel = getHotspotBandBySoftApConfigure(configuration)
                    val inetIpv6Address =
                        getIpv6AddressOfInterface("wlan2")

                    Log.i(TAG, "onReceive: inetIpv6Address is $inetIpv6Address")

                    mSoftApConfigurationStateFlow.value = HostapdInfo(
                        mHostapdSsid = configuration.ssid,
                        mHostapdPassphrase = configuration.passphrase,
                        mHostapdBand = bandAndChannel?.first,
                        mHostapdChannel = bandAndChannel?.second,
                        mHostapdIPAddressV6 = inetIpv6Address,
                        mHostapdSecurityType = configuration.mapSecurityType()
                    )
                    Log.i(TAG, "hostapd configuration changed: " + mSoftApConfigurationStateFlow.value)

                    if (isHostapdSecurityTypeCorrect()) {
                        if (mHostapdIpv6Job?.isActive == true) {
                            mHostapdIpv6Job?.cancel()
                            mHostapdIpv6Job = null
                        }
                        mHostapdIpv6Job = mNetServiceScope.launch {
                            mHostapdEventSharedFlow.filter {
                                it is NetEvent.KernelNetLinkChangedEvent && it.inetName == "wlan2"
                            }.firstOrNull()?.let {
                                mSoftApConfigurationStateFlow.value?.mHostapdIPAddressV6 = (it as NetEvent.KernelNetLinkChangedEvent).inetAddress
                                mHostapdEventSharedFlow.tryEmit(NetEvent.HostapdConfigurationChangedEvent(mSoftApConfigurationStateFlow.value))
                            }
                        }
                    } else {
                        Log.i(TAG, "onReceive: this open hostapd state is not support")
                    }
                }
            } else if (hostapdState == HostapdState.WIFI_AP_STATE_DISABLED) {
                if (mHostapdIpv6Job?.isActive == true) {
                    mHostapdIpv6Job?.cancel()
                    mHostapdIpv6Job = null
                }
            }
        }
    }

    inner class HostapdLocalBinder: Binder() {
        fun hostapdEventSharedFlow() = mHostapdEventSharedFlow
        fun getCarplayCompatSoftApConfigureBinder() = getCarplayCompatSoftApConfigure()
    }

    private fun registerHostapdBroadcastReceiver() {
        registerReceiver(mBluetoothReceiver, mBtIntentFilter)
        Log.d(TAG, "registerBtBroadcastReceiver")
    }


    private fun unRegisterHostapdBroadcastReceiver() {
        unregisterReceiver(mBluetoothReceiver)
        Log.d(TAG, "unRegisterBtBroadcastReceiver")
    }


    override fun onCreate() {
        super.onCreate()
        mWifiManager = getSystemService(Context.WIFI_SERVICE) as WifiManager
        registerHostapdBroadcastReceiver()
        val inetIpv6Address =
            getIpv6AddressOfInterface("wlan2")
        Log.i(TAG, "onReceive: inetIpv6Address is $inetIpv6Address")
    }

    override fun onBind(intent: Intent?): IBinder {
        return mHostapdLocalBinder
    }

    private fun getSoftApConfigViaReflection(): SoftApConfiguration? {
        return runCatching {
            val method = mWifiManager?.javaClass?.getMethod("getSoftApConfiguration")
            method?.invoke(mWifiManager) as? SoftApConfiguration?
        }.getOrNull()
    }

    private fun getHostapdState(): Int {
        return runCatching {
            mWifiManager?.javaClass?.getDeclaredMethod("getWifiApState")?.apply {
                isAccessible = true
            }?.invoke(mWifiManager) as Int
        }.getOrDefault(WIFI_AP_STATE_FAILED)
    }


    private fun isHostapdEnable() = getHostapdState() == HostapdState.WIFI_AP_STATE_ENABLED

    private fun getHotspotBandBySoftApConfigure(softApConfiguration: SoftApConfiguration): Pair<Int?, Int?>? =
        runCatching {
            val getChannelsMethod = softApConfiguration.javaClass.getMethod("getChannels")
            val channels =
                getChannelsMethod.invoke(softApConfiguration) as? SparseIntArray ?: SparseIntArray()

            val band = runCatching {
                channels.keyAt(0)
            }.getOrNull()

            val channel = runCatching {
                channels.valueAt(0)
            }.getOrNull()

            Log.d(
                TAG, "hostapd band name: ${
                    when (band) {
                        1 -> "2.4 GHz"
                        2 -> "5 GHz"
                        3 -> "6 GHz"
                        else -> "未知频段($band)"
                    }
                } , hostapd channel is: $channel"
            )

            band to channel
        }.getOrNull()

    private fun SoftApConfiguration.mapSecurityType(): HostapdSecurityType {
        return when (securityType) {
            SECURITY_TYPE_WPA3_SAE_TRANSITION -> HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL
            SECURITY_TYPE_WPA3_SAE -> HostapdSecurityType.WPA3_PERSONAL_ONLY
            else -> HostapdSecurityType.WPA3_PERSONAL_ERROR
        }
    }

    private fun getIpv6AddressOfInterface(interfaceName: String): String = runCatching {
        val networkInterfaces = NetworkInterface.getNetworkInterfaces()
        networkInterfaces.toList().firstOrNull {
            it.name == interfaceName
        }?.inetAddresses?.toList().orEmpty().firstOrNull {
            !it.isLoopbackAddress && it is Inet6Address
        }?.hostAddress?.substringBefore('%').orEmpty()
    }.getOrThrow()

    private fun isHostapdSecurityTypeCorrect(): Boolean {
        val hostapdStateInfo = mSoftApConfigurationStateFlow.value
        Log.i(TAG, "isHostapdSecurityTypeCorrect mHostapdSecurityType is $hostapdStateInfo")

        if (hostapdStateInfo == null) return false
        return hostapdStateInfo.mHostapdSecurityType.let { securityType ->
            securityType == HostapdSecurityType.WPA3_PERSONAL_ONLY ||
                    securityType == HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL
        }
    }

    private fun generateSsid(prefix: String = "AP_", digits: Int = 4): String {
        require(digits >= 1)
        val mSecureRandom = SecureRandom()
        val maxBytes = 32
        val digitPart = (0 until digits).map { ('0' + mSecureRandom.nextInt(10)) }.joinToString("")
        var ssid = prefix + digitPart

        while (ssid.toByteArray(Charset.forName("UTF-8")).size > maxBytes) {
            if (prefix.isNotEmpty()) {
                val newPrefix = prefix.dropLast(1)
                ssid = newPrefix + digitPart
            } else {
                val bytes = ssid.toByteArray(Charset.forName("UTF-8"))
                val truncated = bytes.sliceArray(0 until maxBytes)
                ssid = String(truncated, Charset.forName("UTF-8"))
                break
            }
        }
        return ssid
    }

    private fun generatePassword(length: Int = 12): String {
        require(length in 8..63) { "WPA passphrase length must be 8..63" }

        val mSecureRandom = SecureRandom()

        val upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        val lower = "abcdefghijklmnopqrstuvwxyz"
        val digits = "0123456789"

        val symbols = "!@#\$%&*()-_=+[]{}:;,.?/<>"
        val all = (upper + lower + digits + symbols).toCharArray()

        val pwChars = mutableListOf<Char>()
        pwChars += upper[mSecureRandom.nextInt(upper.length)]
        pwChars += lower[mSecureRandom.nextInt(lower.length)]
        pwChars += digits[mSecureRandom.nextInt(digits.length)]
        pwChars += symbols[mSecureRandom.nextInt(symbols.length)]

        while (pwChars.size < length) {
            pwChars += all[mSecureRandom.nextInt(all.size)]
        }

        pwChars.shuffle(mSecureRandom)
        return pwChars.joinToString("")
    }

    @SuppressLint("DiscouragedPrivateApi", "BlockedPrivateApi", "PrivateApi")
    fun setSoftApConfigAndroid12Plus(context: Context): Boolean {
        return runCatching {

            val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager

            val builderClass = Class.forName("android.net.wifi.SoftApConfiguration\$Builder")
            val builderInstance = builderClass.getDeclaredConstructor().newInstance()

            val setSsidMethod = builderClass.getDeclaredMethod("setSsid", String::class.java)
            setSsidMethod.isAccessible = true
            setSsidMethod.invoke(builderInstance, generateSsid())

            /**
             * 设置加密方式过渡模式。并且重新设置密码
             */
            val setPassphraseMethod = builderClass.getDeclaredMethod(
                "setPassphrase",
                String::class.java,
                Int::class.javaPrimitiveType
            )
            setPassphraseMethod.isAccessible = true
            setPassphraseMethod.invoke(builderInstance, generatePassword(), 2)

            /**
             * 设置频段为5Ghz
             */
            val setBandMethod = builderClass.getDeclaredMethod(
                "setBand",
                Int::class.javaPrimitiveType
            )
            setBandMethod.isAccessible = true
            setBandMethod.invoke(builderInstance, 2)

            /**
             * 设置channel是149， 5Ghz
             */
            val setChannelMethod = builderClass.getDeclaredMethod(
                "setChannel",
                Int::class.javaPrimitiveType,
                Int::class.javaPrimitiveType
            )
            setChannelMethod.isAccessible = true
            setChannelMethod.invoke(builderInstance, 149, 2)

            val buildMethod = builderClass.getDeclaredMethod("build")
            buildMethod.isAccessible = true
            val softApConfig = buildMethod.invoke(builderInstance)

            val setConfigMethod = WifiManager::class.java.getDeclaredMethod(
                "setSoftApConfiguration",
                Class.forName("android.net.wifi.SoftApConfiguration")
            )
            setConfigMethod.isAccessible = true
            setConfigMethod.invoke(wifiManager, softApConfig) as Boolean
        }.getOrDefault(false)
    }

    @SuppressLint("PrivateApi")
    private suspend fun stopHotspotAndroid12PlusReflectWithCallback(context: Context): Boolean =
        suspendCancellableCoroutine { cont ->
            runCatching {
                val tetheringManager = context.getSystemService("tethering")
                    ?: throw RuntimeException("not get TetheringManager")
                val tetheringManagerClass = tetheringManager.javaClass
                val stopTetheringMethod = tetheringManagerClass.getDeclaredMethod(
                    "stopTethering",
                    Int::class.javaPrimitiveType
                )
                stopTetheringMethod.isAccessible = true

                val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager
                val softApCallbackClass =
                    Class.forName("android.net.wifi.WifiManager\$SoftApCallback")

                val callbackProxy = Proxy.newProxyInstance(
                    softApCallbackClass.classLoader,
                    arrayOf(softApCallbackClass)
                ) { proxy, method, args ->
                    if (method.name == "onStateChanged") {
                        val state = args?.get(0) as? Int ?: -1
                        if (state == 11) {
                            if (cont.isActive) {
                                cont.resume(true)
                            }
                            val unregisterMethod = wifiManager.javaClass.getMethod(
                                "unregisterSoftApCallback",
                                softApCallbackClass
                            )
                            unregisterMethod.invoke(wifiManager, proxy)
                        }
                    }
                    null
                }

                val registerMethod = wifiManager.javaClass.getMethod(
                    "registerSoftApCallback",
                    Executor::class.java,
                    softApCallbackClass
                )
                registerMethod.invoke(wifiManager, context.mainExecutor, callbackProxy)

                stopTetheringMethod.invoke(tetheringManager, 0)

                cont.invokeOnCancellation {
                    val unregisterMethod = wifiManager.javaClass.getMethod(
                        "unregisterSoftApCallback",
                        softApCallbackClass
                    )
                    unregisterMethod.invoke(wifiManager, callbackProxy)
                }
            }
        }

    /**
     * Android 12+ 使用 TetheringManager 来开启热点
     */
    @SuppressLint("PrivateApi")
    private suspend fun startHotspotAndroid12PlusReflect(context: Context) =
        suspendCancellableCoroutine { cont ->
            Log.d(TAG, "startHotspotAndroid12PlusReflect invoke")
            runCatching {
                val tetheringManager =
                    context.getSystemService("tethering")
                        ?: throw RuntimeException("not get TetheringManager")
                val tetheringManagerClass = tetheringManager.javaClass

                val tetheringRequestBuilderClass =
                    Class.forName("android.net.TetheringManager\$TetheringRequest\$Builder")

                val builderCtor =
                    tetheringRequestBuilderClass.getDeclaredConstructor(Int::class.javaPrimitiveType)
                builderCtor.isAccessible = true
                val builderInstance = builderCtor.newInstance(0 /* TETHERING_WIFI = 0 */)

                val buildMethod = tetheringRequestBuilderClass.getDeclaredMethod("build")
                buildMethod.isAccessible = true
                val tetheringRequest = buildMethod.invoke(builderInstance)

                val callbackClass =
                    Class.forName("android.net.TetheringManager\$StartTetheringCallback")

                val callbackProxy = Proxy.newProxyInstance(
                    callbackClass.classLoader,
                    arrayOf(callbackClass)
                ) { _, method, args ->
                    when (method.name) {
                        "onTetheringStarted" -> {
                            Log.i(TAG, "hostapd start success")
                            cont.resume(true)
                        }

                        "onTetheringFailed" -> {
                            Log.i(TAG, "hostapd start failure and error code is : ${args?.get(0)}")
                            cont.resume(false)
                        }
                    }
                    null
                }

                val startTetheringMethod = tetheringManagerClass.getDeclaredMethod(
                    "startTethering",
                    tetheringRequest.javaClass,
                    Executor::class.java,
                    callbackClass
                )
                startTetheringMethod.isAccessible = true

                startTetheringMethod.invoke(
                    tetheringManager,
                    tetheringRequest,
                    context.mainExecutor,
                    callbackProxy
                )
            }
        }

    fun getCarplayCompatSoftApConfigure() {
        mNetServiceScope.launch {
            Log.d(TAG, "check hostapd ready")
            if (isHostapdEnable().not()) {
                Log.d(TAG, "current hostapd not start, so will start hostapd.")
                if (!isHostapdSecurityTypeCorrect()) {
                    Log.d(TAG, "hostapd config not compat carplay, so will reConfig hostapd")

                    //change security type
                    val result =
                        setSoftApConfigAndroid12Plus(this@HostapdService)
                    Log.d(TAG, "softApConfiguration is: $result")

                    //open ap
                    val startApTakeMillSeconds = measureTime {
                        startHotspotAndroid12PlusReflect(this@HostapdService)
                    }.toInt(DurationUnit.MILLISECONDS)
                    Log.i(TAG, "startApTakeMillSeconds toke $startApTakeMillSeconds MillSeconds")
                }
            } else {
                Log.d(TAG, "current hostapd started, next to check")
                if (!isHostapdSecurityTypeCorrect()) {
                    Log.d(TAG, "hostapd config not compat carplay, so will reConfig hostapd")
                    //change security type
                    val result =
                        setSoftApConfigAndroid12Plus(this@HostapdService)
                    Log.d(TAG, "softApConfiguration is: $result")

                    //close ap
                    val stopApTakeMillSeconds = measureTime {
                        stopHotspotAndroid12PlusReflectWithCallback(this@HostapdService)
                    }.toInt(DurationUnit.MILLISECONDS)
                    Log.i(TAG, "stopApTakeMillSeconds toke $stopApTakeMillSeconds MillSeconds")

                    //open ap
                    val startApTakeMillSeconds = measureTime {
                        startHotspotAndroid12PlusReflect(this@HostapdService)
                    }.toInt(DurationUnit.MILLISECONDS)
                    Log.i(TAG, "startApTakeMillSeconds toke $startApTakeMillSeconds MillSeconds")
                } else {
                    Log.d(TAG, "hostapd config is compat carplay")
                    mHostapdEventSharedFlow.filter {
                        it is NetEvent.KernelNetLinkChangedEvent && it.inetName == "wlan2"
                    }.firstOrNull()?.let {
                        mSoftApConfigurationStateFlow.value?.mHostapdIPAddressV6 = (it as NetEvent.KernelNetLinkChangedEvent).inetAddress
                        mHostapdEventSharedFlow.tryEmit(NetEvent.HostapdConfigurationChangedEvent(mSoftApConfigurationStateFlow.value))
                    }
                }
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        unRegisterHostapdBroadcastReceiver()
        mWifiManager = null
    }
}