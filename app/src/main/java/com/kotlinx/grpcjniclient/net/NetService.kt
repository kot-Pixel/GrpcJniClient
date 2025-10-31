package com.kotlinx.grpcjniclient.net

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log
import com.kotlinx.grpcjniclient.net.event.NetEvent
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow

open class NetService: Service() {

    protected val mNetServiceScope: CoroutineScope = CoroutineScope(Dispatchers.IO + CoroutineName("NetServiceCoroutine"))

    protected val mHostapdEventSharedFlow: MutableSharedFlow<NetEvent> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 5,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )
    private external fun netLinkStart()

    private external fun netLinkStop()

    companion object {
        private const val TAG = "NetService"
    }

    fun onIpv6Ready(inetName: String, ipv6Address: String) {
        Log.i(TAG, "onIpv6Ready: inetName $inetName ipv6Address $ipv6Address")
        mHostapdEventSharedFlow.tryEmit(NetEvent.KernelNetLinkChangedEvent(inetName, ipv6Address))
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onCreate() {
        super.onCreate()
        netLinkStart()
    }

    override fun onDestroy() {
        super.onDestroy()
        netLinkStop()
        mNetServiceScope.cancel()
    }
}