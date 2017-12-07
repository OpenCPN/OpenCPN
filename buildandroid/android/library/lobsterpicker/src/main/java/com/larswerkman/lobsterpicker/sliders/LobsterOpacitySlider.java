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
package com.larswerkman.lobsterpicker.sliders;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Path;
import android.graphics.Region;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.larswerkman.lobsterpicker.LobsterPicker;
import com.larswerkman.lobsterpicker.LobsterSlider;

/**
 * Slider that is able to manipulate the Opacity value of a color.
 */
public class LobsterOpacitySlider extends LobsterSlider {

    private LobsterPicker.Chain chain = LobsterPicker.EMPTY_CHAIN;
    private Path pointerPath;

    private int chainedColor;
    private boolean pointerPressed;
    private float[] hsv = new float[3];

    private int opacity = 255;

    public LobsterOpacitySlider(Context context) {
        super(context);
        init(context, null, 0);
    }

    public LobsterOpacitySlider(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs, 0);
    }

    public LobsterOpacitySlider(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs, defStyleAttr);
    }

    private void init(Context context, AttributeSet attrs, int defStyle) {
        pointerPath = new Path();

        chainedColor = 0xFF000000;
        updateShader();

        invalidate();
    }

    @Override
    public void onColorChanged(LobsterPicker.Chain chain, int color) {
        this.chain = chain;

        chainedColor = color;
        pointerPaint.setColor(color);
        opacity = (int) (((float) pointerPosition.x / length) * 255);

        updateShader();
        updateColor();
        chain.setColor(this, chainedColor);

        if(Color.alpha(color) != 0xFF){
            setOpacity(Color.alpha(color));
        }
        invalidate();
    }

    /** {@inheritDoc} */
    @Override
    public int getColor() {
        return chainedColor;
    }

    /**
     * Returns the selected opacity value
     *
     * @return opacity >= 0 and <= 255
     */
    public int getOpacity() {
        return opacity;
    }

    /**
     * Set the opacity value
     *
     * @param opacity value >= 0 and <= 255
     */
    public void setOpacity(int opacity) {
        this.opacity = opacity;
        updateColor();
        chain.setColor(this, chainedColor);
        getMoveAnimation().start();
    }

    private void updateShader() {
        paint.setShader(new LinearGradient(0, 0,
                length, 0, new int[]{0x00FFFFFF & chainedColor, chainedColor}, null,
                Shader.TileMode.CLAMP));
    }

    private void updateColor() {
        Color.colorToHSV(chainedColor, hsv);
        chainedColor = Color.HSVToColor(opacity, hsv);
        pointerPaint.setColor(chainedColor);
        pointerShadowPaint.setColor(Color.HSVToColor(0x59, hsv));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.translate(pointerShadowRadius, getHeight() / 2);

        canvas.save();

        pointerPath.reset();
        pointerPath.addCircle(pointerPosition.x, pointerPosition.y, pointerRadius, Path.Direction.CW);
        pointerPath.close();

        canvas.clipPath(pointerPath, Region.Op.DIFFERENCE);
        canvas.drawLine(0, 0, length, 0, paint);
        canvas.drawCircle(pointerPosition.x, pointerPosition.y, pointerShadowRadius, pointerShadowPaint);

        canvas.restore();

        canvas.drawCircle(pointerPosition.x, pointerPosition.y, pointerRadius, pointerPaint);
    }

    private ValueAnimator getMoveAnimation() {
        ValueAnimator animator = ValueAnimator.ofInt(pointerPosition.x,
                (int) (((float) opacity / 255.f) * (float) length));
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                pointerPosition.x = (Integer) animation.getAnimatedValue();
                invalidate();
            }
        });
        return animator;
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (x >= pointerShadowRadius && x <= pointerShadowRadius + length) {
                    pointerPosition.x = (int) x - pointerShadowRadius;
                } else if (x < pointerShadowRadius) {
                    pointerPosition.x = 0;
                } else if (x > pointerShadowRadius + length) {
                    pointerPosition.x = length;
                }
                pointerPressed = true;

                opacity = (int) (((float) pointerPosition.x / length) * 255);
                updateColor();
                chain.setColor(this, chainedColor);

                getGrowAnimation().start();
                break;
            case MotionEvent.ACTION_MOVE:
                if (pointerPressed) {
                    if (x >= pointerShadowRadius && x <= pointerShadowRadius + length) {
                        pointerPosition.x = (int) x - pointerShadowRadius;
                    } else if (x < pointerShadowRadius) {
                        pointerPosition.x = 0;
                    } else if (x > pointerShadowRadius + length) {
                        pointerPosition.x = length;
                    }

                    opacity = (int) (((float) pointerPosition.x / length) * 255);
                    updateColor();
                    chain.setColor(this, chainedColor);

                    invalidate();
                }
                break;
            case MotionEvent.ACTION_UP:
                if (pointerPressed) {
                    pointerPosition.x = (int) x - pointerShadowRadius;
                    if(pointerPosition.x > length){
                        pointerPosition.x = length;
                    } else if(pointerPosition.x < 0){
                        pointerPosition.x = 0;
                    }

                    opacity = (int) (((float) pointerPosition.x / length) * 255);
                    updateColor();
                    chain.setColor(this, chainedColor);

                    getShrinkAnimation().start();
                }
                pointerPressed = false;
                break;
        }
        return true;
    }
}
