/*
 * Viry3D
 * Copyright 2014-2019 by Stack - stackos@qq.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.viry3d.lib;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.view.Surface;
import android.view.WindowManager;

import java.io.File;

public class JNI {
    private Activity mActivity;

    JNI(Activity activity) {
        mActivity = activity;
    }

    public int backToHome() {
        Intent home = new Intent(Intent.ACTION_MAIN);
        home.addCategory(Intent.CATEGORY_HOME);
        mActivity.startActivity(home);
        return 0;
    }

    public String getPackagePath() {
        return mActivity.getPackageResourcePath();
    }

    public String getFilesDirPath() {
        File files_dir = mActivity.getExternalFilesDir(null);
        if(files_dir == null) {
            files_dir = mActivity.getFilesDir();
        }
        return files_dir.getAbsolutePath();
    }

    public int quitApplication() {
        mActivity.finish();
        return 0;
    }

    public int keepScreenOn(final boolean enable) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(enable) {
                    mActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                } else {
                    mActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            }
        });
        return 0;
    }

    public native void engineCreate(Surface surface, int width, int height, AssetManager assetManager);
    public native void engineDestroy();
    public native void engineSurfaceResize(Surface surface, int width, int height);
    public native void engineSurfaceDestroy();
    public native void enginePause();
    public native void engineResume();
    public native void engineDraw();
    public native void engineKeyDown(int keyCode);
    public native void engineKeyUp(int keyCode);
    public native void engineTouch(byte[] touchData);

    static {
        System.loadLibrary("Viry3DApp");
    }
}
