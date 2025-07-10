package com.kotlinx.grpcjniclient.bt

import android.os.ParcelUuid
import java.util.Locale
import java.util.UUID

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

/**
 * transform uuid string to UUID
 */
fun String.uuidString2SUUID() : Result<UUID> {
    return runCatching { UUID.fromString(this) }
}