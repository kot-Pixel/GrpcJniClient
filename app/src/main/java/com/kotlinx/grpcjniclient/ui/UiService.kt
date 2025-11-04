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

        fun uiHidEventFlow() = mUiHidSharedFlow.asSharedFlow()

        fun uiMotionEventChanged(
            touch1State: Boolean,
            touch1X: Int,
            touch1Y: Int,
            touch2State: Boolean,
            touch2X: Int,
            touch2Y: Int
        ): Boolean = updateUiMotionEventChanged(touch1State, touch1X, touch1Y, touch2State, touch2X, touch2Y)
    }

    private val mUiSharedFlow: MutableSharedFlow<UiEvent?> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 5,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    private val mUiHidSharedFlow: MutableSharedFlow<UiEvent> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 60,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    private val mUiServiceLocalProxy: UiServiceLocalProxy = UiServiceLocalProxy()

    private fun updateUiMotionEventChanged(
        touch1State: Boolean,
        touch1X: Int,
        touch1Y: Int,
        touch2State: Boolean,
        touch2X: Int,
        touch2Y: Int
    ): Boolean {
        var packed = 0L
        packed = packed or ((touch1X.toLong() and 0xFFF) shl 0)
        packed = packed or ((touch1Y.toLong() and 0xFFF) shl 12)
        packed = packed or ((if (touch1State) 1L else 0L) shl 24)

        packed = packed or ((touch2X.toLong() and 0xFFF) shl 25)
        packed = packed or ((touch2Y.toLong() and 0xFFF) shl 37)
        packed = packed or ((if (touch2State) 1L else 0L) shl 49)
        return mUiHidSharedFlow.tryEmit(UiEvent.HidTouchEvent(packed))
    }

    override fun onCreate() {
        super.onCreate()
    }

    override fun onBind(intent: Intent?): IBinder {
        return mUiServiceLocalProxy
    }
}