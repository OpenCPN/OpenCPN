/*
 * Copyright (C) 2015 Marie Schweiz & Lars Werkman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.larswerkman.lobsterpicker;

//import android.support.annotation.ColorInt;

/**
 * Interface used to detect when a color has changed and is selected.
 */
public interface OnColorListener {
    /**
     * Called when the color changes.
     *
     * @param color ARGB color
     */
    void onColorChanged( int color);

    /**
     * Called when the color is selected,
     * determined when the {@link android.view.MotionEvent} has ended.
     *
     * @param color ARGB color
     */
    void onColorSelected( int color);
}
