package com.roadforge.bussim;

import android.app.Activity;
import android.os.Bundle;
import android.view.Choreographer;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

/**
 * RoadForge Bus Sim Faz 0 ana aktivitesi.
 *
 * Java katmanı yalnızca Android yaşam döngüsü, Surface ve dokunma olaylarını
 * native motora taşır. Oyun kuralı, fizik, ekonomi veya render kararı burada
 * tutulmaz; bu sınır ileride motorun test edilebilir kalması için bilinçli.
 */
public final class MainActivity extends Activity implements SurfaceHolder.Callback, View.OnTouchListener, Choreographer.FrameCallback {
    static {
        System.loadLibrary("roadforge");
    }

    private SurfaceView surfaceView;
    private long nativeHandle = 0L;
    private boolean surfaceReady = false;
    private boolean paused = true;
    private boolean frameLoopRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        nativeHandle = nativeCreate();

        surfaceView = new SurfaceView(this);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setFocusable(true);
        surfaceView.setFocusableInTouchMode(true);
        surfaceView.setKeepScreenOn(true);
        surfaceView.setOnTouchListener(this);
        setContentView(surfaceView);
        hideSystemUi();
    }

    @Override
    protected void onResume() {
        super.onResume();
        paused = false;
        hideSystemUi();
        if (nativeHandle != 0L) {
            nativeOnResume(nativeHandle);
        }
        updateFrameLoop();
    }

    @Override
    protected void onPause() {
        paused = true;
        updateFrameLoop();
        if (nativeHandle != 0L) {
            nativeOnPause(nativeHandle);
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        stopFrameLoop();
        if (nativeHandle != 0L) {
            nativeDestroy(nativeHandle);
            nativeHandle = 0L;
        }
        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        surfaceReady = true;
        Surface surface = holder.getSurface();
        if (nativeHandle != 0L && surface != null && surface.isValid()) {
            nativeSurfaceCreated(nativeHandle, surface);
        }
        updateFrameLoop();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (nativeHandle != 0L) {
            nativeSurfaceChanged(nativeHandle, width, height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        surfaceReady = false;
        updateFrameLoop();
        if (nativeHandle != 0L) {
            nativeSurfaceDestroyed(nativeHandle);
        }
    }

    @Override
    public boolean onTouch(View view, MotionEvent event) {
        if (nativeHandle == 0L) {
            return true;
        }
        final int action = event.getActionMasked();
        final int pointerIndex = event.getActionIndex();
        final float x = event.getX(pointerIndex);
        final float y = event.getY(pointerIndex);
        final int pointerCount = event.getPointerCount();
        nativeTouch(nativeHandle, action, x, y, pointerCount);
        return true;
    }

    @Override
    public void doFrame(long frameTimeNanos) {
        if (!frameLoopRunning) {
            return;
        }
        if (nativeHandle != 0L) {
            nativeFrame(nativeHandle, frameTimeNanos);
        }
        Choreographer.getInstance().postFrameCallback(this);
    }

    private void updateFrameLoop() {
        if (surfaceReady && !paused && nativeHandle != 0L) {
            startFrameLoop();
        } else {
            stopFrameLoop();
        }
    }

    private void startFrameLoop() {
        if (frameLoopRunning) {
            return;
        }
        frameLoopRunning = true;
        Choreographer.getInstance().postFrameCallback(this);
    }

    private void stopFrameLoop() {
        if (!frameLoopRunning) {
            return;
        }
        frameLoopRunning = false;
        Choreographer.getInstance().removeFrameCallback(this);
    }

    private void hideSystemUi() {
        final View decorView = getWindow().getDecorView();
        if (android.os.Build.VERSION.SDK_INT >= 30) {
            final WindowInsetsController controller = decorView.getWindowInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            decorView.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    private static native long nativeCreate();
    private static native void nativeDestroy(long handle);
    private static native void nativeSurfaceCreated(long handle, Surface surface);
    private static native void nativeSurfaceChanged(long handle, int width, int height);
    private static native void nativeSurfaceDestroyed(long handle);
    private static native void nativeOnPause(long handle);
    private static native void nativeOnResume(long handle);
    private static native void nativeTouch(long handle, int action, float x, float y, int pointerCount);
    private static native void nativeFrame(long handle, long frameTimeNanos);
}
