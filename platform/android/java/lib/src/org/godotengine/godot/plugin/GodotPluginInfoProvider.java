/**************************************************************************/
/*  GodotPluginInfoProvider.java                                          */
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

package org.godotengine.godot.plugin;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.Set;

/**
 * Provides the set of information expected from a Godot plugin.
 */
public interface GodotPluginInfoProvider {
	/**
	 * Returns the name of the plugin.
	 */
	@NonNull
	String getPluginName();

	/**
	 * Returns the list of signals to be exposed to Godot.
	 */
	@NonNull
	default Set<SignalInfo> getPluginSignals() {
		return Collections.emptySet();
	}

	/**
	 * Returns the paths for the plugin's gdextension libraries (if any).
	 *
	 * The paths must be relative to the 'assets' directory and point to a '*.gdextension' file.
	 */
	@NonNull
	default Set<String> getPluginGDExtensionLibrariesPaths() {
		return Collections.emptySet();
	}

	/**
	 * This is invoked on the render thread when the plugin described by this instance has been
	 * registered.
	 */
	default void onPluginRegistered() {
	}
}
