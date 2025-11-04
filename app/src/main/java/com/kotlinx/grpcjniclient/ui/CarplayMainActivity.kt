package com.kotlinx.grpcjniclient.ui

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
import com.kotlinx.grpcjniclient.core.ServiceBinderManager
import com.kotlinx.grpcjniclient.databinding.ActivityCarplayMainBinding
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub.notifySurfaceAvailable
import com.kotlinx.grpcjniclient.screen.CarplayScreenStub.notifySurfaceUnavailable
import kotlinx.coroutines.launch

class CarplayMainActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "CarplayMainActivity"
    }

    private lateinit var mCarplayMainBinding: ActivityCarplayMainBinding

    private var mUiServiceBinder: UiService.UiServiceLocalProxy? = null

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        mCarplayMainBinding = ActivityCarplayMainBinding.inflate(layoutInflater)
        setContentView(mCarplayMainBinding.root)

        lifecycleScope.launch {
            mUiServiceBinder = ServiceBinderManager(this@CarplayMainActivity).bindService(UiService::class.java)
        }

        mCarplayMainBinding.surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
                notifySurfaceAvailable(holder.surface)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                notifySurfaceUnavailable()
            }
        })

        mCarplayMainBinding.surfaceView.setOnTouchListener { _, event ->

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
            mUiServiceBinder?.uiMotionEventChanged(state[0], x[0], y[0], state[1], x[1], y[1]) == true
        }
    }
}