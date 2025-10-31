package com.kotlinx.grpcjniclient.bt.transport

import kotlinx.coroutines.Job

data class BluetoothRfcommTransform(
    val channel: BluetoothRfcommChannel,
    val job: Job)