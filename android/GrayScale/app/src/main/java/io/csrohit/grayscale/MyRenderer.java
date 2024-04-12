package io.csrohit.grayscale;

import static io.csrohit.grayscale.ElementAttribute.ATTRIBUTE_POSITION;
import static io.csrohit.grayscale.ElementAttribute.ATTRIBUTE_TEXCOORDS;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyRenderer implements GLSurfaceView.Renderer {

    public static final String TAG = "MyRenderer";

    private final Context context;

    private int shaderProgramId;
    private int mvpMatrixUniform;
    private int textureSamplerUniform;
    private int grayScaleHeightFactorUniform;

    private float grayScaleHeightFactor;
    private int vwHeightUniform;
    public float vwHeight;
    private int mixFactorUniform;
    public float mixFactor;
    private boolean isGrascale;
    private IntBuffer smiley_texture;
    private int[] vao = new int[1];
    private int[] vboPosition = new int[1];
    private int[] vboColor = new int[1];
    private float[] projectionMatrix = new float[16];

    public MyRenderer(Context context) {
        this.context = context;
    }

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
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        display();
        update();
    }

    private int loadGLTexture(int imageFileResourceID, IntBuffer buffer)
    {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;

        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), imageFileResourceID, options);

        int[] texture = new int[1];

        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);
        GLES32.glGenTextures(1, texture, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture[0]);

        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);

        //push the data to texture memory
        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);
        buffer.put(0, texture[0]);
        return (texture[0]);
    }

    public void initialize() {
        final String vertexShaderSourceCode =
                            "#version 320 es" +
                            "\n" +
                            "in      vec4 vPosition;" +
                            "in      vec2 vTexCoords;" +
                            "uniform mat4 uMVPMatrix;" +
                            "out     vec2 oTexCoord;" +
                            "void main(void)" +
                            "{" +
                            "   gl_Position = uMVPMatrix * vPosition;" +
                            "   oTexCoord   = vTexCoords;" +
                            "}";
        final String fragmentShaderSourceCode =
                                "#version 320 es" +
                                "\n" +
                                "precision highp float;" +
                                "in      vec2       oTexCoord;" +
                                "out     vec4       FragColor;" +
                                "uniform sampler2D  uTextureSampler;" +
                                "uniform float      uGrayScaleHeightFactor;" +
                                "uniform float      uVWHeight;" +
                                "uniform float      uConversionFactor;" +
                                "void main(void)" +
                                "{" +
                                "   vec4 color  = texture(uTextureSampler, oTexCoord);" +
                                "   float value = gl_FragCoord.x/uVWHeight;" +
                                "   if((value > (uConversionFactor - 0.05)) && (value < uConversionFactor)){" +
                                "       float gsc    = (color.r*0.3) + (color.g * 0.59) + (color.b * 0.11);" +
                                "       vec4  gColor = vec4(gsc, gsc, gsc, 1.0);" +
                                "       FragColor   = mix(gColor, color, (value - uConversionFactor + 0.05)/0.05);" +
                                "   }else if((gl_FragCoord.x/uVWHeight) < uConversionFactor){" +
                                "       float gsc   = (color.r*0.3) + (color.g * 0.59) + (color.b * 0.11);" +
                                "       FragColor   = vec4(gsc, gsc, gsc, 1.0);" +
                                "   }" +
                                "   else{" +
                                "       FragColor   = color;" +
                                "   }" +
                                "}";

        final float[] positions = new float[]
                {
                        1.0f,  1.0f, 0.0f,
                        -1.0f,  1.0f, 0.0f,
                        -1.0f, -1.0f, 0.0f,
                        1.0f, -1.0f, 0.0f,

                };
        final float[] colors = new float[]
                {
                        1.0f, 1.0f,
                        0.0f, 1.0f,
                        0.0f, 0.0f,
                        1.0f, 0.0f,
                };
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

        GLES32.glBindAttribLocation(shaderProgramId, ATTRIBUTE_POSITION, "vPosition");
        GLES32.glBindAttribLocation(shaderProgramId, ATTRIBUTE_TEXCOORDS, "vTexCoords");
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
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramId, "uTextureSampler");
        mixFactorUniform = GLES32.glGetUniformLocation(shaderProgramId, "uConversionFactor");
        grayScaleHeightFactorUniform = GLES32.glGetUniformLocation(shaderProgramId, "uGrayScaleHeightFactor");
        vwHeightUniform  = GLES32.glGetUniformLocation(shaderProgramId, "uVWHeight");

        FloatBuffer vertexBuffer;
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
        {
            GLES32.glGenBuffers(1, vboPosition, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPosition[0]);
            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(positions.length * Float.BYTES);
            byteBuffer.order(ByteOrder.nativeOrder());
            vertexBuffer = byteBuffer.asFloatBuffer();
            vertexBuffer.put(positions);
            vertexBuffer.position(0);
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, positions.length * Float.BYTES, vertexBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(ElementAttribute.ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(ElementAttribute.ATTRIBUTE_POSITION);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

            GLES32.glGenBuffers(1, vboColor, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboColor[0]);
            ByteBuffer colorByteBuffer = ByteBuffer.allocateDirect(colors.length * Float.BYTES);
            colorByteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer colorBuffer = colorByteBuffer.asFloatBuffer();
            colorBuffer.put(colors);
            colorBuffer.position(0);
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, colors.length * Float.BYTES, colorBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(ElementAttribute.ATTRIBUTE_TEXCOORDS, 2, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(ElementAttribute.ATTRIBUTE_TEXCOORDS);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glEnable(GLES32.GL_CULL_FACE);
        //load textures
        smiley_texture = IntBuffer.allocate(1);
        loadGLTexture(R.raw.img, smiley_texture);

        GLES32.glClearColor(0.0F, 0.0F, 0.2F, 1.0F);
        Matrix.setIdentityM(projectionMatrix, 0);

        mixFactor = 0.0f;
        isGrascale = false;
    }

    public void resize(int width, int height) {
        if (height == 0) {
            height = 1;
        }
        vwHeight = (float)width;
        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(projectionMatrix, 0, 45.0f, (float) width / (float) height, 0.1f, 100.0f);
    }

    public void display() {
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramId);

        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.5f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, projectionMatrix, 0, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smiley_texture.get(0));
        GLES32.glUniform1i(textureSamplerUniform, 0);

        GLES32.glUniform1f(mixFactorUniform, mixFactor);
        GLES32.glUniform1f(vwHeightUniform, vwHeight);

        GLES32.glBindVertexArray(vao[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);
    }

    public void update() {
//        if(isGrascale){
//            if(1.0f > mixFactor){
//                mixFactor+= 0.01f;
//            }
//        }else {
//            if(0.0f < mixFactor){
//                mixFactor-= 0.01f;
//            }
//        }
    }

    public void toggleGrayscale(){
        isGrascale = !isGrascale;
    }

    public void uninitialize() {
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

        if(0 != smiley_texture.get(0)){
            GLES32.glDeleteTextures(1, smiley_texture);
        }

        if (0 != shaderProgramId) {
            GLES32.glDeleteProgram(shaderProgramId);
            shaderProgramId = 0;
        }
    }
}
