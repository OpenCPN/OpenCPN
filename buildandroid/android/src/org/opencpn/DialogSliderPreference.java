package org.opencpn;

import android.content.Context;
import android.preference.DialogPreference;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;

import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.LayoutInflater;
import android.widget.SeekBar;
import android.widget.TextView;

import android.os.Bundle;

import org.opencpn.opencpn.R;


/**
 * Created by dsr on 6/18/15.
 */
public class DialogSliderPreference extends DialogPreference
{
    private SeekBar m_bar;
    private int m_val;
    public TextView m_text;

    public DialogSliderPreference(Context oContext, AttributeSet attrs)
    {
         super(oContext, attrs);
         setDialogLayoutResource(R.layout.dialog_slider_preference_layout);
         setPersistent(true);
    }

    public int persistValue(){
        if(m_bar != null){
            m_val = m_bar.getProgress();
            String aa;
            aa = String.format("%d", m_val);
            persistString(aa);
            Log.i("DEBUGGER_TAG", "persisting " + aa);

        }
        return m_val;
    }


    @Override
    protected void onDialogClosed (boolean positiveResult){

        if(positiveResult){
            if(m_bar != null){
                m_val = m_bar.getProgress();
                String aa;
                aa = String.format("%d", m_val);
                persistString(aa);

            }

        }
    }


    @Override
    public void onBindDialogView(View view){
        m_bar = (SeekBar)view.findViewById(R.id.dialogSeekbar);
        m_text = (TextView)view.findViewById(R.id.sliderText);

        super.onBindDialogView(view);
    }

    @Override
       protected  View onCreateView(ViewGroup parent) {

         View view  =  LayoutInflater.from(getContext()).inflate(R.layout.dialog_slider_preference_layout,  null );
         m_text = (TextView)view.findViewById(R.id.sliderText);

         // Stupid....
         if(m_text != null){
            if(getKey().equals("prefs_UIScaleFactor"))
                m_text.setText("User Interface Scale Factor");

            else if(getKey().equals("prefs_chartScaleFactor"))
                m_text.setText("Chart Display Scale Factor");
         }

         // Set inital value
         SharedPreferences prefs  =  getPreferenceManager().getSharedPreferences();
         String s  =  prefs.getString(getKey(), "13");        // initial value

         int myNum = 0;

         try {
             myNum = Integer.parseInt(s);
         } catch(NumberFormatException nfe) {
            System.out.println("Could not parse " + nfe);
         }


         m_bar = (SeekBar)view.findViewById(R.id.dialogSeekbar);
         m_bar.setProgress(myNum);

         return  view;
       }
}
