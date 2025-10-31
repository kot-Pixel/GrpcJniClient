package com.kotlinx.grpcjniclient.ui

import android.app.Service
import android.content.Intent
import android.os.Binder
import android.os.IBinder
import com.kotlinx.grpcjniclient.ui.event.UiEvent
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow

class UiService: Service() {

    inner class UiServiceLocalProxy: Binder() {
        fun uiSharedFlow() = mUiSharedFlow.asSharedFlow()
    }

    private val mUiSharedFlow: MutableSharedFlow<UiEvent?> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 5,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    private val mUiServiceLocalProxy: UiServiceLocalProxy = UiServiceLocalProxy()

    override fun onCreate() {
        super.onCreate()
    }

    override fun onBind(intent: Intent?): IBinder {
        return mUiServiceLocalProxy
    }


}