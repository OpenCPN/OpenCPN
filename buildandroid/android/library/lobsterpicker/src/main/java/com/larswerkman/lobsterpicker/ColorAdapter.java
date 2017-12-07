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
 * Adapter used to determine the color pallet.
 *
 * The pallet is determined by the size and the size of the shades for each color.
 */
public interface ColorAdapter {

    /**
     * Return the corresponding color for index and shade index.
     *
     * @param position color index
     * @param shade shade index
     *
     * @return ARGB color
     */
    int color(int position, int shade);

    /**
     * How many shades are in the adapter for the color index.
     *
     * @param position color index
     *
     * @return Count of shades
     */
    int shades(int position);

    /**
     * How many colors are in the adapter
     *
     * @return Count of colors
     */
    int size();
}
