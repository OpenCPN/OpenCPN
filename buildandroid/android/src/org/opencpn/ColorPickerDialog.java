/*
 * <Copyright 2017 David S Register>
 *
 *  This file is part of aFileDialog.
 *
 *  aFileDialog is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  aFileDialog is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the code. If not, see <http://www.gnu.org/licenses/>.
 */

package org.opencpn.opencpn;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

import org.opencpn.opencpn.R;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.view.View;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Button;
import android.util.Log;

import com.larswerkman.lobsterpicker.LobsterPicker;
/**
 * A color picker implemented in a Dialog.
 */
public abstract class ColorPickerDialog extends Dialog  {

        // ----- Attributes ----- //

        private int initColor = 0;

        public Context mContext;
        public Activity mActivity;

        private Button mOK;
        private Button mCancel;
        public LobsterPicker mvlp;

        // ----- Constructors ----- //

        /**
         * Creates a dialog .
         *
         * @param context The current context.
         */
        public ColorPickerDialog(Context context) {
                this(context, 0);
        }

        public ColorPickerDialog(Context context, int initialColor) {
                // Call superclass constructor.
            super(context);

            mContext = context;
            mActivity = getOwnerActivity();

                // Set layout.
            this.setContentView(R.layout.colorpicker_dialog);

                // Maximize the dialog.
            WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
            lp.copyFrom(this.getWindow().getAttributes());
            lp.width = WindowManager.LayoutParams.FILL_PARENT;
            lp.height = WindowManager.LayoutParams.FILL_PARENT;
            this.getWindow().setAttributes(lp);

            View v = getLayoutInflater().inflate(R.layout.colorpicker_dialog, null);

            mvlp = (LobsterPicker) v.findViewById(R.id.lobsterpicker);

            mvlp.setColor(initialColor);

            mOK = (Button) v.findViewById(R.id.buttonSelectColorOK);
            mOK.setOnClickListener(new View.OnClickListener()
            {

              @Override
              public void onClick(View v)
              {
                  //Log.i("DEBUGGER_TAG", "ColorPickerDialog on OK button");

                  dismiss();
                  onOK( mvlp.getColor());
              }
            });


            mCancel = (Button) v.findViewById(R.id.buttonColorCancel);
            mCancel.setOnClickListener(new View.OnClickListener()
            {

              @Override
              public void onClick(View v)
              {
                  //Log.i("DEBUGGER_TAG", "ColorPickerDialog on Cancel button");


                  dismiss();
                  onCancel();
              }
            });

            setCancelable(false);
            setCanceledOnTouchOutside(false);

            this.setContentView(v);

        }

        public abstract void onOK( int selectedColor);

        public abstract void onCancel();


}
