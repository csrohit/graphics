package io.csrohit.ColoredPyramid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class OpenGLView extends GLSurfaceView implements GestureDetector.OnGestureListener {

    public static final String TAG = "OpenGLView";
    private final Context context;
    private final OpenGLRenderer renderer;

    public OpenGLView(Context context) {
        super(context);

        this.context = context;
        setEGLContextClientVersion(3);

        renderer = new OpenGLRenderer();
        setRenderer(renderer);
//        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Log.i(TAG, "Touch event detected");
        return super.onTouchEvent(event);
    }


    public void uninitialize(){
        renderer.uninitialize();
    }

    @Override
    public boolean onDown(@NonNull MotionEvent e) {
        return false;
    }


    @Override
    public void onShowPress(@NonNull MotionEvent e) {

    }


    @Override
    public boolean onSingleTapUp(@NonNull MotionEvent e) {
        return false;
    }


    @Override
    public boolean onScroll(@Nullable MotionEvent e1, @NonNull MotionEvent e2, float distanceX, float distanceY) {
        System.exit(0);
        return true;
    }

    @Override
    public void onLongPress(@NonNull MotionEvent e) {

    }

    @Override
    public boolean onFling(@Nullable MotionEvent e1, @NonNull MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }
}
