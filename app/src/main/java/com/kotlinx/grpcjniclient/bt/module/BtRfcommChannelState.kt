package com.kotlinx.grpcjniclient.bt.module

enum class BtRfcommChannelState {
    INIT,
    AVAILABLE,
    TRANSFORMING,
    UNAVAILABLE
}