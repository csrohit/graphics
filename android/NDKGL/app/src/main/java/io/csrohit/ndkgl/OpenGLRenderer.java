package io.csrohit.ndkgl;

import static android.opengl.GLES32.*;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {

    public static final String TAG = "OpenGLRenderer";
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initialize();
    }

    public native int initialize();
    public native void display();
    public native void resize(int width, int height);

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {

        display();
    }
}
