package com.kotlinx.grpcjniclient.bt.event

import android.bluetooth.BluetoothDevice
import android.os.ParcelUuid
import java.util.UUID

sealed class BluetoothEvent {
    data class AclChangedEvent(
        val device: BluetoothDevice,
        val connected: Boolean
    ) : BluetoothEvent()

    data class A2dpSinkProfileChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent()

    data class HfpClientProfileChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent()

    data class AvrcpControlProfileChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent()

    data class PbapClientProfileChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent()

    data class ConnectStateChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent()

    data class DeviceFoundChangedEvent(
        val device: BluetoothDevice,
    ) : BluetoothEvent()

    data class UuidChangedEvent(
        val device: BluetoothDevice,
        val uuids: Array<out ParcelUuid>
    ) : BluetoothEvent() {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as UuidChangedEvent

            if (device != other.device) return false
            if (!uuids.contentEquals(other.uuids)) return false

            return true
        }

        override fun hashCode(): Int {
            var result = device.hashCode()
            result = 31 * result + uuids.contentHashCode()
            return result
        }
    }

    data class BondChangedEvent(
        val device: BluetoothDevice,
        val changedValue: Int
    ) : BluetoothEvent() {
        companion object {
            const val BOND_STATE_START_BOND = 10
            const val BOND_STATE_BONDING = 11
            const val BOND_STATE_BONDED = 12
        }
    }

    data class RfcommTransformEvent(
        val device: BluetoothDevice,
        val uuid: UUID,
        val data: ByteArray
    ): BluetoothEvent() {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as RfcommTransformEvent

            if (device != other.device) return false
            if (uuid != other.uuid) return false
            if (!data.contentEquals(other.data)) return false

            return true
        }

        override fun hashCode(): Int {
            var result = device.hashCode()
            result = 31 * result + uuid.hashCode()
            result = 31 * result + data.contentHashCode()
            return result
        }
    }
}
