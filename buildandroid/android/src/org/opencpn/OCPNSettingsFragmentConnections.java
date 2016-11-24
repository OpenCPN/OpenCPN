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

//import com.arieslabs.assetbridge.Assetbridge;

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

import org.kde.necessitas.ministro.IMinistro;
import org.kde.necessitas.ministro.IMinistroCallback;

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
import android.preference.PreferenceManager;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.CheckBoxPreference;
import android.preference.PreferenceScreen;
import android.content.SharedPreferences;

import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
//@ANDROID-11

public class OCPNSettingsFragmentConnections extends PreferenceFragment {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Log.i("OpenCPN", "SettingsFragment connections!");

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences_connections);

        boolean bPL2303 = false;
        boolean bdAISy = false;
        boolean bFT232R = false;
        boolean bFT231X = false;
        boolean bMCP000A = false;
        boolean bMCP0205 = false;

        // Retrieve initial arguments
        Bundle extras = getArguments();

        if (extras != null) {
            String serialString = extras.getString("DETECTEDSERIALPORTS_STRING");
            if(null != serialString){
                //Log.i("OpenCPN", "OCPNSettingsFragmentConnections.DETECTEDSERIALPORTS_STRING: " + serialString);

                if(serialString.contains("2303"))
                    bPL2303 = true;
                if(serialString.contains("dAISy"))
                    bdAISy = true;
                if(serialString.contains("FT232R"))
                    bFT232R = true;
                if(serialString.contains("FT231X"))
                    bFT231X = true;
                if(serialString.contains("MCP000A"))
                    bMCP000A = true;
                if(serialString.contains("MCP0205"))
                    bMCP0205 = true;

            }
        }


         SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getActivity());
         PreferenceScreen screen = getPreferenceScreen();

         CheckBoxPreference cPref = (CheckBoxPreference)findPreference("prefb_PL2303");
         if(null != cPref){
             if(!bPL2303){
                 screen.removePreference(cPref);
             }
         }

         cPref = (CheckBoxPreference)findPreference("prefb_dAISy");
         if(null != cPref){
             if(!bdAISy){
                 screen.removePreference(cPref);
             }
         }

         cPref = (CheckBoxPreference)findPreference("prefb_FT232R");
         if(null != cPref){
             if(!bFT232R){
                 screen.removePreference(cPref);
             }
         }

         cPref = (CheckBoxPreference)findPreference("prefb_FT231X");
         if(null != cPref){
             if(!bFT231X){
                 screen.removePreference(cPref);
             }
         }

         cPref = (CheckBoxPreference)findPreference("prefb_MCP000A");
         if(null != cPref){
             if(!bMCP000A){
                 screen.removePreference(cPref);
             }
         }

         cPref = (CheckBoxPreference)findPreference("prefb_MCP0205");
         if(null != cPref){
             if(!bMCP0205){
                 screen.removePreference(cPref);
             }
         }

    }
}
