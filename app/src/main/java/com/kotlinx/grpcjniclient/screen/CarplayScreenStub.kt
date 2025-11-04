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

    private var mStubSurface: Surface? = null
    private var mStubSurfaceTexture: SurfaceTexture? = null

    private val onFrameAvailableListener = SurfaceTexture.OnFrameAvailableListener { surfaceTexture ->
        Log.e("CarplayScreenStub", "OnFrameAvailableListener triggered for $surfaceTexture")
        notifyFrameAvailable()
    }

    private external fun notifyFrameAvailable()

    external fun notifySurfaceAvailable(surface: Surface)

    external fun notifySurfaceUnavailable()

    external fun notifySurfaceInputEvent()

    @JvmStatic
    fun createOesSurfaceTexture(oesTex: Int): SurfaceTexture? {
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
}