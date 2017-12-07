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

import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.PointF;
//import android.support.annotation.ColorInt;
import android.util.AttributeSet;
import android.view.View;

/**
 * Abstract slider view to set up default size's and paints
 */
public abstract class LobsterSlider extends View implements ColorDecorator {

    protected int thickness;
    protected int length;

    protected int pointerRadius;
    protected int originalPointerRadius;
    protected int pointerShadowRadius;

    protected Point pointerPosition;

    protected Paint paint;
    protected Paint pointerPaint;
    protected Paint pointerShadowPaint;

    public LobsterSlider(Context context) {
        super(context);
        init(context, null, 0);
    }

    public LobsterSlider(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs, 0);
    }

    public LobsterSlider(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs, defStyleAttr);
    }

    private void init(Context context, AttributeSet attrs, int defStyle) {
        final TypedArray a = context.obtainStyledAttributes(attrs,
                R.styleable.LobsterSlider, defStyle, 0);
        final Resources b = context.getResources();

        thickness = a.getDimensionPixelSize(
                R.styleable.LobsterSlider_color_slider_thickness,
                b.getDimensionPixelSize(R.dimen.color_slider_thickness));
        length = a.getDimensionPixelSize(
                R.styleable.LobsterSlider_color_slider_length,
                b.getDimensionPixelSize(R.dimen.color_slider_length));
        pointerRadius = originalPointerRadius = a.getDimensionPixelSize(
                R.styleable.LobsterSlider_color_slider_pointer_radius,
                b.getDimensionPixelSize(R.dimen.color_slider_pointer_radius));
        pointerShadowRadius = a.getDimensionPixelSize(
                R.styleable.LobsterSlider_color_slider_pointer_shadow_radius,
                b.getDimensionPixelSize(R.dimen.color_slider_pointer_shadow_radius));
        a.recycle();

        paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(thickness);

        pointerPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        pointerPaint.setStyle(Paint.Style.FILL);

        pointerShadowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        pointerShadowPaint.setStyle(Paint.Style.FILL);

        pointerPosition = new Point(length, 0);
    }

    /**
     * Returns the selected color
     *
     * @return an ARGB color
     */
    public abstract  int getColor();

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final int intrinsicWidth = length + pointerShadowRadius * 2;
        final int intrinsicHeight = pointerShadowRadius * 2;

        int widthMode = MeasureSpec.getMode(widthMeasureSpec);
        int widthSize = MeasureSpec.getSize(widthMeasureSpec);
        int heightMode = MeasureSpec.getMode(heightMeasureSpec);
        int heightSize = MeasureSpec.getSize(heightMeasureSpec);

        int width;
        int height;

        if (widthMode == MeasureSpec.EXACTLY) {
            width = widthSize;
        } else if (widthMode == MeasureSpec.AT_MOST) {
            width = Math.min(intrinsicWidth, widthSize);
        } else {
            width = intrinsicWidth;
        }

        if (heightMode == MeasureSpec.EXACTLY) {
            height = heightSize;
        } else if (heightMode == MeasureSpec.AT_MOST) {
            height = Math.min(intrinsicHeight, heightSize);
        } else {
            height = intrinsicHeight;
        }

        setMeasuredDimension(width, height);
    }

    protected ValueAnimator getShrinkAnimation() {
        ValueAnimator animator = ValueAnimator.ofInt(pointerRadius, originalPointerRadius);
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                pointerRadius = (Integer) animation.getAnimatedValue();
                invalidate();
            }
        });
        return animator;
    }

    protected ValueAnimator getGrowAnimation() {
        ValueAnimator animator = ValueAnimator.ofInt(pointerRadius, pointerShadowRadius);
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                pointerRadius = (Integer) animation.getAnimatedValue();
                invalidate();
            }
        });
        return animator;
    }
}
