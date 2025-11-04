package com.kotlinx.grpcjniclient.bt

import android.Manifest
import android.annotation.SuppressLint
import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothHeadset
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Build
import android.os.IBinder
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import com.kotlinx.grpcjniclient.bt.profile.BluetoothProfileManager
import com.kotlinx.grpcjniclient.bt.transport.BluetoothRfcommManager
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import java.lang.reflect.Method

open class BluetoothService: Service() {

    companion object {
        private const val TAG: String = "BluetoothService"
        const val ACTION_A2DP_SINK_STATE = "android.bluetooth.a2dp-sink.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_HFP_CLIENT_STATE = "android.bluetooth.headsetclient.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_AVRCP_CONTROL_STATE = "android.bluetooth.avrcp-controller.profile.action.CONNECTION_STATE_CHANGED"
        const val ACTION_PBAP_CLIENT_STATE = "android.bluetooth.pbapclient.profile.action.CONNECTION_STATE_CHANGED"
    }

    protected val mBluetoothDeviceObservers = mutableListOf<BluetoothDeviceObserver>()

    protected var mBluetoothProfileManager: BluetoothProfileManager? = null

    protected var mBluetoothRfcommManager: BluetoothRfcommManager? = null

    protected val mBluetoothServiceScope  = CoroutineScope(Dispatchers.IO + SupervisorJob() + CoroutineName("CarplayBluetoothServiceCoroutine"))

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

                        val parcelables = intent.getParcelableArrayExtra(BluetoothDevice.EXTRA_UUID)
                        val btDeviceUUID = parcelables?.filterIsInstance<ParcelUuid>()?.toTypedArray() ?: emptyArray()

                        btDevice?.let {
                            uuidChanged(btDevice, btDeviceUUID)
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
        mBluetoothRfcommManager = BluetoothRfcommManager()
        registerBtBroadcastReceiver()
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(TAG, "invoke bt service onDestroy")
        unRegisterBtBroadcastReceiver()
        mBluetoothServiceScope.cancel()
    }

    private var tmpCurrentDevice: BluetoothDevice? = null

    private fun aclChanged(device: BluetoothDevice, changedValue: Boolean) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onAclChanged(device, changedValue)
            }
        }

        tmpCurrentDevice = if (changedValue) {
            device
        } else {
            null
        }
    }

    private fun a2dpSinkProfileChanged(device: BluetoothDevice, a2dpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onA2dpSinkProfileChanged(device, a2dpState)
            }
        }
    }

    private fun hfpClientProfileChanged(device: BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onHfpClientProfileChanged(device, hfpState)
            }
        }
    }

    private fun avrcpControlProfileChanged(device:BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onAvrcpControlProfileChanged(device, hfpState)
            }
        }
    }

    private fun pbapClientProfileChanged(device:BluetoothDevice, hfpState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onPbapClientProfileChanged(device, hfpState)
            }
        }
    }

    private fun connectStateChanged(device: BluetoothDevice, connectState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onConnectStateChanged(device, connectState)
            }
        }
    }

    private fun deviceFoundChanged(device: BluetoothDevice) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onDeviceFoundChanged(device)
            }
        }
    }

    private fun uuidChanged(device: BluetoothDevice, uuids: Array<out ParcelUuid>) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.uuidChanged(device, uuids)
            }
        }
    }

    private fun onBondStateChanged(device: BluetoothDevice, connectState: Int) {
        synchronized(mBluetoothDeviceObservers) {
            mBluetoothDeviceObservers.onEach {
                it.onBondChanged(device, connectState)
            }
        }
    }


    /**
     * 断开 Classic 蓝牙设备
     * @param device BluetoothDevice
     * @return 状态码（参考 BluetoothStatusCodes）
     */
    @SuppressLint("DiscouragedPrivateApi", "PrivateApi")
    protected fun disconnect(device: BluetoothDevice): Int {
        return try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) {
                // Android 12 及以下
                val method = device.javaClass.getDeclaredMethod("disconnect")
                method.isAccessible = true
                method.invoke(device) as? Int ?: -2
            } else {
                // Android 13+
                val adapter = BluetoothAdapter.getDefaultAdapter()
                val serviceField = BluetoothAdapter::class.java.getDeclaredField("mService")
                serviceField.isAccessible = true
                val iBluetooth = serviceField.get(adapter)

                    val attributionSourceClass = Class.forName("android.app.AttributionSource")
                    val attributionSourceConstructor =
                        attributionSourceClass.getConstructor(Int::class.javaPrimitiveType)
                    val attributionSource = attributionSourceConstructor.newInstance(0)

                val disconnectMethod = iBluetooth.javaClass.getDeclaredMethod(
                    "disconnectAllEnabledProfiles",
                    BluetoothDevice::class.java,
                    attributionSourceClass
                )
                disconnectMethod.isAccessible = true
                disconnectMethod.invoke(iBluetooth, device, attributionSource) as? Int ?: -2
            }
        } catch (e: Exception) {
            Log.e(TAG, "disconnect failed", e)
            -1
        }
    }
}