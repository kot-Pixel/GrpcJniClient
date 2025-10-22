package com.kotlinx.grpcjniclient

import android.Manifest
import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.kotlinx.grpcjniclient.bt.BluetoothService
import com.kotlinx.grpcjniclient.databinding.ActivityMainBinding
import com.kotlinx.grpcjniclient.rpc.CarplayRuntime
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub.initMediaCodec3333
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub.notifySurfaceAvailable
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlin.concurrent.thread
import kotlin.time.DurationUnit
import kotlin.time.measureTime


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        CoroutineScope(Dispatchers.IO).launch {
            val invokerTime = measureTime {
                CarplayRuntime.hostapdManager?.isHostapdReady()
            }.toInt(DurationUnit.MILLISECONDS)

            Log.d("MainActivity", "hostapd take $invokerTime milliseconds")
        }

        binding.surface.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                Log.d("wdf", "surfaceCreated")
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
                Log.d("wdf", "surfaceChanged width is $width height $height")

                notifySurfaceAvailable(holder.surface)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                Log.d("wdf", "surfaceDestroyed")
            }
        })
    }

    /**
     * A native method that is implemented by the 'grpcjniclient' native library,
     * which is packaged with this application.
     */

}