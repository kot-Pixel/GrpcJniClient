package com.kotlinx.grpcjniclient.bt.profile

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.location.Address
import android.util.Log
import androidx.annotation.RequiresPermission

class BluetoothProfileManager(ctx: Context) {

    companion object {
        private const val TAG = "BluetoothProfileManager"
        private const val BLUETOOTH_PROFILE_A2DP_SINK = 11
        private const val BLUETOOTH_PROFILE_AVRCP_CONTROLLER = 12
        private const val BLUETOOTH_PROFILE_HEADSET_CLIENT = 16
        private const val BLUETOOTH_PROFILE_PBAP_CLIENT = 17
    }

    private var mBluetoothManager: BluetoothManager? = null
    private var mA2dpSinkBluetoothProfileProxy: BluetoothProfile? = null
    private var mHfpClientBluetoothProfileProxy: BluetoothProfile? = null
    private var mAvrcpControlBluetoothProfileProxy: BluetoothProfile? = null
    private var mPbapClientBluetoothProfileProxy: BluetoothProfile? = null

    init {
        mBluetoothManager = ctx.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object : BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                Log.i(TAG, "onServiceConnected: A2DP_SINK")
                mA2dpSinkBluetoothProfileProxy = proxy
            }

            override fun onServiceDisconnected(profile: Int) {
                mA2dpSinkBluetoothProfileProxy = null
                Log.i(TAG, "onServiceDisconnected: A2DP_SINK")
            }

        }, BLUETOOTH_PROFILE_A2DP_SINK)

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object : BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                Log.i(TAG, "onServiceConnected: HEADSET_CLIENT")
                mHfpClientBluetoothProfileProxy = proxy
            }

            override fun onServiceDisconnected(profile: Int) {
                mHfpClientBluetoothProfileProxy = null
                Log.i(TAG, "onServiceDisconnected: HEADSET_CLIENT")
            }

        }, BLUETOOTH_PROFILE_HEADSET_CLIENT)

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object : BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                mAvrcpControlBluetoothProfileProxy = proxy
            }

            override fun onServiceDisconnected(profile: Int) {
                mAvrcpControlBluetoothProfileProxy = null
            }

        }, BLUETOOTH_PROFILE_AVRCP_CONTROLLER)

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object : BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                mPbapClientBluetoothProfileProxy = proxy
            }

            override fun onServiceDisconnected(profile: Int) {
                mPbapClientBluetoothProfileProxy = null
            }

        }, BLUETOOTH_PROFILE_PBAP_CLIENT)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun queryA2dpConnectionStateByBtDevice(device: BluetoothDevice): Int? {
        return mA2dpSinkBluetoothProfileProxy?.getConnectionState(device)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun queryHfpConnectionStateByBtDevice(device: BluetoothDevice): Int? {
        return mA2dpSinkBluetoothProfileProxy?.getConnectionState(device)
    }

    @SuppressLint("DiscouragedPrivateApi", "MissingPermission")
    private fun disableDeviceA2dpSinkProfile(device: BluetoothDevice) {
        runCatching {
            mA2dpSinkBluetoothProfileProxy?.javaClass?.getDeclaredMethod(
                "disconnect",
                BluetoothDevice::class.java
            )?.apply {
                isAccessible = true
            }?.invoke(mA2dpSinkBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDeviceA2dpProfile: onFailure: ${it.message}")
        }.onSuccess {
            Log.e(
                TAG,
                "disableDeviceA2dpProfile: onSuccess: name - ${device.name}, address - ${device.address} "
            )
        }
    }

    @SuppressLint("MissingPermission")
    private fun disableDeviceHfpClientProfile(device: BluetoothDevice) {
        runCatching {
            mHfpClientBluetoothProfileProxy?.javaClass?.getDeclaredMethod(
                "disconnect",
                BluetoothDevice::class.java
            )?.apply {
                isAccessible = true
            }?.invoke(mHfpClientBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDeviceHfpProfile: onFailure: ${it.message}")
        }.onSuccess {
            Log.e(
                TAG,
                "disableDeviceHfpProfile: onSuccess: name - ${device.name}, address - ${device.address} "
            )
        }
    }

    @Deprecated(message = "disconnect a2dp profile will result to pbap profile disconnect")
    @SuppressLint("MissingPermission")
    private fun disableDeviceAvrcpControlProfile(device: BluetoothDevice) {
        runCatching {
            mAvrcpControlBluetoothProfileProxy?.javaClass?.getDeclaredMethod(
                "disconnect",
                BluetoothDevice::class.java
            )?.apply {
                isAccessible = true
            }?.invoke(mAvrcpControlBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDeviceHfpClientProfile: onFailure: ${it.message}")
        }.onSuccess {
            Log.e(
                TAG,
                "disableDeviceHfpClientProfile: onSuccess: name - ${device.name}, address - ${device.address} "
            )
        }
    }

    @Deprecated(message = "disconnect hfp profile will result to pbap profile disconnect")
    @SuppressLint("MissingPermission")
    private fun disableDevicePbapControlProfile(device: BluetoothDevice) {
        runCatching {
            mPbapClientBluetoothProfileProxy?.javaClass?.getDeclaredMethod(
                "disconnect",
                BluetoothDevice::class.java
            )?.apply {
                isAccessible = true
            }?.invoke(mPbapClientBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDevicePbapControlProfile: onFailure: ${it.message}")
        }.onSuccess {
            Log.e(
                TAG,
                "disableDevicePbapControlProfile: onSuccess: name - ${device.name}, address - ${device.address} "
            )
        }
    }

    fun disableA2dpSinkProfileByAddress(address: String) {
        mA2dpSinkBluetoothProfileProxy?.connectedDevices?.firstOrNull {
            it.address == address
        }?.let { currentA2dpProfileBtDevice ->
            disableDeviceA2dpSinkProfile(currentA2dpProfileBtDevice)
        }
    }

    fun disableHfpClientProfileByAddress(address: String) {
        mHfpClientBluetoothProfileProxy?.connectedDevices?.firstOrNull {
            it.address == address
        }?.let { currentA2dpProfileBtDevice ->
            disableDeviceHfpClientProfile(currentA2dpProfileBtDevice)
        }
    }
}