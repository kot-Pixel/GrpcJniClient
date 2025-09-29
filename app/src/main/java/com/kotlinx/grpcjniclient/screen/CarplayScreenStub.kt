package com.kotlinx.grpcjniclient.screen

import android.graphics.SurfaceTexture
import android.media.MediaCodec
import android.media.MediaFormat
import android.util.Log
import android.view.Surface
import kotlin.concurrent.thread


object CarplayScreenStub {

    private const val STUB_TEXTURE_WIDTH = 1
    private const val STUB_TEXTURE_HEIGHT = 1
    private const val SURFACE_TEXTURE_STUB_NAME = 0
    private var mStubSurface: Surface? = null
    private var mStubSurfaceTexture: SurfaceTexture? = null

    private var mMediaCodec: MediaCodec?= null
    private const val CarplayScreenStubTag = "CarplayScreenStub"

    @JvmStatic
    fun createStubSurface() : Surface? {
        return runCatching {
            mStubSurfaceTexture ?: run {
                mStubSurfaceTexture = SurfaceTexture(0).apply {
                    setDefaultBufferSize(STUB_TEXTURE_WIDTH, STUB_TEXTURE_HEIGHT)
                }
            }

            mStubSurface ?: run {
                mStubSurface = Surface(mStubSurfaceTexture)
            }

            mStubSurface
        }.getOrNull()
    }

    @JvmStatic
    fun releaseStubSurface() {
        mStubSurfaceTexture?.release()
        mStubSurface?.release()

        mStubSurfaceTexture = null
        mStubSurface = null
    }

    fun initMediaCodec3333(surface: Surface) {
        runCatching {
            mMediaCodec = MediaCodec.createDecoderByType("video/avc")

            val mMediaFormat = MediaFormat().apply {
                setString(MediaFormat.KEY_MIME,"video/avc")
                setInteger(MediaFormat.KEY_WIDTH,1920)
                setInteger(MediaFormat.KEY_HEIGHT,1080)
            }

            mMediaCodec?.configure(mMediaFormat, surface, null, 0)

            mMediaCodec?.start()

            mMediaCodec
        }.onSuccess { mCodec ->

            thread {
                Log.i(CarplayScreenStubTag,"mediacodec onSuccess")
                val mMediaCodecOutputBuffer: MediaCodec.BufferInfo = MediaCodec.BufferInfo()
                while(true) {
                    val index = mCodec?.dequeueOutputBuffer(mMediaCodecOutputBuffer, 500000) ?: -1

                    Log.i(CarplayScreenStubTag, "initMediaCodec: output index is $index")

                    if (index >= 0) {
                        mCodec?.releaseOutputBuffer(index, true)
                    }
                }
            }

            Thread.sleep(2000)

            val sps = byteArrayOf(
                0x00, 0x00, 0x00, 0x01, 0x27, 0x64.toByte(), 0x00, 0x2a, 0xac.toByte(), 0x13,
                0x14, 0x50, 0x1e, 0x01, 0xe6.toByte(), 0x9b.toByte(), 0x80.toByte(), 0x86.toByte(),
                0x83.toByte(), 0x03, 0x68, 0x22, 0x11, 0x96.toByte(),
                0x00, 0x00, 0x00, 0x01, 0x28, 0xee.toByte(), 0x06, 0xf2.toByte()
            )

            if (mCodec != null) {
                try {
                    val index = mCodec.dequeueInputBuffer(-1)
                    if (index >= 0) {
                        val inputBuffer = mCodec.getInputBuffer(index)
                        if (inputBuffer != null) {
                            inputBuffer.clear()
                            inputBuffer.put(sps)
                        }
                        mCodec.queueInputBuffer(
                            index,
                            0,
                            sps.size,
                            0,
                            MediaCodec.BUFFER_FLAG_CODEC_CONFIG
                        )
                        Log.d(CarplayScreenStubTag, "queueInputBuffer SPS/PPS success")
                    } else {
                        Log.d(CarplayScreenStubTag, "dequeueInputBuffer failure")
                    }
                } catch (e: Exception) {
                    Log.e(CarplayScreenStubTag, "configureMediaCodec failed: ${e.message}", e)
                }
            }
        }.onFailure {
            Log.i(CarplayScreenStubTag,"mediacodec onFailure")
        }
    }

}