package com.kotlinx.grpcjniclient

import android.Manifest
import android.app.Activity
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.os.IBinder
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

    private var mBluetoothService: BluetoothService? = null

    private val mServiceConnection: ServiceConnection = object: ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
            mBluetoothService = (service as BluetoothService.BluetoothServiceBinder).getBluetoothService()
        }

        override fun onServiceDisconnected(name: ComponentName?) {
            mBluetoothService = null
        }

    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val intent = Intent(this, BluetoothService::class.java)
        bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE)

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

    override fun onDestroy() {
        super.onDestroy()
        unbindService(mServiceConnection)
    }

    /**
     * A native method that is implemented by the 'grpcjniclient' native library,
     * which is packaged with this application.
     */

}