package com.kotlinx.grpcjniclient.bt

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.content.Intent
import android.os.Binder
import android.os.IBinder
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresPermission
import com.kotlinx.grpcjniclient.bt.event.BluetoothEvent
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.CoroutineStart
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.launch
import java.util.UUID

class CarplayBluetoothService : BluetoothService(), BluetoothDeviceObserver {

    companion object {
        private const val TAG = "CarplayBluetoothService"
    }

    private var mRfcommEventCollectJob: Job? = null

    private val mCarplayBluetoothSharedFlow = MutableSharedFlow<BluetoothEvent>(
        replay = 0,
        extraBufferCapacity = 10,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    inner class CarplayBluetoothServiceBinder: Binder() {
        fun carplayBluetoothService() = this@CarplayBluetoothService

        fun carplayBluetoothEventSharedFlow() = mCarplayBluetoothSharedFlow

        fun disableA2dpProfile(address: String) {
            mBluetoothProfileManager?.disableA2dpSinkProfileByAddress(address)
        }

        fun disableHfpProfile(address: String) {
            mBluetoothProfileManager?.disableHfpClientProfileByAddress(address)
        }

        fun disablePbapProfile(address: String) {
            mBluetoothProfileManager?.disablePbapClientProfileByAddress(address)
        }

        fun disableAvrcpProfile(address: String) {
            mBluetoothProfileManager?.disableAvrcpClientProfileByAddress(address)
        }


        fun startRfcommTransform(device: BluetoothDevice, uuid: UUID) {
            mBluetoothServiceScope.launch {
                mBluetoothRfcommManager?.startRfcommTransportJob(device, uuid)
            }
        }

        fun stopRfcommTransform(device: BluetoothDevice, uuid: UUID) {
            mBluetoothServiceScope.launch {
                mBluetoothRfcommManager?.stopRfcommTransportJob(device, uuid)
            }
        }

        fun sendDataByRfcomm(device: BluetoothDevice, uuid: UUID, byteArray: ByteArray) {
            mBluetoothRfcommManager?.sendDataByRfcomm(device, uuid, byteArray)
        }

        fun disconnectBluetoothBinder(device: BluetoothDevice) = disconnect(device)
    }

    private val mCarplayBluetoothServiceBinder = CarplayBluetoothServiceBinder()

    override fun onCreate() {
        super.onCreate()
        mBluetoothDeviceObservers.add(this)
        mRfcommEventCollectJob = mBluetoothServiceScope.launch {
            mBluetoothRfcommManager?.rfcommChannelDataSharedFlow?.collect { transformItem ->
                mCarplayBluetoothSharedFlow.tryEmit(
                    BluetoothEvent.RfcommTransformEvent(
                        device = transformItem.bluetoothRfcommChannel.device,
                        transformItem.bluetoothRfcommChannel.uuid,
                        transformItem.byteArray
                    )
                )
            }
        }
    }

    override fun onBind(intent: Intent?): IBinder {
        return mCarplayBluetoothServiceBinder
    }

    override fun onDestroy() {
        super.onDestroy()
        mBluetoothDeviceObservers.remove(this)
        mRfcommEventCollectJob?.cancel()
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    override fun onAclChanged(bluetoothDevice: BluetoothDevice, changedValue: Boolean) {
        Log.d(TAG, "onAclChanged, acl state is $changedValue")

        if (changedValue) {
            Log.d(TAG, "acl state is true, will trigger sdp :" +  bluetoothDevice.fetchUuidsWithSdp())
        }
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.AclChangedEvent(bluetoothDevice, changedValue))
    }

    override fun onA2dpSinkProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        Log.d(TAG, "onA2dpSinkProfileChanged, a2dp sink profile state is $changedValue")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.A2dpSinkProfileChangedEvent(bluetoothDevice, changedValue))
    }

    override fun onHfpClientProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        Log.d(TAG, "onHfpClientProfileChanged, headset client profile state is $changedValue")
            mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.HfpClientProfileChangedEvent(bluetoothDevice, changedValue))
    }

    override fun onAvrcpControlProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        Log.d(TAG, "onAvrcpControlProfileChanged, avrcp control profile state is $changedValue")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.AvrcpControlProfileChangedEvent(bluetoothDevice, changedValue))
    }

    override fun onPbapClientProfileChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.PbapClientProfileChangedEvent(bluetoothDevice, changedValue))
    }

    override fun onConnectStateChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        Log.d(TAG, "onConnectStateChanged, connect state is $changedValue")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.ConnectStateChangedEvent(bluetoothDevice, changedValue))
    }

    @SuppressLint("MissingPermission")
    override fun onDeviceFoundChanged(bluetoothDevice: BluetoothDevice) {
        Log.d(TAG, "onDeviceFoundChanged, connect device name is ${bluetoothDevice.name}")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.DeviceFoundChangedEvent(bluetoothDevice))
    }

    override fun uuidChanged(bluetoothDevice: BluetoothDevice, uuids: Array<out ParcelUuid>) {
        Log.d(TAG, "uuidChanged, uuid")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.UuidChangedEvent(bluetoothDevice, uuids))
    }

    override fun onBondChanged(bluetoothDevice: BluetoothDevice, changedValue: Int) {
        Log.d(TAG, "onBondChanged, bond state is $changedValue")
        mCarplayBluetoothSharedFlow.tryEmit(BluetoothEvent.BondChangedEvent(bluetoothDevice, changedValue))
    }
}