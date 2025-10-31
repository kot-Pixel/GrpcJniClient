package com.kotlinx.grpcjniclient.core

import android.app.Service
import android.content.Context
import kotlinx.coroutines.CoroutineScope
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.IBinder
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume

class ServiceBinderManager(
    private val context: Context
) {
    private val binders = mutableMapOf<Class<*>, IBinder>()

    @Suppress("UNCHECKED_CAST")
    suspend fun <T : IBinder> bindService(cls: Class<out Service>): T {
        return suspendCancellableCoroutine { cont ->
            val conn = object : ServiceConnection {
                override fun onServiceConnected(name: ComponentName, service: IBinder) {
                    binders[cls] = service
                    cont.resume(service as T)
                }

                override fun onServiceDisconnected(name: ComponentName) {
                    binders.remove(cls)
                }
            }
            context.bindService(Intent(context, cls), conn, Context.BIND_AUTO_CREATE)
        }
    }


    @Suppress("UNCHECKED_CAST")
    fun <T : IBinder> getBinder(cls: Class<out Service>): T? {
        return binders[cls] as? T
    }
}
