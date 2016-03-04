/*
    Copyright (c) 2012-2013, BogDan Vatra <bogdan@kde.org>
    Contact: http://www.qt-project.org/legal

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
package org.opencpn;


import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageInfo;
import android.content.res.Configuration;
import android.content.res.Resources.Theme;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.view.accessibility.AccessibilityEvent;
import dalvik.system.DexClassLoader;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.preference.ListPreference;
import android.preference.PreferenceManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;

import android.content.SharedPreferences;

import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
//@ANDROID-11

public class OCPNGRIBFragmentDownload extends PreferenceFragment {

    public OnDownloadButtonSelectedListener mListener;
    public SharedPreferences.OnSharedPreferenceChangeListener m_listener;

    public ListPreference mGRIB_modelPreference;
    public ListPreference mGRIB_TimestepPreference;
    public ListPreference mGRIB_DaysPreference;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

     //   Log.i("DEBUGGER_TAG", "SettingsFragment display!");

        //OCPNSettingsActivity act1 = (OCPNSettingsActivity)getActivity();


        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.grib_download_settings);

        //  Set up inital values of "Summaries" fields
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getActivity());


        mGRIB_modelPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_model");
        mGRIB_modelPreference.setSummary(mGRIB_modelPreference.getEntry().toString());

        mGRIB_TimestepPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_timestep");
        String ts = "Set the time interval between forecasts";
        String pts = mGRIB_TimestepPreference.getEntry().toString();
        mGRIB_TimestepPreference.setSummary(ts + " ... " + pts);

        mGRIB_DaysPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_days");
        ts = "Set the number of days in GRIB";
        pts = mGRIB_DaysPreference.getEntry().toString();
        mGRIB_DaysPreference.setSummary(ts + " ... " + pts);


          // Set up a listener for key changes

         m_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
           public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {

             // listener implementation

             // Set new summary, when a preference value changes
             if (key.equals("GRIB_prefs_timestep")) {
                 String ts = "Set the time interval between forecasts";
                 String pts = mGRIB_TimestepPreference.getEntry().toString();
                 mGRIB_TimestepPreference.setSummary(ts + " ... " + pts);
             }

             if (key.equals("GRIB_prefs_days")) {
                 String ts = "Set the number of days in GRIB";
                 String pts = mGRIB_DaysPreference.getEntry().toString();
                 mGRIB_DaysPreference.setSummary(ts + " ... " + pts);
             }

             if (key.equals("GRIB_prefs_model")) {
                 mGRIB_modelPreference.setSummary(mGRIB_modelPreference.getEntry().toString());
             }

         }
         };

         getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listener);




//        DialogSliderPreference a = (DialogSliderPreference)getPreferenceScreen().findPreference("prefs_UIScaleFactor");
//        a.m_text.setText("User Interface Scale Factor");

//        DialogSliderPreference b = (DialogSliderPreference)getPreferenceScreen().findPreference("prefs_chartScaleFactor");
//        b.m_text.setText("Chart Display Scale Factor");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        LinearLayout v = (LinearLayout) super.onCreateView(inflater, container, savedInstanceState);

        Button btn = new Button(getActivity().getApplicationContext());
        btn.setText("Download GRIB");

        if (v != null) {
            v.addView(btn);
        }


        btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {


                mListener.onDownloadButtonClick();
            }
        });

        return v;
    }

    private void ShowTextDialog(final String message){
        AlertDialog.Builder builder1 = new AlertDialog.Builder(getActivity());
        builder1.setMessage(message);
        builder1.setCancelable(false);
        builder1.setNeutralButton("OK",
        new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
            dialog.cancel();
//            if(!m_licenseOK)
//                finish();
            }
        });
   /*
   builder1.setNegativeButton("No",
   new DialogInterface.OnClickListener() {
   public void onClick(DialogInterface dialog, int id) {
   dialog.cancel();
   }
   });
   */
        AlertDialog alert11 = builder1.create();
        alert11.show();
   }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);

        OCPNGRIBActivity act1 = (OCPNGRIBActivity)this.getActivity();

        Context ctx = getActivity();

        try {
            mListener = (OnDownloadButtonSelectedListener) ctx;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnDownloadButtonSelectedListener");
        }

        int yyp = 4;
    }


    // Container Activity must implement this interface
    public interface OnDownloadButtonSelectedListener {
        public void onDownloadButtonClick();
    }

}

