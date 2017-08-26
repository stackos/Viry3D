package com.viry3d.lib;

import android.app.NativeActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.WindowManager;

import java.io.File;

public class ActivityBase extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    int backToHome() {
        Intent home = new Intent(Intent.ACTION_MAIN);
        home.addCategory(Intent.CATEGORY_HOME);
        startActivity(home);
        return 0;
    }

    String getPackagePath() {
        return this.getPackageResourcePath();
    }

    String getFilesDirPath() {
        File files_dir = this.getExternalFilesDir(null);
        if(files_dir == null) {
            files_dir = this.getFilesDir();
        }
        return files_dir.getAbsolutePath();
    }

    int keepScreenOn(boolean enable) {
        if(enable) {
            this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
        return 0;
    }

    int quitApplication() {
        android.os.Process.killProcess(android.os.Process.myPid());
        return 0;
    }
}