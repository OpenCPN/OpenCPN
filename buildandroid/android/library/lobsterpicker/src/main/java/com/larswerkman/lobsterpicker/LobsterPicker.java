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
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.RectF;
import android.graphics.Region;
//import android.support.annotation.ColorInt;
//import android.support.annotation.NonNull;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.MotionEvent;
import android.view.View;

import com.larswerkman.lobsterpicker.adapters.BitmapColorAdapter;

import java.util.ArrayList;
import java.util.List;

/**
 * Select colors from a range of specified color, which can be set using an {@link ColorAdapter}.
 *
 * <p>
 *     Use {@link #getColor()} to retrieve the selected color
 *     Use {@link #setColorAdapter(ColorAdapter)} to set a custom color adapter
 *     Use {@link #addDecorator(ColorDecorator)} to add an decorator
 * </p>
 */
public class LobsterPicker extends View {

    /**
     * Chain back to the parent
     */
    public interface Chain {
        /**
         * Used to update the current color.
         *
         * @param caller {@link ColorDecorator} current decorator
         * @param color  to update to
         */
        void setColor(ColorDecorator caller,  int color);

        /**
         * Used to update the current shade used.
         *
         * @param position of the next shade
         */
        void setShade(int position);

        /**
         * Returns the current used {@link ColorAdapter} from the parent
         *
         * @return current {@link ColorAdapter}
         */
        ColorAdapter getAdapter();

        /**
         * Get the current position of the used color of the {@link ColorAdapter}
         *
         * @return current position
         */
        int getAdapterPosition();

        /**
         * Get the current position of the used shade of the {@link ColorAdapter}
         *
         * @return current position
         */
        int getShadePosition();
    }

    /**
     * Unimplemented {@link Chain} to prevent nullity's
     */
    public static final Chain EMPTY_CHAIN = new Chain() {
        @Override
        public void setColor(ColorDecorator callback,  int color) {

        }

        @Override
        public void setShade(int position) {

        }

        @Override
        public ColorAdapter getAdapter() {
            return null;
        }

        @Override
        public int getAdapterPosition() {
            return 0;
        }

        @Override
        public int getShadePosition() {
            return 0;
        }
    };

    /**
     * Updates color by invoking each {@link ColorDecorator} in the order they are added.
     */
    private Chain chain = new Chain() {
        @Override
        public void setColor(ColorDecorator callback,  int color) {
            int index = decorators.indexOf(callback);
            if (index < (decorators.size() - 1)) {
                decorators.get(index + 1).onColorChanged(this, color);
            } else {
                if(chainedColor != color) {
                    for (OnColorListener listener : listeners) {
                        listener.onColorChanged(color);
                    }
                }

                //Set the color.
                chainedColor = color;
                invalidate();
            }
        }

        @Override
        public void setShade(int position) {
            shadePosition = position;
            color = adapter.color(colorPosition, shadePosition);
            pointerPaint.setColor(color);
        }

        @Override
        public ColorAdapter getAdapter() {
            return adapter;
        }

        @Override
        public int getAdapterPosition() {
            return colorPosition;
        }

        @Override
        public int getShadePosition() {
            return shadePosition;
        }
    };

    private ColorDecorator updateDecorator = new ColorDecorator() {
        @Override
        public void onColorChanged(Chain chain,  int color) {
            chain.setColor(this, color);
        }
    };

    private List<ColorDecorator> decorators;
    private List<OnColorListener> listeners;
    private ColorAdapter adapter;

    private int radius;
    private int pointerRadius;
    private int historyRadius;

    private float slopX;
    private float slopY;
    private float translationOffset;

    private boolean pointerPressed = false;
    private boolean wheelPressed = false;

    private PointF pointerPosition = new PointF();
    private RectF wheelRectangle = new RectF();
    private RectF historyRectangle = new RectF();

    private int colorPosition;
    private int shadePosition;

    private Paint wheelPaint;
    private Paint pointerPaint;
    private Paint historyPaint;

    private Bitmap pointerShadow;

    private Path largeRadiusPath;
    private Path smallRadiusPath;

    private int color;
    private int chainedColor;
    private int historicColor;

    private boolean colorHistoryEnabled;

    public LobsterPicker(Context context) {
        super(context);
        init(context, null, 0);
    }

    public LobsterPicker(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs, 0);
    }

    public LobsterPicker(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context, attrs, defStyle);
    }

    private void init(Context context, AttributeSet attrs, int defStyle) {
        final TypedArray a = context.obtainStyledAttributes(attrs,
                R.styleable.LobsterPicker, defStyle, 0);
        final Resources b = context.getResources();

        int thickness = a.getDimensionPixelSize(
                R.styleable.LobsterPicker_color_wheel_thickness,
                b.getDimensionPixelSize(R.dimen.color_wheel_thickness));
        radius = a.getDimensionPixelSize(
                R.styleable.LobsterPicker_color_wheel_radius,
                b.getDimensionPixelSize(R.dimen.color_wheel_radius));
        pointerRadius = a.getDimensionPixelSize(
                R.styleable.LobsterPicker_color_wheel_pointer_radius,
                b.getDimensionPixelSize(R.dimen.color_wheel_pointer_radius));
        historyRadius = a.getDimensionPixelSize(
                R.styleable.LobsterPicker_color_history_radius,
                b.getDimensionPixelSize(R.dimen.color_history_radius));
        colorHistoryEnabled = a.getBoolean(
                R.styleable.LobsterPicker_color_history_enabled,
                false);
        int pointerShadowRadius = a.getDimensionPixelSize(
                R.styleable.LobsterPicker_color_wheel_pointer_shadow_radius,
                b.getDimensionPixelSize(R.dimen.color_wheel_pointer_shadow_radius));
        int pointerShadowColor = a.getColor(R.styleable.LobsterPicker_color_wheel_pointer_shadow,
                b.getColor(R.color.lobsterpicker_pointer_shadow));
        int schemeRes = a.getResourceId(R.styleable.LobsterPicker_color_wheel_scheme,
                R.drawable.default_pallete);

        a.recycle();

        decorators = new ArrayList<ColorDecorator>();
        listeners = new ArrayList<OnColorListener>();

        decorators.add(updateDecorator);

        wheelPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        wheelPaint.setStyle(Paint.Style.STROKE);
        wheelPaint.setStrokeWidth(thickness);

        pointerPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        pointerPaint.setStyle(Paint.Style.FILL);

        historyPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        historyPaint.setStyle(Paint.Style.FILL);

        Paint pointerShadowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        pointerShadowPaint.setStyle(Paint.Style.FILL);
        pointerShadowPaint.setColor(pointerShadowColor);

        //Predraw the pointers shadow
        pointerShadow = Bitmap.createBitmap(pointerShadowRadius * 2, pointerShadowRadius * 2, Bitmap.Config.ARGB_8888);
        Canvas pointerShadowCanvas = new Canvas(pointerShadow);
        pointerShadowCanvas.drawCircle(pointerShadowRadius, pointerShadowRadius, pointerShadowRadius, pointerShadowPaint);

        //Outer wheel ring
        largeRadiusPath = new Path();
        largeRadiusPath.addCircle(0, 0, radius + thickness / 2, Path.Direction.CW);
        largeRadiusPath.close();

        //inner wheel ring
        smallRadiusPath = new Path();
        smallRadiusPath.addCircle(0, 0, radius - thickness / 2, Path.Direction.CW);
        smallRadiusPath.close();

        //Default color adapter
        adapter = new BitmapColorAdapter(context, schemeRes);
        updateColorAdapter();

        invalidate();
    }

    /**
     * Set a custom {@link ColorAdapter}
     *
     * @param adapter to retrieve colors from
     */
    public void setColorAdapter( ColorAdapter adapter) {
        float oldAngle = getAngle(colorPosition);
        this.adapter = adapter;

        updateColorAdapter();
        getMoveAnimation(oldAngle, getAngle(colorPosition)).start();
    }

    /**
     * Returns current used adapter
     *
     * @return {@link ColorAdapter} instance
     */
    public ColorAdapter getColorAdapter() {
        return adapter;
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
     *
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
        if(!listeners.contains(listener)){
            listeners.add(listener);
        }
    }

    /**
     * Remove a {@link OnColorListener}
     *
     * @param listener to be removed
     *
     * @return true if it could remove the listener
     */
    public boolean removeOnColorListener( OnColorListener listener){
        return listeners.remove(listener);
    }

    /**
     * Enable the color history view, showing the current selected color and a historic color,
     * which can be set with {@link #setHistory(int)}.
     *
     * By default the color history is disabled
     *
     * @param enabled a boolean indicating if it should be enabled
     */
    public void setColorHistoryEnabled(boolean enabled) {
        colorHistoryEnabled = enabled;
        invalidate();
    }

    /**
     * Checks if the color history is enabled and drawn.
     *
     * @return true if color feedback is enabled
     */
    public boolean isColorFeedbackEnabled() {
        return colorHistoryEnabled;
    }

    public void setColor( int color){
        float oldAngle = getAngle(colorPosition);

        setClosestColorPosition(color);
        setPointerPosition(getAngle(colorPosition));

        this.color = chainedColor = adapter.color(colorPosition, shadePosition);
        pointerPaint.setColor(this.color);
        historyPaint.setColor(this.color);

        chainDecorators(Color.alpha(color));

        getMoveAnimation(oldAngle, getAngle(colorPosition)).start();
    }

    /**
     * Returns the selected color.
     *
     * @return an ARGB color
     */
    public  int getColor() {
        return chainedColor;
    }

    /**
     * Set the historic color
     *
     * @param color ARGB color
     */
    public void setHistory( int color) {
        historicColor = color;
        invalidate();
    }

    /**
     * Get the current historic color.
     *
     * @return ARGB color
     */
    public  int getHistory() {
        return historicColor;
    }

    /**
     * Set the current position of the selected color inside of the {@link ColorAdapter}
     *
     * @param position > 0 and < MAX number of colors inside of the {@link ColorAdapter}
     */
    public void setColorPosition(int position) {
        float oldAngle = getAngle(colorPosition);

        colorPosition = position;
        setPointerPosition(getAngle(colorPosition));
        color = chainedColor = adapter.color(colorPosition, shadePosition);
        pointerPaint.setColor(color);
        historyPaint.setColor(color);
        chainDecorators();

        getMoveAnimation(oldAngle, getAngle(colorPosition)).start();
    }

    /**
     * Get the selected position of the color inside of the {@link ColorAdapter}
     *
     * @return index of {@link ColorAdapter}
     */
    public int getColorPosition(){
        return colorPosition;
    }

    public void setShadePosition(int position) {
        shadePosition = position;
        color = chainedColor = adapter.color(colorPosition, shadePosition);
        pointerPaint.setColor(color);
        historyPaint.setColor(color);
        chainDecorators();
    }

    public int getShadePosition(){
        return shadePosition;
    }

    private void setClosestColorPosition( int color){
        double closestDistance = Double.MAX_VALUE;

        for(int i = 0; i < adapter.size(); i++){
            for(int j = 0; j < adapter.shades(i); j++){
                int adapterColor = adapter.color(i, j);

                double distance = Math.sqrt(
                        Math.pow(Color.alpha(color) - Color.alpha(adapterColor), 2)
                        + Math.pow(Color.red(color) - Color.red(adapterColor), 2)
                        + Math.pow(Color.green(color) - Color.green(adapterColor), 2)
                        + Math.pow(Color.blue(color) - Color.blue(adapterColor), 2));

                if(distance < closestDistance){
                    closestDistance = distance;

                    colorPosition = i;
                    shadePosition = j;
                }
            }
        }
    }

    private void updateColorAdapter() {
        if(colorPosition >= adapter.size()){
            colorPosition = adapter.size() - 1;
        }
        if(shadePosition >= adapter.shades(colorPosition)){
            shadePosition = adapter.shades(colorPosition) - 1;
        }
        setPointerPosition(getAngle(colorPosition));
        color = historicColor = chainedColor = adapter.color(colorPosition, shadePosition);
        pointerPaint.setColor(color);
        chainDecorators();
    }

    private void chainDecorators() {
        for (ColorDecorator decorator : decorators) {
            decorator.onColorChanged(chain, color);
        }
    }

    private void chainDecorators(int alpha){
        for (ColorDecorator decorator : decorators) {
            color &= (0x00FFFFFF);
            color |= (alpha << 24);
            decorator.onColorChanged(chain, color);
        }
    }

    private void setPointerPosition(float radians) {
        float x = (float) (radius * Math.cos(radians));
        float y = (float) (radius * Math.sin(radians));
        pointerPosition.set(x, y);
    }

    private int getColorPosition(float radians) {
        int degrees = (int) Math.toDegrees(radians) + 90;
        degrees = degrees > 0 ? degrees : degrees + 360;
        degrees = degrees == 360 ? 359 : degrees;
        return (int) (adapter.size() / 360.0f * degrees);
    }

    private float getAngle(int position) {
        int nbOfSegments = adapter.size();
        int segmentWidth = 360 / nbOfSegments;
        int degrees = (position * segmentWidth) + (segmentWidth / 2) - 90;
        if (degrees > 180) {
            degrees -= 360;
        }

        return (float) Math.toRadians(degrees);
    }

    private ValueAnimator getMoveAnimation(float oldAngle, float newAngle) {
        ValueAnimator animator = ValueAnimator.ofFloat(oldAngle, newAngle);
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                setPointerPosition((Float) animation.getAnimatedValue());
                invalidate();
            }
        });
        return animator;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final int intrinsicSize = 2 * (radius + pointerRadius);

        int widthMode = MeasureSpec.getMode(widthMeasureSpec);
        int widthSize = MeasureSpec.getSize(widthMeasureSpec);
        int heightMode = MeasureSpec.getMode(heightMeasureSpec);
        int heightSize = MeasureSpec.getSize(heightMeasureSpec);

        int width;
        int height;

        if (widthMode == MeasureSpec.EXACTLY) {
            width = widthSize;
        } else if (widthMode == MeasureSpec.AT_MOST) {
            width = Math.min(intrinsicSize, widthSize);
        } else {
            width = intrinsicSize;
        }

        if (heightMode == MeasureSpec.EXACTLY) {
            height = heightSize;
        } else if (heightMode == MeasureSpec.AT_MOST) {
            height = Math.min(intrinsicSize, heightSize);
        } else {
            height = intrinsicSize;
        }

        int min = Math.min(width, height);
        setMeasuredDimension(min, min);
        translationOffset = min * 0.5f;

        wheelRectangle.set(
                -radius, -radius,
                radius, radius
        );
        historyRectangle.set(
                -historyRadius, -historyRadius,
                historyRadius, historyRadius
        );
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.translate(translationOffset, translationOffset);

        int nbOfSegments = adapter.size();
        int segmentWidth = 360 / nbOfSegments;
        for (int i = 0; i < nbOfSegments; i++) {
            wheelPaint.setColor(adapter.color(i, shadePosition));

            //Add an extra degree to the angle so the arc doesn't have a wedge in between
            //the start should sweep one back and the last one should not have an extra degree
            canvas.drawArc(
                    wheelRectangle,
                    (segmentWidth * i - 90) - (i == 0 ? 1 : 0),
                    segmentWidth + (i < nbOfSegments - 1 ? 1 : 0),
                    false, wheelPaint
            );
        }

        if (colorHistoryEnabled) {
            historyPaint.setColor(historicColor);
            canvas.drawArc(historyRectangle, -90, 180, true, historyPaint);

            historyPaint.setColor(chainedColor);
            canvas.drawArc(historyRectangle, 90, 180, true, historyPaint);
        }

        canvas.save();

        canvas.clipPath(largeRadiusPath);
        canvas.clipPath(smallRadiusPath, Region.Op.DIFFERENCE);

        canvas.drawBitmap(pointerShadow,
                pointerPosition.x - (pointerShadow.getWidth() / 2),
                pointerPosition.y - (pointerShadow.getHeight() / 2), null);

        canvas.restore();

        canvas.drawCircle(
                pointerPosition.x, pointerPosition.y,
                pointerRadius, pointerPaint
        );

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        getParent().requestDisallowInterceptTouchEvent(true);

        // Convert coordinates to our internal coordinate system
        float x = event.getX() - translationOffset;
        float y = event.getY() - translationOffset;


        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                // Check whether the user pressed on the pointer.
                if (x >= (pointerPosition.x - pointerRadius)
                        && x <= (pointerPosition.x + pointerRadius)
                        && y >= (pointerPosition.y - pointerRadius)
                        && y <= (pointerPosition.y + pointerRadius)) {

                    slopX = x - pointerPosition.x;
                    slopY = y - pointerPosition.y;
                    pointerPressed = true;
                }
                // Check whether the user pressed anywhere on the wheel.
                else if (Math.sqrt(x * x + y * y) <= radius + pointerRadius
                        && Math.sqrt(x * x + y * y) >= radius - pointerRadius) {
                    wheelPressed = true;

                    float angle = (float) Math.atan2(y - slopY, x - slopX);
                    setPointerPosition(angle);
                    colorPosition = getColorPosition(angle);
                    color = adapter.color(colorPosition, shadePosition);
                    pointerPaint.setColor(color);
                    historyPaint.setColor(color);
                    chainDecorators();

                    invalidate();
                } else {
                    getParent().requestDisallowInterceptTouchEvent(false);
                    return false;
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if (pointerPressed || wheelPressed) {
                    float angle = (float) Math.atan2(y - slopY, x - slopX);

                    setPointerPosition(angle);
                    colorPosition = getColorPosition(angle);
                    color = adapter.color(colorPosition, shadePosition);
                    pointerPaint.setColor(color);
                    historyPaint.setColor(color);
                    chainDecorators();

                    invalidate();
                }
                // If user did not press pointer, report event not handled
                else {
                    getParent().requestDisallowInterceptTouchEvent(false);
                    return false;
                }
                break;
            case MotionEvent.ACTION_UP:
                float angle = (float) Math.atan2(y - slopY, x - slopX);

                if (wheelPressed) {
                    setPointerPosition(angle);
                    colorPosition = getColorPosition(angle);
                    color = adapter.color(colorPosition, shadePosition);
                    pointerPaint.setColor(color);
                    historyPaint.setColor(color);
                    chainDecorators();
                }

                wheelPressed = false;
                pointerPressed = false;

                for(OnColorListener listener : listeners) {
                    listener.onColorSelected(chainedColor);
                }

                getMoveAnimation(angle, getAngle(colorPosition)).start();
                break;
        }
        return true;
    }
}
