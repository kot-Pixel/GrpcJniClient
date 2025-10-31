package com.kotlinx.grpcjniclient.bt

import android.bluetooth.BluetoothDevice
import android.os.ParcelUuid
import java.util.UUID

interface BluetoothDeviceObserver {
    fun onAclChanged(bluetoothDevice: BluetoothDevice, changedValue: Boolean)

    fun onA2dpSinkProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)

    fun onHfpClientProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)

    fun onAvrcpControlProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)

    fun onPbapClientProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)

    fun onConnectStateChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)

    fun onDeviceFoundChanged(bluetoothDevice: BluetoothDevice)

    fun uuidChanged(bluetoothDevice: BluetoothDevice, uuids: Array<out ParcelUuid>)

    fun onBondChanged(bluetoothDevice: BluetoothDevice, changedValue: Int)
}