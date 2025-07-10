package com.kotlinx.grpcjniclient.rpc

object BluetoothRpc {
    external fun bluetoothDeviceIap2DeviceState()

    external fun receiveBtIap2Data(rfcommData: ByteArray, dataLength: Int) : Boolean
}