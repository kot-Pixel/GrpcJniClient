package com.kotlinx.grpcjniclient

import android.app.Application
import android.content.Intent
import com.kotlinx.grpcjniclient.core.CarplayRuntime
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class CarplayApplication : Application() {

    private val TAG = "com.kotlinx.grpcjniclient.CarplayApplication"

    private val mApplicationScope: CoroutineScope = CoroutineScope(SupervisorJob() + CoroutineName("") + Dispatchers.IO)

    private val mCarplayRuntime: CarplayRuntime = CarplayRuntime()

    companion object {
        init {
            System.loadLibrary("grpcjniclient")
        }
    }

    override fun onCreate() {
        super.onCreate()
//        startService(Intent(this, CarplayService::class.java))
        mApplicationScope.launch {
            mCarplayRuntime.initCarplayRuntime(this@CarplayApplication)
        }
    }

    override fun onTerminate() {
        super.onTerminate()
        mApplicationScope.cancel()
    }
}