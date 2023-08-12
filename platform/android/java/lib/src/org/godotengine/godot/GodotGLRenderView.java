/**************************************************************************/
/*  GodotGLRenderView.java                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

package org.godotengine.godot;

import org.godotengine.godot.gl.GLSurfaceView;
import org.godotengine.godot.gl.GodotRenderer;
import org.godotengine.godot.input.GodotInputHandler;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.os.Build;
import android.text.TextUtils;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.PointerIcon;
import android.view.SurfaceView;

import androidx.annotation.Keep;

import java.io.InputStream;

/**
 * A simple GLSurfaceView sub-class that demonstrate how to perform
 * OpenGL ES 2.0 rendering into a GL Surface. Note the following important
 * details:
 *
 * - The class must use a custom context factory to enable 2.0 rendering.
 *   See ContextFactory class definition below.
 *
 * - The class must use a custom EGLConfigChooser to be able to select
 *   an EGLConfig that supports 3.0. This is done by providing a config
 *   specification to eglChooseConfig() that has the attribute
 *   EGL10.ELG_RENDERABLE_TYPE containing the EGL_OPENGL_ES2_BIT flag
 *   set. See ConfigChooser class definition below.
 *
 * - The class must select the surface's format, then choose an EGLConfig
 *   that matches it exactly (with regards to red/green/blue/alpha channels
 *   bit depths). Failure to do so would result in an EGL_BAD_MATCH error.
 */
public class GodotGLRenderView extends GLSurfaceView implements GodotRenderView {
	private final GodotHost host;
	private final Godot godot;
	private final GodotInputHandler inputHandler;
	private final GodotRenderer godotRenderer;
	private final SparseArray<PointerIcon> customPointerIcons = new SparseArray<>();

	public GodotGLRenderView(GodotHost host, Godot godot, boolean useDebugOpengl) {
		super(host.getActivity());

		this.host = host;
		this.godot = godot;
		this.inputHandler = new GodotInputHandler(this);
		this.godotRenderer = new GodotRenderer();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			setPointerIcon(PointerIcon.getSystemIcon(getContext(), PointerIcon.TYPE_DEFAULT));
		}
		init();
	}

	@Override
	public SurfaceView getView() {
		return this;
	}

	@Override
	public void initInputDevices() {
		this.inputHandler.initInputDevices();
	}

	@Override
	public void queueOnRenderThread(Runnable event) {
		queueEvent(event);
	}

	@Override
	public void onActivityPaused() {
		onPause();
	}

	@Override
	public void onActivityResumed() {
		onResume();
	}

	@Override
	public void onBackPressed() {
		godot.onBackPressed(host);
	}

	@Override
	public GodotInputHandler getInputHandler() {
		return inputHandler;
	}

	@SuppressLint("ClickableViewAccessibility")
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);
		return inputHandler.onTouchEvent(event);
	}

	@Override
	public boolean onKeyUp(final int keyCode, KeyEvent event) {
		return inputHandler.onKeyUp(keyCode, event) || super.onKeyUp(keyCode, event);
	}

	@Override
	public boolean onKeyDown(final int keyCode, KeyEvent event) {
		return inputHandler.onKeyDown(keyCode, event) || super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		return inputHandler.onGenericMotionEvent(event) || super.onGenericMotionEvent(event);
	}

	@Override
	public boolean onCapturedPointerEvent(MotionEvent event) {
		return inputHandler.onGenericMotionEvent(event);
	}

	@Override
	public void onPointerCaptureChange(boolean hasCapture) {
		super.onPointerCaptureChange(hasCapture);
		inputHandler.onPointerCaptureChange(hasCapture);
	}

	@Override
	public void requestPointerCapture() {
		if (canCapturePointer()) {
			super.requestPointerCapture();
			inputHandler.onPointerCaptureChange(true);
		}
	}

	@Override
	public void releasePointerCapture() {
		super.releasePointerCapture();
		inputHandler.onPointerCaptureChange(false);
	}

	/**
	 * Used to configure the PointerIcon for the given type.
	 *
	 * Called from JNI
	 */
	@Keep
	@Override
	public void configurePointerIcon(int pointerType, String imagePath, float hotSpotX, float hotSpotY) {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
			try {
				Bitmap bitmap = null;
				if (!TextUtils.isEmpty(imagePath)) {
					if (godot.getDirectoryAccessHandler().filesystemFileExists(imagePath)) {
						// Try to load the bitmap from the file system
						bitmap = BitmapFactory.decodeFile(imagePath);
					} else if (godot.getDirectoryAccessHandler().assetsFileExists(imagePath)) {
						// Try to load the bitmap from the assets directory
						AssetManager am = getContext().getAssets();
						InputStream imageInputStream = am.open(imagePath);
						bitmap = BitmapFactory.decodeStream(imageInputStream);
					}
				}

				PointerIcon customPointerIcon = PointerIcon.create(bitmap, hotSpotX, hotSpotY);
				customPointerIcons.put(pointerType, customPointerIcon);
			} catch (Exception e) {
				// Reset the custom pointer icon
				customPointerIcons.delete(pointerType);
			}
		}
	}

	/**
	 * called from JNI to change pointer icon
	 */
	@Keep
	@Override
	public void setPointerIcon(int pointerType) {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			PointerIcon pointerIcon = customPointerIcons.get(pointerType);
			if (pointerIcon == null) {
				pointerIcon = PointerIcon.getSystemIcon(getContext(), pointerType);
			}
			setPointerIcon(pointerIcon);
		}
	}

	@Override
	public PointerIcon onResolvePointerIcon(MotionEvent me, int pointerIndex) {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			return getPointerIcon();
		}
		return super.onResolvePointerIcon(me, pointerIndex);
	}

	private void init() {
		setPreserveEGLContextOnPause(true);
		setFocusableInTouchMode(true);
	}

	@Override
	public void startRenderer() {
		/* Set the renderer responsible for frame rendering */
		setRenderer(godotRenderer);
	}

	@Override
	public void onResume() {
		super.onResume();

		queueEvent(() -> {
			// Resume the renderer
			godotRenderer.onActivityResumed();
			GodotLib.focusin();
		});
	}

	@Override
	public void onPause() {
		super.onPause();

		queueEvent(() -> {
			GodotLib.focusout();
			// Pause the renderer
			godotRenderer.onActivityPaused();
		});
	}
}
