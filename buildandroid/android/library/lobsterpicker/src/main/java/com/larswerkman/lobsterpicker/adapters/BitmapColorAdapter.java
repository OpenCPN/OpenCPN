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
package com.larswerkman.lobsterpicker.adapters;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
//import android.support.annotation.ColorInt;
//import android.support.annotation.DrawableRes;

import com.larswerkman.lobsterpicker.ColorAdapter;

/**
 * {@link ColorAdapter} implementation that uses a {@link android.graphics.drawable.Drawable} resource
 * as source. Each vertical pixel represents a color and each horizontal pixel represents a shade for
 * that color
 */
public class BitmapColorAdapter implements ColorAdapter {

    private Bitmap bitmap;

    public BitmapColorAdapter(Context context,  int resource) {
        bitmap = BitmapFactory.decodeResource(context.getResources(), resource);
    }

    @Override
    public  int color(int color, int shade) {
        int pixel = bitmap.getPixel(shade, color);
        while(pixel == 0x0){
            pixel = bitmap.getPixel(shade--, color);
        }
        return pixel;
    }

    @Override
    public int size() {
        return bitmap.getHeight();
    }

    @Override
    public int shades(int position) {
        int[] pixels = new int[bitmap.getWidth()];
        bitmap.getPixels(pixels, 0, bitmap.getWidth(), 0, position, bitmap.getWidth(), 1);
        for (int i = 0; i < pixels.length; i++) {
            if (pixels[i] == 0x0) {
                return i;
            }
        }
        return bitmap.getWidth();
    }
}
