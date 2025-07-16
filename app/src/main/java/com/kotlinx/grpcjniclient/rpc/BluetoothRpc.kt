package com.kotlinx.grpcjniclient.rpc

object BluetoothRpc {
    external fun startBtIap2Link(): Boolean

    external fun receiveBtIap2Data(rfcommData: ByteArray, dataLength: Int) : Boolean
}