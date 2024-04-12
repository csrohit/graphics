package io.csrohit.ColoredPyramid;

import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {
    public static final String TAG = "OpenGLRenderer";
    private int[] vao = new int[1];
    private int[] vboPosition = new int[1];
    private int[] vboColor = new int[1];

    private int shaderProgramId;

    private int mvpMatrixUniform;
    private float perspectiveProjectionMatrix[] = new float[16];

    private float angle = 0.0f;

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.i(TAG, "OpenGL Version = " + gl.glGetString(GL10.GL_VERSION));
        Log.i(TAG, "GLSL Version = " + gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION));
        Log.i(TAG, "GLSL Renderer = " + gl.glGetString(GLES32.GL_RENDERER));
        Log.i(TAG, "GLSL Vendor = " + gl.glGetString(GLES32.GL_VENDOR));
        Log.i(TAG, "size of float " + Float.BYTES);

        initialize();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.i(TAG, "Surface changed " + width + "x" + height);
        if (height == 0) {
            height = 1;
        }

        GLES32.glViewport(0, 0, width, height);

        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float) width / (float) height, 0.1f, 100.0f);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        display();
        update();
    }

    public void initialize() {
        final float[] triangleVertices = new float[]
                {
                        0.0f, 1.0f, 0.0f,
                        -1.0f, -1.0f, 1.0f,
                        1.0f, -1.0f, 1.0f,

                        //right
                        0.0f, 1.0f, 0.0f,
                        1.0f, -1.0f, 1.0f,
                        1.0f, -1.0f, -1.0f,

                        //far
                        0.0f, 1.0f, 0.0f,
                        1.0f, -1.0f, -1.0f,
                        -1.0f, -1.0f, -1.0f,

                        //left
                        0.0f, 1.0f, 0.0f,
                        -1.0f, -1.0f, -1.0f,
                        -1.0f, -1.0f, 1.0f

                };
        final float[] triangleColor = new float[]
                {
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f,

                        //right
                        1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 1.0f, 0.0f,

                        //far
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f,

                        //left
                        1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 1.0f, 0.0f

                };
        final String vertexShaderSourceCode = String.format
                (
                        "#version 320 es" +
                                "\n" +
                                "in vec4 vPosition;" +
                                "in vec4 vColor;" +
                                "uniform mat4 uMVPMatrix;" +
                                "out vec4 oColor;" +
                                "void main(void)" +
                                "{" +
                                "   gl_Position = uMVPMatrix * vPosition;" +
                                "   oColor = vColor;" +
                                "}"
                );
        final String fragmentShaderSourceCode = String.format
                (
                        "#version 320 es" +
                                "\n" +
                                "precision highp float;" +
                                "in vec4 oColor;" +
                                "out vec4 FragColor;" +
                                "void main(void)" +
                                "{" +
                                "   FragColor = oColor;" +
                                "}"
                );

        int[] status = new int[1];

        Log.i(TAG, "Compiling Vertex shader");
        int vertexShaderId = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderId, vertexShaderSourceCode);
        GLES32.glCompileShader(vertexShaderId);

        GLES32.glGetShaderiv(vertexShaderId, GLES32.GL_COMPILE_STATUS, status, 0);
        if (GLES32.GL_FALSE == status[0]) {
            int[] szInfoLogLength = new int[1];
            GLES32.glGetShaderiv(vertexShaderId, GLES32.GL_INFO_LOG_LENGTH, szInfoLogLength, 0);
            if (0 < szInfoLogLength[0]) {
                String log = GLES32.glGetShaderInfoLog(vertexShaderId);
                Log.e(TAG, "Failed to compile vertex shader" + log);
                System.exit(-1);
            }
        }
        Log.i(TAG, "Done Compiling Vertex shader");

        Log.i(TAG, "Compiling Fragment shader");
        int fragmentShaderId = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderId, fragmentShaderSourceCode);
        GLES32.glCompileShader(fragmentShaderId);
        GLES32.glGetShaderiv(fragmentShaderId, GLES32.GL_COMPILE_STATUS, status, 0);
        if (GLES32.GL_FALSE == status[0]) {
            int[] szInfoLogLength = new int[1];
            GLES32.glGetShaderiv(fragmentShaderId, GLES32.GL_INFO_LOG_LENGTH, szInfoLogLength, 0);
            if (0 < szInfoLogLength[0]) {
                String log = GLES32.glGetShaderInfoLog(fragmentShaderId);
                Log.e(TAG, "Failed to compile Fragment shader" + log);
                System.exit(-1);
            }
        }
        Log.i(TAG, "Done Compiling Fragment shader");

        Log.i(TAG, "Linking program");
        shaderProgramId = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramId, vertexShaderId);
        GLES32.glAttachShader(shaderProgramId, fragmentShaderId);

        GLES32.glBindAttribLocation(shaderProgramId, 0, "vPosition");
        GLES32.glBindAttribLocation(shaderProgramId, 1, "vColor");
        GLES32.glLinkProgram(shaderProgramId);
        GLES32.glGetProgramiv(shaderProgramId, GLES32.GL_LINK_STATUS, status, 0);
        if (GLES32.GL_FALSE == status[0]) {
            int[] szInfoLogLength = new int[1];
            GLES32.glGetProgramiv(shaderProgramId, GLES32.GL_INFO_LOG_LENGTH, szInfoLogLength, 0);
            if (0 < szInfoLogLength[0]) {
                String log = GLES32.glGetProgramInfoLog(shaderProgramId);
                Log.e(TAG, "Failed to link program" + log);
                GLES32.glDetachShader(shaderProgramId, vertexShaderId);
                GLES32.glDetachShader(shaderProgramId, fragmentShaderId);

                GLES32.glDeleteShader(vertexShaderId);
                GLES32.glDeleteShader(fragmentShaderId);

                System.exit(-1);
            }

        } else {
            GLES32.glDetachShader(shaderProgramId, vertexShaderId);
            GLES32.glDetachShader(shaderProgramId, fragmentShaderId);

            GLES32.glDeleteShader(vertexShaderId);
            GLES32.glDeleteShader(fragmentShaderId);
        }
        Log.i(TAG, "Done Linking program");
        mvpMatrixUniform = GLES32.glGetUniformLocation(shaderProgramId, "uMVPMatrix");

        FloatBuffer vertexBuffer;
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
        {
            GLES32.glGenBuffers(1, vboPosition, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPosition[0]);
            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(triangleVertices.length * Float.BYTES);
            byteBuffer.order(ByteOrder.nativeOrder());
            vertexBuffer = byteBuffer.asFloatBuffer();
            vertexBuffer.put(triangleVertices);
            vertexBuffer.position(0);
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, triangleVertices.length * Float.BYTES, vertexBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(0, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

            GLES32.glGenBuffers(1, vboColor, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboColor[0]);
            ByteBuffer colorByteBuffer = ByteBuffer.allocateDirect(triangleColor.length * Float.BYTES);
            colorByteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer colorBuffer = colorByteBuffer.asFloatBuffer();
            colorBuffer.put(triangleColor);
            colorBuffer.position(0);
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, triangleColor.length * Float.BYTES, colorBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(1, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(1);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        GLES32.glClearColor(0.0F, 0.0F, 0.2F, 1.0F);
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    public void update() {
        angle += 0.5f;
        if (360.0f > angle) {
            angle = angle - 360.0f;
        }
    }

    public void display() {
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];
        float[] rotationMatrix_y = new float[16];
        float[] translationMatrix = new float[16];

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramId);

        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(rotationMatrix_y, 0);
        Matrix.setIdentityM(translationMatrix, 0);

        Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -5.0f);
        Matrix.rotateM(rotationMatrix_y, 0, angle, 0.0f, 1.0f, 0.0f);

        Matrix.multiplyMM(modelViewMatrix, 0, translationMatrix, 0, rotationMatrix_y, 0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

        GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);
    }

    void uninitialize() {
        Log.i(TAG, "Uninitializing renderer");
        if (0 != vao[0]) {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
        }
        if (0 != vboPosition[0]) {
            GLES32.glDeleteVertexArrays(1, vboPosition, 0);
            vboPosition[0] = 0;
        }
        if (0 != vboColor[0]) {
            GLES32.glDeleteVertexArrays(1, vboColor, 0);
            vboColor[0] = 0;
        }

        if (0 != shaderProgramId) {
            GLES32.glDeleteProgram(shaderProgramId);
            shaderProgramId = 0;
        }
    }


}
