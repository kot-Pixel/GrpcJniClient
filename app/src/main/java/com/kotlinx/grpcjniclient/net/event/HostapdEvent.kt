package com.kotlinx.grpcjniclient.net.event

import com.kotlinx.grpcjniclient.net.module.HostapdInfo

sealed class NetEvent {

    data class KernelNetLinkChangedEvent(
        val inetName: String,
        val inetAddress: String
    ): NetEvent()

    data class HostapdConfigurationChangedEvent(
        val info: HostapdInfo?
    ): NetEvent()
}

