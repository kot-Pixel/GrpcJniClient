package com.kotlinx.grpcjniclient.net.module

/**
 * only support two wifi security type
 */
enum class HostapdSecurityType(val value: Int) {
    WPA3_PERSONAL_TRANSITION_MODEL(3),
    WPA3_PERSONAL_ONLY(4),
    WPA3_PERSONAL_ERROR(-1)
}