package com.kotlinx.grpcjniclient.bt.profile

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothA2dp
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothHeadset
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.util.Log
import androidx.annotation.RequiresPermission

class BluetoothProfileManager(val ctx: Context) {

    companion object {
        private const val TAG = "BluetoothProfileManager"

    }

    private var mBluetoothManager: BluetoothManager? = null
    private var mA2dpBluetoothProfileProxy: BluetoothA2dp? = null
    private var mHfpBluetoothProfileProxy: BluetoothHeadset? = null

    init {
        mBluetoothManager = ctx.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object: BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                mA2dpBluetoothProfileProxy = if (proxy is BluetoothA2dp) proxy else null
            }

            override fun onServiceDisconnected(profile: Int) {
                mA2dpBluetoothProfileProxy = null
            }

        }, BluetoothProfile.A2DP)

        mBluetoothManager?.adapter?.getProfileProxy(ctx, object: BluetoothProfile.ServiceListener {
            override fun onServiceConnected(profile: Int, proxy: BluetoothProfile?) {
                mHfpBluetoothProfileProxy = if (proxy is BluetoothHeadset) proxy else null
            }

            override fun onServiceDisconnected(profile: Int) {
                mHfpBluetoothProfileProxy = null
            }

        }, BluetoothProfile.HEADSET)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun queryA2dpConnectionStateByBtDevice(device: BluetoothDevice): Int? {
        return mA2dpBluetoothProfileProxy?.getConnectionState(device)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun queryHfpConnectionStateByBtDevice(device: BluetoothDevice) : Int? {
        return mHfpBluetoothProfileProxy?.getConnectionState(device)
    }

    @SuppressLint("DiscouragedPrivateApi")
    fun disableDeviceA2dpProfile(device: BluetoothDevice) {
        val disableResult = runCatching {
            mA2dpBluetoothProfileProxy?.javaClass?.getDeclaredMethod("disconnect", BluetoothDevice::class.java)?.apply {
                isAccessible = true
            }?.invoke(mA2dpBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDeviceA2dpProfile: onFailure: ${it.message}")
        }.getOrDefault(false)
    }

    fun disableDeviceHfpProfile(device: BluetoothDevice) {
        val disableResult = runCatching {
            mHfpBluetoothProfileProxy?.javaClass?.getDeclaredMethod("disconnect", BluetoothDevice::class.java)?.apply {
                isAccessible = true
            }?.invoke(mHfpBluetoothProfileProxy, device) as Boolean
        }.onFailure {
            Log.e(TAG, "disableDeviceHfpProfile: onFailure: ${it.message}")
        }.getOrDefault(false)
    }

}