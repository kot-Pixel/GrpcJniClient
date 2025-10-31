package com.kotlinx.grpcjniclient.bt.transport

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.util.Log
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.isActive
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class BluetoothRfcommChannel(
    val device: BluetoothDevice,
    val uuid: UUID
) {

    private var mSocket: BluetoothSocket? = null
    private var mInput: InputStream? = null
    private var mOutput: OutputStream? = null

    private val mRfcommTransformCoroutineScope = CoroutineScope(Dispatchers.IO + CoroutineName("rfcommTransformScope"))

    @SuppressLint("MissingPermission")
    fun connect(): Result<Boolean> {
        runCatching {
            val mRfcommSocket = device.createRfcommSocketToServiceRecord(uuid)
            mRfcommSocket.connect()

            mSocket = mRfcommSocket
            mInput = mRfcommSocket.inputStream
            mOutput = mRfcommSocket.outputStream
        }.fold(
            onSuccess = {
                return Result.success(true)
            },
            onFailure = {
                return Result.failure(it)
            }
        )
    }

    fun write(data: ByteArray) {
        try {
            val hexString = data.joinToString(" ") { "%02X ".format(it) }
            Log.e(TAG, "send rfcomm data is: $hexString")

            mOutput?.write(data)
            mOutput?.flush()
        } catch (e: IOException) {
            Log.e(TAG, "write error: ${e.message}")
        }
    }

    fun readLoop(onData: (BluetoothRfcommChannel, ByteArray) -> Unit): Job {
        return mRfcommTransformCoroutineScope.launch {
            val buffer = ByteArray(1024)
            try {
                while (isActive) {
                    val len = mInput?.read(buffer) ?: break
                    if (len > 0) {
                        onData(this@BluetoothRfcommChannel, buffer.copyOf(len))
                    }
                }
            } catch (e: IOException) {
                Log.e(TAG, "read error: ${e.message}")
            }
        }
    }

    fun close() {
        runCatching {
            mRfcommTransformCoroutineScope.cancel()
            mSocket?.close()
            mInput?.close()
            mOutput?.close()
        }
        mSocket = null
        mInput = null
        mOutput = null
    }

    companion object {
        private const val TAG = "BluetoothRfcommChannel"
    }
}
