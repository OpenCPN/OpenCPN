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
import android.os.Environment;

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
import android.preference.PreferenceScreen;
import android.preference.PreferenceGroup;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.preference.ListPreference;
import android.preference.PreferenceManager;
import android.preference.PreferenceCategory;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import org.json.JSONException;
import org.json.JSONObject;
import android.text.format.Time;
import ar.com.daidalos.afiledialog.*;

import static java.util.TimeZone.getTimeZone;

import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
//@ANDROID-11


public class OCPNGRIBDownloadPrefActivity extends PreferenceActivity {

    //public OnDownloadButtonSelectedListener mListener;
    public SharedPreferences.OnSharedPreferenceChangeListener m_listener;

    public ListPreference mGRIB_modelPreference;
    public ListPreference mGRIB_TimestepPreference;
    public ListPreference mGRIB_DaysPreference;

    public CheckBoxPreference mGRIB_WavesPreference;

    public PreferenceCategory prefCatSea;
    public PreferenceCategory prefCatWaves;
    public PreferenceCategory prefCatGeneral;


    public JSONObject  m_grib_PrefsJSON;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

     //   Log.i("DEBUGGER_TAG", "SettingsFragment display!");

        //OCPNSettingsActivity act1 = (OCPNSettingsActivity)getActivity();

        setContentView(R.layout.grib_download_activity_layout);

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.grib_download_settings);


        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        String settings = "";

        if (preferences != null) {
            settings = preferences.getString("GRIB_PREFS_JSON", "{}");

            try {
                m_grib_PrefsJSON  = new JSONObject( settings );
            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }



        //  Set up inital values of "Summaries" fields

        mGRIB_modelPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_model");
        if(null != mGRIB_modelPreference)
            mGRIB_modelPreference.setSummary(mGRIB_modelPreference.getEntry().toString());

        mGRIB_TimestepPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_timestep");
        if(null != mGRIB_TimestepPreference){
            String ts = "Set the time interval between forecasts";
            String pts = mGRIB_TimestepPreference.getEntry().toString();
            mGRIB_TimestepPreference.setSummary(ts + " ... " + pts);
        }

        mGRIB_DaysPreference = (ListPreference)getPreferenceScreen().findPreference("GRIB_prefs_days");
        if(null != mGRIB_DaysPreference){
            String ts = "Set the number of days in forecast";
            String pts = mGRIB_DaysPreference.getEntry().toString();
            mGRIB_DaysPreference.setSummary(ts + " ... " + pts);
        }


        PreferenceScreen screen = getPreferenceScreen();
        prefCatSea= (PreferenceCategory)findPreference("GRIB_prefcat_sea");
        prefCatWaves= (PreferenceCategory)screen.findPreference("GRIB_prefcat_waves");
        prefCatGeneral= (PreferenceCategory)screen.findPreference("GRIB_prefcat_general");


        if(null != mGRIB_modelPreference){
            String model = preferences.getString("GRIB_prefs_model", "??");
            if(model.contains("GFS")){
                screen.removePreference(prefCatSea);
                screen.removePreference(prefCatWaves);
            }
            else if(model.contains("WW3")){
                screen.removePreference(prefCatSea);
                screen.removePreference(prefCatGeneral);
            }
            else if(model.contains("RTOFS")){
                screen.removePreference(prefCatWaves);
                screen.removePreference(prefCatGeneral);
            }
        }









          // Set up a listener for key changes

         m_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
           public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {

             // listener implementation

             // Set new summary, when a preference value changes
             if (key.equals("GRIB_prefs_timestep")) {
                 if(null != mGRIB_TimestepPreference){
                     String ts = "Set the time interval between forecasts";
                     String pts = mGRIB_TimestepPreference.getEntry().toString();
                     mGRIB_TimestepPreference.setSummary(ts + " ... " + pts);
                 }
              }

             if (key.equals("GRIB_prefs_days")) {
                 if(null != mGRIB_DaysPreference){
                     String ts = "Set the number of days in forecast";
                     String pts = mGRIB_DaysPreference.getEntry().toString();
                     mGRIB_DaysPreference.setSummary(ts + " ... " + pts);
                 }
             }

             if (key.equals("GRIB_prefs_model")) {
                 if(null != mGRIB_modelPreference){
                     mGRIB_modelPreference.setSummary(mGRIB_modelPreference.getEntry().toString());

                     PreferenceScreen screen = getPreferenceScreen();
                     String model = mGRIB_modelPreference.getEntry().toString();
                     if(model.contains("GFS")){
                         screen.addPreference(prefCatGeneral);
                         screen.removePreference(prefCatSea);
                         screen.removePreference(prefCatWaves);
                     }
                     else if(model.contains("WW3")){
                         screen.addPreference(prefCatWaves);
                         screen.removePreference(prefCatSea);
                         screen.removePreference(prefCatGeneral);
                     }
                     else if(model.contains("RTOFS")){
                         screen.addPreference(prefCatSea);
                         screen.removePreference(prefCatWaves);
                         screen.removePreference(prefCatGeneral);
                     }
                 }
             }

         }
         };

         getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listener);



         Button button_download = (Button)findViewById(R.id.DownloadPrefButton);
         button_download.setOnClickListener(new View.OnClickListener() {
           @Override
           public void onClick(View v) {
               Log.i("GRIB", "DownloadPrefActivity button_download");

               onDownloadButtonClick();
           }
         });



    }


    private PreferenceGroup getParent(Preference preference)
    {
        return getParent(getPreferenceScreen(), preference);
    }

    private PreferenceGroup getParent(PreferenceGroup root, Preference preference)
    {
        for (int i = 0; i < root.getPreferenceCount(); i++)
        {
            Preference p = root.getPreference(i);
            if (p == preference)
                return root;
            if (PreferenceGroup.class.isInstance(p))
            {
                PreferenceGroup parent = getParent((PreferenceGroup)p, preference);
                if (parent != null)
                    return parent;
            }
        }
        return null;
    }



    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.i("GRIB", "onActivityResult");

        // Check which request we're responding to
        if (requestCode == 0xf3ec) {
            Log.i("GRIB", "onActivityResult from Download");

            // Make sure the request was successful
            if (resultCode == RESULT_OK) {
                ShowTextDialog("GRIB file downloaded OK");
            }
            else if(resultCode == DownloadFile.ERROR_NO_INTERNET){
                ShowTextDialog("No Internet available\nPlease check your device's Mobile Data or WiFi settings.");
            }
            else if(resultCode == DownloadFile.ERROR_NO_CONNECTION){
                ShowTextDialog("Connection Failed");
            }
            else {
                ShowTextDialog("Download Failed\n  Please check logs.");
            }
        }




    }


    @Override
    public void finish() {
        Log.i("DEBUGGER_TAG", "GRIB Download Activity finish");

        String json = "{}";
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        try {
            m_grib_PrefsJSON.put("model", preferences.getString("GRIB_prefs_model", "?"));
            m_grib_PrefsJSON.put("days", preferences.getString("GRIB_prefs_days", "?"));
            m_grib_PrefsJSON.put("time_step", preferences.getString("GRIB_prefs_timestep", "?"));
            m_grib_PrefsJSON.put("grib_file", preferences.getString("GRIB_dest_file", "?"));

            json = m_grib_PrefsJSON.toString(2);

        } catch (JSONException e) {
            throw new RuntimeException(e);
        }


        SharedPreferences.Editor editor = preferences.edit();
        editor.putString("GRIB_PREFS_JSON", json);
        editor.apply();

        Bundle b = new Bundle();
        b.putString("GRIB_JSON", json);
        Intent i = new Intent();
        i.putExtras(b);
        setResult(RESULT_OK, i);

        super.finish();
    }





    private String downloadGFSDirect(){


        //  Get some parameters from the Preferences.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        String model = preferences.getString("GRIB_prefs_model", "GFS100");
        int time_step = Integer.parseInt(preferences.getString("GRIB_prefs_timestep", "3"));
        int days = Integer.parseInt(preferences.getString("GRIB_prefs_days", "2"));
        Boolean bWind = preferences.getBoolean("GRIB_prefb_wind", true);
        Boolean bPressure = preferences.getBoolean("GRIB_prefb_pressure", true);

        int lat_min = 0;
        int lat_max = 1;
        int lon_min = 0;
        int lon_max = 1;
        String serverBase = "";

        //  Get some parameters from the JSON
        if(null != m_grib_PrefsJSON) {

            try {
                if (m_grib_PrefsJSON.has("latMin")) {
                    lat_min = m_grib_PrefsJSON.getInt("latMin");
                }
                if (m_grib_PrefsJSON.has("latMax")) {
                    lat_max = m_grib_PrefsJSON.getInt("latMax");
                }
                if (m_grib_PrefsJSON.has("lonMin")) {
                    lon_min = m_grib_PrefsJSON.getInt("lonMin");
                }
                if (m_grib_PrefsJSON.has("lonMax")) {
                    lon_max = m_grib_PrefsJSON.getInt("lonMax");
                }

            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }

        Time tm = new Time(Time.getCurrentTimezone());
        tm.setToNow();
        tm.switchTimezone("UTC");
        String startTime = tm.format("%Y%m%d");
        int tHours = tm.hour;
        int hour6 = (tHours / 6) * 6;

        //  GFS forcasts are late, sometimes, and UTC0000 forecast is not present yet
        //  So get the 1800 for previous day.
        if(hour6 == 0){
            Time tp = new Time(Time.getCurrentTimezone());
            tp.setToNow();
            tp.switchTimezone("UTC");
            if(tm.monthDay > 1)
                tp.set(tm.monthDay-1, tm.month, tm.year);
            else
                tp.set(29, tm.month-1, tm.year);

            startTime = tp.format("%Y%m%d");

            hour6 = 18;
        }
        else
            hour6 -= 6;

        startTime = startTime.concat(String.format("%02d", hour6));
        String T0 = startTime.substring( startTime.length()-2, startTime.length());

        int loop_count = (days * 24) / time_step;
//        String msga = String.format( "%d %d %d\n", days, time_step, loop_count);
//        Log.i("GRIB", msga);


        Time tn = new Time(Time.getCurrentTimezone());
        tn.setToNow();
        String formattedTime = tn.format("%Y%m%d_%H%M%S");

        String tdest_file = "gribs/gfs_" + formattedTime + ".grb2";
        File trootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File tdFile = new File(trootDir.getAbsolutePath() , tdest_file);
        String dest_file = tdFile.getAbsolutePath();

        Log.i("GRIB", "dest_file: " + dest_file);

        // persist the target file name
        Editor editor = preferences.edit();
        editor.putString("GRIB_dest_file", dest_file);
        editor.apply();

        Log.i("GRIB", "model: " + model);

        String gfsFilter = "";
        String gfsFilterPl = "";
        if(model.equals("GFS25")){
            gfsFilterPl = "0p25.pl?";
            gfsFilter = "z.pgrb2full.0p25.";
        }
        else if(model.equals("GFS50")){
            gfsFilterPl = "0p50.pl?";
            gfsFilter = "z.pgrb2full.0p50.";
        }
        else{
            gfsFilterPl = "1p00.pl?";
            gfsFilter = "z.pgrb2.1p00.";
        }


        String URL_GETGFS = "http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_" + gfsFilterPl;
        String DIR = "%2Fgfs." + startTime;
        String fileSaveName = "gribs/";

        //http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p50.pl?file=gfs.t18z.pgrb2full.0p50.f003&dir=%2Fgfs.2016031418&......

        // RTOFS Model
        //http://nomads.ncep.noaa.gov/cgi-bin/filter_rtofs_hires.pl?file=ofs_atl.t00z.f001.atl.hires.grb.std.grib2&var_WTMP=on&var_UOGRD=on&var_VOGRD=on&subregion=&leftlon=200&rightlon=360&toplat=40&bottomlat=0&dir=%2Fofs.20160315

        ArrayList<String> URLList = new ArrayList<String>();
        ArrayList<String> fileNameList = new ArrayList<String>();

        int t=0;
        // in a loop, get the files
        for(int x=0 ; x < loop_count ; x++){
            String time = String.format("f%03d", t);
            String fileName = "file=gfs.t" + T0 + gfsFilter + time;


            boolean blev10m = false;
            boolean blevmsl = false;
            boolean blevsurface = false;
            boolean blev2m = false;

            //  Make the required URL
            String URL_FETCH = URL_GETGFS + fileName;
            if(bWind){
                URL_FETCH += "&var_UGRD=on&var_VGRD=on";
                blev10m = true;
            }
            if(bPressure){
                URL_FETCH += "&var_PRMSL=on";
                blevmsl = true;
            }

            if(preferences.getBoolean("GRIB_prefb_windgust", false)){
                URL_FETCH += "&var_GUST=on";
                blevsurface = true;
            }

            if(preferences.getBoolean("GRIB_prefb_rainfall", false)){
                URL_FETCH += "&var_APCP=on";                            // Not working March 2016...
                blevsurface = true;
            }

            if(preferences.getBoolean("GRIB_prefb_cloudcover", false)){
                URL_FETCH += "&var_TCDC=on";                            // Not working March 2016...
            }

            if(preferences.getBoolean("GRIB_prefb_airtemp", false)){
                URL_FETCH += "&var_TMP=on";
                blev2m = true;
            }


            if(preferences.getBoolean("GRIB_prefb_CAPE", false)){
                URL_FETCH += "&var_CAPE=on";
                blevsurface = true;
            }


            if(preferences.getBoolean("GRIB_prefb_relhum", false)){
                URL_FETCH += "&var_RH=on";
                blevsurface = true;
            }

/*
        // Not available in GFS model
            if(preferences.getBoolean("GRIB_prefb_seatemp", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_alt", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_waves", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_current", false)){
            }
*/

            if(blev10m)
                URL_FETCH += "&lev_10_m_above_ground=on";

            if(blevmsl)
                URL_FETCH += "&lev_mean_sea_level=on";

            if(blevsurface)
                URL_FETCH += "&lev_surface=on";

            if(blev2m)
                URL_FETCH += "&lev_2_m_above_ground=on";


            URL_FETCH +="&subregion="
                + "&leftlon=" + String.format("%d", lon_min)
                + "&rightlon=" + String.format("%d", lon_max)
                + "&toplat=" + String.format("%d", lat_max)
                + "&bottomlat=" + String.format("%d", lat_min)
                + "&dir=" + DIR;

            Log.i("GRIB", "URL_FETCH: " + URL_FETCH);

            // Make the server local storage file name
            String sequence = String.format("SEQ%02d", t);
            String localFileName = fileSaveName + sequence;
            File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            File dFile = new File(rootDir.getAbsolutePath() , localFileName);
            String fullFileName = dFile.getAbsolutePath();

//            Log.i("GRIB", "localFileName: " + fullFileName);

            URLList.add(URL_FETCH);
            fileNameList.add(fullFileName);

            t += time_step;

        }

        //  Calculate estimate block size
        double nBlock = (lat_max - lat_min) * (lon_max - lon_min);
        double factor = 1.0;
        if(model.equals("GFS25"))
            factor = 16;;
        if(model.equals("GFS50"))
            factor = 4;

        nBlock *= factor;
        int nParam = 0;
        nParam = 0;
        if(bWind)
            nParam += 2;
        if(bPressure)
            nParam ++;

        nBlock *= nParam;
        int niBlock = (int)nBlock;

        String bmsg = String.format("%d\n", niBlock);
        Log.i("GRIB", "nBlock: " + bmsg);


        Intent intent = new Intent(this, org.opencpn.downloadGFSCombine.class);
        intent.putExtra("URLList",URLList);
        intent.putExtra("fileNameList",fileNameList);

        intent.putExtra("GRIB_dest_file",dest_file);
        intent.putExtra("niBlock",niBlock);
        startActivityForResult(intent, 0xf3ec /*OCPN_GRIB_REQUEST_CODE*/);


/*
        $t = 0;
         for ($x = 0; $x < $loop_count; $x++) {

             $TIME = sprintf("f%03d", $t);

             $FILE_NAME = "file=gfs.t" . $T0 . "z.pgrb2." . $gfs_filter . $TIME;  //file=gfs.t18z.pgrb2.0p25.f000

             $t += $_GET['time_step'];

             //  Make the required URL
             $URL_FETCH = $URL_GETGFS . $FILE_NAME . "&" . $LEVEL . "&var_UGRD=on&var_VGRD=on" . "&subregion="
                 . "&leftlon=" . $LEFT_LON
                 . "&rightlon=" . $RIGHT_LON
                 . "&toplat=" . $TOP_LAT
                 . "&bottomlat=" . $BOTTOM_LAT
                 . "&dir=" . $DIR;


#                echo $URL_FETCH . "<br>";

             // Make the server local storage file name
             $SEQ = sprintf("SEQ%02d", $x);
             $localFileName = $fileSaveName.$SEQ;
             #echo $localFileName . "<br>";

             //  Get the file
             get_grib( $URL_FETCH, $localFileName);

             #usleep(200000);         // sleep 0.2 secs.

#                echo "<br>";

         }
*/


        return "OK";
    }

    private String downloadWW3Direct(){


        //  Get some parameters from the Preferences.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        String model = preferences.getString("GRIB_prefs_model", "GFS100");
        int time_step = Integer.parseInt(preferences.getString("GRIB_prefs_timestep", "3"));
        int days = Integer.parseInt(preferences.getString("GRIB_prefs_days", "2"));
        Boolean bWind = preferences.getBoolean("GRIB_prefb_wind", true);
        Boolean bPressure = preferences.getBoolean("GRIB_prefb_pressure", true);

        int lat_min = 0;
        int lat_max = 1;
        int lon_min = 0;
        int lon_max = 1;
        String serverBase = "";

        //  Get some parameters from the JSON
        if(null != m_grib_PrefsJSON) {

            try {
                if (m_grib_PrefsJSON.has("latMin")) {
                    lat_min = m_grib_PrefsJSON.getInt("latMin");
                }
                if (m_grib_PrefsJSON.has("latMax")) {
                    lat_max = m_grib_PrefsJSON.getInt("latMax");
                }
                if (m_grib_PrefsJSON.has("lonMin")) {
                    lon_min = m_grib_PrefsJSON.getInt("lonMin");
                }
                if (m_grib_PrefsJSON.has("lonMax")) {
                    lon_max = m_grib_PrefsJSON.getInt("lonMax");
                }

            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }

        Time tm = new Time(Time.getCurrentTimezone());
        tm.setToNow();
        tm.switchTimezone("UTC");
        String startTime = tm.format("%Y%m%d");
        int tHours = tm.hour;
        int hour6 = (tHours / 6) * 6;

        //  GFS forcasts are late, sometimes, and UTC0000 forecast is not present yet
        //  So get the 1800 for previous day.
        if(hour6 == 0){
            Time tp = new Time(Time.getCurrentTimezone());
            tp.setToNow();
            tp.switchTimezone("UTC");
            if(tm.monthDay > 1)
                tp.set(tm.monthDay-1, tm.month, tm.year);
            else
                tp.set(29, tm.month-1, tm.year);

            startTime = tp.format("%Y%m%d");

            hour6 = 18;
        }
        else
            hour6 -= 6;

        startTime = startTime.concat(String.format("%02d", hour6));
        String T0 = startTime.substring( startTime.length()-2, startTime.length());

        int loop_count = (days * 24) / time_step;
//        String msga = String.format( "%d %d %d\n", days, time_step, loop_count);
//        Log.i("GRIB", msga);


        Time tn = new Time(Time.getCurrentTimezone());
        tn.setToNow();
        String formattedTime = tn.format("%Y%m%d_%H%M%S");

        String tdest_file = "gribs/gfs_" + formattedTime + ".grb2";
        File trootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File tdFile = new File(trootDir.getAbsolutePath() , tdest_file);
        String dest_file = tdFile.getAbsolutePath();

        Log.i("GRIB", "dest_file: " + dest_file);

        // persist the target file name
        Editor editor = preferences.edit();
        editor.putString("GRIB_dest_file", dest_file);
        editor.apply();

        Log.i("GRIB", "model: " + model);

        String gfsFilter = "";
        String gfsFilterPl = "";
        if(model.equals("GFS25")){
            gfsFilterPl = "0p25.pl?";
            gfsFilter = "z.pgrb2full.0p25.";
        }
        else if(model.equals("GFS50")){
            gfsFilterPl = "0p50.pl?";
            gfsFilter = "z.pgrb2full.0p50.";
        }
        else{
            gfsFilterPl = "1p00.pl?";
            gfsFilter = "z.pgrb2.1p00.";
        }


        String URL_GETGFS = "http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_" + gfsFilterPl;
        String DIR = "%2Fgfs." + startTime;
        String fileSaveName = "gribs/";

        //http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p50.pl?file=gfs.t18z.pgrb2full.0p50.f003&dir=%2Fgfs.2016031418&......

        // RTOFS Model
        //http://nomads.ncep.noaa.gov/cgi-bin/filter_rtofs_hires.pl?file=ofs_atl.t00z.f001.atl.hires.grb.std.grib2&var_WTMP=on&var_UOGRD=on&var_VOGRD=on&subregion=&leftlon=200&rightlon=360&toplat=40&bottomlat=0&dir=%2Fofs.20160315

        ArrayList<String> URLList = new ArrayList<String>();
        ArrayList<String> fileNameList = new ArrayList<String>();

        int t=0;
        // in a loop, get the files
        for(int x=0 ; x < loop_count ; x++){
            String time = String.format("f%03d", t);
            String fileName = "file=gfs.t" + T0 + gfsFilter + time;


            boolean blev10m = false;
            boolean blevmsl = false;
            boolean blevsurface = false;
            boolean blev2m = false;

            //  Make the required URL
            String URL_FETCH = URL_GETGFS + fileName;
            if(bWind){
                URL_FETCH += "&var_UGRD=on&var_VGRD=on";
                blev10m = true;
            }
            if(bPressure){
                URL_FETCH += "&var_PRMSL=on";
                blevmsl = true;
            }

            if(preferences.getBoolean("GRIB_prefb_windgust", false)){
                URL_FETCH += "&var_GUST=on";
                blevsurface = true;
            }

            if(preferences.getBoolean("GRIB_prefb_rainfall", false)){
                URL_FETCH += "&var_APCP=on";                            // Not working March 2016...
                blevsurface = true;
            }

            if(preferences.getBoolean("GRIB_prefb_cloudcover", false)){
                URL_FETCH += "&var_TCDC=on";                            // Not working March 2016...
            }

            if(preferences.getBoolean("GRIB_prefb_airtemp", false)){
                URL_FETCH += "&var_TMP=on";
                blev2m = true;
            }


            if(preferences.getBoolean("GRIB_prefb_CAPE", false)){
                URL_FETCH += "&var_CAPE=on";
                blevsurface = true;
            }


            if(preferences.getBoolean("GRIB_prefb_relhum", false)){
                URL_FETCH += "&var_RH=on";
                blevsurface = true;
            }

/*
        // Not available in GFS model
            if(preferences.getBoolean("GRIB_prefb_seatemp", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_alt", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_waves", false)){
            }

            if(preferences.getBoolean("GRIB_prefb_current", false)){
            }
*/

            if(blev10m)
                URL_FETCH += "&lev_10_m_above_ground=on";

            if(blevmsl)
                URL_FETCH += "&lev_mean_sea_level=on";

            if(blevsurface)
                URL_FETCH += "&lev_surface=on";

            if(blev2m)
                URL_FETCH += "&lev_2_m_above_ground=on";


            URL_FETCH +="&subregion="
                + "&leftlon=" + String.format("%d", lon_min)
                + "&rightlon=" + String.format("%d", lon_max)
                + "&toplat=" + String.format("%d", lat_max)
                + "&bottomlat=" + String.format("%d", lat_min)
                + "&dir=" + DIR;

            Log.i("GRIB", "URL_FETCH: " + URL_FETCH);

            // Make the server local storage file name
            String sequence = String.format("SEQ%02d", t);
            String localFileName = fileSaveName + sequence;
            File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            File dFile = new File(rootDir.getAbsolutePath() , localFileName);
            String fullFileName = dFile.getAbsolutePath();

//            Log.i("GRIB", "localFileName: " + fullFileName);

            URLList.add(URL_FETCH);
            fileNameList.add(fullFileName);

            t += time_step;

        }

        //  Calculate estimate block size
        double nBlock = (lat_max - lat_min) * (lon_max - lon_min);
        double factor = 1.0;
        if(model.equals("GFS25"))
            factor = 16;;
        if(model.equals("GFS50"))
            factor = 4;

        nBlock *= factor;
        int nParam = 0;
        nParam = 0;
        if(bWind)
            nParam += 2;
        if(bPressure)
            nParam ++;

        nBlock *= nParam;
        int niBlock = (int)nBlock;

        String bmsg = String.format("%d\n", niBlock);
        Log.i("GRIB", "nBlock: " + bmsg);


        Intent intent = new Intent(this, org.opencpn.downloadGFSCombine.class);
        intent.putExtra("URLList",URLList);
        intent.putExtra("fileNameList",fileNameList);

        intent.putExtra("GRIB_dest_file",dest_file);
        intent.putExtra("niBlock",niBlock);
        startActivityForResult(intent, 0xf3ec /*OCPN_GRIB_REQUEST_CODE*/);


/*
        $t = 0;
         for ($x = 0; $x < $loop_count; $x++) {

             $TIME = sprintf("f%03d", $t);

             $FILE_NAME = "file=gfs.t" . $T0 . "z.pgrb2." . $gfs_filter . $TIME;  //file=gfs.t18z.pgrb2.0p25.f000

             $t += $_GET['time_step'];

             //  Make the required URL
             $URL_FETCH = $URL_GETGFS . $FILE_NAME . "&" . $LEVEL . "&var_UGRD=on&var_VGRD=on" . "&subregion="
                 . "&leftlon=" . $LEFT_LON
                 . "&rightlon=" . $RIGHT_LON
                 . "&toplat=" . $TOP_LAT
                 . "&bottomlat=" . $BOTTOM_LAT
                 . "&dir=" . $DIR;


#                echo $URL_FETCH . "<br>";

             // Make the server local storage file name
             $SEQ = sprintf("SEQ%02d", $x);
             $localFileName = $fileSaveName.$SEQ;
             #echo $localFileName . "<br>";

             //  Get the file
             get_grib( $URL_FETCH, $localFileName);

             #usleep(200000);         // sleep 0.2 secs.

#                echo "<br>";

         }
*/


        return "OK";
    }

    private String downloadRTOFSDirect(){


        //  Get some parameters from the Preferences.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        String model = preferences.getString("GRIB_prefs_model", "GFS100");
        int time_step = Integer.parseInt(preferences.getString("GRIB_prefs_timestep", "3"));
        int days = Integer.parseInt(preferences.getString("GRIB_prefs_days", "2"));

        int lat_min = 0;
        int lat_max = 1;
        int lon_min = 0;
        int lon_max = 1;
        String serverBase = "";

        //  Get some parameters from the JSON
        if(null != m_grib_PrefsJSON) {

            try {
                if (m_grib_PrefsJSON.has("latMin")) {
                    lat_min = m_grib_PrefsJSON.getInt("latMin");
                }
                if (m_grib_PrefsJSON.has("latMax")) {
                    lat_max = m_grib_PrefsJSON.getInt("latMax");
                }
                if (m_grib_PrefsJSON.has("lonMin")) {
                    lon_min = m_grib_PrefsJSON.getInt("lonMin");
                }
                if (m_grib_PrefsJSON.has("lonMax")) {
                    lon_max = m_grib_PrefsJSON.getInt("lonMax");
                }

            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }

        if(lon_min < 0)
            lon_min += 360;
        if(lon_max < 0)
            lon_max += 360.;

        Time tm = new Time(Time.getCurrentTimezone());
        tm.setToNow();
        tm.switchTimezone("UTC");
        String startDate = tm.format("%Y%m%d");
        int tHours = tm.hour;

        //  NOAA forcasts are late, sometimes, and UTC0000 forecast is not present yet
        //  So get the previous day.
        if(tHours < 6){
            Time tp = new Time(Time.getCurrentTimezone());
            tp.setToNow();
            tp.switchTimezone("UTC");
            if(tm.monthDay > 1)
                tp.set(tm.monthDay-1, tm.month, tm.year);
            else
                tp.set(29, tm.month-1, tm.year);

            startDate = tp.format("%Y%m%d");

        }


//        startTime = startTime.concat(String.format("%02d", hour6));
//        String T0 = startTime.substring( startTime.length()-2, startTime.length());

        int loop_count = (days * 24) / time_step;
//        String msga = String.format( "%d %d %d\n", days, time_step, loop_count);
//        Log.i("GRIB", msga);


        Time tn = new Time(Time.getCurrentTimezone());
        tn.setToNow();
        String formattedTime = tn.format("%Y%m%d_%H%M%S");

        String tdest_file = "gribs/gfs_" + formattedTime + ".grb2";
        File trootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File tdFile = new File(trootDir.getAbsolutePath() , tdest_file);
        String dest_file = tdFile.getAbsolutePath();

        Log.i("GRIB", "dest_file: " + dest_file);

        // persist the target file name
        Editor editor = preferences.edit();
        editor.putString("GRIB_dest_file", dest_file);
        editor.apply();


        String URL_GETRTOFS = "http://nomads.ncep.noaa.gov/cgi-bin/filter_rtofs_hires.pl?";
        String DIR = "%2Fofs." + startDate;
        String fileSaveName = "gribs/";


        // RTOFS Model
        //http://nomads.ncep.noaa.gov/cgi-bin/filter_rtofs_hires.pl?file=ofs_atl.t00z.f001.atl.hires.grb.std.grib2&var_WTMP=on&var_UOGRD=on&var_VOGRD=on&subregion=&leftlon=200&rightlon=360&toplat=40&bottomlat=0&dir=%2Fofs.20160315

        ArrayList<String> URLList = new ArrayList<String>();
        ArrayList<String> fileNameList = new ArrayList<String>();

        int t=0;
        // in a loop, get the files
        for(int x=0 ; x < loop_count ; x++){
            String time = String.format("f%03d", t);
            if(t == 0)
                time = String.format("n%03d", t);

            String fileName = "file=ofs_atl.t00z." + time + ".atl.hires.grb.std.grib2";


            //  Make the required URL
            String URL_FETCH = URL_GETRTOFS + fileName;


            if(preferences.getBoolean("GRIB_prefb_seatemp", false)){
                URL_FETCH += "&var_WTMP=on";
            }

            if(preferences.getBoolean("GRIB_prefb_current", false)){
                URL_FETCH += "&var_UOGRD=on&var_VOGRD=on";
            }



            URL_FETCH += "&subregion="
                + "&leftlon=" + String.format("%d", lon_min)
                + "&rightlon=" + String.format("%d", lon_max)
                + "&toplat=" + String.format("%d", lat_max)
                + "&bottomlat=" + String.format("%d", lat_min)
                + "&dir=" + DIR;

            Log.i("GRIB", "URL_FETCH: " + URL_FETCH);

            // Make the server local storage file name
            String sequence = String.format("SEQ%02d", t);
            String localFileName = fileSaveName + sequence;
            File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            File dFile = new File(rootDir.getAbsolutePath() , localFileName);
            String fullFileName = dFile.getAbsolutePath();

            Log.i("GRIB", "localFileName: " + fullFileName);

            URLList.add(URL_FETCH);
            fileNameList.add(fullFileName);

            t += time_step;

        }


        //  Calculate estimate block size
        double nBlock = (lat_max - lat_min) * (lon_max - lon_min);
        double factor = 1.0;
        if(model.equals("GFS25"))
            factor = 16;;
        if(model.equals("GFS50"))
            factor = 4;

        nBlock *= factor;
        int nParam = 0;
        nParam = 0;
//        if(bWind)
//            nParam += 2;
//        if(bPressure)
//            nParam ++;

        nBlock *= nParam;
        int niBlock = (int)nBlock;

        String bmsg = String.format("%d\n", niBlock);
        Log.i("GRIB", "nBlock: " + bmsg);


        Intent intent = new Intent(this, org.opencpn.downloadGFSCombine.class);
        intent.putExtra("URLList",URLList);
        intent.putExtra("fileNameList",fileNameList);

        intent.putExtra("GRIB_dest_file",dest_file);
        intent.putExtra("niBlock",niBlock);
        startActivityForResult(intent, 0xf3ec );    //OCPN_GRIB_REQUEST_CODE





        return "OK";
    }


    public void onDownloadButtonClick(){

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        String model = preferences.getString("GRIB_prefs_model", "GFS100");

        if(model.startsWith("GFS", 0)){
            downloadGFSDirect();
        }
        else if(model.contains("WW3")){
            downloadWW3Direct();
        }
        else if(model.contains("RTOFS")){
            downloadRTOFSDirect();
        }
/*
        else{
            String url = CreateDownloadURL();
//            Log.i("GRIB", "url: " + url);


        //  Create the destination file name
        Calendar calendar = new GregorianCalendar(getTimeZone ("UTC"));

        String dest_file = "gfs_"
                + calendar.get(Calendar.YEAR)
                + calendar.get(Calendar.MONTH)
                + calendar.get(Calendar.DAY_OF_MONTH)
                + "_"
                + calendar.get(Calendar.HOUR_OF_DAY)
                + calendar.get(Calendar.MINUTE)
                + calendar.get(Calendar.SECOND)
                + ".grb2";


            Time t = new Time(Time.getCurrentTimezone());
            t.setToNow();
            String formattedTime = t.format("%Y%m%d_%H%M%S");

            String tdest_file = "gribs/gfs_" + formattedTime + ".grb2";
            File trootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            File tdFile = new File(trootDir.getAbsolutePath() , tdest_file);
            String dest_file = tdFile.getAbsolutePath();

            Log.i("GRIB", "dest_file: " + dest_file);

            // persist the target file name
            Editor editor = preferences.edit();
            editor.putString("GRIB_dest_file", dest_file);
            editor.apply();


            Intent intent = new Intent(this, org.opencpn.DownloadFile.class);
            intent.putExtra("URL",url);
            intent.putExtra("FILE_NAME",dest_file);
            startActivityForResult(intent, 0xf3ec OCPN_GRIB_REQUEST_CODE);
        }
*/
    }

    private String CreateDownloadURL(){

        //  Craft the download URL from the parameters in preferences bundle and JSON object.
        String url = "";

        //  Get some parameters from the Preferences.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        String model = preferences.getString("GRIB_prefs_model", "GFS100");
        int time_step = Integer.parseInt(preferences.getString("GRIB_prefs_timestep", "3"));
        int days = Integer.parseInt(preferences.getString("GRIB_prefs_days", "2"));
        Boolean bWind = preferences.getBoolean("GRIB_prefb_wind", true);
        Boolean bPressure = preferences.getBoolean("GRIB_prefb_pressure", true);

        //String startTime = "";
        int lat_min = 0;
        int lat_max = 1;
        int lon_min = 0;
        int lon_max = 1;
        String serverBase = "";

        //  Get some parameters from the JSON
        if(null != m_grib_PrefsJSON) {

            try {
          //      if (m_grib_PrefsJSON.has("server_base")) {
          //          serverBase = m_grib_PrefsJSON.getString("server_base");
          //      }
          //      if (m_grib_PrefsJSON.has("start_time")) {
          //          startTime = m_grib_PrefsJSON.getString("start_time");
          //      }
                if (m_grib_PrefsJSON.has("latMin")) {
                    lat_min = m_grib_PrefsJSON.getInt("latMin");
                }
                if (m_grib_PrefsJSON.has("latMax")) {
                    lat_max = m_grib_PrefsJSON.getInt("latMax");
                }
                if (m_grib_PrefsJSON.has("lonMin")) {
                    lon_min = m_grib_PrefsJSON.getInt("lonMin");
                }
                if (m_grib_PrefsJSON.has("lonMax")) {
                    lon_max = m_grib_PrefsJSON.getInt("lonMax");
                }

            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }



        //  All depends on the model....

        OCPNGRIBActivity.GRIB_model model_enum = OCPNGRIBActivity.GRIB_model.valueOf(model.toUpperCase());
        switch(model_enum) {
            case GFS25:
            case GFS50:
            case GFS100: {

                Time t = new Time(Time.getCurrentTimezone());
                t.setToNow();
                t.switchTimezone("UTC");
                String startTime = t.format("%Y%m%d");
                int tHours = t.hour;
                int hour6 = (tHours / 6) * 6;

                //  GFS forceast are late, sometimes, and UTC0000 forecast is not present yet
                //  So get the 1800 for previous day.
                if(hour6 == 0){
                    Time tp = new Time(Time.getCurrentTimezone());
                    tp.setToNow();
                    tp.switchTimezone("UTC");
                    tp.set(t.monthDay-1, t.month, t.year);
                    startTime = tp.format("%Y%m%d");

                    hour6 = 18;
                }

                startTime = startTime.concat(String.format("%02d", hour6));
/*
                //  Create the start time field
                Calendar calendar = new GregorianCalendar(getTimeZone ("UTC"));


                String dest_file = "gfs_"
                        + calendar.get(Calendar.YEAR)
                        + calendar.get(Calendar.MONTH)
                        + calendar.get(Calendar.DAY_OF_MONTH)
                        + "_"
                        + calendar.get(Calendar.HOUR_OF_DAY)
                        + calendar.get(Calendar.MINUTE)
                        + calendar.get(Calendar.SECOND)
                        + ".grb2";

*/
                serverBase = "http://192.168.37.99/get_grib.php";
                url = serverBase + "?";
                url = url.concat("model=" + model);
                url = url.concat("&start_time=" + startTime);

                url = url.concat("&time_step=" + String.format("%d", time_step));

                int time_count = (days * 24) / time_step;
                url = url.concat("&time_count=" + String.format("%d", time_count));

                url = url.concat("&lat_min=" + String.format("%d", lat_min));
                url = url.concat("&lat_max=" + String.format("%d", lat_max));
                url = url.concat("&lon_min=" + String.format("%d", lon_min));
                url = url.concat("&lon_max=" + String.format("%d", lon_max));

                //Log.i("GRIB", "url: " + url);

                //http://localhost/get_grib.php?model=GFS25&start_time=2016021218&time_step=3&time_count=2
                // &lat_min=20&lat_max=40&lon_min=280&lon_max=300
                break;

            }

            default:
                break;
        }

        return url;
    }



    private void ShowTextDialog(final String message){
        AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
        builder1.setMessage(message);
        builder1.setCancelable(true);
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


}

