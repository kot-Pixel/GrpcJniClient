package com.kotlinx.grpcjniclient.ui

import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.lifecycle.lifecycleScope
import com.kotlinx.grpcjniclient.R
import com.kotlinx.grpcjniclient.core.ServiceBinderManager
import com.kotlinx.grpcjniclient.databinding.ActivityCarplayMainBinding
import com.kotlinx.grpcjniclient.databinding.ActivityMainBinding
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub.notifySurfaceAvailable
import kotlinx.coroutines.launch

class CarplayMainActivity : AppCompatActivity() {

    private lateinit var mCarplayMainBinding: ActivityCarplayMainBinding

    private var mUiServiceBinder: UiService.UiServiceLocalProxy? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        mCarplayMainBinding = ActivityCarplayMainBinding.inflate(layoutInflater)
        setContentView(mCarplayMainBinding.root)

        lifecycleScope.launch {
            mUiServiceBinder = ServiceBinderManager(this@CarplayMainActivity).bindService(UiService::class.java)
        }

        mCarplayMainBinding.surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
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
}