package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothProfile
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Binder
import android.os.IBinder
import android.util.Log
import com.kotlinx.grpcjniclient.bt.profile.BluetoothProfileManager
import java.util.UUID

class BluetoothService: Service() {

    private val mBluetoothDeviceObservers = mutableListOf<BluetoothDeviceObserver>()

    fun aclChanged(device: BluetoothDevice, changedValue: Boolean) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onAclChanged(device, changedValue)
            }
        }
    }

    fun a2dpSinkProfileChanged(device: BluetoothDevice, a2dpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onA2dpSinkProfileChanged(device, a2dpState)
            }
        }
    }

    fun hfpClientProfileChanged(device: BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onHfpClientProfileChanged(device, hfpState)
            }
        }
    }

    fun avrcpControlProfileChanged(device:BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onAvrcpControlProfileChanged(device, hfpState)
            }
        }
    }

    fun pbapClientProfileChanged(device:BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onPbapClientProfileChanged(device, hfpState)
            }
        }
    }

    fun connectStateChanged(device: BluetoothDevice, connectState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onConnectStateChanged(device, connectState)
            }
        }
    }

    fun deviceFoundChanged(device: BluetoothDevice) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onDeviceFoundChanged(device)
            }
        }
    }

    fun uuidChanged(device: BluetoothDevice, uuid: UUID) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.uuidChanged(device, uuid)
            }
        }
    }

    fun onBondStateChanged(device: BluetoothDevice, connectState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onBondChanged(device, connectState)
            }
        }
    }

    fun disableA2dpProfile(device: BluetoothDevice) {
        mBluetoothProfileManager?.disableDeviceA2dpProfile(device)
    }

    fun disableHfpProfile(device: BluetoothDevice) {
        mBluetoothProfileManager?.disableDeviceHfpProfile(device)
    }

    inner class BluetoothServiceBinder: Binder() {
        fun getBluetoothService() = this@BluetoothService
    }

    private val mBluetoothServiceBinder = BluetoothServiceBinder()

    companion object {
        private const val TAG: String = "CarplayBluetoothService"
        const val ACTION_A2DP_SINK_STATE = "android.bluetooth.a2dp-sink.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_HFP_CLIENT_STATE = "android.bluetooth.headsetclient.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_AVRCP_CONTROL_STATE = "android.bluetooth.avrcp-controller.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_PBAP_CLIENT_STATE = "android.bluetooth.pbapclient.profile.action.CONNECTION_STATE_CHANGED"
    }

    private var mBluetoothProfileManager: BluetoothProfileManager? = null

    private val btIntentFilter: IntentFilter = IntentFilter().apply {
        addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED)
        addAction(BluetoothDevice.ACTION_PAIRING_REQUEST)
        addAction(BluetoothAdapter.ACTION_STATE_CHANGED)
        addAction(BluetoothDevice.ACTION_ACL_CONNECTED)
        addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED)
        addAction(BluetoothDevice.ACTION_UUID)
        addAction(BluetoothDevice.ACTION_FOUND)

        addAction(ACTION_A2DP_SINK_STATE)
        addAction(ACTION_HFP_CLIENT_STATE)
        addAction(ACTION_AVRCP_CONTROL_STATE)
        addAction(ACTION_PBAP_CLIENT_STATE)
    }

    @SuppressLint("MissingPermission")
    fun Intent.onBondStateChanged() {
        val bondState: Int = getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.ERROR)

        when(bondState) {
            Integer.MIN_VALUE -> {
                Log.e(TAG, "btDevice bond state is error")
            }

            BluetoothDevice.BOND_BONDING -> {
                Log.d(TAG, "btDevice bonding...")
            }

            BluetoothDevice.BOND_BONDED-> {
                Log.d(TAG, "btDevice bonded...")

                val btDevice: BluetoothDevice? =
                    getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                btDevice?.run {
                    Log.d(TAG, "btDevice name is $name")
                    Log.d(TAG, "btDevice is address $address")
                    Log.d(TAG, "btDevice is uuids ${uuids.joinToString(",")}")

                    val uuidsLists = uuids.transformUuidsUpper()

                    Log.d(TAG, "btDevice is uuids ${
                        uuidsLists.joinToString("\n")}")

                    Log.d(TAG, "btDevice check support iap2 ${
                        uuidsLists.checkBtUuidSupportIap2()}")

                    if (uuidsLists.checkBtUuidSupportIap2()) {
                        BluetoothRfcommManager.connectIap2DeviceProtoc(this)
                    }
                } ?: run {
                    Log.d(TAG, "btDevice is null so return")
                    return
                }

            }

            BluetoothDevice.BOND_NONE -> {
                Log.d(TAG, "btDevice cancel bond or bond failure")
            }
        }
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
                    BluetoothDevice.ACTION_ACL_CONNECTED -> {
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        Log.d(TAG, "---- ACTION_ACL_CONNECTED ----")
                        btDevice?.let { aclChanged(it, true) }
                    }

                    BluetoothDevice.ACTION_ACL_DISCONNECTED -> {
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        Log.d(TAG, "---- ACTION_ACL_DISCONNECTED ----")
                        btDevice?.let { aclChanged(it, false) }
                    }

                    BluetoothDevice.ACTION_BOND_STATE_CHANGED -> {
                        Log.d(TAG, "---- ACTION_BOND_STATE_CHANGED ----")
                        val bondState: Int = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.BOND_NONE)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
//                        intent.onBondStateChanged()
                        btDevice?.let { onBondStateChanged(btDevice, bondState) }
                    }

                    BluetoothAdapter.ACTION_STATE_CHANGED -> {
                        Log.d(TAG, "---- ACTION_STATE_CHANGED ----")
                        val connectState: Int = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.ERROR)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { connectStateChanged(btDevice, connectState) }
                    }

                    ACTION_A2DP_SINK_STATE -> {
                        Log.d(TAG, "---- ACTION_A2DP_STATE ----")
                        val a2dpState: Int = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,BluetoothProfile.STATE_DISCONNECTED)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { a2dpSinkProfileChanged(btDevice, a2dpState) }
                    }

                    ACTION_HFP_CLIENT_STATE -> {
                        Log.d(TAG, "---- ACTION_HFP_STATE ----")
                        val hfpState: Int = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,BluetoothProfile.STATE_DISCONNECTED)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { hfpClientProfileChanged(btDevice, hfpState) }
                    }

                    ACTION_AVRCP_CONTROL_STATE -> {
                        Log.d(TAG, "---- ACTION_AVRCP_CONTROL_STATE ----")
                        val hfpState: Int = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,BluetoothProfile.STATE_DISCONNECTED)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { avrcpControlProfileChanged(btDevice, hfpState) }
                    }

                    ACTION_PBAP_CLIENT_STATE -> {
                        Log.d(TAG, "---- ACTION_PBAP_CLIENT_STATE ----")
                        val hfpState: Int = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,BluetoothProfile.STATE_DISCONNECTED)
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { pbapClientProfileChanged(btDevice, hfpState) }
                    }

                    BluetoothDevice.ACTION_FOUND -> {
                        Log.d(TAG, "---- ACTION_FOUND ----")
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        btDevice?.let { deviceFoundChanged(btDevice) }
                    }

                    BluetoothDevice.ACTION_UUID -> {
                        Log.d(TAG, "---- ACTION_UUID ----")
                        val btDevice: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        val btDeviceUUID: UUID? = intent.getParcelableExtra(BluetoothDevice.EXTRA_UUID) as UUID?
                        btDevice?.let {
                            btDeviceUUID?.let { uuid ->
                                uuidChanged(btDevice, uuid)
                            }
                        }
                    }

                    else -> { /* do nothing */ }
                }
            }
        }
    }

    private fun registerBtBroadcastReceiver() {
        registerReceiver(_bluetoothReceiver, btIntentFilter)
        Log.d(TAG, "registerBtBroadcastReceiver")
    }


    private fun unRegisterBtBroadcastReceiver() {
        unregisterReceiver(_bluetoothReceiver)
        Log.d(TAG, "unRegisterBtBroadcastReceiver")
    }

    override fun onCreate() {
        super.onCreate()
        Log.d(TAG, "invoke bt service onCreate")
        mBluetoothProfileManager = BluetoothProfileManager(this)
        registerBtBroadcastReceiver()
    }

    override fun onBind(intent: Intent?): IBinder {
        return mBluetoothServiceBinder
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(TAG, "invoke bt service onDestroy")
        unRegisterBtBroadcastReceiver()
    }
}