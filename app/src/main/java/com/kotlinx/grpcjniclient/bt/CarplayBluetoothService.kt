package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.IBinder
import android.util.Log


class CarplayBluetoothService: Service() {

    private val TAG: String = "CarplayBluetoothService"

    private val btIntentFilter: IntentFilter = IntentFilter().apply {
        addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED)
        addAction(BluetoothDevice.ACTION_PAIRING_REQUEST)
        addAction(BluetoothAdapter.ACTION_STATE_CHANGED)
        addAction(BluetoothDevice.ACTION_ACL_CONNECTED)
        addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED)
        addAction(BluetoothDevice.ACTION_UUID)
        addAction(BluetoothDevice.ACTION_FOUND)
    }

    private val _bluetoothReceiver: BroadcastReceiver = object : BroadcastReceiver() {

        @SuppressLint("NewApi")
        override fun onReceive(context: Context?, intent: Intent?) {
            context ?: kotlin.run {
                Log.e(TAG, "_bluetoothReceiver context is null")
                return
            }
            intent ?: run {
                Log.e(TAG, "_bluetoothReceiver intent is null")
                return
            }

            intent.action?.let { btAction ->
                Log.d(TAG, "$TAG onReceive: action is: $btAction")

                when(btAction) {
                    BluetoothDevice.ACTION_BOND_STATE_CHANGED -> {
                        intent.onBondStateChanged()
                    }
                }
            }
        }
    }

    private fun registerBtBroadcastReceiver() {
        registerReceiver(_bluetoothReceiver, btIntentFilter)
    }


    private fun unRegisterBtBroadcastReceiver() {
        unregisterReceiver(_bluetoothReceiver)
        Log.d(TAG, "unRegisterBtBroadcastReceiver")
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        registerBtBroadcastReceiver()
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        unRegisterBtBroadcastReceiver()
    }
}