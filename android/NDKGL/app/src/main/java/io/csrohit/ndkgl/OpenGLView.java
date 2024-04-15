package io.csrohit.ndkgl;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;


public class OpenGLView extends GLSurfaceView implements GestureDetector.OnGestureListener, GestureDetector.OnDoubleTapListener {
    private final AppCompatActivity activityContext;
    private OpenGLRenderer renderer;

    private GestureDetector gestureDetector;

    public static final String TAG = "OpenGLView";

    public native void setFilesDirectory(String filesDirectory);

    public native int openLogFile(String logFileDirectory);

    public native void uninitialize();


    public OpenGLView(Context context) {
        super(context);

        /* Copy Models from Assets to externalFiles */
        File externalFiles = context.getExternalFilesDir(null);
        File externalStorage = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        openLogFile(externalStorage.getAbsolutePath() + "/log.txt");
        setFilesDirectory(externalFiles.getAbsolutePath());
        setEGLContextClientVersion(3);

        AssetManager assetManager = context.getAssets();
        String[] files = null;
        try {
            files = assetManager.list("gl_assets");


            for (String filename : files) {
                Log.i(TAG, "Copying file " + filename);
                InputStream in = null;
                OutputStream out = null;
                try {
                    in = assetManager.open("gl_assets/" + filename);


                    File outFile = new File(externalFiles.getAbsolutePath(), filename);
                    if(!outFile.exists())
                    {
                        out = new FileOutputStream(outFile);
                        copyFile(in, out);
                        in.close();
                        in = null;
                        out.flush();
                        out.close();
                        out = null;
                        Log.i(TAG, "copied file " + filename + " to " + outFile.getAbsolutePath());
                    }
                    else
                    {

                        Log.i(TAG, "file " + filename + " already exists as " + outFile.getAbsolutePath());
                    }
                } catch (IOException e) {
                    Log.e("tag", "Failed to copy asset file: " + filename, e);
                }
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        activityContext = (AppCompatActivity) context;
        setEGLContextClientVersion(3);

        renderer = new OpenGLRenderer();
        setRenderer(renderer);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
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


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int eventaction = event.getActionMasked();

        if (!gestureDetector.onTouchEvent(event)) {
            super.onTouchEvent(event);
        }

        float x = event.getX();
        float y = event.getY();
        switch (eventaction) {
            case MotionEvent.ACTION_MOVE:
                break;
            case MotionEvent.ACTION_DOWN:
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
        activityContext.finish();
        return true;
    }

    @Override
    public void onLongPress(@NonNull MotionEvent e) {
    }

    @Override
    public boolean onFling(@Nullable MotionEvent e1, @NonNull MotionEvent e2, float velocityX, float velocityY) {
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(@NonNull MotionEvent e) {
        return true;
    }

    @Override
    public boolean onDoubleTap(@NonNull MotionEvent e) {
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(@NonNull MotionEvent e) {
        return false;
    }

    public void destroy() {
        uninitialize();
    }
}

