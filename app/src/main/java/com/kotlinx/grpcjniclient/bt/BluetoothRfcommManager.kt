package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.util.Log
import com.kotlinx.grpcjniclient.bt.module.BluetoothRfcomm
import com.kotlinx.grpcjniclient.bt.module.BtRfcommChannelState
import com.kotlinx.grpcjniclient.rpc.BluetoothRpc
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

object BluetoothRfcommManager {

    private val TAG = "BluetoothRfcommManager"

    val rfcommMutableList: List<BluetoothRfcomm> get() = _rfcommMutableList

    private val _rfcommMutableList: MutableList<BluetoothRfcomm> = mutableListOf()

    private val _mBluetoothRfcommManagerScope = CoroutineScope(Dispatchers.IO)

    @SuppressLint("MissingPermission")
    fun connectIap2DeviceProtoc(device: BluetoothDevice) {
        IAP2_BT_DEVICE.uuidString2SUUID().onSuccess { uuid ->
            val deviceIap2Channel = BluetoothRfcommChannel(device, uuid)
            deviceIap2Channel.connect().onSuccess {
                Log.d(TAG, "connectIap2DeviceProtoc: onSuccess")
                _rfcommMutableList.add(BluetoothRfcomm(
                    deviceIap2Channel, uuid, device.address, device.name, BtRfcommChannelState.AVAILABLE
                ))

                _mBluetoothRfcommManagerScope.launch {
                    deviceIap2Channel.readLoop {
                        Log.d(TAG, "connectIap2DeviceProtoc: read rfcomm data size is: ${it.size} ")
                        val sendRpcResult = BluetoothRpc.receiveBtIap2Data(it, it.size)
                        Log.d(TAG, "connectIap2DeviceProtoc: read rfcomm data result is: $sendRpcResult ")
                    }
                }
                deviceIap2Channel.write( byteArrayOf(0xFF.toByte(),0x55, 0x02,0x00,0xEE.toByte(),0x10))
            }.onFailure {
                Log.d(TAG, "connectIap2DeviceProtoc: onFailure msg is: ${it.message}")
                deviceIap2Channel.close()
            }
        }
    }
}