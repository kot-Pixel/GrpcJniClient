package com.kotlinx.grpcjniclient.bt.transport

import android.bluetooth.BluetoothDevice
import android.util.Log
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import java.util.UUID

class BluetoothRfcommManager {

    companion object {
        private const val TAG = "BluetoothRfcommManager"
    }

    private val _rfcommTransformList: MutableList<BluetoothRfcommTransform> = mutableListOf()

    private val mutex: Mutex = Mutex()

    private val mutableSharedFlow: MutableSharedFlow<BluetoothRfcommChannelItem> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 5,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    val rfcommChannelDataSharedFlow: SharedFlow<BluetoothRfcommChannelItem> get() = mutableSharedFlow

    private val onData: (BluetoothRfcommChannel, ByteArray) -> Unit = { bluetoothRfcommChannel: BluetoothRfcommChannel, bytes: ByteArray ->
        mutableSharedFlow.tryEmit(BluetoothRfcommChannelItem(bluetoothRfcommChannel, bytes))
    }

    private fun connectRfcommTransformChannel(
        device: BluetoothDevice,
        uuid: UUID
    ): BluetoothRfcommChannel {
        return BluetoothRfcommChannel(device, uuid)
    }

    /**
     * 开始rfcomm传输, 这个是同步的方法。
     */
    suspend fun startRfcommTransportJob(device: BluetoothDevice, uuid: UUID): Boolean {
        return mutex.withLock {
            val rfcommChannel = connectRfcommTransformChannel(device, uuid)

            rfcommChannel.connect().fold(
                onSuccess = {
                    val job = rfcommChannel.readLoop(onData)
                    _rfcommTransformList.add(BluetoothRfcommTransform(channel = rfcommChannel, job))
                    Log.d(TAG, "rfcommChannel connect success")
                    true
                }, onFailure = {
                    rfcommChannel.close()
                    Log.d(TAG, "rfcommChannel connect failure, msg is ${it.message}")
                    false
                }
            )
        }
    }

    /**
     * 结束rfcommTransportJob
     */
    suspend fun stopRfcommTransportJob(device: BluetoothDevice, uuid: UUID) {
        mutex.withLock {
            queryRfcommTransform(device, uuid)?.let {
                it.job.cancelAndJoin()
                it.channel.close()
                _rfcommTransformList.remove(it)
            }
        }
    }

    private fun queryRfcommTransform(device: BluetoothDevice, uuid: UUID): BluetoothRfcommTransform? {
        return _rfcommTransformList.firstOrNull {
            it.channel.device == device && it.channel.uuid == uuid
        }?.apply {
            Log.i(TAG, "queryRfcommTransform result is channel is: $channel,  job is: $job")
        }
    }

    /**
     * rfcomm发送数据
     */
    fun sendDataByRfcomm(device: BluetoothDevice, uuid: UUID, byteArray: ByteArray) {
        queryRfcommTransform(device, uuid)?.channel?.write(byteArray)
    }
}