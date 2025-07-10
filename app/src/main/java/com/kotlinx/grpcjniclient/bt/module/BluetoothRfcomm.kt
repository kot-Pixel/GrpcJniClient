package com.kotlinx.grpcjniclient.bt.module

import com.kotlinx.grpcjniclient.bt.BluetoothRfcommChannel
import java.util.UUID

data class BluetoothRfcomm(
    val btRfcommChannel: BluetoothRfcommChannel,
    val btRfcommChannelUUID: UUID,
    val btDeviceAddress: String,
    val btDeviceName: String? = null,
    val btRfcommChannelState: BtRfcommChannelState = BtRfcommChannelState.INIT
)