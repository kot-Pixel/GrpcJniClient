package com.kotlinx.grpcjniclient.wifi

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
import java.net.Inet6Address
import java.net.NetworkInterface


class HostapdService(private val ctx: Context) {

    companion object {
        private const val TAG = "HostapdService"
    }

    private val mWifiService: WifiManager? = ctx.getSystemService(WifiManager::class.java)

    private var mCurrentHostapdInfo: HostapdInfo? = null

    fun getHostapdConfigure(): HostapdInfo? {
        if (isHostapdEnable().not()) return null
        mWifiService?.let {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                //softApConfigure
                getSoftApConfigViaReflection(mWifiService)?.let {
                    Log.d(TAG, "softApConfigure ssid is: ${it.ssid}")
                    Log.d(TAG, "softApConfigure passphrase is: ${it.passphrase}")
                    val bandAndChannel = getHotspotBandBySoftApConfigure(it)
                    it.mapSecurityType()
                    Log.d(
                        TAG,
                        "hostapd band is: ${bandAndChannel?.first} channel is: ${bandAndChannel?.second}"
                    )
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

                    return null
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


    private fun getHotspotBandBySoftApConfigure(softApConfiguration: SoftApConfiguration): Pair<Int, Int>? =
        runCatching {
            val getChannelsMethod = softApConfiguration.javaClass.getMethod("getChannels")
            val channels =
                getChannelsMethod.invoke(softApConfiguration) as? SparseIntArray ?: return null

            val band = runCatching {
                channels.keyAt(0)
            }.getOrDefault(2)

            val channel = runCatching {
                channels.valueAt(0)
            }.getOrDefault(149)

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

    private fun getHotspotBandAndChannelByWifiConfigure(wifiConfiguration: WifiConfiguration): Pair<Int, Int>? =
        runCatching {
            val apBand = runCatching {
                val field = wifiConfiguration.javaClass.getDeclaredField("apBand")
                field.isAccessible = true
                field.getInt(wifiConfiguration)
            }.getOrDefault(2)

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
            }.getOrDefault(149)

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

    fun checkHostapdIsReady(): Boolean {
        return mCurrentHostapdInfo != null && isHostapdEnable() && isHostapdSecurityTypeCorrect()
    }


    fun makeHostapdReady() {
        if (isHostapdEnable().not()) {
            if (isHostapdSecurityTypeCorrect().not()) {
                //change security type
            }
//            mWifiService.startTetheredHotspot(null)
            //open ap
        } else {
            if (isHostapdSecurityTypeCorrect().not()) {
                //change security type
            }
            //close ap

            //open ap
        }
    }
}