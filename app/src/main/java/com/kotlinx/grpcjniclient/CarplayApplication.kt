package com.kotlinx.grpcjniclient

import android.app.Application
import android.util.Log
import com.kotlinx.grpcjniclient.rpc.CarplayRpcManager

class CarplayApplication : Application() {

    private val TAG = "com.kotlinx.grpcjniclient.CarplayApplication"

    companion object {
        init {
            System.loadLibrary("grpcjniclient")
        }
    }

    override fun onCreate() {
        super.onCreate()
        val rpcInitResult = CarplayRpcManager.initCarplayRpcClient()
        Log.d(TAG, "onCreate: init carplay rpc client result is: $rpcInitResult")
    }
}