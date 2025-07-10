package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.util.Log
import com.kotlinx.grpcjniclient.bt.module.BluetoothRfcomm
import com.kotlinx.grpcjniclient.bt.module.BtRfcommChannelState
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
                _rfcommMutableList.add(BluetoothRfcomm(
                    deviceIap2Channel, uuid, device.address, device.name, BtRfcommChannelState.AVAILABLE
                ))

                _mBluetoothRfcommManagerScope.launch {
                    deviceIap2Channel.readLoop {
                        Log.d(TAG, "connectIap2DeviceProtoc: read rfcomm data size is: ${it.size} ")
                    }
                }

            }.onFailure {
                deviceIap2Channel.close()
            }
        }
    }
}