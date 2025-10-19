package com.kotlinx.grpcjniclient.screen

import android.graphics.SurfaceTexture
import android.media.MediaCodec
import android.media.MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar
import android.media.MediaFormat
import android.opengl.EGL14
import android.opengl.EGLContext
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.view.Surface
import javax.microedition.khronos.egl.EGL10
import kotlin.concurrent.thread


object CarplayScreenStub {

    private const val STUB_TEXTURE_WIDTH = 1872
    private const val STUB_TEXTURE_HEIGHT = 756
    private const val SURFACE_TEXTURE_STUB_NAME = 0
    private var mStubSurface: Surface? = null
    private var mStubSurfaceTexture: SurfaceTexture? = null

    private val onFrameAvailableListener = SurfaceTexture.OnFrameAvailableListener { surfaceTexture ->
        Log.e("CarplayScreenStub", "OnFrameAvailableListener triggered for $surfaceTexture")
        notifyFrameAvailable()
    }

    private val mSurfaceTexture: SurfaceTexture = SurfaceTexture(0).apply {
        setDefaultBufferSize(STUB_TEXTURE_WIDTH, STUB_TEXTURE_HEIGHT)
        setOnFrameAvailableListener(onFrameAvailableListener)
//        COLOR_FormatYUV420Planar
    }

    private var sGLThread: HandlerThread? = null
    private var sGLHandler: Handler? = null

    private var mMediaCodec: MediaCodec?= null
    private const val CarplayScreenStubTAG = "CarplayScreenStub"

    external fun notifyFrameAvailable()

    external fun notifySurfaceAvailable(surface: Surface)



    fun initStubSurface() {
        mStubSurfaceTexture ?: run {
            mStubSurfaceTexture = SurfaceTexture(0).apply {
                setDefaultBufferSize(1920, 1080)
                setOnFrameAvailableListener(onFrameAvailableListener)
            }
        }

        mStubSurface ?: run {
            mStubSurface = Surface(mStubSurfaceTexture)
        }
    }

//    fun getStubSurfaceTexture() {
//        return mStubSurfaceTexture.
//    }

    @JvmStatic
    fun createStubSurface(): Surface? {
//        return runCatching {
//            mStubSurfaceTexture ?: run {
//                mStubSurfaceTexture = SurfaceTexture(0).apply {
//                    setDefaultBufferSize(STUB_TEXTURE_WIDTH, STUB_TEXTURE_HEIGHT)
//                    setOnFrameAvailableListener(onFrameAvailableListener)
//                }
//            }
//
//            mStubSurface ?: run {
//                mStubSurface = Surface(mStubSurfaceTexture)
//            }
//
//            mStubSurface
//        }.getOrNull()
        return  null
    }

    fun checkTextureValid(texId: Int) {
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texId)
        val error = GLES20.glGetError()
        Log.d("OESCheck", "glBindTexture -> glGetError() = 0x${Integer.toHexString(error)}")
    }

    fun checkCurrentCtx() {
        val extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS)
        if (!extensions.contains("GL_OES_EGL_image_external")) {
            Log.e("OESCheck", "GL_OES_EGL_image_external not supported!")
        }
    }

    fun checkEGLContext() {
        val eglContext: EGLContext = EGL14.eglGetCurrentContext()

        if (eglContext == EGL10.EGL_NO_CONTEXT) {
            Log.e("EGLContextCheck", "No current EGLContext")
        } else {
            Log.i("EGLContextCheck", "Current EGLContext: $eglContext")
        }
    }

    @JvmStatic
    fun createOesSurfaceTexture(oesTex: Int): Surface? {

        checkTextureValid(oesTex)

        checkEGLContext()

        checkCurrentCtx()

        return runCatching {
            mStubSurfaceTexture ?: run {
                mStubSurfaceTexture = SurfaceTexture(oesTex).apply {
                    setDefaultBufferSize(STUB_TEXTURE_WIDTH, STUB_TEXTURE_HEIGHT)
                    setOnFrameAvailableListener(onFrameAvailableListener)
                }
            }

            mStubSurface ?: run {
                mStubSurface = Surface(mStubSurfaceTexture)
            }

            mStubSurface
        }.getOrNull()
    }


    @JvmStatic
    fun createOesSurfaceTexture2(oesTex: Int): SurfaceTexture? {

        return runCatching {
            mStubSurfaceTexture ?: run {
                mStubSurfaceTexture = SurfaceTexture(oesTex).apply {
                    setDefaultBufferSize(STUB_TEXTURE_WIDTH, STUB_TEXTURE_HEIGHT)
                    setOnFrameAvailableListener(onFrameAvailableListener)
                }
            }

            mStubSurfaceTexture
        }.getOrNull()
    }

    @JvmStatic
    fun updateSurfaceTexture() {
        Log.e("CarplayScreenStub", "updateTexImage invoke")
        mStubSurfaceTexture?.updateTexImage()
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
                Log.i(CarplayScreenStubTAG,"mediacodec onSuccess")
                val mMediaCodecOutputBuffer: MediaCodec.BufferInfo = MediaCodec.BufferInfo()
                while(true) {
                    val index = mCodec?.dequeueOutputBuffer(mMediaCodecOutputBuffer, 500000) ?: -1

                    Log.i(CarplayScreenStubTAG, "initMediaCodec: output index is $index")

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
                        Log.d(CarplayScreenStubTAG, "queueInputBuffer SPS/PPS success")
                    } else {
                        Log.d(CarplayScreenStubTAG, "dequeueInputBuffer failure")
                    }
                } catch (e: Exception) {
                    Log.e(CarplayScreenStubTAG, "configureMediaCodec failed: ${e.message}", e)
                }
            }
        }.onFailure {
            Log.i(CarplayScreenStubTAG,"mediacodec onFailure")
        }
    }

}