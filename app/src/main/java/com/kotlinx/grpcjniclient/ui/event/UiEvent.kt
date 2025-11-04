package com.kotlinx.grpcjniclient.ui.event

import android.view.Surface

sealed class UiEvent {
    data object LaunchActivityEvent : UiEvent()

    data class ActivitySurfaceAttached(
        val surface: Surface
    ): UiEvent()

    data class ActivitySurfaceDetach(
        val surface: Surface
    ): UiEvent()

    data class HidTouchEvent(
        val touchValue: Long
    ): UiEvent()
}