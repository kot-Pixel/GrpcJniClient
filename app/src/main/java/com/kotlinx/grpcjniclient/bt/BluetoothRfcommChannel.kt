package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.Job
import kotlinx.coroutines.isActive
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class BluetoothRfcommChannel(
    private val device: BluetoothDevice,
    private val uuid: UUID,
    private val scope: CoroutineScope = CoroutineScope(Dispatchers.IO + SupervisorJob())
) {

    private var socket: BluetoothSocket? = null
    private var input: InputStream? = null
    private var output: OutputStream? = null

    @SuppressLint("MissingPermission")
    fun connect(): Result<Unit> = runCatching {
        val s = device.createRfcommSocketToServiceRecord(uuid)
        s.connect()
        socket = s
        input = s.inputStream
        output = s.outputStream
    }

    fun write(data: ByteArray): Job = scope.launch {
        try {

            val hexString = data.joinToString(" ") { "%02X ".format(it) }

            Log.e(TAG, "send rfcomm data is: $hexString")

            output?.write(data)
            output?.flush()
        } catch (e: IOException) {
            Log.e(TAG, "write error: ${e.message}")
        }
    }

    fun readLoop(onData: suspend (ByteArray) -> Unit): Job = scope.launch {
        val buffer = ByteArray(1024)
        try {
            while (isActive) {
                val len = input?.read(buffer) ?: break
                if (len > 0) {

                    onData(buffer.copyOf(len))
                }
            }
        } catch (e: IOException) {
            Log.e(TAG, "read error: ${e.message}")
        }
    }

    fun close() {
        runCatching {
            socket?.close()
            input?.close()
            output?.close()
        }
        socket = null
        input = null
        output = null
    }

    companion object {
        private const val TAG = "BluetoothRfcommChannel"
    }
}
