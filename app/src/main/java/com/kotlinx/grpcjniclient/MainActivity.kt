package com.kotlinx.grpcjniclient

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.kotlinx.grpcjniclient.bt.BluetoothService
import com.kotlinx.grpcjniclient.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)


//        stringFromJNI()
//
//        Log.d("PermissionCheck", "Has permission = ${ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED}")
//
//        startService(Intent(this, BluetoothService::class.java))
    }

    /**
     * A native method that is implemented by the 'grpcjniclient' native library,
     * which is packaged with this application.
     */

}