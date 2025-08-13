package com.kotlinx.grpcjniclient.wifi

import android.annotation.SuppressLint
import android.content.Context
import android.net.wifi.SoftApConfiguration
import android.net.wifi.SoftApConfiguration.SECURITY_TYPE_WPA3_SAE
import android.net.wifi.SoftApConfiguration.SECURITY_TYPE_WPA3_SAE_TRANSITION
import android.net.wifi.WifiConfiguration
import android.net.wifi.WifiManager
import android.os.Build
import android.util.Log
import android.util.SparseIntArray
import com.kotlinx.grpcjniclient.wifi.module.HostapdInfo
import com.kotlinx.grpcjniclient.wifi.module.HostapdSecurityType
import com.kotlinx.grpcjniclient.wifi.module.HostapdState
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.suspendCancellableCoroutine
import java.lang.reflect.Proxy
import java.net.Inet6Address
import java.net.NetworkInterface
import java.nio.charset.Charset
import java.security.SecureRandom
import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.time.DurationUnit
import kotlin.time.measureTime


class HostapdManager(private val ctx: Context) {

    companion object {
        private const val TAG = "HostapdManager"
    }

    private val mWifiService: WifiManager? = ctx.getSystemService(WifiManager::class.java)

    private var mCurrentHostapdInfo: HostapdInfo? = null

    private val rng = SecureRandom()

    private fun generateSsid(prefix: String = "AP_", digits: Int = 4): String {
        require(digits >= 1)
        val maxBytes = 32
        val digitPart = (0 until digits).map { ('0' + rng.nextInt(10)) }.joinToString("")
        var ssid = prefix + digitPart

        // 如果超出 32 字节（极端情况：prefix 含多字节字符），截断 prefix 部分
        while (ssid.toByteArray(Charset.forName("UTF-8")).size > maxBytes) {
            // 尝试缩短 prefix，保留后面的数字
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

        val upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        val lower = "abcdefghijklmnopqrstuvwxyz"
        val digits = "0123456789"

        val symbols = "!@#\$%&*()-_=+[]{}:;,.?/<>"
        val all = (upper + lower + digits + symbols).toCharArray()

        val pwChars = mutableListOf<Char>()
        pwChars += upper[rng.nextInt(upper.length)]
        pwChars += lower[rng.nextInt(lower.length)]
        pwChars += digits[rng.nextInt(digits.length)]
        pwChars += symbols[rng.nextInt(symbols.length)]

        while (pwChars.size < length) {
            pwChars += all[rng.nextInt(all.size)]
        }

        pwChars.shuffle(rng)
        return pwChars.joinToString("")
    }

    fun getHostapdConfigure(): HostapdInfo? {
        if (isHostapdEnable().not()) return null
        mWifiService?.let {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                //softApConfigure
                getSoftApConfigViaReflection(mWifiService)?.let {
                    Log.d(TAG, "softApConfigure ssid is: ${it.ssid}")
                    Log.d(TAG, "softApConfigure passphrase is: ${it.passphrase}")
                    val bandAndChannel = getHotspotBandBySoftApConfigure(it)
                    Log.d(TAG, "hostapd band is: ${bandAndChannel?.first} channel is: ${bandAndChannel?.second}")
                    val v6Address = getIpv6AddressOfInterface("wlan2").orEmpty()
                    Log.d(TAG, "softApConfigure getIpv6AddressOfInterface is: $v6Address")
                    mCurrentHostapdInfo = HostapdInfo(
                        it.ssid,
                        it.passphrase,
                        bandAndChannel?.second,
                        v6Address,
                        it.mapSecurityType()
                    )
                    Log.d(TAG, "info is $mCurrentHostapdInfo")
                    return mCurrentHostapdInfo
                } ?: return null
            } else {
                //wifi Configuration
                getWifiApConfiguration(mWifiService)?.let {
                    Log.d(TAG, "wifiConfiguration ssid is: ${it.SSID}")
                    Log.d(TAG, "wifiConfiguration passphrase is: ${it.preSharedKey}")

                    Log.d(TAG, "wifiConfiguration securityType is: getEncryptionType(it)}")
                    val bandAndChannel = getHotspotBandAndChannelByWifiConfigure(it)
                    Log.d(
                        TAG,
                        "hostapd band is: ${bandAndChannel?.first} channel is: ${bandAndChannel?.second}"
                    )
                    val v6Address = getIpv6AddressOfInterface("wlan2")

                    Log.d(TAG, "wifiConfiguration getIpv6AddressOfInterface is: $v6Address")
                    mCurrentHostapdInfo = HostapdInfo(
                        it.SSID,
                        it.preSharedKey,
                        bandAndChannel?.second,
                        v6Address,
                        HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL
                    )
                    Log.d(TAG, "info is $mCurrentHostapdInfo")
                    return mCurrentHostapdInfo
                } ?: return null
            }
        }
        return null
    }

    private fun getHostapdState(): Int {
        return runCatching {
            mWifiService?.javaClass?.getDeclaredMethod("getWifiApState")?.apply {
                isAccessible = true
            }?.invoke(mWifiService) as Int
        }.getOrDefault(HostapdState.WIFI_AP_STATE_FAILED)
    }

    private fun isHostapdEnable() = getHostapdState() == HostapdState.WIFI_AP_STATE_ENABLED

    private fun isHostapdSecurityTypeCorrect(): Boolean {
        return mCurrentHostapdInfo?.mHostapdSecurityType != HostapdSecurityType.WPA3_PERSONAL_ONLY
                && mCurrentHostapdInfo?.mHostapdSecurityType != HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL
    }

    private fun isCurrentHostapdSsidAndPwdCorrect(): Boolean {
        return mCurrentHostapdInfo != null && mCurrentHostapdInfo?.mHostapdSsid.isNullOrEmpty().not() || mCurrentHostapdInfo?.mHostapdPassphrase.isNullOrEmpty().not()
    }

    private fun getSoftApConfigViaReflection(wifiManager: WifiManager): SoftApConfiguration? {
        return runCatching {
            val method = wifiManager.javaClass.getMethod("getSoftApConfiguration")
            method.invoke(wifiManager) as SoftApConfiguration
        }.getOrNull()
    }

    private fun getWifiApConfiguration(wifiManager: WifiManager): WifiConfiguration? {
        return runCatching {
            val method = wifiManager.javaClass.getMethod("getWifiApConfiguration")
            method.invoke(wifiManager) as WifiConfiguration
        }.getOrNull()
    }


    private fun getHotspotBandBySoftApConfigure(softApConfiguration: SoftApConfiguration): Pair<Int?, Int?>? =
        runCatching {
            val getChannelsMethod = softApConfiguration.javaClass.getMethod("getChannels")
            val channels =
                getChannelsMethod.invoke(softApConfiguration) as? SparseIntArray ?: return null

            val band = runCatching {
                channels.keyAt(0)
            }.getOrNull()

            val channel = runCatching {
                channels.valueAt(0)
            }.getOrNull()

            val bandName = when (band) {
                1 -> "2.4 GHz"
                2 -> "5 GHz"
                3 -> "6 GHz"
                else -> "未知频段($band)"
            }
            Log.d(TAG, "hostapd band name: $bandName , hostapd channel is: $channel")

            band to channel
        }.getOrElse {
            it.printStackTrace()
            null
        }

    @SuppressLint("DiscouragedPrivateApi")
    private fun getHotspotBandAndChannelByWifiConfigure(wifiConfiguration: WifiConfiguration): Pair<Int?, Int?>? =
        runCatching {
            val apBand = runCatching {
                val field = wifiConfiguration.javaClass.getDeclaredField("apBand")
                field.isAccessible = true
                field.getInt(wifiConfiguration)
            }.getOrNull()

            val bandName = when (apBand) {
                1 -> "2.4 GHz"
                2 -> "5 GHz"
                3 -> "6 GHz"
                else -> "未知频段($apBand)"
            }

            val apChannel = runCatching {
                val field = wifiConfiguration.javaClass.getDeclaredField("apChannel")
                field.isAccessible = true
                field.getInt(wifiConfiguration)
            }.getOrNull()

            Log.d(TAG, "hostapd band name: $bandName , hostapd channel is: $apChannel")

            apBand to apChannel
        }.getOrElse {
            it.printStackTrace()
            null
        }

    private fun getIpv6AddressOfInterface(interfaceName: String): String? = runCatching {
        val networkInterfaces = NetworkInterface.getNetworkInterfaces()
        networkInterfaces.asSequence()
            .firstOrNull { it.name == interfaceName }
            ?.inetAddresses
            ?.asSequence()
            ?.firstOrNull { !it.isLoopbackAddress && it is Inet6Address }
            ?.hostAddress
            ?.substringBefore('%')
    }.getOrElse {
        it.printStackTrace()
        null
    }

    private fun SoftApConfiguration.mapSecurityType(): HostapdSecurityType {
        return when (securityType) {
            SECURITY_TYPE_WPA3_SAE_TRANSITION -> HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL
            SECURITY_TYPE_WPA3_SAE -> HostapdSecurityType.WPA3_PERSONAL_ONLY
            else -> HostapdSecurityType.WPA3_PERSONAL_ERROR
        }
    }

    @SuppressLint("DiscouragedPrivateApi", "BlockedPrivateApi", "PrivateApi")
    fun setSoftApConfigAndroid12Plus(context: Context, ssid: String, password: String): Boolean {
        return runCatching {





            val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager

            val builderClass = Class.forName("android.net.wifi.SoftApConfiguration\$Builder")
            val builderInstance = builderClass.getDeclaredConstructor().newInstance()

            val setSsidMethod = builderClass.getDeclaredMethod("setSsid", String::class.java)
            setSsidMethod.isAccessible = true
            setSsidMethod.invoke(builderInstance, if (isCurrentHostapdSsidAndPwdCorrect().not()) generateSsid() else ssid)

            val setPassphraseMethod = builderClass.getDeclaredMethod(
                "setPassphrase",
                String::class.java,
                Int::class.javaPrimitiveType
            )
            setPassphraseMethod.isAccessible = true

            //WPA3_WPA2 过渡模式
            setPassphraseMethod.invoke(builderInstance, if (isCurrentHostapdSsidAndPwdCorrect().not()) generatePassword() else password, 2)

            val setBandMethod = builderClass.getDeclaredMethod(
                "setBand",
                Int::class.javaPrimitiveType
            )
            setBandMethod.isAccessible = true
            // 5GHz 常量值
            setBandMethod.invoke(builderInstance, 2)

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

    @SuppressLint("DiscouragedPrivateApi", "BlockedPrivateApi", "PrivateApi")
    suspend fun stopHotspotAndroid12PlusReflectWithCallback(context: Context): Boolean = suspendCancellableCoroutine { cont ->
        runCatching {
            val tetheringManager = context.getSystemService("tethering") ?: throw RuntimeException("not get TetheringManager")
            val tetheringManagerClass = tetheringManager.javaClass
            val stopTetheringMethod = tetheringManagerClass.getDeclaredMethod("stopTethering", Int::class.javaPrimitiveType)
            stopTetheringMethod.isAccessible = true

            val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager
            val softApCallbackClass = Class.forName("android.net.wifi.WifiManager\$SoftApCallback")

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
                        val unregisterMethod = wifiManager.javaClass.getMethod("unregisterSoftApCallback", softApCallbackClass)
                        unregisterMethod.invoke(wifiManager, proxy)
                    }
                }
                null
            }

            val registerMethod = wifiManager.javaClass.getMethod("registerSoftApCallback", Executor::class.java, softApCallbackClass)
            registerMethod.invoke(wifiManager, context.mainExecutor, callbackProxy)

            stopTetheringMethod.invoke(tetheringManager, 0)

            cont.invokeOnCancellation {
                val unregisterMethod = wifiManager.javaClass.getMethod("unregisterSoftApCallback", softApCallbackClass)
                unregisterMethod.invoke(wifiManager, callbackProxy)
            }
        }
    }

    /**
     * Android 12+ 使用 TetheringManager 来开启热点
     */
    @SuppressLint("PrivateApi")
    private suspend fun startHotspotAndroid12PlusReflect(context: Context) = suspendCancellableCoroutine { cont ->
        Log.d(TAG, "startHotspotAndroid12PlusReflect invoke")
        runCatching {
            val tetheringManager =
                context.getSystemService("tethering") ?: throw RuntimeException("not get TetheringManager")
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

            val callbackClass = Class.forName("android.net.TetheringManager\$StartTetheringCallback")

            val callbackProxy = Proxy.newProxyInstance(
                callbackClass.classLoader,
                arrayOf(callbackClass)
            ) { _, method, args ->
                when (method.name) {
                    "onTetheringStarted" -> {
                        Log.i(TAG, "hostapd start success")
                        getHostapdConfigure()
                        cont.resume(true)
                    }
                    "onTetheringFailed" -> {
                        Log.i(TAG,"hostapd start failure and error code is : ${args?.get(0)}")
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

    suspend fun isHostapdReady() = coroutineScope {
        Log.d(TAG, "check hostapd ready")
        if (isHostapdEnable().not()) {
            Log.d(TAG, "current hostapd not start, so will start hostapd.")
            if (isHostapdSecurityTypeCorrect()) {
                Log.d(TAG, "hostapd config not compat carplay, so will reConfig hostapd")

                //change security type
                val result = setSoftApConfigAndroid12Plus(ctx, "AP_12342222", "88888888")
                Log.d(TAG, "softApConfiguration is: $result")

                //close ap
                val stopApTakeMillSeconds = measureTime {
                    stopHotspotAndroid12PlusReflectWithCallback(ctx)
                }.toInt(DurationUnit.MILLISECONDS)
                Log.i(TAG, "stopApTakeMillSeconds toke $stopApTakeMillSeconds MillSeconds")
            }
            val startApTakeMillSeconds = measureTime {
                startHotspotAndroid12PlusReflect(ctx)
            }.toInt(DurationUnit.MILLISECONDS)
            Log.i(TAG, "startApTakeMillSeconds toke $startApTakeMillSeconds MillSeconds")

            return@coroutineScope startApTakeMillSeconds
        } else {
            Log.d(TAG, "current hostapd started, next to check")
            if (isHostapdSecurityTypeCorrect()) {
                Log.d(TAG, "hostapd config not compat carplay, so will reConfig hostapd")
                //change security type
                val result = setSoftApConfigAndroid12Plus(ctx, "AP_1234", "88888888")
                Log.d(TAG, "softApConfiguration is: $result")

                //close ap
                val stopApTakeMillSeconds = measureTime {
                    stopHotspotAndroid12PlusReflectWithCallback(ctx)
                }.toInt(DurationUnit.MILLISECONDS)
                Log.i(TAG, "stopApTakeMillSeconds toke $stopApTakeMillSeconds MillSeconds")

                //open ap
                val startApTakeMillSeconds = measureTime {
                    startHotspotAndroid12PlusReflect(ctx)
                }.toInt(DurationUnit.MILLISECONDS)
                Log.i(TAG, "startApTakeMillSeconds toke $startApTakeMillSeconds MillSeconds")

                return@coroutineScope startApTakeMillSeconds
            } else {
                Log.d(TAG, "hostapd config is compat carplay")
                return@coroutineScope true
            }
        }
    }
}