package com.kotlinx.grpcjniclient.rpc

import android.view.Surface
import com.kotlinx.grpcjniclient.ui.event.UiEvent
import kotlinx.coroutines.CompletableDeferred

sealed class RpcEvent {

    data class Iap2DataEvent(
        val dataByteArray: ByteArray
    ) : RpcEvent() {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as Iap2DataEvent

            return dataByteArray.contentEquals(other.dataByteArray)
        }

        override fun hashCode(): Int {
            return dataByteArray.contentHashCode()
        }
    }

    data class CarplayAvailableEvent(
        val wiredAvailable: Boolean,
        val usbTransportIdentifier: String,
        val wirelessAvailable: Boolean,
        val bluetoothTransportIdentifier: String
    ): RpcEvent()


    data class DisableBluetoothEvent(
        val macAddress: String,
    ): RpcEvent()

    data object MediaCodecFormatChangeEvent : RpcEvent()
}