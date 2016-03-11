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
import android.view.Menu;
import android.view.MenuItem;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;

import android.view.MotionEvent;
import android.view.View;
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
import org.opencpn.opencpn.R;

import android.widget.Toast;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;

import ar.com.daidalos.afiledialog.*;

public class OCPNGRIBFragmentFile extends PreferenceFragment {


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String dir = "";
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getActivity());
        if (preferences != null) {
            String file = preferences.getString("GRIB_dest_file", "?");
            if(file != "?"){
                File fp = new File(file);
                if(null != fp){
                    File parent = fp.getParentFile();
                    dir = parent.getAbsolutePath();
                }
            }
            else{
                dir = "/mnt/sdcard/Download/gribs";
            }

        }

        File fdir = new File(dir);
        if((null == fdir) || !fdir.exists() )
            dir = "/mnt/sdcard/Download/gribs";


        Log.i("GRIB", "OCPNGRIBFragmentFile:dir: " + dir);




        Intent intent = new Intent(getActivity(), FileChooserActivity.class);
        intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, dir);
        intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, false);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_FULL_PATH_IN_TITLE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_ONLY_SELECTABLE, true);
        startActivityForResult(intent, 0);

//        FileChooserDialog dialog = new FileChooserDialog(getActivity());
//        dialog.setCanCreateFiles(false);
//        dialog.show();

        // Load the preferences from an XML resource
///        addPreferencesFromResource(R.xml.preferences_grib_file);

        //m_frag = OCPNSettingsFragmentCharts.this;

        // Rerieve initial arguments
        //Bundle b = getArguments();
        //if(b != null)
            //mbs52 = b.getString("S52").equalsIgnoreCase("TRUE");

        //Preference s52Options = findPreference("pref_s52Options");
        //if(s52Options != null){
          //  s52Options.setEnabled(mbs52);
        //}

    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.i("GRIB", "OCPNGRIBFragmentFile:onActivityResult");
        // Check which request we're responding to
        if (requestCode == 0) {
            // Make sure the request was successful
            if (resultCode == FileChooser.RESULT_OK) {
                boolean fileCreated = false;
                String filePath = "";

                Bundle bundle = data.getExtras();
                if(bundle != null)
                {
                    if(bundle.containsKey(FileChooserActivity.OUTPUT_NEW_FILE_NAME)) {
                        fileCreated = true;
                        File folder = (File) bundle.get(FileChooserActivity.OUTPUT_FILE_OBJECT);
                        String name = bundle.getString(FileChooserActivity.OUTPUT_NEW_FILE_NAME);
                        filePath = folder.getAbsolutePath() + "/" + name;
                    } else {
                        fileCreated = false;
                        File file = (File) bundle.get(FileChooserActivity.OUTPUT_FILE_OBJECT);
                        filePath = file.getAbsolutePath();
                    }
                }

                String message = fileCreated? "File created" : "File opened";
                message += ": " + filePath;
                Log.i("DEBUGGER_TAG", message);

//                 OCPNSettingsFragmentCharts cfrag = OCPNSettingsFragmentCharts.getFragment();
//                 cfrag.updateChartDirListView();

                ((OCPNGRIBActivity)getActivity()).onFileSelected(filePath, this);

            }
        }
    }



    //public static OCPNSettingsFragmentCharts getFragment(){ return m_frag; }
    //private static OCPNSettingsFragmentCharts m_frag;



}
