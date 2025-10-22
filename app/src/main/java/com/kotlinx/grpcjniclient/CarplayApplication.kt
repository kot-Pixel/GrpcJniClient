package com.kotlinx.grpcjniclient

import android.app.Application
import android.content.Intent
import com.kotlinx.grpcjniclient.bt.BluetoothService
import com.kotlinx.grpcjniclient.rpc.CarplayRuntime
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub

class CarplayApplication : Application() {

    private val TAG = "com.kotlinx.grpcjniclient.CarplayApplication"

    companion object {
        init {
            System.loadLibrary("grpcjniclient")
        }
    }

    override fun onCreate() {
        super.onCreate()
        CarplayRuntime.initCarplayRuntime(this)
        startService(Intent(this, BluetoothService::class.java))
    }
}