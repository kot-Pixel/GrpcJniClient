package com.kotlinx.grpcjniclient.rpc

import android.app.Service
import android.content.Intent
import android.os.Binder
import android.os.IBinder
import android.util.Log
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import java.nio.ByteBuffer
class CarplayRpcManager: Service() {

    companion object {
        private const val TAG = "CarplayRpcManager"
    }

    private val mCarplayRpcServiceBinder = CarplayRpcServiceBinder()

    private val mRpcEventSharedFlow: MutableSharedFlow<RpcEvent> = MutableSharedFlow(
        replay = 0,
        extraBufferCapacity = 10,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    val rpcEventSharedFlow: SharedFlow<RpcEvent> get() = mRpcEventSharedFlow

    //rpc external function
    private external fun initCarplayRpc(): Boolean

    private external fun destroyCarplayRpc(): Boolean

    private external fun startCarplaySession(
        hostapdSsid: String,
        hostapdPwd: String,
        hostapdChannel: Int,
        hostapdNetInterfaceV6Address: String,
        hostapdSecurityType: Int
    ): Boolean

    /**
     * 启动一个无线蓝牙CarplayRunLooper事件循环。
     */
    external fun startBtIap2Link(macString: String): Boolean

    external fun receiveBtIap2Data(rfcommData: ByteArray, dataLength: Int) : Boolean

    external fun touchScreenHid(touchHidVale: Long)

    inner class CarplayRpcServiceBinder: Binder() {
        fun getRpcEventSharedFlow() = rpcEventSharedFlow

        fun startBtIap2LinkBinder(macString: String): Boolean = startBtIap2Link(macString)

        fun receiveBtIap2DataBinder(rfcommData: ByteArray, dataLength: Int): Boolean = receiveBtIap2Data(rfcommData, dataLength)

        fun startCarplaySessionBinder(
            hostapdSsid: String,
            hostapdPwd: String,
            hostapdChannel: Int,
            hostapdNetInterfaceV6Address: String,
            hostapdSecurityType: Int
        ) = startCarplaySession(hostapdSsid, hostapdPwd, hostapdChannel, hostapdNetInterfaceV6Address, hostapdSecurityType)

        fun touchScreenHidBinder(touchHidVale: Long) = touchScreenHid(touchHidVale)
    }

    private fun initCarplayRuntime() {
        Log.i(TAG, "initCarplayRuntime")

        val rpcInitResult = initCarplayRpc()
        Log.d(TAG, "init carplay rpc client result is: $rpcInitResult")
    }

    fun destroyCarplayRuntime() {

    }

    /**
     * rpc jni 回调 carplay available
     */
    fun callbackCarplayAvailable(
        wiredAvailable: Boolean,
        usbTransportIdentifier: String?,
        wirelessAvailable: Boolean,
        bluetoothTransportIdentifier: String?
    ) {
        val emitResult = mRpcEventSharedFlow.tryEmit(RpcEvent.CarplayAvailableEvent(
            wiredAvailable = wiredAvailable,
            usbTransportIdentifier = usbTransportIdentifier.orEmpty(),
            wirelessAvailable = wirelessAvailable,
            bluetoothTransportIdentifier = bluetoothTransportIdentifier.orEmpty(),
        ))
        Log.d(TAG, "send carplay available result: $emitResult")
    }

    /**
     * rpc jni 回调 iap2 报文
     */
    fun callbackWithByteArray(data: ByteBuffer) {
        val size = data.remaining()
        val byteArray = ByteArray(size)
        data.get(byteArray)
        val emitResult = mRpcEventSharedFlow.tryEmit(RpcEvent.Iap2DataEvent(byteArray))
        Log.d(TAG, "send iap2 data array : $emitResult")
    }

    /**
     * rpc jni 回调 mediaCodec format changed
     */

    fun mediaCodecFormatChanged() {
        val emitResult = mRpcEventSharedFlow.tryEmit(RpcEvent.MediaCodecFormatChangeEvent)
        Log.d(TAG, "send mediaCodecFormatChanged bluetoothAddress : $emitResult")
    }

    /**
     * rpc jni 回调 断开蓝牙
     */
    fun disableBluetooth(
        bluetoothAddress: String
    ) {
        val emitResult = mRpcEventSharedFlow.tryEmit(RpcEvent.DisableBluetoothEvent(bluetoothAddress))
        Log.d(TAG, "send disableBluetooth bluetoothAddress : $emitResult")
    }

    override fun onCreate() {
        super.onCreate()
        initCarplayRuntime()

        Log.i(TAG, "onCreate: rpc service")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder {
        return mCarplayRpcServiceBinder
    }

    override fun onDestroy() {
        super.onDestroy()
        destroyCarplayRpc()
    }
}