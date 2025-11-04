package com.kotlinx.grpcjniclient

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.MotionEvent.ACTION_CANCEL
import android.view.MotionEvent.ACTION_DOWN
import android.view.MotionEvent.ACTION_POINTER_DOWN
import android.view.MotionEvent.ACTION_POINTER_UP
import android.view.MotionEvent.ACTION_UP
import android.view.SurfaceHolder
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.kotlinx.grpcjniclient.bt.CarplayBluetoothService
import com.kotlinx.grpcjniclient.core.ServiceBinderManager
import com.kotlinx.grpcjniclient.databinding.ActivityMainBinding
import com.kotlinx.grpcjniclient.ui.CarplayMainActivity
import com.kotlinx.grpcjniclient.ui.CarplayMainActivity.Companion
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding


    private var mServiceBinderManager: ServiceBinderManager? = null

    private var mBtServiceBinder: CarplayBluetoothService.CarplayBluetoothServiceBinder? = null

    private val TAG = "MainActivity"

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        mServiceBinderManager = ServiceBinderManager(this)

        lifecycleScope.launch {
            mBtServiceBinder = mServiceBinderManager?.bindService(CarplayBluetoothService::class.java)
        }

        binding.surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {

            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
                Log.i(TAG, "onCreate: action is: width " + width)
                Log.i(TAG, "onCreate: event x height " + height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {

            }
        })


        binding.surfaceView.setOnTouchListener { _, event ->

            val pointerCount = event.pointerCount.coerceAtMost(2)
            val x = IntArray(2)
            val y = IntArray(2)
            val state = BooleanArray(2) { false }

            when (event.actionMasked) {
                ACTION_DOWN -> {
                    x[0] = event.getX(0).toInt()
                    y[0] = event.getY(0).toInt()
                    state[0] = true
                }

                ACTION_POINTER_DOWN, MotionEvent.ACTION_MOVE -> {
                    for (i in 0 until pointerCount) {
                        x[i] = event.getX(i).toInt()
                        y[i] = event.getY(i).toInt()
                        state[i] = true
                    }
                }

                ACTION_POINTER_UP -> {
                    val index = event.actionIndex
                    x[index] = event.getX(index).toInt()
                    y[index] = event.getY(index).toInt()
                    state[index] = false
                    for (i in 0 until pointerCount) {
                        if (i != index) {
                            x[i] = event.getX(i).toInt()
                            y[i] = event.getY(i).toInt()
                            state[i] = true
                        }
                    }
                }

                ACTION_UP, ACTION_CANCEL -> {
                    for (i in 0..1) {
                        state[i] = false
                        x[i] = 0
                        y[i] = 0
                    }
                }
            }

            Log.i(TAG, "Touch1: (${x[0]}, ${y[0]}) state=${state[0]}")
            Log.i(TAG, "Touch2: (${x[1]}, ${y[1]}) state=${state[1]}")

            true
        }
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    /**
     * A native method that is implemented by the 'grpcjniclient' native library,
     * which is packaged with this application.
     */

}