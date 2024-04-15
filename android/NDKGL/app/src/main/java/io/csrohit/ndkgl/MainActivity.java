package io.csrohit.ndkgl;

import static android.os.Environment.getExternalStoragePublicDirectory;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowInsetsCompat;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.WindowInsets;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

import io.csrohit.ndkgl.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'ndkgl' library on application startup.
    static {
        System.loadLibrary("ndkgl");
    }
    public static final String TAG = "MainActivity";

    private OpenGLView view;


    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "OnCreate Called");
        EdgeToEdge.enable(this);
        getWindow().getDecorView().setOnApplyWindowInsetsListener((view, windowInsets)->{
            if (windowInsets.isVisible(WindowInsetsCompat.Type.navigationBars())
                    || windowInsets.isVisible(WindowInsetsCompat.Type.statusBars())) {
                view.getWindowInsetsController().hide(WindowInsets.Type.systemBars());
            }
            return view.onApplyWindowInsets(windowInsets);
        });

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        view = new OpenGLView(this);
        setContentView(view);
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "OnDestroy Called");
        super.onDestroy();
        view.destroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        view.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        view.onResume();
    }
}