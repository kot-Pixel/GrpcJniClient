package com.kotlinx.grpcjniclient.core

import android.bluetooth.BluetoothDevice
import android.content.Context
import android.content.Intent
import android.util.Log
import com.kotlinx.grpcjniclient.bt.CarplayBluetoothService
import com.kotlinx.grpcjniclient.bt.IAP2_BT_DEVICE
import com.kotlinx.grpcjniclient.bt.event.BluetoothEvent
import com.kotlinx.grpcjniclient.bt.event.BluetoothEvent.BondChangedEvent.Companion.BOND_STATE_BONDED
import com.kotlinx.grpcjniclient.bt.uuidString2SUUID
import com.kotlinx.grpcjniclient.rpc.CarplayRpcManager
import com.kotlinx.grpcjniclient.rpc.RpcEvent
import com.kotlinx.grpcjniclient.net.HostapdService
import com.kotlinx.grpcjniclient.net.event.NetEvent
import com.kotlinx.grpcjniclient.net.module.HostapdSecurityType
import com.kotlinx.grpcjniclient.ui.CarplayMainActivity
import com.kotlinx.grpcjniclient.ui.UiService
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class CarplayRuntime {

    companion object {
        private const val TAG = "CarplayRuntime"
    }

    private var mServiceBinderManager: ServiceBinderManager? = null

    private var mBtServiceBinder: CarplayBluetoothService.CarplayBluetoothServiceBinder? = null

    private var mRpcServiceBinder: CarplayRpcManager.CarplayRpcServiceBinder? = null

    private var mHostapdServiceBinder: HostapdService.HostapdLocalBinder? = null

    private var mUiServiceBinder: UiService.UiServiceLocalProxy? = null

    private val mRuntimeCoroutineScope: CoroutineScope =
        CoroutineScope(Dispatchers.IO + CoroutineName("CarplayRuntimeCoroutineScope"))

    private var mBtRfcommDevice: BluetoothDevice? = null

    suspend fun initCarplayRuntime(ctx: Context) {

        mServiceBinderManager = ServiceBinderManager(ctx)

        mBtServiceBinder = mServiceBinderManager?.bindService(CarplayBluetoothService::class.java)

        mRpcServiceBinder = mServiceBinderManager?.bindService(CarplayRpcManager::class.java)

        mHostapdServiceBinder = mServiceBinderManager?.bindService(HostapdService::class.java)

        mUiServiceBinder = mServiceBinderManager?.bindService(UiService::class.java)

        Log.i(TAG, "initCarplayRuntime: mBtServiceBinder is $mBtServiceBinder")
        Log.i(TAG, "initCarplayRuntime: mRpcServiceBinder is $mRpcServiceBinder")
        Log.i(TAG, "initCarplayRuntime: mRpcServiceBinder is $mHostapdServiceBinder")
        Log.i(TAG, "initCarplayRuntime: mRpcServiceBinder is $mUiServiceBinder")

        mRuntimeCoroutineScope.launch {
            mBtServiceBinder?.carplayBluetoothEventSharedFlow()?.collect { event ->
                Log.i(TAG, "carplayBluetoothEventSharedFlow $event")
                when (event) {
                    is BluetoothEvent.BondChangedEvent -> {
                        if (event.changedValue == BOND_STATE_BONDED) {
                            mRpcServiceBinder?.startBtIap2LinkBinder(event.device.address.orEmpty())
                            mBtRfcommDevice = event.device
                            val uuid = IAP2_BT_DEVICE.uuidString2SUUID().getOrNull()
                            if (uuid != null) {
                                mBtServiceBinder?.startRfcommTransform(event.device, uuid)
                            }
                        }
                    }

                    is BluetoothEvent.RfcommTransformEvent -> {
                        mRpcServiceBinder?.receiveBtIap2DataBinder(event.data, event.data.size)
                    }

                    else -> {

                    }
                }
            }
        }


        mRuntimeCoroutineScope.launch {
            mRpcServiceBinder?.getRpcEventSharedFlow()?.collect { event ->
                Log.i(TAG, "getRpcEventSharedFlow $event")
                when (event) {
                    is RpcEvent.Iap2DataEvent -> {
                        mBtRfcommDevice?.let {
                            val uuid = IAP2_BT_DEVICE.uuidString2SUUID().getOrNull()
                            if (uuid != null) {
                                mBtServiceBinder?.sendDataByRfcomm(it, uuid, event.dataByteArray)
                            }
                        }
                    }

                    is RpcEvent.CarplayAvailableEvent -> {
                        if (event.wirelessAvailable) {
                            mHostapdServiceBinder?.getCarplayCompatSoftApConfigureBinder()
                        }
                    }

                    is RpcEvent.DisableBluetoothEvent -> {
                        mBtRfcommDevice?.let {
                            val uuid = IAP2_BT_DEVICE.uuidString2SUUID().getOrNull()
                            if (uuid != null) {
                                mBtServiceBinder?.stopRfcommTransform(it, uuid)
                            }

                            mBtServiceBinder?.disableHfpProfile(event.macAddress)
                            mBtServiceBinder?.disableA2dpProfile(event.macAddress)
                            mBtServiceBinder?.disablePbapProfile(event.macAddress)
                            mBtServiceBinder?.disableAvrcpProfile(event.macAddress)
                        }
                    }

                    is RpcEvent.MediaCodecFormatChangeEvent -> {
                        ctx.startActivity(Intent(ctx, CarplayMainActivity::class.java).apply {
                            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                        })
                    }
                }
            }
        }

        mRuntimeCoroutineScope.launch {
            mHostapdServiceBinder?.hostapdEventSharedFlow()?.collect { event ->
                Log.i(TAG, "mHostapdServiceBinder $event")
                when (event) {
                    is NetEvent.HostapdConfigurationChangedEvent -> {
                        Log.i(TAG, "HostapdConfigurationCompat ${event.info}")
                        mRpcServiceBinder?.startCarplaySessionBinder(
                            event.info?.mHostapdSsid.orEmpty(),
                            event.info?.mHostapdPassphrase.orEmpty(),
                            event.info?.mHostapdChannel ?: 149,
                            event.info?.mHostapdIPAddressV6.orEmpty(),
                            event.info?.mHostapdSecurityType?.value
                                ?: HostapdSecurityType.WPA3_PERSONAL_TRANSITION_MODEL.value)
                    }

                    else -> {}
                }
            }
        }
    }
}