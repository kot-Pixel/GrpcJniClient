package com.kotlinx.grpcjniclient.bt

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.content.Intent
import android.os.Build
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresApi
import java.util.Locale

@SuppressLint("MissingPermission")
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
fun Intent.onBondStateChanged() {
    val bondState: Int = getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.ERROR)

    when(bondState) {
        Integer.MIN_VALUE -> {
            Log.e("CarplayBluetooth", "btDevice bond state is error")
        }

        BluetoothDevice.BOND_BONDING -> {
            Log.d("CarplayBluetooth", "btDevice bonding...")
        }

        BluetoothDevice.BOND_BONDED-> {
            Log.d("CarplayBluetooth", "btDevice bonded...")

            val btDevice: BluetoothDevice? =
                getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
            btDevice?.run {
                Log.d("CarplayBluetooth", "btDevice name is $name")
                Log.d("CarplayBluetooth", "btDevice is address $address")
                Log.d("CarplayBluetooth", "btDevice is uuids ${uuids.joinToString(",")}")

                val uuidsLists = uuids.transformUuidsUpper()

                Log.d("CarplayBluetooth", "btDevice is uuids ${
                    uuidsLists.joinToString("\n")}")

                Log.d("CarplayBluetooth", "btDevice check support iap2 ${
                    uuidsLists.checkBtUuidSupportIap2()}")
            } ?: run {
                Log.d("CarplayBluetooth", "btDevice is null so return")
                return
            }

        }

        BluetoothDevice.BOND_NONE -> {
            Log.d("CarplayBluetooth", "btDevice cancel bond or bond failure")
        }
    }
}

/**
 * judge if bt uuids support iap2 protoc
 */
fun List<String>.checkBtUuidSupportIap2(): Boolean {
    return any {
        it == IAP2_BT_HOST || it == IAP2_BT_DEVICE
    }
}

/**
 * transform uuids to upper uuids
 */
fun Array<out ParcelUuid>.transformUuidsUpper(): List<String> {
    return map {
        it.uuid.toString().uppercase(Locale.ROOT)
    }
}