package com.kotlinx.grpcjniclient.rpc

import android.content.Context
import android.util.Log
import com.kotlinx.grpcjniclient.wifi.HostapdManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch

object CarplayRuntime {
    //rpc external function
    private external fun initCarplayRpc(): Boolean

    private external fun startCarplaySession(
        hostapdSsid: String,
        hostapdPwd: String,
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
    }

    private suspend fun checkHostapdReady() = coroutineScope {
        val hostapdReadyTakeMillSeconds = mHostapdManager?.isHostapdReady()
        Log.d(TAG, "isHostapdReady take $hostapdReadyTakeMillSeconds")
        mHostapdManager?.getHostapdConfigure()?.let {
            Log.i(TAG, "current hostapd info: $it")
            startCarplaySession(it.mHostapdSsid.orEmpty(), it.mHostapdPassphrase.orEmpty(), it.mHostapdIPAddressV6.orEmpty(), it.mHostapdSecurityType.value)
        }
    }

    fun carplayAvailable() {
        mCarplayRuntimeCoroutineScope.launch {
            checkHostapdReady()
        }
    }
}