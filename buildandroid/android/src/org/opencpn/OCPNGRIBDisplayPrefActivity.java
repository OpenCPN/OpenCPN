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
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Map;
import android.app.Activity;

import android.content.Intent;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.SwitchPreference;
import android.preference.CheckBoxPreference;
import android.text.format.Time;
import android.util.Log;

import android.view.View;
import android.widget.Toast;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Environment;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.app.FragmentManager;
import android.app.FragmentTransaction;

import ar.com.daidalos.afiledialog.*;

import org.json.JSONException;
import org.json.JSONObject;
import org.opencpn.opencpn.R;
import org.opencpn.DownloadFile;

import static java.util.TimeZone.getTimeZone;

//@ANDROID-11


public class OCPNGRIBDisplayPrefActivity extends PreferenceActivity

{
    public final static int OCPN_GRIB_DISPLAY_REQUEST_CODE = 1; // request code used to know when Display settings activity is done

    public JSONObject  m_grib_PrefsJSON;

    public static String m_settings;
    String m_newSettings;

    public enum GRIB_model {
        GFS100,
        GFS50,
        GFS25
      }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

 //       addPreferencesFromResource(R.xml.preferences_grib_display);
 //      setContentView(R.layout.grib_display_activity_layout);

/*
        // get fragment manager
        FragmentManager fm = getFragmentManager();

        // add DisplaySettings Fragment
        FragmentTransaction ft = fm.beginTransaction();
        ft.replace(android.R.id.content, new org.opencpn.OCPNGRIBFragmentDisplay(), "display");
        ft.commit();
*/
        Log.i("GRIB", "onCreate");
        if(savedInstanceState == null){

//            getFragmentManager().beginTransaction().replace(android.R.id.content,
//                       new org.opencpn.OCPNGRIBFragmentDisplay(), "display").commit();
        }

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

        m_settings = settings;
        m_newSettings = "stuff";

    }


    @Override
    public void onBuildHeaders(List<Header> target) {

        loadHeadersFromResource(R.xml.ocpn_grib_display_headers, target);


    }


/*
    //  We use this method to pass initial arguments to fragments
    //  Note:  Same bundle goes to all fragments.
    @Override
    public void onHeaderClick(Header header, int position) {



        if(header.fragmentArguments == null)
        {
            header.fragmentArguments = new Bundle();
        }

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            String settings = extras.getString("GRIB_PREFS_JSON");
            header.fragmentArguments.putString("GRIB_PREFS_JSON", settings);
        }


        super.onHeaderClick(header, position);


    }
*/

/*
    public void addDirectory(View vw)
    {
        Toast.makeText(this, "OCPNsettingsactivity Add Dir ", Toast.LENGTH_SHORT).show();

        Intent intent = new Intent(this, FileChooserActivity.class);
        intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, "/mnt/sdcard");
        intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_FULL_PATH_IN_TITLE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_ONLY_SELECTABLE, true);
     //   startActivityForResult(intent, 0);

        FileChooserDialog dialog = new FileChooserDialog(this);
        dialog.setCanCreateFiles(true);
        dialog.show();

    }

    public void removeDirectory(View vw)
    {
        Toast.makeText(this, "OCPNsettingsactivity Remove Dir ", Toast.LENGTH_SHORT).show();

        OCPNSettingsFragmentCharts cfrag = OCPNSettingsFragmentCharts.getFragment();
        String removalCandidate = cfrag.getSelected();


        cfrag.updateChartDirListView();
    }

    public void selectFile()
    {
//        Toast.makeText(this, "OCPNsettingsactivity Add Dir ", Toast.LENGTH_SHORT).show();

        Intent intent = new Intent(this, FileChooserActivity.class);
        intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, "/mnt/sdcard");
        intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_FULL_PATH_IN_TITLE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_ONLY_SELECTABLE, true);
     //   startActivityForResult(intent, 0);

        FileChooserDialog dialog = new FileChooserDialog(this);
        dialog.setCanCreateFiles(false);
        dialog.show();

    }

   @Override
   protected void onActivityResult(int requestCode, int resultCode, Intent data) {
       Log.i("GRIB", "onActivityResult");
       // Check which request we're responding to
       if (requestCode == 0) {
           // Make sure the request was successful
           if (resultCode == RESULT_OK) {
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

                OCPNSettingsFragmentCharts cfrag = OCPNSettingsFragmentCharts.getFragment();
                cfrag.updateChartDirListView();

           }
       }

       if (requestCode == 0xf3ec) {
           Log.i("GRIB", "onActivityResult from Download");

           // Make sure the request was successful
           if (resultCode == RESULT_OK) {
               ShowTextDialog(getResources().getString(R.string.ocpn_GRIB_download_dialog_OK));
           }
           else if(resultCode == DownloadFile.ERROR_NO_INTERNET){
               ShowTextDialog(getResources().getString(R.string.ocpn_GRIB_download_dialog_NO_INTERNET));
           }
           else if(resultCode == DownloadFile.ERROR_NO_CONNECTION){
               ShowTextDialog(getResources().getString(R.string.ocpn_GRIB_download_dialog_NO_CONNECTION));
           }
           else {
               ShowTextDialog(getResources().getString(R.string.ocpn_GRIB_download_dialog_DOWN_FAIL));
           }

       }




   }

   @Override
    public void onResume() {
        super.onResume();
        Log.i("GRIB", "DisplayPrefActivity onResume");
     }

*/

   @Override
   protected void onPause()
   {
       super.onPause();

       Log.i("DEBUGGER_TAG", "DisplayPrefActivity onPause");

       //createSettingsString();

       Bundle b = new Bundle();
       b.putString("SettingsString", m_newSettings);
//       Intent i = getIntent(); //gets the intent that called this intent
       Intent i = new Intent();
       i.putExtras(b);
       setResult(RESULT_OK, i);

   }

/*
   public void onFileSelected(String filePath, OCPNGRIBFragmentFile frag){
       String message = "File selected";
       message += ": " + filePath;
       Log.i("GRIB", message);

       SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
       SharedPreferences.Editor editor = preferences.edit();
       editor.putString("GRIB_dest_file", filePath);
       editor.apply();

       //getFragmentManager().beginTransaction().remove(frag).commit();
       getFragmentManager().popBackStackImmediate();

       ShowTextDialog("Download Failed\n  Please check logs.");

   }
*/



   @Override
   public void finish() {
       Log.i("DEBUGGER_TAG", "GRIB Activity finish");

       String json = persistJSON();
       Log.i("GRIB", "persist json:" + json);

       SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
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

   public void JSONPersist(SharedPreferences prefs, Map<String,?> keys, String page){
       try {
            for(Map.Entry<String,?> entry : keys.entrySet()){
//                    Log.d("map values",entry.getKey() + ": " + entry.getValue().toString());

               String testB = "grib_prefb_" + page;
               String testS = "grib_prefs_" + page;
               if(entry.getKey().startsWith(testS, 0)){
                   String jsonkey = entry.getKey().substring(11);
                   //Log.d("jsonkey string: ",jsonkey + ": " + entry.getValue().toString());
                   m_grib_PrefsJSON.put(jsonkey, prefs.getString( entry.getKey(), "?"));
               }
               if(entry.getKey().startsWith(testB, 0)){
                   String jsonkey = entry.getKey().substring(11);
                   //Log.d("jsonkey bool: ",jsonkey + ": " + entry.getValue().toString());
                   m_grib_PrefsJSON.put(jsonkey, prefs.getBoolean( entry.getKey(), true));

               }
            }
       } catch (JSONException e) {
           throw new RuntimeException(e);
       }
   }

    private String persistJSON() {
        //  Update everything interesting to the member JSON, and return a compact string

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        if (preferences != null) {
            Log.i("GRIB", "persisting");

            preferences.edit().clear();
            PreferenceManager.setDefaultValues(this, R.xml.grib_wind_display_settings, true);
            Map<String,?> keys = preferences.getAll();
            JSONPersist(preferences, keys, "Wind");

            preferences.edit().clear();
            PreferenceManager.setDefaultValues(this, R.xml.grib_pressure_display_settings, true);
            keys = preferences.getAll();
            JSONPersist(preferences, keys, "Pressure");

            try {
/*
            for(Map.Entry<String,?> entry : keys.entrySet()){
//                    Log.d("map values",entry.getKey() + ": " + entry.getValue().toString());

                    if(entry.getKey().startsWith("grib_prefs_Wind", 0)){
                        String jsonkey = entry.getKey().substring(11);
                        Log.d("jsonkey string: ",jsonkey + ": " + entry.getValue().toString());
                        m_grib_PrefsJSON.put(jsonkey, preferences.getString( entry.getKey(), "?"));
                    }
                    if(entry.getKey().startsWith("grib_prefb_Wind", 0)){
                        String jsonkey = entry.getKey().substring(11);
                        Log.d("jsonkey bool: ",jsonkey + ": " + entry.getValue().toString());
                        m_grib_PrefsJSON.put(jsonkey, preferences.getBoolean( entry.getKey(), true));

                    }
            }
*/




                m_grib_PrefsJSON.put("model", preferences.getString("GRIB_prefs_model", "?"));
                m_grib_PrefsJSON.put("days", preferences.getString("GRIB_prefs_days", "?"));
                m_grib_PrefsJSON.put("time_step", preferences.getString("GRIB_prefs_timestep", "?"));

                // Prepend the global "Download" directory to the file spec.
//                File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
//                File nFile = new File(rootDir.getAbsolutePath() , preferences.getString("GRIB_prefs_file", "?"));

                m_grib_PrefsJSON.put("grib_file", preferences.getString("GRIB_dest_file", "?"));

                //  GRIB Display Wind Fragment



            } catch (JSONException e) {
                throw new RuntimeException(e);
            }

        }

        try{
            return m_grib_PrefsJSON.toString(2);
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }

    }

    @Override
    protected boolean isValidFragment (String fragmentName) {

         return true; //"com.fullpackage.MyPreferenceFragment".equals(fragmentName);

    }


/*
    private String appendBoolSetting(String key, Boolean value)
    {
        String ret = key;
        if(value)
            ret = ret.concat(":1;");
        else
            ret = ret.concat(":0;");

        return ret;
    }

    private String appendStringSetting(String key, String value)
    {
        String ret = key;
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

*/
/*
    public static class junkGRIBWindSettings extends PreferenceFragment {
        public org.opencpn.opencpn.SeekBarPreference mBarbSpacingPixels;
        public org.opencpn.opencpn.SeekBarPreference mBarbNumberSpacingPixels;
        public SwitchPreference m_BarbSpacingCustom;
        public SwitchPreference m_BarbNumberSpacingCustom;
        public ListPreference m_ArrowcolorsPreference;
        public ListPreference m_ValueUnitsPreference;
        public ListPreference m_OverlaycolorsPreference;

        public SharedPreferences.OnSharedPreferenceChangeListener m_listenerVector;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Can retrieve arguments from preference XML.
            Log.i("args", "Arguments: " + getArguments());

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_wind_display_settings);

            //  Set up listeners for a few items, so that the "summaries" will update properly

            m_BarbSpacingCustom = (SwitchPreference)getPreferenceScreen().findPreference("grib_prefb_WindBarbedArrowFixedSpacing");
            mBarbSpacingPixels = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_WindBarbedArrowSpacing");

            m_BarbNumberSpacingCustom = (SwitchPreference)getPreferenceScreen().findPreference("grib_prefb_WindNumbersFixedSpacing");
            mBarbNumberSpacingPixels = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_WindNumbersSpacing");

            m_ArrowcolorsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_WindBarbedColors");
            m_ArrowcolorsPreference.setSummary(m_ArrowcolorsPreference.getEntry().toString());

            m_ValueUnitsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_WindUnits");
            m_ValueUnitsPreference.setSummary(m_ValueUnitsPreference.getEntry().toString());

            m_OverlaycolorsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_WindOverlayMapColors");
            m_OverlaycolorsPreference.setSummary(m_OverlaycolorsPreference.getEntry().toString());

            m_listenerVector = new SharedPreferences.OnSharedPreferenceChangeListener() {
                public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                    // listener implementation
                    // Set new summary, when a preference value changes

                    if (key.equals("grib_prefb_WindBarbedArrowFixedSpacing")) {
                        if (null != mBarbSpacingPixels) {
                            Log.i("GRIB", "key equals");
                            mBarbSpacingPixels.setEnabled(m_BarbSpacingCustom.isChecked());
                        }
                    }
                    if (key.equals("grib_prefb_WindNumbersFixedSpacing")) {
                        if (null != mBarbNumberSpacingPixels) {
                            mBarbNumberSpacingPixels.setEnabled(m_BarbNumberSpacingCustom.isChecked());
                        }
                    }
                    if (key.equals("grib_prefs_WindBarbedColors")) {
                        m_ArrowcolorsPreference.setSummary(m_ArrowcolorsPreference.getEntry().toString());
                    }

                    if (key.equals("grib_prefs_WindUnits")) {
                        m_ValueUnitsPreference.setSummary(m_ValueUnitsPreference.getEntry().toString());
                    }

                    if (key.equals("grib_prefs_WindOverlayMapColors")) {
                        m_OverlaycolorsPreference.setSummary(m_OverlaycolorsPreference.getEntry().toString());
                    }

                }
            };

            getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listenerVector);




        }
    }
*/

    public static class GRIBGeneralSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_general_display_settings);
            page = "General";

            setListeners();
        }

    }

    public static class GRIBWindSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_wind_display_settings);
            page = "Wind";

            setListeners();
        }

    }


    public static class GRIBPressureSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_pressure_display_settings);
            page = "Pressure";

            setListeners();
        }

    }

    public static class GRIBGustSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_gust_display_settings);
            page = "WindGust";

            setListeners();
        }

    }


    public static class GRIBWaveSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_wave_display_settings);
            page = "Waves";

            setListeners();
        }

    }


    public static class GRIBCurrentSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_current_display_settings);
            page = "Current";

            setListeners();
        }

    }


    public static class GRIBRainfallSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_rainfall_display_settings);
            page = "Rainfall";

            setListeners();
        }

    }

    public static class GRIBCloudSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_cloud_display_settings);
            page = "CloudCover";

            setListeners();
        }

    }

    public static class GRIBAirtempSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_airtemp_display_settings);
            page = "AirTemperature";

            setListeners();
        }

    }

    public static class GRIBSeatempSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_seatemp_display_settings);
            page = "SeaTemperature";

            setListeners();
        }

    }


    public static class GRIBCAPESettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_cape_display_settings);
            page = "CAPE";

            setListeners();
        }

    }

    public static class GRIBAltGeoSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_altgeo_display_settings);
            page = "Altitude";

            setListeners();
        }

    }

    public static class GRIBRelHumSettings extends GRIBDisplayItemSettings {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.grib_relhum_display_settings);
            page = "RelativeHumidity";

            setListeners();
        }

    }






    public static class GRIBDisplayItemSettings extends PreferenceFragment {

        public SwitchPreference m_BarbSpacingCustom;
        public org.opencpn.opencpn.SeekBarPreference m_BarbSpacingPixels;

        public org.opencpn.opencpn.SeekBarPreference m_ArrowSpacingPixels;
        public org.opencpn.opencpn.SeekBarPreference m_NumberSpacingPixels;
        public SwitchPreference m_ArrowSpacingCustom;
        public SwitchPreference m_NumberSpacingCustom;
        public ListPreference m_ValueUnitsPreference;
        public ListPreference m_OverlaycolorsPreference;
        public CheckBoxPreference m_IsobarShow;
        public org.opencpn.opencpn.SeekBarPreference m_IsobarDelta;
        public ListPreference m_BarbcolorsPreference;
        public ListPreference m_DirectionArrowForm;

        public String page = "";

        public SharedPreferences.OnSharedPreferenceChangeListener m_listenerVector;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            Log.i("GRIB", "GRIBDisplayItemSettings Create");

        }

        public void setListeners(){
            //  Set up initial "Summary" fields

            m_BarbSpacingCustom = (SwitchPreference)getPreferenceScreen().findPreference("grib_prefb_" + page + "BarbedArrowFixedSpacing");
            m_BarbSpacingPixels = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_" + page + "BarbedArrowSpacing");
            if(null != m_BarbSpacingPixels)
                m_BarbSpacingPixels.setEnabled(m_BarbSpacingCustom.isChecked());

            m_BarbcolorsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_" + page + "BarbedColors");
            if( (null != m_BarbcolorsPreference) && (null != m_BarbcolorsPreference.getEntry()))
                m_BarbcolorsPreference.setSummary(m_BarbcolorsPreference.getEntry().toString());

            m_NumberSpacingCustom = (SwitchPreference)getPreferenceScreen().findPreference("grib_prefb_" + page + "NumbersFixedSpacing");
            m_NumberSpacingPixels = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_" + page + "NumbersSpacing");
            if(null != m_NumberSpacingPixels)
                m_NumberSpacingPixels.setEnabled(m_NumberSpacingCustom.isChecked());

            m_ArrowSpacingCustom = (SwitchPreference)getPreferenceScreen().findPreference("grib_prefb_" + page + "DirectionArrowFixedSpacing");
            m_ArrowSpacingPixels = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_" + page + "DirectionArrowSpacing");
            if(null != m_ArrowSpacingPixels)
                m_ArrowSpacingPixels.setEnabled(m_ArrowSpacingCustom.isChecked());

            m_DirectionArrowForm = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_" + page + "DirectionArrowForm");
            if( (null != m_DirectionArrowForm) && (null != m_DirectionArrowForm.getEntry()))
                m_DirectionArrowForm.setSummary(m_DirectionArrowForm.getEntry().toString());

            m_ValueUnitsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_" + page + "Units");
            if( (null != m_ValueUnitsPreference) && (null != m_ValueUnitsPreference.getEntry()))
                m_ValueUnitsPreference.setSummary(m_ValueUnitsPreference.getEntry().toString());

            m_OverlaycolorsPreference = (ListPreference)getPreferenceScreen().findPreference("grib_prefs_" + page + "OverlayMapColors");
            if( (null != m_OverlaycolorsPreference) && (null != m_OverlaycolorsPreference.getEntry()))
                m_OverlaycolorsPreference.setSummary(m_OverlaycolorsPreference.getEntry().toString());

            m_IsobarShow = (CheckBoxPreference)getPreferenceScreen().findPreference("grib_prefb_" + page + "DisplayIsobars");
            m_IsobarDelta = (org.opencpn.opencpn.SeekBarPreference) getPreferenceScreen().findPreference("grib_prefs_" + page + "IsoBarSpacing");
            if(null != m_IsobarDelta)
                m_IsobarDelta.setEnabled(m_IsobarShow.isChecked());

            m_listenerVector = new SharedPreferences.OnSharedPreferenceChangeListener() {
                public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                    // listener implementation
                    // Set new summary, when a preference value changes

                    if (key.equals("grib_prefb_" + page + "BarbedArrowFixedSpacing")) {
                        if (null != m_BarbSpacingPixels)
                           m_BarbSpacingPixels.setEnabled(m_BarbSpacingCustom.isChecked());
                    }
                    else if (key.equals("grib_prefb_" + page + "DirectionArrowFixedSpacing")) {
                        if (null != m_ArrowSpacingPixels)
                           m_ArrowSpacingPixels.setEnabled(m_ArrowSpacingCustom.isChecked());
                    }

                    else if (key.equals("grib_prefb_" + page + "NumbersFixedSpacing")) {
                        if (null != m_NumberSpacingPixels)
                            m_NumberSpacingPixels.setEnabled(m_NumberSpacingCustom.isChecked());
                    }
                    else if (key.equals("grib_prefs_" + page + "Units")) {
                        if (null != m_ValueUnitsPreference)
                            m_ValueUnitsPreference.setSummary(m_ValueUnitsPreference.getEntry().toString());
                    }

                    else if (key.equals("grib_prefs_" + page + "OverlayMapColors")) {
                        if (null != m_OverlaycolorsPreference)
                            m_OverlaycolorsPreference.setSummary(m_OverlaycolorsPreference.getEntry().toString());
                    }

                    else if (key.equals("grib_prefs_" + page + "BarbedColors")) {
                        if (null != m_BarbcolorsPreference)
                            m_BarbcolorsPreference.setSummary(m_BarbcolorsPreference.getEntry().toString());
                    }

                    else if (key.equals("grib_prefb_" + page + "DisplayIsobars")) {
                        if (null != m_IsobarDelta)
                            m_IsobarDelta.setEnabled(m_IsobarShow.isChecked());
                    }

                    else if (key.equals("grib_prefs_" + page + "DirectionArrowForm")) {
                        if (null != m_DirectionArrowForm)
                            m_DirectionArrowForm.setSummary(m_DirectionArrowForm.getEntry().toString());
                    }



                }
            };

            getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listenerVector);




        }
    }



}





