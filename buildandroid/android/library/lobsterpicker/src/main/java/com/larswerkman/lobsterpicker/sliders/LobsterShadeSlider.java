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

import android.animation.AnimatorSet;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
//import android.support.annotation.ColorInt;
//import android.support.annotation.NonNull;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.larswerkman.lobsterpicker.ColorAdapter;
import com.larswerkman.lobsterpicker.ColorDecorator;
import com.larswerkman.lobsterpicker.LobsterPicker;
import com.larswerkman.lobsterpicker.LobsterSlider;
import com.larswerkman.lobsterpicker.OnColorListener;
import com.larswerkman.lobsterpicker.R;
import com.larswerkman.lobsterpicker.adapters.BitmapColorAdapter;

import java.util.ArrayList;
import java.util.List;


/**
 * Slider that is able to choose the shade from the {@link ColorAdapter},
 * And also is able to be used standalone without attaching it to the {@link LobsterPicker}
 */
public class LobsterShadeSlider extends LobsterSlider {

    /**
     * Updates color by invoking each {@link ColorDecorator} in the order they are added.
     */
    private LobsterPicker.Chain chain = new LobsterPicker.Chain() {
        @Override
        public void setColor(ColorDecorator callback, int color) {
            int index = decorators.indexOf(callback);
            if (index < (decorators.size() - 1)) {
                decorators.get(index + 1).onColorChanged(this, color);
            } else {
                if (chainedColor != color) {
                    for (OnColorListener listener : listeners) {
                        listener.onColorChanged(color);
                    }
                }

                //Set the color.
                chainedColor = color;
            }
        }

        @Override
        public ColorAdapter getAdapter() {
            return adapter;
        }

        @Override
        public int getAdapterPosition() {
            return 0;
        }

        @Override
        public int getShadePosition() {
            return shadePosition;
        }

        @Override
        public void setShade(int position) {
            //Not implemented
        }
    };

    private ColorDecorator updateDecorator = new ColorDecorator() {
        @Override
        public void onColorChanged(LobsterPicker.Chain chain,  int color) {
            chain.setColor(this, color);
        }
    };

    private List<ColorDecorator> decorators;
    private List<OnColorListener> listeners;

    private LobsterPicker.Chain lobsterPickerChain = LobsterPicker.EMPTY_CHAIN;
    private ColorAdapter adapter;
    private ValueAnimator currentAnimation;

    private boolean pointerPressed;

    private int[] shades;
    private int segmentLength;
    private int shadePosition;
    private int currentAnimationEnd;
    private int chainedColor;

    public LobsterShadeSlider(Context context) {
        super(context);
        init(context, null, 0);
    }

    public LobsterShadeSlider(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs, 0);
    }

    public LobsterShadeSlider(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs, defStyleAttr);
    }

    private void init(Context context, AttributeSet attrs, int defStyle) {
        final TypedArray a = context.obtainStyledAttributes(attrs,
                R.styleable.LobsterShadeSlider, defStyle, 0);
        int schemeRes = a.getResourceId(
                R.styleable.LobsterShadeSlider_color_slider_scheme, R.drawable.default_shader_pallete);
        a.recycle();

        decorators = new ArrayList<ColorDecorator>();
        listeners = new ArrayList<OnColorListener>();
        adapter = new BitmapColorAdapter(getContext(), schemeRes);
        shadePosition = adapter.shades(0) - 1;
        decorators.add(updateDecorator);

        updateColorAdapter();
        pointerPosition.x = (segmentLength * shadePosition) + (segmentLength / 2);
        invalidate();
    }

    @Override
    public void onColorChanged(LobsterPicker.Chain chain, int color) {
        this.lobsterPickerChain = chain;

        int size = chain.getAdapter().shades(chain.getAdapterPosition());

        segmentLength = length / size;
        shades = new int[size];
        for (int i = 0; i < size; i++) {
            shades[size - 1 - i] = chain.getAdapter().color(chain.getAdapterPosition(), i);
        }

        shadePosition = chain.getShadePosition();
        shadePosition = size - 1 - shadePosition;
        if (shadePosition == shades.length) {
            shadePosition--;
        } else if (shadePosition < 0) {
            shadePosition = 0;
        }

        updatePointer();
        chain.setShade(getShadePosition());
        chain.setColor(this, shades[shadePosition]);

        if (chainedColor != shades[shadePosition]) {
            for (OnColorListener listener : listeners) {
                listener.onColorChanged(chainedColor);
            }
        }
        chainedColor = shades[shadePosition];

        int upcomingPosition = (segmentLength * shadePosition) + (segmentLength / 2);
        if (upcomingPosition != currentAnimationEnd) {
            if (currentAnimation != null && currentAnimation.isStarted()) {
                currentAnimation.cancel();
            }
            currentAnimation = getMoveAnimation();
            currentAnimation.start();
        }
        invalidate();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getColor() {
        return chainedColor;
    }

    public void setColor( int color) {
        setClosestColorPosition(color);
        int oldColor = chainedColor;

        updatePointer();
        chainDecorators(Color.alpha(color));

        if (chainedColor != oldColor) {
            for (OnColorListener listener : listeners) {
                listener.onColorSelected(chainedColor);
            }
        }
        getMoveAnimation().start();
    }

    /**
     * Set a custom {@link ColorAdapter}
     *
     * @param adapter to retrieve colors from
     */
    public void setColorAdapter( ColorAdapter adapter) {
        int oldColor = chainedColor;
        this.adapter = adapter;
        if (getShadePosition() >= adapter.shades(0) - 1) {
            shadePosition = 0;
        } else if (shadePosition >= adapter.shades(0)) {
            shadePosition = adapter.shades(0) - 1 - getShadePosition();
        }

        updateColorAdapter();

        if (chainedColor != oldColor) {
            for (OnColorListener listener : listeners) {
                listener.onColorSelected(chainedColor);
            }
        }
        getMoveAnimation().start();
    }

    /**
     * Add a {@link ColorDecorator} which can decorate the current color,
     * decorators get called in the order they are added.
     *
     * @param decorator to be added
     */
    public void addDecorator( ColorDecorator decorator) {
        if (!decorators.contains(decorator)) {
            decorators.add(decorator);
            chainDecorators();
        }
    }

    /**
     * Remove a {@link ColorDecorator}.
     *
     * @param decorator to be removed
     * @return true if it could remove the decorator
     */
    public boolean removeDecorator( ColorDecorator decorator) {
        return decorators.remove(decorator);
    }

    /**
     * Add a {@link OnColorListener}, can't be added multiple times.
     *
     * @param listener to be added
     */
    public void addOnColorListener( OnColorListener listener) {
        if (!listeners.contains(listener)) {
            listeners.add(listener);
        }
    }

    /**
     * Remove a {@link OnColorListener}
     *
     * @param listener to be removed
     * @return true if it could remove the listener
     */
    public boolean removeOnColorListener( OnColorListener listener) {
        return listeners.remove(listener);
    }

    /**
     * Set shade position
     *
     * @param position > 0 and < MAX shades inside {@link ColorAdapter}
     */
    public void setShadePosition(int position) {
        shadePosition = shades.length - 1 - position;

        int oldColor = chainedColor;
        updateShade();
        if (chainedColor != oldColor) {
            for (OnColorListener listener : listeners) {
                listener.onColorSelected(chainedColor);
            }
        }
        getMoveAnimation().start();
    }

    /**
     * Get shade position
     *
     * @return index of current selected shade
     */
    public int getShadePosition() {
        return shades.length - 1 - shadePosition;
    }

    private void setClosestColorPosition( int color) {
        double closestDistance = Double.MAX_VALUE;

        for (int i = 0; i < adapter.shades(0); i++) {
            int adapterColor = adapter.color(0, i);

            double distance = Math.sqrt(
                    Math.pow(Color.alpha(color) - Color.alpha(adapterColor), 2)
                            + Math.pow(Color.red(color) - Color.red(adapterColor), 2)
                            + Math.pow(Color.green(color) - Color.green(adapterColor), 2)
                            + Math.pow(Color.blue(color) - Color.blue(adapterColor), 2));

            if (distance < closestDistance) {
                closestDistance = distance;
                shadePosition = adapter.shades(0) - 1 - i;
            }
        }
    }

    private void updateColorAdapter() {
        if (lobsterPickerChain == LobsterPicker.EMPTY_CHAIN) {
            int size = adapter.shades(0);
            segmentLength = length / size;
            shades = new int[size];
            for (int i = 0; i < size; i++) {
                shades[size - 1 - i] = adapter.color(0, i);
            }

            updatePointer();
            chainDecorators();
        }
    }

    private void updateShade() {
        updatePointer();
        lobsterPickerChain.setShade(getShadePosition());
        lobsterPickerChain.setColor(this, shades[shadePosition]);
        chainDecorators();
    }

    private void updatePointer() {
        int color = shades[shadePosition];
        pointerPaint.setColor(shades[shadePosition]);
        pointerShadowPaint.setColor(Color.argb(0x59, Color.red(color), Color.green(color), Color.blue(color)));
    }

    private void chainDecorators() {
        for (ColorDecorator decorator : decorators) {
            decorator.onColorChanged(chain, shades[shadePosition]);
        }
    }

    private void chainDecorators(int alpha){
        for (ColorDecorator decorator : decorators) {
            shades[shadePosition] &= (0x00FFFFFF);
            shades[shadePosition] |= (alpha << 24);
            decorator.onColorChanged(chain, shades[shadePosition]);
        }
    }

    private int findShadePosition() {
        int position = (int) (((float) shades.length / length) * pointerPosition.x);
        if (position == shades.length) {
            position--;
        }
        return position;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.translate(pointerShadowRadius, getHeight() / 2);

        for (int i = 0; i < shades.length; i++) {
            paint.setColor(shades[i]);
            canvas.drawLine(segmentLength * i, 0, segmentLength * (i + 1), 0, paint);
        }

        canvas.drawCircle(pointerPosition.x, pointerPosition.y, pointerShadowRadius, pointerShadowPaint);
        canvas.drawCircle(pointerPosition.x, pointerPosition.y, pointerRadius, pointerPaint);
    }

    private ValueAnimator getMoveAnimation() {
        ValueAnimator animator = ValueAnimator.ofInt(pointerPosition.x,
                currentAnimationEnd = (segmentLength * shadePosition) + (segmentLength / 2));
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
        getParent().requestDisallowInterceptTouchEvent(true);

        float x = event.getX();
        int newShade;

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

                newShade = findShadePosition();
                if (newShade != shadePosition) {
                    shadePosition = newShade;
                    updateShade();
                }
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
                    newShade = findShadePosition();
                    if (newShade != shadePosition) {
                        shadePosition = newShade;
                        updateShade();
                    }
                    invalidate();
                }
                break;
            case MotionEvent.ACTION_UP:
                if (pointerPressed) {

                    pointerPosition.x = (int) x - pointerShadowRadius;
                    if (pointerPosition.x > length) {
                        pointerPosition.x = length;
                    } else if (pointerPosition.x < 0) {
                        pointerPosition.x = 0;
                    }

                    newShade = findShadePosition();
                    if (newShade != shadePosition) {
                        shadePosition = newShade;
                        updateShade();
                    }

                    AnimatorSet set = new AnimatorSet();
                    set.playTogether(getMoveAnimation(), getShrinkAnimation());
                    set.start();
                }

                pointerPressed = false;

                for (OnColorListener listener : listeners) {
                    listener.onColorSelected(chainedColor);
                }
                break;
        }
        return true;
    }
}
