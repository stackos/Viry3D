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
import android.content.Context;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.LinkedList;

public class VRSurfaceView extends SurfaceView {
    static final String TAG = "VRSurfaceView";

    Activity mActivity;
    JNI jni;
    Thread mRenderThread;
    boolean mEngineCreated = false;
    volatile boolean mStopRenderThread = false;
    volatile boolean mPaused = false;
    final Object mLock = new Object();
    LinkedList<Runnable> mActions = new LinkedList<>();

    public VRSurfaceView(Context context) {
        super(context);

        mActivity = (Activity) context;
        jni = new JNI(mActivity);
        mRenderThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while (!mStopRenderThread) {
                    synchronized (mLock) {
                        for (int i = 0; i < mActions.size(); ++i) {
                            Runnable r = mActions.get(i);
                            if (r != null) {
                                r.run();
                            }
                        }
                        mActions.clear();
                    }

                    if (!mPaused) {
                        if (mEngineCreated) {
                            jni.engineDraw();
                        }
                    }
                }

                if (mEngineCreated) {
                    jni.engineDestroy();
                }
            }
        });
        mRenderThread.start();

        this.getHolder().setFormat(PixelFormat.RGB_888);
        this.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                Log.e(TAG, "surfaceCreated");
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
                Log.e(TAG, "surfaceChanged");

                queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        SurfaceHolder holder = VRSurfaceView.this.getHolder();
                        Rect rect = holder.getSurfaceFrame();

                        if (!mEngineCreated) {
                            mEngineCreated = true;
                            jni.engineCreate(holder.getSurface(), rect.width(), rect.height(), mActivity.getAssets());
                        } else {
                            jni.engineSurfaceResize(holder.getSurface(), rect.width(), rect.height());
                        }
                    }
                });
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                Log.e(TAG, "surfaceDestroyed");

                queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        if (mEngineCreated) {
                            jni.engineSurfaceDestroy();
                        }
                    }
                });
            }
        });
    }

    public void queueEvent(Runnable r) {
        synchronized (mLock) {
            mActions.addLast(r);
        }
    }

    public void onPause() {
        Log.e(TAG, "onPause");
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                mPaused = true;
                if (mEngineCreated) {
                    jni.enginePause();
                }
            }
        });
    }

    public void onResume() {
        Log.e(TAG, "onResume");
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                mPaused = false;
                if (mEngineCreated) {
                    jni.engineResume();
                }
            }
        });
    }

    public void onDestroy() {
        Log.e(TAG, "onDestroy");
        mStopRenderThread = true;
    }

    public boolean onKeyDown(int keyCode, final KeyEvent event) {
        Log.e(TAG, "onKeyDown");
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (mEngineCreated) {
                    jni.engineKeyDown(event.getKeyCode());
                }
            }
        });
        return super.onKeyDown(keyCode, event);
    }

    public boolean onKeyUp(int keyCode, final KeyEvent event) {
        Log.e(TAG, "onKeyUp");
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (mEngineCreated) {
                    jni.engineKeyUp(event.getKeyCode());
                }
            }
        });
        return super.onKeyUp(keyCode, event);
    }

    public boolean onTouchEvent(final MotionEvent event) {
        Log.e(TAG, "onTouchEvent");
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                int action = event.getAction();
                int count = event.getPointerCount();
                int index = (action & 0xff00) >> 8;
                int id = event.getPointerId(index);
                long time = event.getEventTime();
                int act = action & 0xff;

                try {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    DataOutputStream dos = new DataOutputStream(baos);

                    dos.writeInt(act);
                    dos.writeInt(index);
                    dos.writeInt(id);
                    dos.writeInt(count);
                    dos.writeLong(time);
                    for (int i = 0; i < count; i++) {
                        dos.writeFloat(event.getX(i));
                        dos.writeFloat(event.getY(i));
                    }

                    if (mEngineCreated) {
                        jni.engineTouch(baos.toByteArray());
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        return super.onTouchEvent(event);
    }
}
