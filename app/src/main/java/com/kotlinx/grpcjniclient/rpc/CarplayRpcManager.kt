package com.kotlinx.grpcjniclient.rpc

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import com.kotlinx.grpcjniclient.bt.BluetoothRfcommChannel
import com.kotlinx.grpcjniclient.wifi.HostapdManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch

object CarplayRuntime {
    //rpc external function
    private external fun initCarplayRpc(): Boolean
    external fun initCarplayRpc22(surface: Surface): Boolean

    external fun initCarplayRpc333(surface: Surface): Boolean

    private external fun startCarplaySession(
        hostapdSsid: String,
        hostapdPwd: String,
        hostapdChannel: Int,
        hostapdNetInterfaceV6Address: String,
        hostapdSecurityType: Int
    ): Boolean

    private const val TAG = "CarplayRuntime"

    private val mCarplayRuntimeCoroutineScope = CoroutineScope(Dispatchers.IO)

    private var mHostapdManager: HostapdManager? = null

    val hostapdManager: HostapdManager? get() = mHostapdManager

    fun initCarplayRuntime(ctx: Context) {
        Log.i(TAG, "initCarplayRuntime")

        val rpcInitResult = initCarplayRpc()
        Log.d(TAG, "init carplay rpc client result is: $rpcInitResult")

        mHostapdManager = HostapdManager(ctx)

        mHostapdManager?.getHostapdConfigure()
    }

    private suspend fun checkHostapdReady() = coroutineScope {
        val hostapdReadyTakeMillSeconds = mHostapdManager?.isHostapdReady()
        Log.d(TAG, "isHostapdReady take $hostapdReadyTakeMillSeconds")
        mHostapdManager?.getHostapdConfigure()?.let {
            Log.i(TAG, "current hostapd info: $it, will start carplay session")
            startCarplaySession(it.mHostapdSsid.orEmpty(), it.mHostapdPassphrase.orEmpty(), (it.mHostapdChannel ?: 149), it.mHostapdIPAddressV6.orEmpty(), it.mHostapdSecurityType.value)
        }?: run {
            Log.i(TAG, "current hostapd info is null")
        }
    }

    fun carplayAvailable() {
        mCarplayRuntimeCoroutineScope.launch {
            checkHostapdReady()
        }
    }
}