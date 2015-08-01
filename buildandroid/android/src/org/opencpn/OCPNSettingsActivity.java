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
import java.util.List;
import java.util.StringTokenizer;


import android.content.Intent;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.util.Log;

import android.view.View;
import android.widget.Toast;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;
import android.preference.Preference;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import ar.com.daidalos.afiledialog.*;

import org.opencpn.opencpn.R;

//@ANDROID-11


public class OCPNSettingsActivity extends PreferenceActivity
{

    private boolean mbS52 = false;

    @Override
    public void onBuildHeaders(List<Header> target) {
        loadHeadersFromResource(R.xml.ocpn_preference_headers, target);

        m_chartDirList = new ArrayList<String>();

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            String settings = extras.getString("SETTINGS_STRING");

            Log.i("DEBUGGER_TAG", "OCPNSettingsActivity");
//            Log.i("DEBUGGER_TAG", settings);

            m_settings = settings;
            m_newSettings = "";

            //  Create the various settings elements

            StringTokenizer tkz = new StringTokenizer(settings, ";");

            while(tkz.hasMoreTokens()){
                String tk = tkz.nextToken();
//                Log.i("DEBUGGER_TAG", tk);

                if( tk.startsWith("ChartDir") ){
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String dir = tk.substring(mark+1);
//                        Log.i("DEBUGGER_TAG", dir);
                        m_chartDirList.add(dir);
                    }
                }

            }

            // Stuff the application shared preferences data from the settings string
            SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
            Editor editor = preferences.edit();

            StringTokenizer tkzp = new StringTokenizer(settings, ";");

            while(tkzp.hasMoreTokens()){
                String tk = tkzp.nextToken();

                if( tk.startsWith("prefb_") ){       // simple Boolean
                    mbS52 |= isPrefS52(tk);
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String key = tk.substring(0, mark);
                        String value = tk.substring(mark+1);
                        if(value.equals("1"))
                            editor.putBoolean(key, true);
                        else
                            editor.putBoolean(key, false);
                    }
                }

                else if( tk.startsWith("prefs_") ){       // simple string
                    mbS52 |= isPrefS52(tk);
                    int mark = tk.indexOf(":");
                    if(mark > 0){
                        String key = tk.substring(0, mark);
//                        Log.i("DEBUGGER_TAG", key);
//                        Log.i("DEBUGGER_TAG", (mbS52)?"True":"False");
                        String value = tk.substring(mark+1);
                        editor.putString(key, value);
                    }
                }

            }

           editor.commit();
        }

    }

     //  We use this method to pass initial arguments to fragments
     //  Note:  Same bundle goes to all fragments.
     @Override
     public void onHeaderClick(Header header, int position) {
         if(header.fragmentArguments == null)
         {
             header.fragmentArguments = new Bundle();
         }

         if(mbS52){
             header.fragmentArguments.putString("S52", "TRUE");
         }
         else{
             header.fragmentArguments.putString("S52", "FALSE");
         }


         super.onHeaderClick(header, position);

     }

     private boolean isPrefS52( String s ){

        if(s.startsWith("prefs_displaycategory")) return true;
        if(s.startsWith("prefs_shallowdepth")) return true;
        if(s.startsWith("prefs_safetydepth")) return true;
        if(s.startsWith("prefs_deepdepth")) return true;
        if(s.startsWith("prefs_vectorgraphicsstyle")) return true;
        if(s.startsWith("prefs_vectorboundarystyle")) return true;
        if(s.startsWith("prefs_vectorchartcolors")) return true;
        if(s.startsWith("prefb_showsound")) return true;
        if(s.startsWith("prefb_showATONLabels")) return true;
        if(s.startsWith("prefb_showlightldesc")) return true;
        if(s.startsWith("prefb_showimptext")) return true;
        if(s.startsWith("prefb_showSCAMIN")) return true;

        return false;
    }

    public void addDirectory(View vw)
    {
//        Toast.makeText(this, "OCPNsettingsactivity Add Dir ", Toast.LENGTH_SHORT).show();

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        Intent intent = new Intent(OCPNSettingsActivity.this, FileChooserActivity.class);
        intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, preferences.getString("prefs_chartInitDir", "/mnt/sdcard"));
        intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_FULL_PATH_IN_TITLE, true);
        intent.putExtra(FileChooserActivity.INPUT_SHOW_ONLY_SELECTABLE, true);
        intent.putExtra(FileChooserActivity.INPUT_TITLE_STRING, "Add Chart Directory");

        startActivityForResult(intent, 0);
   }

    public void removeDirectory(View vw)
    {
//        Toast.makeText(this, "OCPNsettingsactivity Remove Dir ", Toast.LENGTH_SHORT).show();

        OCPNSettingsFragmentCharts cfrag = OCPNSettingsFragmentCharts.getFragment();
        String removalCandidate = cfrag.getSelected();

        m_chartDirList.remove(removalCandidate);

        cfrag.updateChartDirListView();
    }

   @Override
   protected void onActivityResult(int requestCode, int resultCode, Intent data) {
       // Check which request we're responding to
       if (requestCode == 0) {
           // Make sure the request was successful
           if (resultCode == RESULT_OK) {
               boolean fileCreated = false;
               String filePath = "";
               String parentDir = "";
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
                       parentDir = file.getParent();
                       if(null == parentDir)
                            parentDir = "/mnt";
                   }
               }

               String message = fileCreated? "File created" : "File opened";
               message += ": " + filePath + " parentDir:" + parentDir;;
//               Log.i("DEBUGGER_TAG", message);
               m_chartDirList.add(filePath);

               SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
               Editor editor = preferences.edit();
               editor.putString("prefs_chartInitDir", parentDir);
               editor.commit();


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

       Log.i("DEBUGGER_TAG", "SettingsActivity onPause");

       createSettingsString();

       Bundle b = new Bundle();
       b.putString("SettingsString", m_newSettings);

       Intent i = new Intent();
       i.putExtras(b);
       setResult(RESULT_OK, i);

   }

   @Override
   public void finish() {
       Log.i("DEBUGGER_TAG", "SettingsActivity finish");

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
        m_newSettings = "";


        // Record the chart dir list contents
        for(int i=0 ; i < m_chartDirList.size() ; ++i){
            m_newSettings = m_newSettings.concat("ChartDir:");
            String dir = m_chartDirList.get(i);
//            Log.i("DEBUGGER_TAG", dir);
            m_newSettings = m_newSettings.concat(dir);
            m_newSettings = m_newSettings.concat(";");
//            Log.i("DEBUGGER_TAG", m_newSettings);

        }

        // Get the android persisted values, one-by-one
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        //  Simple Booleans
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_lookahead", preferences.getBoolean("prefb_lookahead", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_quilt", preferences.getBoolean("prefb_quilt", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showgrid", preferences.getBoolean("prefb_showgrid", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showoutlines", preferences.getBoolean("prefb_showoutlines", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showdepthunits", preferences.getBoolean("prefb_showdepthunits", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_lockwp", preferences.getBoolean("prefb_lockwp", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_confirmdelete", preferences.getBoolean("prefb_confirmdelete", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_expertmode", preferences.getBoolean("prefb_expertmode", false)));
        m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_internalGPS", preferences.getBoolean("prefb_internalGPS", false)));

        if(mbS52){
         m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showsound", preferences.getBoolean("prefb_showsound", false)));
         m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showATONLabels", preferences.getBoolean("prefb_showATONLabels", false)));
         m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showlightldesc", preferences.getBoolean("prefb_showlightldesc", false)));
         m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showimptext", preferences.getBoolean("prefb_showimptext", false)));
         m_newSettings = m_newSettings.concat(appendBoolSetting("prefb_showSCAMIN", preferences.getBoolean("prefb_showSCAMIN", false)));
        }


        // Other assorted string values
        if(mbS52){
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_displaycategory", preferences.getString("prefs_displaycategory", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_shallowdepth", preferences.getString("prefs_shallowdepth", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_safetydepth", preferences.getString("prefs_safetydepth", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_deepdepth", preferences.getString("prefs_deepdepth", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_vectorgraphicsstyle", preferences.getString("prefs_vectorgraphicsstyle", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_vectorboundarystyle", preferences.getString("prefs_vectorboundarystyle", "?")));
         m_newSettings = m_newSettings.concat(appendStringSetting("prefs_vectorchartcolors", preferences.getString("prefs_vectorchartcolors", "?")));
        }

        m_newSettings = m_newSettings.concat(appendStringSetting("prefs_navmode", preferences.getString("prefs_navmode", "?")));
        m_newSettings = m_newSettings.concat(appendStringSetting("prefs_UIScaleFactor", preferences.getString("prefs_UIScaleFactor", "?")));
        m_newSettings = m_newSettings.concat(appendStringSetting("prefs_chartScaleFactor", preferences.getString("prefs_chartScaleFactor", "?")));
        m_newSettings = m_newSettings.concat(appendStringSetting("prefs_chartInitDir", preferences.getString("prefs_chartInitDir", "?")));

//        Log.i("DEBUGGER_TAG", m_newSettings);

    }

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



    /**
     * This fragment contains a second-level set of preference
     * for Charts->Vector Chart Settings.
     */
    public static class ChartsVectorSettings extends PreferenceFragment {

        public ListPreference mDisplayCategoryPreference;
        public ListPreference mGraphicsStylePreference;
        public ListPreference mBoundaryStylePreference;
        public ListPreference mColorsPreference;

        public EditTextPreference mShallowPreference;
        public EditTextPreference mSafetyPreference;
        public EditTextPreference mDeepPreference;

        public SharedPreferences.OnSharedPreferenceChangeListener m_listenerVector;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Can retrieve arguments from preference XML.
//            Log.i("args", "Arguments: " + getArguments());

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.charts_vector_settings);

            //  Set up listeners for a few items, so that the "summaries" will update properly

            // Display Category
            mDisplayCategoryPreference = (ListPreference)getPreferenceScreen().findPreference("prefs_displaycategory");
            // Graphics Style
            mGraphicsStylePreference = (ListPreference)getPreferenceScreen().findPreference("prefs_vectorgraphicsstyle");
            // Boundary Style
            mBoundaryStylePreference = (ListPreference)getPreferenceScreen().findPreference("prefs_vectorboundarystyle");
            // Colors
            mColorsPreference = (ListPreference)getPreferenceScreen().findPreference("prefs_vectorchartcolors");

            mShallowPreference = (EditTextPreference)getPreferenceScreen().findPreference("prefs_shallowdepth");
            mSafetyPreference = (EditTextPreference)getPreferenceScreen().findPreference("prefs_safetydepth");
            mDeepPreference = (EditTextPreference)getPreferenceScreen().findPreference("prefs_deepdepth");

            m_listenerVector = new SharedPreferences.OnSharedPreferenceChangeListener() {
                public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                    // listener implementation
                    // Set new summary, when a preference value changes
                    if (key.equals("prefs_displaycategory")) {
                        mDisplayCategoryPreference.setSummary(mDisplayCategoryPreference.getEntry().toString());
                    }
                    if (key.equals("prefs_vectorgraphicsstyle")) {
                        mGraphicsStylePreference.setSummary(mGraphicsStylePreference.getEntry().toString());
                    }
                    if (key.equals("prefs_vectorboundarystyle")) {
                        mBoundaryStylePreference.setSummary(mBoundaryStylePreference.getEntry().toString());
                    }
                    if (key.equals("prefs_vectorchartcolors")) {
                        mColorsPreference.setSummary(mColorsPreference.getEntry().toString());
                    }
                    if (key.equals("prefs_shallowdepth")) {
                        mShallowPreference.setSummary(mShallowPreference.getText());
                    }
                    if (key.equals("prefs_safetydepth")) {
                        mSafetyPreference.setSummary(mSafetyPreference.getText());
                    }
                    if (key.equals("prefs_deepdepth")) {
                        mDeepPreference.setSummary(mDeepPreference.getText());
                    }
                }
            };

            getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(m_listenerVector);


            // Setup initial values, checking for undefined as yet.

            CharSequence cs = mDisplayCategoryPreference.getEntry();
            if(cs == null)
                mDisplayCategoryPreference.setValueIndex(0);

            cs = mGraphicsStylePreference.getEntry();
            if(cs == null)
                mGraphicsStylePreference.setValueIndex(0);

            cs = mBoundaryStylePreference.getEntry();
            if(cs == null)
                mBoundaryStylePreference.setValueIndex(0);

            cs = mColorsPreference.getEntry();
            if(cs == null)
                mColorsPreference.setValueIndex(0);

            mDisplayCategoryPreference.setSummary(mDisplayCategoryPreference.getEntry().toString());
            mGraphicsStylePreference.setSummary(mGraphicsStylePreference.getEntry().toString());
            mBoundaryStylePreference.setSummary(mBoundaryStylePreference.getEntry().toString());
            mColorsPreference.setSummary(mColorsPreference.getEntry().toString());

            if(mShallowPreference.getText() == null)
                mShallowPreference.setText("2");
            if(mSafetyPreference.getText() == null)
                mSafetyPreference.setText("4");
            if(mDeepPreference.getText() == null)
                mDeepPreference.setText("6");

            mShallowPreference.setSummary(mShallowPreference.getText());
            mSafetyPreference.setSummary(mSafetyPreference.getText());
            mDeepPreference.setSummary(mDeepPreference.getText());

        }
    }


    public static String getSettingsString(){ return m_settings;}
    public static ArrayList<String> getChartDirList(){ return m_chartDirList;}

    private static String m_settings;
    private String m_newSettings;
    private static ArrayList<String> m_chartDirList;
}





