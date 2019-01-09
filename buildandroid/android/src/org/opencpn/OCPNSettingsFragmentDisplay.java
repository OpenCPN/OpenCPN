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
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.preference.ListPreference;
import android.preference.PreferenceManager;

import android.content.SharedPreferences;

import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.widget.Button;
import android.widget.TextView;
//@ANDROID-11

public class OCPNSettingsFragmentDisplay extends PreferenceFragment {

    public static final String KEY_NAVMODE_LIST_PREFERENCE = "prefs_navmode";
    private static OCPNSettingsFragmentDisplay m_frag;
    public ListPreference mNavmodeListPreference;

    public SharedPreferences.OnSharedPreferenceChangeListener m_listener;

    public static OCPNSettingsFragmentDisplay getFragment(){ return m_frag; }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        m_frag = OCPNSettingsFragmentDisplay.this;

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences_display_basic);

        // Get a reference to the NavMode preference
         mNavmodeListPreference = (ListPreference)getPreferenceScreen().findPreference("prefs_navmode");

         // Set up a listener for key changes

         m_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
           public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {

             // listener implementation

             // Set new summary, when a preference value changes
             if (key.equals(OCPNSettingsFragmentDisplay.KEY_NAVMODE_LIST_PREFERENCE)) {
                 mNavmodeListPreference.setSummary(mNavmodeListPreference.getEntry().toString());
             }
            }
         };

         getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listener);


         // Setup some initial values
         mNavmodeListPreference.setSummary(mNavmodeListPreference.getEntry().toString());

   }


    @Override
    public void onStop(){
        super.onStop();


    }


}

