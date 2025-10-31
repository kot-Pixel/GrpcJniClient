package com.kotlinx.grpcjniclient

import android.annotation.SuppressLint
import android.app.Service
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.os.IBinder
import android.util.Log
import com.kotlinx.grpcjniclient.bt.CarplayBluetoothService
import com.kotlinx.grpcjniclient.bt.IAP2_BT_DEVICE
import com.kotlinx.grpcjniclient.bt.checkBtUuidSupportIap2
import com.kotlinx.grpcjniclient.bt.event.BluetoothEvent
import com.kotlinx.grpcjniclient.bt.event.BluetoothEvent.BondChangedEvent.Companion.BOND_STATE_BONDED
import com.kotlinx.grpcjniclient.bt.transformUuidsUpper
import com.kotlinx.grpcjniclient.bt.uuidString2SUUID
import com.kotlinx.grpcjniclient.rpc.CarplayRpcManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class CarplayService: Service() {
    companion object {
        private const val TAG = "CarplayService"
    }

    private var mBluetoothServiceScope: CoroutineScope? = null

    private var mBluetoothServiceBinder: CarplayBluetoothService.CarplayBluetoothServiceBinder? = null

    private var mRpcServiceBinder: CarplayRpcManager.CarplayRpcServiceBinder? = null

    private val mBluetoothServiceConnection: ServiceConnection = object: ServiceConnection {

        @SuppressLint("MissingPermission")
        override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
            Log.i(TAG, "onServiceConnected: BluetoothService Proxy connect")
            mBluetoothServiceBinder = service as CarplayBluetoothService.CarplayBluetoothServiceBinder
            mBluetoothServiceScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
            mBluetoothServiceScope?.launch {
                mBluetoothServiceBinder?.carplayBluetoothEventSharedFlow()?.collect { event ->
                    Log.i(TAG, "event changed $event")
                    when(event) {
                        is BluetoothEvent.BondChangedEvent -> {
                            if (event.changedValue == BOND_STATE_BONDED) {
                                if (event.device.uuids.orEmpty().transformUuidsUpper().checkBtUuidSupportIap2()) {
//                                    mCarplayRuntime.startBtIap2Link(event.device.address)
                                    IAP2_BT_DEVICE.uuidString2SUUID().getOrNull()?.let {
                                        mBluetoothServiceBinder?.startRfcommTransform(event.device, it)
                                    }
                                } else {
                                    Log.i(TAG, "BondChangedEvent bt not support device iap2 uuid")
                                }
                            }
                        }
                        is BluetoothEvent.ConnectStateChangedEvent -> {

                        }

                        is BluetoothEvent.UuidChangedEvent -> {
                            Log.i(TAG, "uidChangedEvent: uuids is ${event.uuids.joinToString(separator = " | ")}")
                        }
                        else -> { /* do nothing */}
                    }
                }
            }

//            mBluetoothServiceScope?.launch {
//                mBluetoothServiceBinder?.carplayBluetoothRfcommTransformSharedFlow()?.collect {
//
//                }
//            }


        }

        override fun onServiceDisconnected(name: ComponentName?) {
            Log.i(TAG, "onServiceConnected: BluetoothService Proxy Disconnect")
            mBluetoothServiceScope?.cancel()
            mBluetoothServiceScope = null
            mBluetoothServiceBinder = null
        }
    }


    private val mRpcServiceConnection : ServiceConnection = object: ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
            mRpcServiceBinder = service as CarplayRpcManager.CarplayRpcServiceBinder

        }

        override fun onServiceDisconnected(name: ComponentName?) {
            mRpcServiceBinder = null
        }
    }

    private fun startAllServices() {
        //bluetooth Service
        bindService(
            Intent(this, CarplayBluetoothService::class.java),
            mBluetoothServiceConnection,
            Context.BIND_AUTO_CREATE
        )

        //hostapd Service

        //rpc Service
        bindService(
            Intent(this, CarplayRpcManager::class.java),
            mRpcServiceConnection,
            Context.BIND_AUTO_CREATE
        )

        //audio Service

        //video Service

        //sensor Service
    }



    override fun onCreate() {
        super.onCreate()
        Log.i(TAG, "onCreate: CarplayService")

        startAllServices()
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onDestroy() {
        super.onDestroy()
    }
}