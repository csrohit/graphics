package io.csrohit.colortriangle;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class OpenGLView extends GLSurfaceView {

    private final Context context;
    private final OpenGLRenderer renderer;

    public OpenGLView(Context context) {
        super(context);

        this.context = context;

        setEGLContextClientVersion(3);

        renderer = new OpenGLRenderer();
        setRenderer(renderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

    }
}
