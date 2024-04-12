package io.csrohit.grayscale;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class OpenGLView extends GLSurfaceView implements GestureDetector.OnGestureListener, GestureDetector.OnDoubleTapListener {
    private final AppCompatActivity activityContext;
    private MyRenderer renderer;

    private GestureDetector gestureDetector;

    public static final String TAG = "OpenGLView";

    public OpenGLView(Context context) {
        super(context);
        activityContext = (AppCompatActivity) context;
        setEGLContextClientVersion(3);

        renderer = new MyRenderer(context);
        setRenderer(renderer);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG, "Surface Destroyed");
        super.surfaceDestroyed(holder);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "Surface created");
        super.surfaceCreated(holder);
    }

    public void uninitialize()
    {
        renderer.uninitialize();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int eventaction = event.getActionMasked();

        if(!gestureDetector.onTouchEvent(event))
        {
            super.onTouchEvent(event);
        }

        float x = event.getX();
        float y = event.getY();
        switch (eventaction){
            case MotionEvent.ACTION_MOVE:
                Log.i(TAG, "Moved to " + x/renderer.vwHeight + " " + y);
                renderer.mixFactor = x/renderer.vwHeight;
                break;
            case MotionEvent.ACTION_DOWN:
                Log.i(TAG, "Down at " + x/renderer.vwHeight + " " + y);
                break;
        }

        return (true);

    }

    @Override
    public boolean onDown(@NonNull MotionEvent e) {
        return false;
    }

    @Override
    public void onShowPress(@NonNull MotionEvent e) {
        Log.i(TAG, "onShowPress");

    }

    @Override
    public boolean onSingleTapUp(@NonNull MotionEvent e) {
        return false;
    }

    @Override
    public boolean onScroll(@Nullable MotionEvent e1, @NonNull MotionEvent e2, float distanceX, float distanceY) {
        Log.i(TAG, "onScroll");
//        activityContext.finish();
        return true;
    }

    @Override
    public void onLongPress(@NonNull MotionEvent e) {
        Log.i(TAG, "onLongPress");
    }

    @Override
    public boolean onFling(@Nullable MotionEvent e1, @NonNull MotionEvent e2, float velocityX, float velocityY) {
        Log.i(TAG, "onFling");
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(@NonNull MotionEvent e) {
        Log.i(TAG, "onSingleTapConfirmed");
        return true;
    }

    @Override
    public boolean onDoubleTap(@NonNull MotionEvent e) {
        Log.i(TAG, "onDoubleTap");
        renderer.toggleGrayscale();
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(@NonNull MotionEvent e) {
        return false;
    }
}
