package io.csrohit.ColoredPyramid;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowInsets;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "MainActivity";
    private OpenGLView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
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
        Log.i(TAG, "OnDestroy is called");
        view.uninitialize();
        super.onDestroy();
    }

    /**
     *
     */
    @Override
    protected void onStart() {
        Log.i(TAG, "OnStart is called");
        super.onStart();
    }

    /**
     *
     */
    @Override
    protected void onStop() {
        Log.i(TAG, "OnStop is called");
        super.onStop();
    }
}