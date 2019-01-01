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

    int keepScreenOn(final boolean enable) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(enable) {
                    ActivityBase.this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                } else {
                    ActivityBase.this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            }
        });
        return 0;
    }
}