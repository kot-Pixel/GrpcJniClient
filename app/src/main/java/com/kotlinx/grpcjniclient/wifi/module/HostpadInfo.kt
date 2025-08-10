package com.kotlinx.grpcjniclient.wifi.module

data class HostapdInfo(
    val mHostapdSsid: String?,
    val mHostapdPassphrase: String?,
    val mHostapdChannel:Int?,
    val mHostapdIPAddressV6:String?,
    val mHostapdSecurityType: HostapdSecurityType
)
