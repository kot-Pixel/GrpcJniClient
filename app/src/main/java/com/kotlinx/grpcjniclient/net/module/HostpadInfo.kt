package com.kotlinx.grpcjniclient.net.module

data class HostapdInfo(
    val mHostapdSsid: String?,
    val mHostapdPassphrase: String?,
    val mHostapdBand: Int?,
    val mHostapdChannel: Int?,
    var mHostapdIPAddressV6: String?,
    val mHostapdSecurityType: HostapdSecurityType
)
