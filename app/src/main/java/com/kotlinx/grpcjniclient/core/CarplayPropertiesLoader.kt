package com.kotlinx.grpcjniclient.core

import android.content.Context
import android.util.Log
import java.util.Properties

class CarplayPropertiesLoader {

    companion object {
        private const val TAG = "CarplayPropertiesLoader"
    }

    private val allConfigProperty: MutableMap<String, Any> = mutableMapOf()

    fun loadCarplayProperties(context: Context) {
        val properties = Properties()
        context.assets.open("configure.properties").use { inputStream ->
            properties.load(inputStream)
        }

        for ((key, value) in properties) {
            Log.i(TAG, "loadCarplayProperties: $key = $value")
        }
    }
}