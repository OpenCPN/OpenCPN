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
import java.util.StringTokenizer;

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
import android.widget.Toast;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;



import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
//@ANDROID-11



public class OCPNSettingsActivity extends PreferenceActivity
{
    @Override
    public void onBuildHeaders(List<Header> target) {
        loadHeadersFromResource(R.xml.ocpn_preference_headers, target);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            String settings = extras.getString("SETTINGS_STRING");

            Log.i("DEBUGGER_TAG", "OCPNSettingsActivity");
            Log.i("DEBUGGER_TAG", settings);

            m_settings = settings;
            m_newSettings = new String();

            //  Create the various settings elements
            m_chartDirList = new ArrayList<String>();

            StringTokenizer tkz = new StringTokenizer(settings, ";");

            while(tkz.hasMoreTokens()){
                String tk = tkz.nextToken();
                Log.i("DEBUGGER_TAG", tk);

                if( tk.startsWith("ChartDir") ){
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String dir = tk.substring(mark+1);
                        Log.i("DEBUGGER_TAG", dir);
                        m_chartDirList.add(dir);
                    }
                }

            }

            // Stuff the application shared preferences data from the settings string
            SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
            SharedPreferences.Editor editor = preferences.edit();

            StringTokenizer tkzp = new StringTokenizer(settings, ";");

            while(tkzp.hasMoreTokens()){
                String tk = tkzp.nextToken();

                if( tk.startsWith("prefb_") ){       // simple Boolean
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String key = tk.substring(0, mark);
                        Log.i("DEBUGGER_TAG", key);
                        String value = tk.substring(mark+1);
                        if(value.equals("1"))
                            editor.putBoolean(key, true);
                        else
                            editor.putBoolean(key, false);
                    }
                }

                else if( tk.startsWith("prefs_") ){       // simple string
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String key = tk.substring(0, mark);
                        String value = tk.substring(mark+1);
                        editor.putString(key, value);
                    }
                }

            }

           editor.commit();
        }

    }

    public void addDirectory(View vw)
    {
        Toast.makeText(this, "OCPNsettingsactivity Add Dir ", Toast.LENGTH_SHORT).show();

        Intent intent = new Intent(OCPNSettingsActivity.this, org.opencpn.FileChooser.class);
        startActivityForResult(intent, 0);
   }

   @Override
   protected void onActivityResult(int requestCode, int resultCode, Intent data) {
       // Check which request we're responding to
       if (requestCode == 0) {
           // Make sure the request was successful
           if (resultCode == RESULT_OK) {
               Log.i("DEBUGGER_TAG", "onActivityResult");
               String dirAdded = data.getStringExtra("SelectedDir");
               Log.i("DEBUGGER_TAG", dirAdded);
               m_chartDirList.add(dirAdded);

 //              org.opencpn.OCPNSettingsFragmentCharts cFrag = getFragmentManager().findFragmentById(R.id.FragmentCharts);
//               PreferenceFragment  Frag = getFragmentManager().findFragmentById(R.id.FragmentCharts);
//               ((org.opencpn.OCPNSettingsFragmentCharts)Frag).updateChartDirListView();

                OCPNSettingsFragmentCharts cfrag = OCPNSettingsFragmentCharts.getFragment();
                cfrag.updateChartDirListView();

           }
       }
   }

   @Override
    public void onResume() {
        super.onResume();
    }



   @Override
   protected void onPause()
   {
       super.onPause();

       Log.i("DEBUGGER_TAG", "onPause");

       createSettingsString();

       Bundle b = new Bundle();
       b.putString("SettingsString", m_newSettings);
//       Intent i = getIntent(); //gets the intent that called this intent
       Intent i = new Intent();
       i.putExtras(b);
       setResult(RESULT_OK, i);

   }

   @Override
   public void finish() {
       Log.i("DEBUGGER_TAG", "finish");

       createSettingsString();

       Bundle b = new Bundle();
       b.putString("SettingsString", m_newSettings);
       Intent i = new Intent();
       i.putExtras(b);
       setResult(RESULT_OK, i);

       super.finish();
   }

   private void createSettingsString()
    {
        m_newSettings = new String();


        // Record the chart dir list contents
        for(int i=0 ; i < m_chartDirList.size() ; ++i){
            m_newSettings = m_newSettings.concat("ChartDir:");
            String dir = m_chartDirList.get(i);
            Log.i("DEBUGGER_TAG", dir);
            m_newSettings = m_newSettings.concat(dir);
            m_newSettings = m_newSettings.concat(";");
            Log.i("DEBUGGER_TAG", m_newSettings);

        }

        // Get the android persisted values, one-by-one
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        //  Simple Booleans
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_lookahead", preferences.getBoolean("prefb_lookahead", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_quilt", preferences.getBoolean("prefb_quilt", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_preservescale", preferences.getBoolean("prefb_preservescale", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_smoothzp", preferences.getBoolean("prefb_smoothzp", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showgrid", preferences.getBoolean("prefb_showgrid", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showoutlines", preferences.getBoolean("prefb_showoutlines", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showdepthunits", preferences.getBoolean("prefb_showdepthunits", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showskewnu", preferences.getBoolean("prefb_showskewnu", false)));

        // Some other assorted string values
        m_newSettings = m_newSettings.concat(appendStringSetting("prefs_navmode", preferences.getString("prefs_navmode", "?")));

        Log.i("DEBUGGER_TAG", m_newSettings);

    }

    private String appendBoolSetting(String key, Boolean value)
    {
        String ret = new String(key);
        if(value)
            ret = ret.concat(":1;");
        else
            ret = ret.concat(":0;");

        return ret;
    }

    private String appendStringSetting(String key, String value)
    {
        String ret = new String(key);
        ret = ret.concat(":");
        ret = ret.concat(value);
        ret = ret.concat(";");

        return ret;
    }


        // info at
        //http://stackoverflow.com/questions/19973034/isvalidfragment-android-api-19
    //    @Override
    //    protected boolean isValidFragment(String fragmentName) {
    //      return StockPreferenceFragment.class.getName().equals(fragmentName);
    //    }

    //    @TargetApi(Build.VERSION_CODES.HONEYCOMB)

        @Override
        protected boolean isValidFragment (String fragmentName) {

             return true; //"com.fullpackage.MyPreferenceFragment".equals(fragmentName);

        }





    public static String getSettingsString(){ return m_settings;}
    public static ArrayList<String> getChartDirList(){ return m_chartDirList;}

    private static String m_settings;
    private String m_newSettings;
    private static ArrayList<String> m_chartDirList;
}





