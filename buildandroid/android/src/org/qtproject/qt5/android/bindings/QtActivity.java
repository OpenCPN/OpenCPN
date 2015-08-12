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
package org.qtproject.qt5.android.bindings;

import com.arieslabs.assetbridge.Assetbridge;

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
import java.lang.Math;
import java.util.concurrent.Semaphore;
import org.kde.necessitas.ministro.IMinistro;
import org.kde.necessitas.ministro.IMinistroCallback;

import android.os.SystemClock;
import android.os.Environment;
import android.app.Activity;
import android.app.ProgressDialog;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.DialogInterface.OnCancelListener;

import android.app.DownloadManager;
import android.app.DownloadManager.Query;
import android.app.DownloadManager.Request;
import android.content.BroadcastReceiver;
import android.database.Cursor;
import android.content.IntentFilter;

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
import android.graphics.drawable.Drawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.Point;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.AttributeSet;
import android.util.Log;
import android.util.DisplayMetrics;
import android.widget.Toast;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import android.widget.Button;

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
import android.view.Display;
import android.view.MenuInflater;

import dalvik.system.DexClassLoader;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.provider.Settings;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import org.opencpn.opencpn.R;

//@ANDROID-11
import android.app.Fragment;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
//@ANDROID-11

import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import java.util.concurrent.atomic.AtomicReference;

import android.bluetooth.BluetoothAdapter;

import org.opencpn.GPSServer;
import org.opencpn.OCPNNativeLib;

import android.bluetooth.BluetoothDevice;
import org.opencpn.BTScanHelper;
import app.akexorcist.bluetotohspp.library.BluetoothSPP;
import app.akexorcist.bluetotohspp.library.BluetoothState;
import app.akexorcist.bluetotohspp.library.BluetoothSPP.OnDataReceivedListener;

import org.opencpn.SpinnerNavItem;
import org.opencpn.TitleNavigationAdapter;

import ar.com.daidalos.afiledialog.*;

public class QtActivity extends Activity implements ActionBar.OnNavigationListener
{
    private final static int MINISTRO_INSTALL_REQUEST_CODE = 0xf3ee; // request code used to know when Ministro instalation is finished
    private final static int OCPN_SETTINGS_REQUEST_CODE = 0xf3ef; // request code used to know when OCPNsettings dialog activity is done
    private final static int OCPN_GOOGLEMAPS_REQUEST_CODE = 0xf3ed; // request code used to know when GoogleMaps activity is done

    private static final int MINISTRO_API_LEVEL = 4; // Ministro api level (check IMinistro.aidl file)
    private static final int NECESSITAS_API_LEVEL = 2; // Necessitas api level used by platform plugin
    private static final int QT_VERSION = 0x050100; // This app requires at least Qt version 5.1.0

    private final static int OCPN_FILECHOOSER_REQUEST_CODE = 0x5555;
    private final static int OCPN_AFILECHOOSER_REQUEST_CODE = 0x5556;

    private final static int OCPN_ACTION_FOLLOW = 0x1000;
    private final static int OCPN_ACTION_ROUTE = 0x1001;
    private final static int OCPN_ACTION_RMD = 0x1002;
    private final static int OCPN_ACTION_SETTINGS_BASIC = 0x1003;
    private final static int OCPN_ACTION_SETTINGS_EXPERT = 0x1004;
    private final static int OCPN_ACTION_TRACK_TOGGLE = 0x1005;
    private final static int OCPN_ACTION_MOB = 0x1006;
    private final static int OCPN_ACTION_TIDES_TOGGLE = 0x1007;
    private final static int OCPN_ACTION_CURRENTS_TOGGLE = 0x1008;
    private final static int OCPN_ACTION_ENCTEXT_TOGGLE = 0x1009;
    private final static int OCPN_ACTION_TRACK_ON = 0x100a;
    private final static int OCPN_ACTION_TRACK_OFF = 0x100b;


    //  Definitions found in OCPN "chart1.h"
    private final static int ID_CMD_APPLY_SETTINGS = 300;
    private final static int ID_CMD_NULL_REFRESH = 301;

    private final static int CHART_TYPE_CM93COMP = 7;       // must line up with OCPN types
    private final static int CHART_FAMILY_RASTER = 1;
    private final static int CHART_FAMILY_VECTOR = 2;


    private static final String ERROR_CODE_KEY = "error.code";
    private static final String ERROR_MESSAGE_KEY = "error.message";
    private static final String DEX_PATH_KEY = "dex.path";
    private static final String LIB_PATH_KEY = "lib.path";
    private static final String LOADER_CLASS_NAME_KEY = "loader.class.name";
    private static final String NATIVE_LIBRARIES_KEY = "native.libraries";
    private static final String ENVIRONMENT_VARIABLES_KEY = "environment.variables";
    private static final String APPLICATION_PARAMETERS_KEY = "application.parameters";
    private static final String BUNDLED_LIBRARIES_KEY = "bundled.libraries";
    private static final String BUNDLED_IN_LIB_RESOURCE_ID_KEY = "android.app.bundled_in_lib_resource_id";
    private static final String BUNDLED_IN_ASSETS_RESOURCE_ID_KEY = "android.app.bundled_in_assets_resource_id";
    private static final String MAIN_LIBRARY_KEY = "main.library";
    private static final String STATIC_INIT_CLASSES_KEY = "static.init.classes";
    private static final String NECESSITAS_API_LEVEL_KEY = "necessitas.api.level";

    /// Ministro server parameter keys
    private static final String REQUIRED_MODULES_KEY = "required.modules";
    private static final String APPLICATION_TITLE_KEY = "application.title";
    private static final String MINIMUM_MINISTRO_API_KEY = "minimum.ministro.api";
    private static final String MINIMUM_QT_VERSION_KEY = "minimum.qt.version";
    private static final String SOURCES_KEY = "sources";               // needs MINISTRO_API_LEVEL >=3 !!!
                                                                       // Use this key to specify any 3rd party sources urls
                                                                       // Ministro will download these repositories into their
                                                                       // own folders, check http://community.kde.org/Necessitas/Ministro
                                                                       // for more details.

    private static final String REPOSITORY_KEY = "repository";         // use this key to overwrite the default ministro repsitory
    private static final String ANDROID_THEMES_KEY = "android.themes"; // themes that your application uses


    public String APPLICATION_PARAMETERS = null; // use this variable to pass any parameters to your application,
                                                               // the parameters must not contain any white spaces
                                                               // and must be separated with "\t"
                                                               // e.g "-param1\t-param2=value2\t-param3\tvalue3"

    public String ENVIRONMENT_VARIABLES = "QT_USE_ANDROID_NATIVE_STYLE=1\tQT_USE_ANDROID_NATIVE_DIALOGS=1\t";
                                                               // use this variable to add any environment variables to your application.
                                                               // the env vars must be separated with "\t"
                                                               // e.g. "ENV_VAR1=1\tENV_VAR2=2\t"
                                                               // Currently the following vars are used by the android plugin:
                                                               // * QT_USE_ANDROID_NATIVE_STYLE - 1 to use the android widget style if available.
                                                               // * QT_USE_ANDROID_NATIVE_DIALOGS -1 to use the android native dialogs.

    public String[] QT_ANDROID_THEMES = null;     // A list with all themes that your application want to use.
                                                  // The name of the theme must be the same with any theme from
                                                  // http://developer.android.com/reference/android/R.style.html
                                                  // The most used themes are:
                                                  //  * "Theme" - (fallback) check http://developer.android.com/reference/android/R.style.html#Theme
                                                  //  * "Theme_Black" - check http://developer.android.com/reference/android/R.style.html#Theme_Black
                                                  //  * "Theme_Light" - (default for API <=10) check http://developer.android.com/reference/android/R.style.html#Theme_Light
                                                  //  * "Theme_Holo" - check http://developer.android.com/reference/android/R.style.html#Theme_Holo
                                                  //  * "Theme_Holo_Light" - (default for API 11-13) check http://developer.android.com/reference/android/R.style.html#Theme_Holo_Light
                                                  //  * "Theme_DeviceDefault" - check http://developer.android.com/reference/android/R.style.html#Theme_DeviceDefault
                                                  //  * "Theme_DeviceDefault_Light" - (default for API 14+) check http://developer.android.com/reference/android/R.style.html#Theme_DeviceDefault_Light

    public String QT_ANDROID_DEFAULT_THEME = null; // sets the default theme.

    private static Activity m_activity = null;

    private static final int INCOMPATIBLE_MINISTRO_VERSION = 1; // Incompatible Ministro version. Ministro needs to be upgraded.
    private static final int BUFFER_SIZE = 1024;

    private ActivityInfo m_activityInfo = null; // activity info object, used to access the libs and the strings
    private DexClassLoader m_classLoader = null; // loader object
    private String[] m_sources = {"https://download.qt-project.org/ministro/android/qt5/qt-5.2"}; // Make sure you are using ONLY secure locations
    private String m_repository = "default"; // Overwrites the default Ministro repository
                                                        // Possible values:
                                                        // * default - Ministro default repository set with "Ministro configuration tool".
                                                        // By default the stable version is used. Only this or stable repositories should
                                                        // be used in production.
                                                        // * stable - stable repository, only this and default repositories should be used
                                                        // in production.
                                                        // * testing - testing repository, DO NOT use this repository in production,
                                                        // this repository is used to push a new release, and should be used to test your application.
                                                        // * unstable - unstable repository, DO NOT use this repository in production,
                                                        // this repository is used to push Qt snapshots.
    private String[] m_qtLibs = null; // required qt libs

    private DownloadManager m_dm;
    private long m_enqueue;

    private static ActivityManager activityManager;

    private Float lastX;
    private Float lastY;

    private Boolean m_GPSServiceStarted = false;
    private GPSServer m_GPSServer;
    public ProgressDialog ringProgressDialog;
    public boolean m_hasGPS;
    private boolean m_backButtonEnable = true;

    private BTScanHelper scanHelper;
    private Boolean m_ScanHelperStarted = false;
    private BluetoothSPP m_BTSPP;
    private Boolean m_BTServiceCreated = false;
    private String m_BTStat;
    private Boolean m_FileChooserDone = false;
    private String m_filechooserString;

    private String m_downloadRet = "";

    OCPNNativeLib nativeLib;

    // action bar
    private ActionBar actionBar;

        // Title navigation Spinner data
    private ArrayList<SpinnerNavItem> navSpinner;
    private SpinnerNavItem spinnerItemRaster;
    private SpinnerNavItem spinnerItemVector;
    private SpinnerNavItem spinnerItemcm93;

        // Navigation adapter
    private TitleNavigationAdapter adapter;

        // Menu item used to indicate "RouteCreate" is active
    MenuItem itemRouteAnnunciator;
    MenuItem itemRouteMenuItem;
    private boolean m_showRouteAnnunciator = false;

    MenuItem itemFollowInActive;
    MenuItem itemFollowActive;
    private boolean m_isFollowActive = false;

    MenuItem itemTrackInActive;
    MenuItem itemTrackActive;
    private boolean m_isTrackActive = false;


    public QtActivity()
    {
        if (Build.VERSION.SDK_INT <= 10) {
            QT_ANDROID_THEMES = new String[] {"Theme_Light"};
            QT_ANDROID_DEFAULT_THEME = "Theme_Light";
        }
        else if (Build.VERSION.SDK_INT >= 11 && Build.VERSION.SDK_INT <= 13) {
            QT_ANDROID_THEMES = new String[] {"Theme_Holo_Light"};
            QT_ANDROID_DEFAULT_THEME = "Theme_Holo_Light";
        } else {
            QT_ANDROID_THEMES = new String[] {"Theme_DeviceDefault_Light"};
            QT_ANDROID_DEFAULT_THEME = "Theme_DeviceDefault_Light";
        }
        m_activity = QtActivity.this;


    }

//    public static QtActivity activity()
//    {
//        return QtActivity.this;
//    }


//    public static  String appData()
//    {
//        Log.d("App Data is ", "Working");
//        return data;
//    }

// This really never gets called, and so dont need m_activity
    public static Activity activity()
    {
//        synchronized (m_mainActivityMutex)
        //Log.i("DEBUGGER_TAG", "activity()");

        {
            return m_activity;
        }
    }

    private String m_settingsReturn;

    public String invokeGoogleMaps(){
        Log.i("DEBUGGER_TAG", "invokeGoogleMaps");


        Intent intent = new Intent(QtActivity.this, org.opencpn.OCPNMapsActivity.class);

        String s = nativeLib.getVPCorners();

        intent.putExtra("VP_CORNERS", s);

        int height = this.getWindow().getDecorView().getHeight();
        int width = this.getWindow().getDecorView().getWidth();
        intent.putExtra("WIDTH", width);
        intent.putExtra("HEIGHT", height);

        startActivityForResult(intent, OCPN_GOOGLEMAPS_REQUEST_CODE);

        int pss = 55;
        String ret;
        ret = String.format("%d", pss);
        return ret;
    }

    public String doAndroidSettings(String settings){
        //Log.i("DEBUGGER_TAG", "doAndroidSettings");
        //Log.i("DEBUGGER_TAG", settings);

        m_settingsReturn = new String();

        Intent intent = new Intent(QtActivity.this, org.opencpn.OCPNSettingsActivity.class);
        intent.putExtra("SETTINGS_STRING",settings);
        startActivityForResult(intent, OCPN_SETTINGS_REQUEST_CODE);

        //Log.i("DEBUGGER_TAG", "after start activity");

        int pss = 55;
        String ret;
        ret = String.format("%d", pss);
        return ret;
    }


    public String checkAndroidSettings(  ){
//        Log.i("DEBUGGER_TAG", "checkAndroidSettings");
//        Log.i("DEBUGGER_TAG", m_settingsReturn);

        return m_settingsReturn;
    }




    public String callFromCpp(int pid){
        //Log.i("DEBUGGER_TAG", "callFromCpp");

//        Intent intent = new Intent(QtActivity.this, org.opencpn.OCPNSettingsActivity.class);
//        startActivity(intent);


        MemoryInfo mi = new MemoryInfo();
//        ActivityManager activityManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
//        activityManager.getMemoryInfo(mi);
        int pids[] = new int[1];
        pids[0] = pid;

        //MemoryInfo[] memoryInfoArray;
        //memoryInfoArray = activityManager.getProcessMemoryInfo( pids );

        android.os.Debug.MemoryInfo[] memoryInfoArray= activityManager.getProcessMemoryInfo( pids );
        int pss = memoryInfoArray[0].getTotalPss();

        String ret;
        ret = String.format("%d", pss);
        return ret;


    }

    public String getMemInfo(int pid){
//        Log.i("DEBUGGER_TAG", "getMemInfo");
        int pids[] = new int[1];
        pids[0] = pid;

        ActivityManager activityManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
        android.os.Debug.MemoryInfo[] memoryInfoArray= activityManager.getProcessMemoryInfo( pids );
        int pss = memoryInfoArray[0].getTotalPss();

        String ret;
        ret = String.format("%d", pss);
        return ret;
    }


    public native String getJniString();
    public native int test();

    public String getDisplayMetrics(){
        //Log.i("DEBUGGER_TAG", "getDisplayDPI");
/*
        int i = nativeLib.test();
        String aa;
        aa = String.format("%d", i);
        Log.i("DEBUGGER_TAG", aa);

        String bb = "$GPRMC...";
        int j = nativeLib.processNMEA(bb);
//      int j = nativeLib.processNMEA( 44);
        aa = String.format("%d", j);
        Log.i("DEBUGGER_TAG", aa);
*/
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        int statusBarHeight = 0;

        int resourceId = getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            statusBarHeight = getResources().getDimensionPixelSize(resourceId);
        }

//        TypedValue typedValue = new TypedValue();
//        if(getTheme().resolveAttribute(android.R.attr.actionBarSize, typedValue, true)){
//            screen_h -= getResources().getDimensionPixelSize(typedValue.resourceId);
//        }

        int actionBarHeight = 0;
        ActionBar actionBar = getActionBar();
        if(actionBar.isShowing())
            actionBarHeight = actionBar.getHeight();

//            float getTextSize() //pixels
        int width = 600;
        int height = 400;

        Display display = getWindowManager().getDefaultDisplay();

        if (Build.VERSION.SDK_INT >= 13) {

            if(Build.VERSION.SDK_INT >= 17){
                //Log.i("DEBUGGER_TAG", "VERSION.SDK_INT >= 17");
                width = dm.widthPixels;
                height = dm.heightPixels;
            }
            else{

                switch (Build.VERSION.SDK_INT){

                    case 16:
                        //Log.i("DEBUGGER_TAG", "VERSION.SDK_INT == 16");
                        width = dm.widthPixels;
                        height = dm.heightPixels;
                        break;

                    case 15:
                    case 14:
                        Point outPoint = new Point();
                        display.getRealSize(outPoint);
                        if (outPoint != null){
                            width = outPoint.x;
                            height = outPoint.y;
                        }
                    break;

                    default:
                        width = dm.widthPixels;
                        height = dm.heightPixels;
                        break;

                }
            }
        }
        else{
            //Log.i("DEBUGGER_TAG", "VERSION.SDK_INT < 13");
            width = display.getWidth();
            height = display.getHeight();
        }


        float tsize = new Button(this).getTextSize();       // in pixels

        String ret;

        ret = String.format("%f;%f;%d;%d;%d;%d;%d;%d;%d;%d;%f", dm.xdpi, dm.density, dm.densityDpi,
               width, height - statusBarHeight,
               width, height,
               dm.widthPixels, dm.heightPixels, actionBarHeight, tsize);

        //Log.i("DEBUGGER_TAG", ret);



        return ret;
    }

    public String getDeviceInfo(){
        String s="Device Info:";
                s += "\n OS Version: " + System.getProperty("os.version") + "(" + android.os.Build.VERSION.INCREMENTAL + ")";
                s += "\n OS API Level: "+android.os.Build.VERSION.RELEASE + "("+android.os.Build.VERSION.SDK_INT+")";
                s += "\n Device: " + android.os.Build.DEVICE;
                s += "\n Model (and Product): " + android.os.Build.MODEL + " ("+ android.os.Build.PRODUCT + ")";

        //Log.i("DEBUGGER_TAG", s);

        return s;
    }


    public String showBusyCircle(){
//        Log.i("DEBUGGER_TAG", "show");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

//                 QtActivity.this.ringProgressDialog.show(QtActivity.this, "", "", true);

                 ringProgressDialog = new ProgressDialog(QtActivity.this,R.style.MyTheme);
                 ringProgressDialog.setCancelable(false);
                 ringProgressDialog.setProgressStyle(android.R.style.Widget_ProgressBar_Small);

                 Drawable myIcon = getResources().getDrawable( R.drawable.progressbar_custom );
                 ringProgressDialog.setIndeterminateDrawable(myIcon);

                 QtActivity.this.ringProgressDialog.show();

             }});

        String ret = "";
        return ret;
    }

    public String hideBusyCircle(){
//        Log.i("DEBUGGER_TAG", "hide");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                 ringProgressDialog.dismiss();

             }});



        String ret = "";
        return ret;
    }


    public String setRouteAnnunciator( final int viz){
     //Log.i("DEBUGGER_TAG", "setRouteAnnunciator");

     m_showRouteAnnunciator = (viz != 0);

//    if( null != itemRouteAnnunciator)
    {
        //Log.i("DEBUGGER_TAG", "setRouteAnnunciatorA");
        //if(viz == 0)
            //Log.i("DEBUGGER_TAG", "setRouteAnnunciatorB");
        //else
            //Log.i("DEBUGGER_TAG", "setRouteAnnunciatorC");

        runOnUiThread(new Runnable() {
                @Override
                public void run() {

//                    itemRouteAnnunciator.setVisible(viz != 0);
                    QtActivity.this.invalidateOptionsMenu();

                 }});

//        itemRouteAnnunciator.setVisible(viz != 0);
//        this.invalidateOptionsMenu();
        return "OK";
     }
//     else
//        return "NO";
    }

    public String setFollowIconState( final int isActive){
        m_isFollowActive = (isActive != 0);

        //if(isActive == 0)
            //Log.i("DEBUGGER_TAG", "setFollowIconStateA");
        //else
            //Log.i("DEBUGGER_TAG", "setFollowIconStateB");

           runOnUiThread(new Runnable() {
                   @Override
                   public void run() {


                       QtActivity.this.invalidateOptionsMenu();

                    }});

           return "OK";
       }

       public String setTrackIconState( final int isActive){
           m_isTrackActive = (isActive != 0);

           //if(isActive == 0)
               //Log.i("DEBUGGER_TAG", "setTrackIconStateA");
           //else
               //Log.i("DEBUGGER_TAG", "setTrackIconStateB");

              runOnUiThread(new Runnable() {
                      @Override
                      public void run() {


                          QtActivity.this.invalidateOptionsMenu();

                       }});

              return "OK";
          }



       public String setBackButtonState( final int isActive){
           //Log.i("DEBUGGER_TAG", "setBackButtonState");
           m_backButtonEnable = (isActive != 0);
           return "OK";
          }


    public String queryGPSServer( final int parm ){

        if( GPSServer.GPS_PROVIDER_AVAILABLE == parm){
            String ret_string = "NO";
            if( m_hasGPS )
                ret_string = "YES";
            return ret_string;
        }



        if(!m_GPSServiceStarted){
            //Log.i("DEBUGGER_TAG", "Start GPS Server");
            m_GPSServer = new GPSServer(getApplicationContext(), nativeLib, this);
            m_GPSServiceStarted = true;
        }

        return m_GPSServer.doService( parm );
    }

    public String hasBluetooth( final int parm ){
        String ret = "Yes";
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            ret = "No";
        }

//        PackageManager pm = context.getPackageManager();
//        boolean hasBluetooth = pm.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH);
        return ret;
    }

    public String startBlueToothScan( final int parm ){
        Log.i("DEBUGGER_TAG", "startBlueToothScan");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                if(!m_ScanHelperStarted){
                    scanHelper = new BTScanHelper(QtActivity.this);
                    m_ScanHelperStarted = true;
                }

                scanHelper.doDiscovery();

             }});


        return( "OK" );

    }

    public String stopBlueToothScan( final int parm ){
        Log.i("DEBUGGER_TAG", "stopBlueToothScan");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                if(m_ScanHelperStarted){
                    scanHelper.doDiscovery();
                    scanHelper.stopDiscovery();
                }


             }});


        return( "OK" );

    }

    public String getBlueToothScanResults( final int parm ){
        String ret_str = "";

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                if(!m_ScanHelperStarted){
                    scanHelper = new BTScanHelper(QtActivity.this);
                    m_ScanHelperStarted = true;
                }


             }});

        if(m_ScanHelperStarted)
            ret_str = scanHelper.getDiscoveredDevices();;

        Log.i("DEBUGGER_TAG", "results");
        Log.i("DEBUGGER_TAG", ret_str);

        return ret_str;

   //     return ("line A;line B;"); //scanHelper.getDiscoveredDevices();


    }


    public String startBTService( final String address){
        Log.i("DEBUGGER_TAG", "startBTService");
        Log.i("DEBUGGER_TAG", address);
        m_BTStat = "Unknown";

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

///
                if(!m_BTServiceCreated){
                    Log.i("DEBUGGER_TAG", "Bluetooth createBTService");
                    m_BTSPP = new BluetoothSPP(getApplicationContext());

                    if(!m_BTSPP.isBluetoothAvailable() || !m_BTSPP.isBluetoothEnabled()) {
 //                           Toast.makeText(getApplicationContext()
 //                                           , "Bluetooth is not available"
 //                                           , Toast.LENGTH_SHORT).show();
                    }

                    else {
                        m_BTSPP.setupService();
                        m_BTServiceCreated = true;
                    }
                }


                m_BTSPP.setOnDataReceivedListener(new OnDataReceivedListener() {
                    public void onDataReceived(byte[] data, String message) {
                        Log.i("DEBUGGER_TAG", message);
                        // Do something when data incoming
                        nativeLib.processBTNMEA( message );

                    }
                });

                if(m_BTSPP.isServiceAvailable()){
                    Log.i("DEBUGGER_TAG", "Bluetooth startService");
                    m_BTSPP.startService(BluetoothState.DEVICE_OTHER);

                    Log.i("DEBUGGER_TAG", "Bluetooth connectA");
//                    m_BTSPP.connect(address);
                    m_BTSPP.autoConnectAddress(address);

                }

                if(!m_BTSPP.isBluetoothEnabled())
                    m_BTStat = "NOK.BTNotEnabled";
                else if(!m_BTSPP.isServiceAvailable())
                    m_BTStat = "NOK.ServiceNotAvailable";
                else
                    m_BTStat = "OK";



             }});


        Log.i("DEBUGGER_TAG", "startBTService return: " + m_BTStat);
        return m_BTStat;
    }


    public String stopBTService( final int parm){
        Log.i("DEBUGGER_TAG", "stopBTService");
        String ret_str = "";

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                if(m_BTServiceCreated){
                    Log.i("DEBUGGER_TAG", "Bluetooth stopService");
                    m_BTSPP.stopService();
                }


             }});

        ret_str = "OK";
        return ret_str;
    }

    private Semaphore mutex = new Semaphore(0);

    public String downloadFile( final String url, final String destination )
    {
        m_downloadRet = "";

        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.i("DEBUGGER_TAG", "onReceive: " + action);

                if (DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(action)) {
                    long downloadId = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, 0);

                    Query query = new Query();
                    query.setFilterById(m_enqueue);
                    Cursor c = m_dm.query(query);


                    if (c.moveToFirst()) {
                        String uriString = c.getString(c.getColumnIndex(DownloadManager.COLUMN_URI));

                        int columnIndex = c.getColumnIndex(DownloadManager.COLUMN_STATUS);
                        if (DownloadManager.STATUS_SUCCESSFUL == c.getInt(columnIndex)) {
                            Log.i("DEBUGGER_TAG", "Download successful");
                        }

                        nativeLib.setDownloadStatus( c.getInt(columnIndex), uriString);


                    }
                }
            }
        };

        registerReceiver(receiver, new IntentFilter(
                DownloadManager.ACTION_DOWNLOAD_COMPLETE));


        mutex = new Semaphore(0);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_dm = (DownloadManager) getSystemService(DOWNLOAD_SERVICE);

                Request request = new Request( Uri.parse(url) );
                request.setDestinationUri( Uri.parse(destination) );

                Log.i("DEBUGGER_TAG", "enqueue");
                m_downloadRet = "PENDING";
                try{
                     m_enqueue = m_dm.enqueue(request);
                     String result = "OK;" + String.valueOf(m_enqueue);
                     Log.i("DEBUGGER_TAG", result);
                     m_downloadRet = result;
                 }
                 catch(Exception e){
                     m_downloadRet = "NOK";
                     Log.i("DEBUGGER_TAG", "exception");
                 }


                 mutex.release();


             }});

        // One way to wait for the runnable to be done...
        try {
            mutex.acquire();            // Cannot get mutex until runnable above exits.
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        Log.i("DEBUGGER_TAG", "m_downloadRet " + m_downloadRet);
        return m_downloadRet;
    }

    public String getDownloadStatus( final int ID )
    {
        Log.i("DEBUGGER_TAG", "getDownloadStatus "  + String.valueOf(ID));

        String ret = "NOSTAT";
        if(m_dm != null){
            Log.i("DEBUGGER_TAG", "mdm");
            Query query = new Query();
            query.setFilterById(ID);
            Cursor c = m_dm.query(query);

            if (c.moveToFirst()) {
                Log.i("DEBUGGER_TAG", "cmtf");
                int columnIndex = c.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int stat = c.getInt(columnIndex);
                String sstat = String.valueOf(stat);

                String sofarBytes = c.getString(c.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR ));
                String totalBytes = c.getString(c.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES));

                ret =  sstat + ";" + sofarBytes + ";" + totalBytes;

            }

        }

        Log.i("DEBUGGER_TAG", ret);
        return ret;
    }


    public String cancelDownload( final int ID )
    {
        Log.i("DEBUGGER_TAG", "cancelDownload "  + String.valueOf(ID));
        if(m_dm != null){
            m_dm.remove( ID );
        }

        return "OK";
    }

    public String FileChooserDialog(final String initialDir, final String Title, final String Suggestion, final String wildcard)
    {
        //Log.i("DEBUGGER_TAG", "FileChooserDialog");
        //Log.i("DEBUGGER_TAG", initialDir);

        m_FileChooserDone = false;

        boolean buseDialog = true;
        if(!buseDialog){
            Intent intent = new Intent(this, FileChooserActivity.class);
            intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, initialDir);
            intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, false);
            intent.putExtra(FileChooserActivity.INPUT_SHOW_FULL_PATH_IN_TITLE, true);
            intent.putExtra(FileChooserActivity.INPUT_TITLE_STRING, Title);


        //  Creating a file?
            if(!Suggestion.isEmpty()){
                //Log.i("DEBUGGER_TAG", "FileChooserDialog Creating");
                intent.putExtra(FileChooserActivity.INPUT_CAN_CREATE_FILES, true);
            }

            this.startActivityForResult(intent, OCPN_AFILECHOOSER_REQUEST_CODE);
        }

        //Log.i("DEBUGGER_TAG", "FileChooserDialog create and show " + initialDir);

        Thread thread = new Thread() {
            @Override
            public void run() {

        // Block this thread for 20 msec.
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                }

// After sleep finishes blocking, create a Runnable to run on the UI Thread.
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        FileChooserDialog dialog = new FileChooserDialog(m_activity, initialDir);

                        dialog.setShowFullPath( true );
                        dialog.setTitle( Title );

                        dialog.addListener(new FileChooserDialog.OnFileSelectedListener() {
                            public void onFileSelected(Dialog source, File file) {
                                source.hide();
                                //Toast toast = Toast.makeText(source.getContext(), "File selected: " + file.getName(), Toast.LENGTH_LONG);
                                //toast.show();

                                m_filechooserString = "file:" + file.getPath();
                                m_FileChooserDone = true;

                            }
                            public void onFileSelected(Dialog source, File folder, String name) {
                                source.hide();
                                //Toast toast = Toast.makeText(source.getContext(), "File created: " + folder.getName() + "/" + name, Toast.LENGTH_LONG);
                                //toast.show();

                                m_filechooserString = "file:" + folder.getPath() + "/" + name;
                                m_FileChooserDone = true;

                            }
                        });

                        dialog.setOnCancelListener(new OnCancelListener() {
                            public void onCancel(DialogInterface dialog) {
                                //Log.i("DEBUGGER_TAG", "FileChooserDialog Cancel");
                                m_filechooserString = "cancel:";
                                m_FileChooserDone = true;
                            }
                        });


                        dialog.setCanCreateFiles(true);
                        dialog.show();

                        //Log.i("DEBUGGER_TAG", "FileChooserDialog Back from show");

                    }
                });
            }
        };

        // Don't forget to start the thread.
        thread.start();

        //Log.i("DEBUGGER_TAG", "FileChooserDialog Returning");

        return "OK";
   }

   public String DirChooserDialog(final String initialDir, final String Title)
   {
       m_FileChooserDone = false;

       boolean buseDialog = true;
       if(!buseDialog){
            Intent intent = new Intent(this, FileChooserActivity.class);
            intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, initialDir);
            intent.putExtra(FileChooserActivity.INPUT_FOLDER_MODE, true);
            this.startActivityForResult(intent, OCPN_AFILECHOOSER_REQUEST_CODE);
        }

        //Log.i("DEBUGGER_TAG", "DirChooserDialog create and show " + initialDir);

        Thread thread = new Thread() {
            @Override
            public void run() {

        // Block this thread for 20 msec.
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                }

// After sleep finishes blocking, create a Runnable to run on the UI Thread.
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        FileChooserDialog dialog = new FileChooserDialog(m_activity, initialDir);

                        dialog.setShowFullPath( true );
                        dialog.setFolderMode( true );
                        dialog.setCanCreateFiles( true );


                        dialog.setTitle( Title );

                        dialog.addListener(new FileChooserDialog.OnFileSelectedListener() {
                            public void onFileSelected(Dialog source, File file) {
                                source.hide();
                                //Toast toast = Toast.makeText(source.getContext(), "File selected: " + file.getName(), Toast.LENGTH_LONG);
                                //toast.show();

                                m_filechooserString = "file:" + file.getPath();
                                m_FileChooserDone = true;

                            }
                            public void onFileSelected(Dialog source, File folder, String name) {
                                source.hide();
                                m_FileChooserDone = true;
                            }

                        });

                        dialog.setOnCancelListener(new OnCancelListener() {
                            public void onCancel(DialogInterface dialog) {
                                //Log.i("DEBUGGER_TAG", "DirChooserDialog Cancel");
                                m_filechooserString = "cancel:";
                                m_FileChooserDone = true;
                            }
                        });


                        dialog.show();

                        //Log.i("DEBUGGER_TAG", "DirChooserDialog Back from show");

                    }
                });
            }
        };

        // Don't forget to start the thread.
        thread.start();

        //Log.i("DEBUGGER_TAG", "DirChooserDialog Returning");

       return "OK";
  }

   public String isFileChooserFinished()
   {
       if(m_FileChooserDone){
            return m_filechooserString;
       }
       else{
           return "no";
       }
   }

   // ActionBar Spinner navigation to select chart display type

   //  Thread safe version, callable from another thread
   public String configureNavSpinnerTS(final int flag, final int sel){
       Thread thread = new Thread() {
            @Override
            public void run() {

       // Block this thread for 20 msec.
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                }

       // After sleep finished blocking, create a Runnable to run on the UI Thread.
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        configureNavSpinner(flag, sel);
                    }
                });
            }
       };

       // Don't forget to start the thread.
       thread.start();

       return "OK";
   }



   public String configureNavSpinner(int flag, int sel){
       //Log.i("DEBUGGER_TAG", "configureNavSpinner");
       //String aa; aa = String.format("%d %d", flag, sel ); Log.i("DEBUGGER_TAG", aa);

       navSpinner.clear();
       int nbits = 0;
       int n93 = -1;
       int nraster = -1;
       int nvector = -1;

       if((flag & 1) == 1){
           nraster = nbits;
           navSpinner.add(spinnerItemRaster);
           nbits++;
       }
       if((flag & 2) == 2){
           nvector = nbits;
           navSpinner.add(spinnerItemVector);
           nbits++;
       }
       if((flag & 4) == 4){
           n93 = nbits;
           navSpinner.add(spinnerItemcm93);
           nbits++;
       }



       // Select the proper item as directed
       int to_sel = 0;
       if(sel == 1)
            to_sel = nraster;
       else if(sel == 2)
            to_sel = nvector;
       else if(sel == 4)
            to_sel = n93;

       // Any bits set?
       if(nbits > 1){
           actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
           actionBar.setSelectedNavigationItem(to_sel);
       }
       else
           actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);

       return "OK";
   }

   public String getSystemDirs(){
       String result = "";

       ApplicationInfo ai = getApplicationInfo();
       if((ai.flags & ApplicationInfo.FLAG_EXTERNAL_STORAGE) ==  ApplicationInfo.FLAG_EXTERNAL_STORAGE){
           //Log.i("DEBUGGER_TAG", "External");
           result = "EXTAPP;";
       }
       else{
           //Log.i("DEBUGGER_TAG", "Internal");
           result = "INTAPP;";
       }



       result = result.concat(getFilesDir().getPath() + ";");
       result = result.concat(getCacheDir().getPath() + ";");
       result = result.concat(getExternalFilesDir(null).getPath() + ";");
       result = result.concat(getExternalCacheDir().getPath() + ";");
       result = result.concat(Environment.getExternalStorageDirectory().getPath() + ";");

       //Log.i("DEBUGGER_TAG", result);

       return result;
   }




//   @Override
//   public void onTabSelected(ActionBar.Tab tab,
//       FragmentTransaction fragmentTransaction) {

//   Log.i("DEBUGGER_TAG", "onTabSelected");
//   Log.i("DEBUGGER_TAG", tab.getText().toString());

     // When the given tab is selected, show the tab contents in the
     // container view.
//     Fragment fragment = new DummySectionFragment();
//     Bundle args = new Bundle();
//     args.putInt(DummySectionFragment.ARG_SECTION_NUMBER,
//         tab.getPosition() + 1);
//     fragment.setArguments(args);
//     getFragmentManager().beginTransaction()
//         .replace(R.id.container, fragment).commit();
//   }

//   @Override
//   public void onTabUnselected(ActionBar.Tab tab,
//       FragmentTransaction fragmentTransaction) {
//   }

//   @Override
//   public void onTabReselected(ActionBar.Tab tab,
//       FragmentTransaction fragmentTransaction) {
//   }


   //  ActionBar drop-down spinner navigation
   @Override
   public boolean onNavigationItemSelected(int itemPosition, long itemId) {
   // Action to be taken after selecting a spinner item from action bar

   //Log.i("DEBUGGER_TAG", "onNavigationItemSelected");
   String aa;
   aa = String.format("%d", itemPosition);
   //Log.i("DEBUGGER_TAG", aa);

        SpinnerNavItem item = navSpinner.get(itemPosition);
        if(item.getTitle().equalsIgnoreCase("cm93")){
            nativeLib.selectChartDisplay(CHART_TYPE_CM93COMP, -1);
            return true;
        }
        else if(item.getTitle().equalsIgnoreCase("raster")){
            nativeLib.selectChartDisplay(-1, CHART_FAMILY_RASTER);
            //Log.i("DEBUGGER_TAG", "onNavigationItemSelectedA");
            return true;
        }
        else if(item.getTitle().equalsIgnoreCase("vector")){
            nativeLib.selectChartDisplay(-1, CHART_FAMILY_VECTOR);
            return true;
        }


       return false;
   }

   private void relocateOCPNPlugins( )
   {
       Log.i("DEBUGGER_TAG", "relocateOCPNPlugins");

       // TODO this should be a recursive directory search
       String path = "/data/data/org.opencpn.opencpn/files/plugins/chartdldr_pi/data";

       Log.d("DEBUGGER_TAG", "Plugin source Path: " + path);
       File f = new File(path);
       File file[] = f.listFiles();

       if(null != file){
         for (int i=0; i < file.length; i++){
             Log.d("DEBUGGER_TAG", "Plugin FileName:" + file[i].getName());
            String source = file[i].getAbsolutePath();
            String dest = "/data/data/org.opencpn.opencpn/" + file[i].getName();


            try {
                InputStream inputStream = new FileInputStream(source);
                OutputStream outputStream = new FileOutputStream(dest);
                copyFile(inputStream, outputStream);
                inputStream.close();
                outputStream.close();
                Log.i("DEBUGGER_TAG", "copyFile OK: " + dest);
            }
            catch (Exception e) {
                e.printStackTrace();
                Log.i("DEBUGGER_TAG", "copyFile Exception");
            }


         }
       }



   }


    // this function is used to load and start the loader
    private void loadApplication(Bundle loaderParams)
    {
        //Log.i("DEBUGGER_TAG", "LoadApplication");

        relocateOCPNPlugins();


        try {
            final int errorCode = loaderParams.getInt(ERROR_CODE_KEY);
            if (errorCode != 0) {
                if (errorCode == INCOMPATIBLE_MINISTRO_VERSION) {
                    downloadUpgradeMinistro(loaderParams.getString(ERROR_MESSAGE_KEY));
                    return;
                }

                // fatal error, show the error and quit
                AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();
                errorDialog.setMessage(loaderParams.getString(ERROR_MESSAGE_KEY));
                errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                });
                errorDialog.show();
                return;
            }

            // add all bundled Qt libs to loader params
            ArrayList<String> libs = new ArrayList<String>();
            if ( m_activityInfo.metaData.containsKey("android.app.bundled_libs_resource_id") )
                libs.addAll(Arrays.asList(getResources().getStringArray(m_activityInfo.metaData.getInt("android.app.bundled_libs_resource_id"))));

                //  We want the default OCPN plugins bundled into the APK and installed
                //  into the proper app-lib.  So they are listed in ANDROID_EXTRA_LIBS.
                //  But we do not want to pre-load them.  So take them out of the DexClassLoader list.

//                libs.remove("dashboard_pi");
//                libs.remove("grib_pi");




            String libName = null;
            if ( m_activityInfo.metaData.containsKey("android.app.lib_name") ) {
                libName = m_activityInfo.metaData.getString("android.app.lib_name");
                loaderParams.putString(MAIN_LIBRARY_KEY, libName); //main library contains main() function
            }

            loaderParams.putStringArrayList(BUNDLED_LIBRARIES_KEY, libs);
            loaderParams.putInt(NECESSITAS_API_LEVEL_KEY, NECESSITAS_API_LEVEL);

            // load and start QtLoader class
            m_classLoader = new DexClassLoader(loaderParams.getString(DEX_PATH_KEY), // .jar/.apk files
                                               getDir("outdex", Context.MODE_PRIVATE).getAbsolutePath(), // directory where optimized DEX files should be written.
                                               loaderParams.containsKey(LIB_PATH_KEY) ? loaderParams.getString(LIB_PATH_KEY) : null, // libs folder (if exists)
                                               getClassLoader()); // parent loader

            @SuppressWarnings("rawtypes")
            Class loaderClass = m_classLoader.loadClass(loaderParams.getString(LOADER_CLASS_NAME_KEY)); // load QtLoader class
            Object qtLoader = loaderClass.newInstance(); // create an instance
            Method perpareAppMethod = qtLoader.getClass().getMethod("loadApplication",
                                                                    Activity.class,
                                                                    ClassLoader.class,
                                                                    Bundle.class);
            if (!(Boolean)perpareAppMethod.invoke(qtLoader, this, m_classLoader, loaderParams))
                throw new Exception("");

            QtApplication.setQtActivityDelegate(qtLoader);

            // now load the application library so it's accessible from this class loader
            if (libName != null)
                System.loadLibrary(libName);

            Method startAppMethod=qtLoader.getClass().getMethod("startApplication");
            if (!(Boolean)startAppMethod.invoke(qtLoader))
                throw new Exception("");


        } catch (Exception e) {
            e.printStackTrace();
            AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();
            if (m_activityInfo.metaData.containsKey("android.app.fatal_error_msg"))
                errorDialog.setMessage(m_activityInfo.metaData.getString("android.app.fatal_error_msg"));
            else
                errorDialog.setMessage("Fatal error, your application can't be started.");

            errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });
            errorDialog.show();
        }
    }

    private ServiceConnection m_ministroConnection=new ServiceConnection() {
        private IMinistro m_service = null;
        @Override
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            m_service = IMinistro.Stub.asInterface(service);
            try {
                if (m_service != null) {
                    Bundle parameters = new Bundle();
                    parameters.putStringArray(REQUIRED_MODULES_KEY, m_qtLibs);
                    parameters.putString(APPLICATION_TITLE_KEY, (String)QtActivity.this.getTitle());
                    parameters.putInt(MINIMUM_MINISTRO_API_KEY, MINISTRO_API_LEVEL);
                    parameters.putInt(MINIMUM_QT_VERSION_KEY, QT_VERSION);
                    parameters.putString(ENVIRONMENT_VARIABLES_KEY, ENVIRONMENT_VARIABLES);
                    if (APPLICATION_PARAMETERS != null)
                        parameters.putString(APPLICATION_PARAMETERS_KEY, APPLICATION_PARAMETERS);
                    parameters.putStringArray(SOURCES_KEY, m_sources);
                    parameters.putString(REPOSITORY_KEY, m_repository);
                    if (QT_ANDROID_THEMES != null)
                        parameters.putStringArray(ANDROID_THEMES_KEY, QT_ANDROID_THEMES);
                    m_service.requestLoader(m_ministroCallback, parameters);
                }
            } catch (RemoteException e) {
                    e.printStackTrace();
            }
        }

        private IMinistroCallback m_ministroCallback = new IMinistroCallback.Stub() {
            // this function is called back by Ministro.
            @Override
            public void loaderReady(final Bundle loaderParams) throws RemoteException {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        unbindService(m_ministroConnection);
                        loadApplication(loaderParams);
                    }
                });
            }
        };

        @Override
        public void onServiceDisconnected(ComponentName name) {
            m_service = null;
        }
    };

    private void downloadUpgradeMinistro(String msg)
    {
        AlertDialog.Builder downloadDialog = new AlertDialog.Builder(this);
        downloadDialog.setMessage(msg);
        downloadDialog.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                try {
                    Uri uri = Uri.parse("market://search?q=pname:org.kde.necessitas.ministro");
                    Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                    startActivityForResult(intent, MINISTRO_INSTALL_REQUEST_CODE);
                } catch (Exception e) {
                    e.printStackTrace();
                    ministroNotFound();
                }
            }
        });

        downloadDialog.setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                QtActivity.this.finish();
            }
        });
        downloadDialog.show();
    }

    private void ministroNotFound()
    {
        AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();

        if (m_activityInfo.metaData.containsKey("android.app.ministro_not_found_msg"))
            errorDialog.setMessage(m_activityInfo.metaData.getString("android.app.ministro_not_found_msg"));
        else
            errorDialog.setMessage("Can't find Ministro service.\nThe application can't start.");

        errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        errorDialog.show();
    }

    static private void copyFile(InputStream inputStream, OutputStream outputStream)
        throws IOException
    {
        byte[] buffer = new byte[BUFFER_SIZE];

        int count;
        while ((count = inputStream.read(buffer)) > 0)
            outputStream.write(buffer, 0, count);
    }


    private void copyAsset(String source, String destination)
        throws IOException
    {
        // Already exists, we don't have to do anything
        File destinationFile = new File(destination);
        if (destinationFile.exists())
            return;

        File parentDirectory = destinationFile.getParentFile();
        if (!parentDirectory.exists())
            parentDirectory.mkdirs();

        destinationFile.createNewFile();

        AssetManager assetsManager = getAssets();
        InputStream inputStream = assetsManager.open(source);
        OutputStream outputStream = new FileOutputStream(destinationFile);
        copyFile(inputStream, outputStream);

        inputStream.close();
        outputStream.close();
    }

    private static void createBundledBinary(String source, String destination)
        throws IOException
    {
        // Already exists, we don't have to do anything
        File destinationFile = new File(destination);
        if (destinationFile.exists())
            return;

        File parentDirectory = destinationFile.getParentFile();
        if (!parentDirectory.exists())
            parentDirectory.mkdirs();

        destinationFile.createNewFile();

        InputStream inputStream = new FileInputStream(source);
        OutputStream outputStream = new FileOutputStream(destinationFile);
        copyFile(inputStream, outputStream);

        inputStream.close();
        outputStream.close();
    }

    private boolean cleanCacheIfNecessary(String pluginsPrefix, long packageVersion)
    {
        //Log.i("DEBUGGER_TAG", "cleanCacheIfNecessary" + pluginsPrefix);
        File versionFile = new File(pluginsPrefix + "cache.version");

        long cacheVersion = 0;
        if (versionFile.exists() && versionFile.canRead()) {
            try {
                DataInputStream inputStream = new DataInputStream(new FileInputStream(versionFile));
                cacheVersion = inputStream.readLong();
                inputStream.close();
             } catch (Exception e) {
                e.printStackTrace();
             }
        }

        if (cacheVersion != packageVersion) {
            deleteRecursively(new File(pluginsPrefix));
            //Log.i("DEBUGGER_TAG", "cleanCacheIfNecessary return true");
            return true;
        } else {
            //Log.i("DEBUGGER_TAG", "cleanCacheIfNecessary return false");

            return false;
        }
    }

    private void extractBundledPluginsAndImports(String pluginsPrefix)
        throws IOException
    {
        ArrayList<String> libs = new ArrayList<String>();

        String dataDir = getApplicationInfo().dataDir + "/";

        long packageVersion = -1;
        try {
            PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
            packageVersion = packageInfo.lastUpdateTime;
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (!cleanCacheIfNecessary(pluginsPrefix, packageVersion))
            return;

        {
            File versionFile = new File(pluginsPrefix + "cache.version");

            File parentDirectory = versionFile.getParentFile();
            if (!parentDirectory.exists())
                parentDirectory.mkdirs();

            versionFile.createNewFile();

            DataOutputStream outputStream = new DataOutputStream(new FileOutputStream(versionFile));
            outputStream.writeLong(packageVersion);
            outputStream.close();
        }

        {
            String key = BUNDLED_IN_LIB_RESOURCE_ID_KEY;
            java.util.Set<String> keys = m_activityInfo.metaData.keySet();
            if (m_activityInfo.metaData.containsKey(key)) {
                String[] list = getResources().getStringArray(m_activityInfo.metaData.getInt(key));

                for (String bundledImportBinary : list) {
                    String[] split = bundledImportBinary.split(":");
                    String sourceFileName = dataDir + "lib/" + split[0];
                    String destinationFileName = pluginsPrefix + split[1];
                    createBundledBinary(sourceFileName, destinationFileName);
                }
            }
        }

        {
            String key = BUNDLED_IN_ASSETS_RESOURCE_ID_KEY;
            if (m_activityInfo.metaData.containsKey(key)) {
                String[] list = getResources().getStringArray(m_activityInfo.metaData.getInt(key));

                for (String fileName : list) {
                    String[] split = fileName.split(":");
                    String sourceFileName = split[0];
                    String destinationFileName = pluginsPrefix + split[1];
                    copyAsset(sourceFileName, destinationFileName);
                }
            }

        }
    }

    private void deleteRecursively(File directory)
    {
        File[] files = directory.listFiles();
        if (files != null) {
            for (File file : files) {
                if (file.isDirectory())
                    deleteRecursively(file);
                else
                    file.delete();
            }

            directory.delete();
        }
    }

    private void cleanOldCacheIfNecessary(String oldLocalPrefix, String localPrefix)
    {
        File newCache = new File(localPrefix);
        if (!newCache.exists()) {
            {
                File oldPluginsCache = new File(oldLocalPrefix + "plugins/");
                if (oldPluginsCache.exists() && oldPluginsCache.isDirectory())
                    deleteRecursively(oldPluginsCache);
            }

            {
                File oldImportsCache = new File(oldLocalPrefix + "imports/");
                if (oldImportsCache.exists() && oldImportsCache.isDirectory())
                    deleteRecursively(oldImportsCache);
            }

            {
                File oldQmlCache = new File(oldLocalPrefix + "qml/");
                if (oldQmlCache.exists() && oldQmlCache.isDirectory())
                    deleteRecursively(oldQmlCache);
            }
        }
    }

    private void startApp(final boolean firstStart)
    {
        try {
            if (m_activityInfo.metaData.containsKey("android.app.qt_sources_resource_id")) {
                int resourceId = m_activityInfo.metaData.getInt("android.app.qt_sources_resource_id");
                m_sources = getResources().getStringArray(resourceId);
            }

            if (m_activityInfo.metaData.containsKey("android.app.repository"))
                m_repository = m_activityInfo.metaData.getString("android.app.repository");

            if (m_activityInfo.metaData.containsKey("android.app.qt_libs_resource_id")) {
                int resourceId = m_activityInfo.metaData.getInt("android.app.qt_libs_resource_id");
                m_qtLibs = getResources().getStringArray(resourceId);
            }

            if (m_activityInfo.metaData.containsKey("android.app.use_local_qt_libs")
                    && m_activityInfo.metaData.getInt("android.app.use_local_qt_libs") == 1) {
                ArrayList<String> libraryList = new ArrayList<String>();


                String localPrefix = "/data/local/tmp/qt/";
                if (m_activityInfo.metaData.containsKey("android.app.libs_prefix"))
                    localPrefix = m_activityInfo.metaData.getString("android.app.libs_prefix");

                String pluginsPrefix = localPrefix;

                boolean bundlingQtLibs = false;
                if (m_activityInfo.metaData.containsKey("android.app.bundle_local_qt_libs")
                    && m_activityInfo.metaData.getInt("android.app.bundle_local_qt_libs") == 1) {
                    localPrefix = getApplicationInfo().dataDir + "/";
                    pluginsPrefix = localPrefix + "qt-reserved-files/";
                    cleanOldCacheIfNecessary(localPrefix, pluginsPrefix);
                    extractBundledPluginsAndImports(pluginsPrefix);
                    bundlingQtLibs = true;
                }

                if (m_qtLibs != null) {
                    for (int i=0;i<m_qtLibs.length;i++) {
                        libraryList.add(localPrefix
                                        + "lib/lib"
                                        + m_qtLibs[i]
                                        + ".so");
                    }
                }

                if (m_activityInfo.metaData.containsKey("android.app.load_local_libs")) {
                    String[] extraLibs = m_activityInfo.metaData.getString("android.app.load_local_libs").split(":");
                    for (String lib : extraLibs) {
                        if (lib.length() > 0) {
                            if (lib.startsWith("lib/"))
                                libraryList.add(localPrefix + lib);
                            else
                                libraryList.add(pluginsPrefix + lib);
                        }
                    }
                }


                String dexPaths = new String();
                String pathSeparator = System.getProperty("path.separator", ":");
                if (!bundlingQtLibs && m_activityInfo.metaData.containsKey("android.app.load_local_jars")) {
                    String[] jarFiles = m_activityInfo.metaData.getString("android.app.load_local_jars").split(":");
                    for (String jar:jarFiles) {
                        if (jar.length() > 0) {
                            if (dexPaths.length() > 0)
                                dexPaths += pathSeparator;
                            dexPaths += localPrefix + jar;
                        }
                    }
                }

                Bundle loaderParams = new Bundle();
                loaderParams.putInt(ERROR_CODE_KEY, 0);
                loaderParams.putString(DEX_PATH_KEY, dexPaths);
                loaderParams.putString(LOADER_CLASS_NAME_KEY, "org.qtproject.qt5.android.QtActivityDelegate");
                if (m_activityInfo.metaData.containsKey("android.app.static_init_classes")) {
                    loaderParams.putStringArray(STATIC_INIT_CLASSES_KEY,
                                                m_activityInfo.metaData.getString("android.app.static_init_classes").split(":"));
                }
                loaderParams.putStringArrayList(NATIVE_LIBRARIES_KEY, libraryList);
                loaderParams.putString(ENVIRONMENT_VARIABLES_KEY, ENVIRONMENT_VARIABLES
                                                                  + "\tQML2_IMPORT_PATH=" + pluginsPrefix + "/qml"
                                                                  + "\tQML_IMPORT_PATH=" + pluginsPrefix + "/imports"
                                                                  + "\tQT_PLUGIN_PATH=" + pluginsPrefix + "/plugins");

                Intent intent = getIntent();
                if (intent != null) {
                    String parameters = intent.getStringExtra("applicationArguments");
                    if (parameters != null)
                        loaderParams.putString(APPLICATION_PARAMETERS_KEY, parameters.replace(' ', '\t'));
                }

                loadApplication(loaderParams);

                activityManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);

                PackageManager packMan = getPackageManager();
                m_hasGPS = packMan.hasSystemFeature(PackageManager.FEATURE_LOCATION_GPS);

                return;
            }

            try {
                if (!bindService(new Intent(org.kde.necessitas.ministro.IMinistro.class.getCanonicalName()),
                                 m_ministroConnection,
                                 Context.BIND_AUTO_CREATE)) {
                    throw new SecurityException("");
                }
            } catch (Exception e) {
                if (firstStart) {
                    String msg = "This application requires Ministro service. Would you like to install it?";
                    if (m_activityInfo.metaData.containsKey("android.app.ministro_needed_msg"))
                        msg = m_activityInfo.metaData.getString("android.app.ministro_needed_msg");
                    downloadUpgradeMinistro(msg);
                } else {
                    ministroNotFound();
                }
            }
        } catch (Exception e) {
            Log.e(QtApplication.QtTAG, "Can't create main activity", e);
        }
    }

    /////////////////////////// forward all notifications ////////////////////////////
    /////////////////////////// Super class calls ////////////////////////////////////
    /////////////// PLEASE DO NOT CHANGE THE FOLLOWING CODE //////////////////////////
    //////////////////////////////////////////////////////////////////////////////////

    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchKeyEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchKeyEvent, event);
        else
            return super.dispatchKeyEvent(event);
    }
    public boolean super_dispatchKeyEvent(KeyEvent event)
    {
        return super.dispatchKeyEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchPopulateAccessibilityEvent(AccessibilityEvent event)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchPopulateAccessibilityEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchPopulateAccessibilityEvent, event);
        else
            return super.dispatchPopulateAccessibilityEvent(event);
    }
    public boolean super_dispatchPopulateAccessibilityEvent(AccessibilityEvent event)
    {
        return super_dispatchPopulateAccessibilityEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev)
    {

        if( (ev.getAction() == MotionEvent.ACTION_MOVE) && (Math.abs(ev.getRawX() - lastX) < 1.0f) && (Math.abs(ev.getRawY() - lastY) < 1.0f))
            return true;

        lastX = ev.getRawX();
        lastY = ev.getRawY();

        if(ev.getAction() == MotionEvent.ACTION_UP){
            lastX = -1.0f;
            lastY = -1.0f;
        }
//        Log.i("Sending", String.format("%d  x = %5.2f, y=%5.2f", ev.getAction(), ev.getRawX(), ev.getRawY()));

        if (QtApplication.m_delegateObject != null && QtApplication.dispatchTouchEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchTouchEvent, ev);
        else
            return super.dispatchTouchEvent(ev);
    }
    public boolean super_dispatchTouchEvent(MotionEvent event)
    {
        return super.dispatchTouchEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchTrackballEvent(MotionEvent ev)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchTrackballEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchTrackballEvent, ev);
        else
            return super.dispatchTrackballEvent(ev);
    }
    public boolean super_dispatchTrackballEvent(MotionEvent event)
    {
        return super.dispatchTrackballEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
//        Log.i("DEBUGGER_TAG", "onActivityResultA");
        if (requestCode == OCPN_SETTINGS_REQUEST_CODE) {
//            Log.i("DEBUGGER_TAG", "onqtActivityResultC");
            // Make sure the request was successful
            if (resultCode == RESULT_OK)
            {
//                Log.i("DEBUGGER_TAG", "onqtActivityResultD");
                m_settingsReturn = data.getStringExtra("SettingsString");
                nativeLib.invokeCmdEventCmdString( ID_CMD_NULL_REFRESH, m_settingsReturn);

                // defer hte application of settings until the screen refreshes
                Handler handler = new Handler();
                handler.postDelayed(new Runnable() {
                     public void run() {
                          nativeLib.invokeCmdEventCmdString( ID_CMD_APPLY_SETTINGS, m_settingsReturn);
                     }
                }, 100);
//                Log.i("DEBUGGER_TAG", m_settingsReturn);
            }
            else if (resultCode == RESULT_CANCELED){
//                Log.i("DEBUGGER_TAG", "onqtActivityResultE");
            }

            super.onActivityResult(requestCode, resultCode, data);

            return;
        }

        if (requestCode == OCPN_FILECHOOSER_REQUEST_CODE) {
            //Log.i("DEBUGGER_TAG", "onqtActivityResultCf");
            // Make sure the request was successful
            if (resultCode == RESULT_OK)
            {
                 //Log.i("DEBUGGER_TAG", "onqtActivityResultDf");
                 m_filechooserString = "file:" + data.getStringExtra("itemSelected");
                 //Log.i("DEBUGGER_TAG", m_filechooserString);
            }
            else if (resultCode == RESULT_CANCELED){
                //Log.i("DEBUGGER_TAG", "onqtActivityResultEf");
                m_filechooserString = "cancel:";
            }

            m_FileChooserDone = true;

            super.onActivityResult(requestCode, resultCode, data);

            return;
        }

        if (requestCode == OCPN_AFILECHOOSER_REQUEST_CODE) {
            //Log.i("DEBUGGER_TAG", "onqtActivityResultCa");
            // Make sure the request was successful
            if (resultCode == Activity.RESULT_OK) {
                //Log.i("DEBUGGER_TAG", "onqtActivityResultDa");
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

                    m_filechooserString = "file:" + filePath;

                }
            }
            else if (resultCode == Activity.RESULT_CANCELED){
                //Log.i("DEBUGGER_TAG", "onqtActivityResultEa");
                m_filechooserString = "cancel:";
            }

            m_FileChooserDone = true;

            super.onActivityResult(requestCode, resultCode, data);

            return;
        }

        if (QtApplication.m_delegateObject != null && QtApplication.onActivityResult != null) {
            QtApplication.invokeDelegateMethod(QtApplication.onActivityResult, requestCode, resultCode, data);
            return;
        }
        //Log.i("DEBUGGER_TAG", "onqtActivityResultB");
        if (requestCode == MINISTRO_INSTALL_REQUEST_CODE)
            startApp(false);

        super.onActivityResult(requestCode, resultCode, data);
    }
    public void super_onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onApplyThemeResource(Theme theme, int resid, boolean first)
    {
        if (!QtApplication.invokeDelegate(theme, resid, first).invoked)
            super.onApplyThemeResource(theme, resid, first);
    }
    public void super_onApplyThemeResource(Theme theme, int resid, boolean first)
    {
        super.onApplyThemeResource(theme, resid, first);
    }
    //---------------------------------------------------------------------------


    @Override
    protected void onChildTitleChanged(Activity childActivity, CharSequence title)
    {
        if (!QtApplication.invokeDelegate(childActivity, title).invoked)
            super.onChildTitleChanged(childActivity, title);
    }
    public void super_onChildTitleChanged(Activity childActivity, CharSequence title)
    {
        super.onChildTitleChanged(childActivity, title);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        //Log.i("DEBUGGER_TAG", "onConfigurationChanged");

        int i = nativeLib.onConfigChange();
        String aa;
        aa = String.format("%d", i);
        //Log.i("DEBUGGER_TAG", aa);

        if (!QtApplication.invokeDelegate(newConfig).invoked)
            super.onConfigurationChanged(newConfig);
    }
    public void super_onConfigurationChanged(Configuration newConfig)
    {
        super.onConfigurationChanged(newConfig);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onContentChanged()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onContentChanged();
    }
    public void super_onContentChanged()
    {
        super.onContentChanged();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onContextItemSelected(MenuItem item)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(item);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onContextItemSelected(item);
    }
    public boolean super_onContextItemSelected(MenuItem item)
    {
        return super.onContextItemSelected(item);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onContextMenuClosed(Menu menu)
    {
        if (!QtApplication.invokeDelegate(menu).invoked)
            super.onContextMenuClosed(menu);
    }
    public void super_onContextMenuClosed(Menu menu)
    {
        super.onContextMenuClosed(menu);
    }
    //---------------------------------------------------------------------------

    private ListView mDrawerList;
    private ArrayAdapter<String> mAdapter;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        //Log.i("DEBUGGER_TAG", "onCreate");
        super.onCreate(savedInstanceState);


        // Navigation Drawer
        //  Doesn't actually work on android qt...

//        setContentView(R.layout.activity_main);

//        mDrawerList = (ListView)findViewById(R.id.left_drawer);
//        String[] osArray = { "Android", "iOS", "Windows", "OS X", "Linux" };
//        mAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, osArray);
//        mDrawerList.setAdapter(mAdapter);

        // Set up ActionBar spinner navigation
        actionBar = getActionBar();

        // Hide the action bar title
//        actionBar.setDisplayShowTitleEnabled(false);

//----------------------------------------------------------------------------
        // Setup Spinner title navigation data
        navSpinner = new ArrayList<SpinnerNavItem>();

        spinnerItemRaster = new SpinnerNavItem("Raster", R.drawable.ic_action_map);
        spinnerItemVector = new SpinnerNavItem("Vector", R.drawable.ic_action_map);
        spinnerItemcm93 = new SpinnerNavItem("cm93", R.drawable.ic_action_map);

        // title drop down adapter
        adapter = new TitleNavigationAdapter(getApplicationContext(), navSpinner);
        // assigning the spinner navigation
        actionBar.setListNavigationCallbacks(adapter, this);

        configureNavSpinner(7, 0);

//----------------------------------------------------------------------------

        nativeLib = new OCPNNativeLib();

        try {
            m_activityInfo = getPackageManager().getActivityInfo(getComponentName(), PackageManager.GET_META_DATA);
            for (Field f : Class.forName("android.R$style").getDeclaredFields()) {
                if (f.getInt(null) == m_activityInfo.getThemeResource()) {
                    QT_ANDROID_THEMES = new String[] {f.getName()};
                    QT_ANDROID_DEFAULT_THEME = f.getName();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            finish();
            return;
        }

        try {
            setTheme(Class.forName("android.R$style").getDeclaredField(QT_ANDROID_DEFAULT_THEME).getInt(null));
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (Build.VERSION.SDK_INT > 10) {
            try {
                requestWindowFeature(Window.class.getField("FEATURE_ACTION_BAR").getInt(null));
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            requestWindowFeature(Window.FEATURE_NO_TITLE);
        }

        if (QtApplication.m_delegateObject != null && QtApplication.onCreate != null) {
            QtApplication.invokeDelegateMethod(QtApplication.onCreate, savedInstanceState);
            return;
        }

        ENVIRONMENT_VARIABLES += "\tQT_ANDROID_THEME=" + QT_ANDROID_DEFAULT_THEME
                              + "/\tQT_ANDROID_THEME_DISPLAY_DPI=" + getResources().getDisplayMetrics().densityDpi + "\t";

    //Log.i("DEBUGGER_TAG", "splash Screen??");

        if (null == getLastNonConfigurationInstance()) {
            // if splash screen is defined, then show it
            if (m_activityInfo.metaData.containsKey("android.app.splash_screen") )
                setContentView(m_activityInfo.metaData.getInt("android.app.splash_screen"));

    String tmpdir = "";
    ApplicationInfo ai = getApplicationInfo();
    if((ai.flags & ApplicationInfo.FLAG_EXTERNAL_STORAGE) ==  ApplicationInfo.FLAG_EXTERNAL_STORAGE)
        tmpdir = getExternalFilesDir(null).getPath();
    else
        tmpdir = getFilesDir().getPath();

    File destinationFile = new File(tmpdir + "/uidata/styles.xml");
    if (destinationFile.exists()){
//        Log.i("DEBUGGER_TAG", tmpdir + "/uidata/styles.xml exists");
    }
    else{
//      Log.i("DEBUGGER_TAG", tmpdir + "/uidata/styles.xml DOES NOT exist");

      //Log.i("DEBUGGER_TAG", "asset bridge start unpack");
      Assetbridge.unpack(this);
      //Log.i("DEBUGGER_TAG", "asset bridge finish unpack");
    }




   /* Turn off multicast filter */
   WifiManager wifi = (WifiManager)getSystemService(Context.WIFI_SERVICE);
   if (wifi != null){
        WifiManager.MulticastLock lock = wifi.createMulticastLock("mylock");
        lock.acquire();
   }


            startApp(true);

            // setup action bar for tabs
 //           ActionBar actionBar = getActionBar();
 //           actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
 //           actionBar.setDisplayShowTitleEnabled(true);

 //           Tab tab = actionBar.newTab()
 //                              .setText("RasterCharts")
 //                              .setTabListener(this);
 //           actionBar.addTab(tab);

 //           tab = actionBar.newTab()
 //                          .setText("Vector Charts")
 //                          .setTabListener(this);
 //           actionBar.addTab(tab);


        }
    }
    //---------------------------------------------------------------------------


    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo)
    {
        if (!QtApplication.invokeDelegate(menu, v, menuInfo).invoked)
            super.onCreateContextMenu(menu, v, menuInfo);
    }
    public void super_onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo)
    {
        super.onCreateContextMenu(menu, v, menuInfo);
    }
    //---------------------------------------------------------------------------

    @Override
    public CharSequence onCreateDescription()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return (CharSequence)res.methodReturns;
        else
            return super.onCreateDescription();
    }
    public CharSequence super_onCreateDescription()
    {
        return super.onCreateDescription();
    }
    //---------------------------------------------------------------------------

    @Override
    protected Dialog onCreateDialog(int id)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(id);
        if (res.invoked)
            return (Dialog)res.methodReturns;
        else
            return super.onCreateDialog(id);
    }
    public Dialog super_onCreateDialog(int id)
    {
        return super.onCreateDialog(id);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        //Log.i("DEBUGGER_TAG", "onCreateOptionsMenu");

//      We don't use Qt menu system, since it does not support ActionBar.
//      We handle ActionBar here, in standard Android manner
//
//        QtApplication.InvokeResult res = QtApplication.invokeDelegate(menu);
//        if (res.invoked)
//            return (Boolean)res.methodReturns;
//        else
//            return super.onCreateOptionsMenu(menu);


        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.activity_main_actions, menu);

        itemRouteAnnunciator = menu.findItem(R.id.ocpn_route_create_active);
        if( null != itemRouteAnnunciator) {
            itemRouteAnnunciator.setVisible(m_showRouteAnnunciator);
        }

         itemRouteMenuItem = menu.findItem(R.id.ocpn_action_createroute);
         if( null != itemRouteMenuItem) {
             itemRouteMenuItem.setVisible(!m_showRouteAnnunciator);
         }


        // Auto follow icon
         itemFollowActive = menu.findItem(R.id.ocpn_action_follow_active);
         if( null != itemFollowActive) {
             itemFollowActive.setVisible(m_isFollowActive);

          }
         itemFollowInActive = menu.findItem(R.id.ocpn_action_follow);
         if( null != itemFollowInActive) {
              itemFollowInActive.setVisible(!m_isFollowActive);
           }

         // Track icon
         itemTrackActive = menu.findItem(R.id.ocpn_action_track_toggle_ison);
         if( null != itemTrackActive) {
             itemTrackActive.setVisible(m_isTrackActive);
         }
         itemTrackInActive = menu.findItem(R.id.ocpn_action_track_toggle_isoff);
         if( null != itemTrackInActive) {
             itemTrackInActive.setVisible(!m_isTrackActive);
         }



        return super.onCreateOptionsMenu(menu);


    }
    public boolean super_onCreateOptionsMenu(Menu menu)
    {
        return super.onCreateOptionsMenu(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreatePanelMenu(int featureId, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onCreatePanelMenu(featureId, menu);
    }
    public boolean super_onCreatePanelMenu(int featureId, Menu menu)
    {
        return super.onCreatePanelMenu(featureId, menu);
    }
    //---------------------------------------------------------------------------


    @Override
    public View onCreatePanelView(int featureId)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId);
        if (res.invoked)
            return (View)res.methodReturns;
        else
            return super.onCreatePanelView(featureId);
    }
    public View super_onCreatePanelView(int featureId)
    {
        return super.onCreatePanelView(featureId);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreateThumbnail(Bitmap outBitmap, Canvas canvas)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(outBitmap, canvas);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onCreateThumbnail(outBitmap, canvas);
    }
    public boolean super_onCreateThumbnail(Bitmap outBitmap, Canvas canvas)
    {
        return super.onCreateThumbnail(outBitmap, canvas);
    }
    //---------------------------------------------------------------------------

    @Override
    public View onCreateView(String name, Context context, AttributeSet attrs)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(name, context, attrs);
        if (res.invoked)
            return (View)res.methodReturns;
        else
            return super.onCreateView(name, context, attrs);
    }
    public View super_onCreateView(String name, Context context, AttributeSet attrs)
    {
        return super.onCreateView(name, context, attrs);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------


    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {

//        Log.i("DEBUGGER_TAG", "onKeyDown");

        if(keyCode==KeyEvent.KEYCODE_BACK){
            //Toast.makeText(getApplicationContext(), "back press",Toast.LENGTH_LONG).show();
            if(!m_backButtonEnable)
                return false;
        }

/*
        if (keyCode == KeyEvent.KEYCODE_MENU) {
            //Log.i("DEBUGGER_TAG", "KEYCODE_MENU");

            int i = nativeLib.onMenuKey();

            return true;
        }
*/

        if (QtApplication.m_delegateObject != null && QtApplication.onKeyDown != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyDown, keyCode, event);
        else
            return super.onKeyDown(keyCode, event);
    }
    public boolean super_onKeyDown(int keyCode, KeyEvent event)
    {
        return super.onKeyDown(keyCode, event);
    }
    //---------------------------------------------------------------------------


    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.onKeyMultiple != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyMultiple, keyCode, repeatCount, event);
        else
            return super.onKeyMultiple(keyCode, repeatCount, event);
    }
    public boolean super_onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
    {
        return super.onKeyMultiple(keyCode, repeatCount, event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onKeyDown != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyUp, keyCode, event);
        else
            return super.onKeyUp(keyCode, event);
    }
    public boolean super_onKeyUp(int keyCode, KeyEvent event)
    {
        return super.onKeyUp(keyCode, event);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onLowMemory()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onLowMemory();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, item);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onMenuItemSelected(featureId, item);
    }
    public boolean super_onMenuItemSelected(int featureId, MenuItem item)
    {
        return super.onMenuItemSelected(featureId, item);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onMenuOpened(int featureId, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onMenuOpened(featureId, menu);
    }
    public boolean super_onMenuOpened(int featureId, Menu menu)
    {
        return super.onMenuOpened(featureId, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onNewIntent(Intent intent)
    {
        if (!QtApplication.invokeDelegate(intent).invoked)
            super.onNewIntent(intent);
    }
    public void super_onNewIntent(Intent intent)
    {
        super.onNewIntent(intent);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
//        QtApplication.InvokeResult res = QtApplication.invokeDelegate(item);
//        if (res.invoked)
//            return (Boolean)res.methodReturns;
//        else
//            return super.onOptionsItemSelected(item);

        // Take appropriate action for each action item click
        switch (item.getItemId()) {
            case R.id.ocpn_action_follow:
                //Log.i("DEBUGGER_TAG", "Invoke OCPN_ACTION_FOLLOW while in-active");
                nativeLib.invokeMenuItem(OCPN_ACTION_FOLLOW);
                return true;

                case R.id.ocpn_action_follow_active:
                    //Log.i("DEBUGGER_TAG", "Invoke OCPN_ACTION_FOLLOW while active");
                    nativeLib.invokeMenuItem(OCPN_ACTION_FOLLOW);
                    return true;

                case R.id.ocpn_action_settings_basic:
                    nativeLib.invokeMenuItem(OCPN_ACTION_SETTINGS_BASIC);
                    return true;

                case R.id.ocpn_action_routemanager:
                    nativeLib.invokeMenuItem(OCPN_ACTION_RMD);
                    return true;

                case R.id.ocpn_action_track_toggle_ison:
                    nativeLib.invokeMenuItem(OCPN_ACTION_TRACK_TOGGLE);
                    return true;

                case R.id.ocpn_action_track_toggle_isoff:
                    nativeLib.invokeMenuItem(OCPN_ACTION_TRACK_TOGGLE);
                    return true;

                case R.id.ocpn_action_createroute:              // entering Route Create Mode
                    nativeLib.invokeMenuItem(OCPN_ACTION_ROUTE);
                    return true;

                case R.id.ocpn_route_create_active:             // exiting Route Create mode
                    nativeLib.invokeMenuItem(OCPN_ACTION_ROUTE);
                    return true;

                case R.id.ocpn_action_mob:
                    nativeLib.invokeMenuItem(OCPN_ACTION_MOB);
                    return true;

                case R.id.ocpn_action_tides:
                    nativeLib.invokeMenuItem(OCPN_ACTION_TIDES_TOGGLE);
                    return true;

                case R.id.ocpn_action_currents:
                    nativeLib.invokeMenuItem(OCPN_ACTION_CURRENTS_TOGGLE);
                    return true;

                case R.id.ocpn_action_encText:
                    nativeLib.invokeMenuItem(OCPN_ACTION_ENCTEXT_TOGGLE);
                    return true;

                case R.id.ocpn_action_googlemaps:
                        invokeGoogleMaps();
                        return true;


            default:
                return super.onOptionsItemSelected(item);
            }

    }
    public boolean super_onOptionsItemSelected(MenuItem item)
    {
        return super.onOptionsItemSelected(item);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onOptionsMenuClosed(Menu menu)
    {
        if (!QtApplication.invokeDelegate(menu).invoked)
            super.onOptionsMenuClosed(menu);
    }
    public void super_onOptionsMenuClosed(Menu menu)
    {
        super.onOptionsMenuClosed(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onPanelClosed(int featureId, Menu menu)
    {
        if (!QtApplication.invokeDelegate(featureId, menu).invoked)
            super.onPanelClosed(featureId, menu);
    }
    public void super_onPanelClosed(int featureId, Menu menu)
    {
        super.onPanelClosed(featureId, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPause()
    {
        //Log.i("DEBUGGER_TAG", "onPause");

        int i = nativeLib.onPause();
        String aa;
        aa = String.format("%d", i);
        //Log.i("DEBUGGER_TAG", aa);

        super.onPause();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPostCreate(Bundle savedInstanceState)
    {
        super.onPostCreate(savedInstanceState);
        QtApplication.invokeDelegate(savedInstanceState);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPostResume()
    {
        super.onPostResume();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPrepareDialog(int id, Dialog dialog)
    {
        if (!QtApplication.invokeDelegate(id, dialog).invoked)
            super.onPrepareDialog(id, dialog);
    }
    public void super_onPrepareDialog(int id, Dialog dialog)
    {
        super.onPrepareDialog(id, dialog);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onPrepareOptionsMenu(Menu menu)
    {
        //Log.i("DEBUGGER_TAG", "onPrepareOptionsMenu");


// Use native instead og Qt
//        QtApplication.InvokeResult res = QtApplication.invokeDelegate(menu);
//        if (res.invoked)
//            return (Boolean)res.methodReturns;
//        else
//            return super.onPrepareOptionsMenu(menu);
        ActionBar actionBar = getActionBar();
        if(actionBar != null){
            // set the icon
            //actionBar.setIcon(R.drawable.opencpn_mobile);
            actionBar.setLogo(R.drawable.opencpn_mobile);
            actionBar.setDisplayUseLogoEnabled(true);

            //  Use transparent ActionBar background?
            //getWindow().requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);//or add in style.xml
            //ColorDrawable newColor = new ColorDrawable(getResources().getColor(R.color.action_bar_color));//your color from res
            //newColor.setAlpha(0);//from 0(0%) to 256(100%)
            //getActionBar().setBackgroundDrawable(newColor);

            actionBar.show();
        }


        return super.onPrepareOptionsMenu(menu);
    }
    public boolean super_onPrepareOptionsMenu(Menu menu)
    {
        return super.onPrepareOptionsMenu(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onPreparePanel(int featureId, View view, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, view, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onPreparePanel(featureId, view, menu);
    }
    public boolean super_onPreparePanel(int featureId, View view, Menu menu)
    {
        return super.onPreparePanel(featureId, view, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onRestart()
    {
        //Log.i("DEBUGGER_TAG", "onRestart");
        super.onRestart();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState)
    {
        if (!QtApplication.invokeDelegate(savedInstanceState).invoked)
            super.onRestoreInstanceState(savedInstanceState);
    }
    public void super_onRestoreInstanceState(Bundle savedInstanceState)
    {
        super.onRestoreInstanceState(savedInstanceState);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onResume()
    {
        //Log.i("DEBUGGER_TAG", "onResume");

        int i = nativeLib.onResume();
//        String aa;
//        aa = String.format("%d", i);
//        Log.i("DEBUGGER_TAG", aa);

        super.onResume();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    public Object onRetainNonConfigurationInstance()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return res.methodReturns;
        else
            return super.onRetainNonConfigurationInstance();
    }
    public Object super_onRetainNonConfigurationInstance()
    {
        return super.onRetainNonConfigurationInstance();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        if (!QtApplication.invokeDelegate(outState).invoked)
            super.onSaveInstanceState(outState);
    }
    public void super_onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);

    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onSearchRequested()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onSearchRequested();
    }
    public boolean super_onSearchRequested()
    {
        return super.onSearchRequested();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onStart()
    {
        //Log.i("DEBUGGER_TAG", "onStart");

//        int i = nativeLib.onStart();
//        String aa;
//        aa = String.format("%d", i);
//        Log.i("DEBUGGER_TAG", aa);

        super.onStart();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onStop()
    {
        //Log.i("DEBUGGER_TAG", "onStop");

        int i = nativeLib.onStop();
        String aa;
        aa = String.format("%d", i);
        //Log.i("DEBUGGER_TAG", aa);

        super.onStop();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onTitleChanged(CharSequence title, int color)
    {
        if (!QtApplication.invokeDelegate(title, color).invoked)
            super.onTitleChanged(title, color);
    }
    public void super_onTitleChanged(CharSequence title, int color)
    {
        super.onTitleChanged(title, color);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onTouchEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onTouchEvent, event);
        else
            return super.onTouchEvent(event);
    }
    public boolean super_onTouchEvent(MotionEvent event)
    {
        return super.onTouchEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onTrackballEvent(MotionEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onTrackballEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onTrackballEvent, event);
        else
            return super.onTrackballEvent(event);
    }
    public boolean super_onTrackballEvent(MotionEvent event)
    {
        return super.onTrackballEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onUserInteraction()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onUserInteraction();
    }
    public void super_onUserInteraction()
    {
        super.onUserInteraction();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onUserLeaveHint()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onUserLeaveHint();
    }
    public void super_onUserLeaveHint()
    {
        super.onUserLeaveHint();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onWindowAttributesChanged(LayoutParams params)
    {
        if (!QtApplication.invokeDelegate(params).invoked)
            super.onWindowAttributesChanged(params);
    }
    public void super_onWindowAttributesChanged(LayoutParams params)
    {
        super.onWindowAttributesChanged(params);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onWindowFocusChanged(boolean hasFocus)
    {
        if (!QtApplication.invokeDelegate(hasFocus).invoked)
            super.onWindowFocusChanged(hasFocus);
    }
    public void super_onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);
    }
    //---------------------------------------------------------------------------

    //////////////// Activity API 5 /////////////
//@ANDROID-5
    @Override
    public void onAttachedToWindow()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onAttachedToWindow();
    }
    public void super_onAttachedToWindow()
    {
        super.onAttachedToWindow();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onBackPressed()
    {

        if (!QtApplication.invokeDelegate().invoked)
            super.onBackPressed();
    }
    public void super_onBackPressed()
    {
        super.onBackPressed();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onDetachedFromWindow()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onDetachedFromWindow();
    }
    public void super_onDetachedFromWindow()
    {
        super.onDetachedFromWindow();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onKeyLongPress != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyLongPress, keyCode, event);
        else
            return super.onKeyLongPress(keyCode, event);
    }
    public boolean super_onKeyLongPress(int keyCode, KeyEvent event)
    {
        return super.onKeyLongPress(keyCode, event);
    }
    //---------------------------------------------------------------------------
//@ANDROID-5

//////////////// Activity API 8 /////////////
//@ANDROID-8
@Override
    protected Dialog onCreateDialog(int id, Bundle args)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(id, args);
        if (res.invoked)
            return (Dialog)res.methodReturns;
        else
            return super.onCreateDialog(id, args);
    }
    public Dialog super_onCreateDialog(int id, Bundle args)
    {
        return super.onCreateDialog(id, args);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args)
    {
        if (!QtApplication.invokeDelegate(id, dialog, args).invoked)
            super.onPrepareDialog(id, dialog, args);
    }
    public void super_onPrepareDialog(int id, Dialog dialog, Bundle args)
    {
        super.onPrepareDialog(id, dialog, args);
    }
    //---------------------------------------------------------------------------
//@ANDROID-8
    //////////////// Activity API 11 /////////////

//@ANDROID-11
    @Override
    public boolean dispatchKeyShortcutEvent(KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.dispatchKeyShortcutEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchKeyShortcutEvent, event);
        else
            return super.dispatchKeyShortcutEvent(event);
    }
    public boolean super_dispatchKeyShortcutEvent(KeyEvent event)
    {
        return super.dispatchKeyShortcutEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onActionModeFinished(ActionMode mode)
    {
        if (!QtApplication.invokeDelegate(mode).invoked)
            super.onActionModeFinished(mode);
    }
    public void super_onActionModeFinished(ActionMode mode)
    {
        super.onActionModeFinished(mode);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onActionModeStarted(ActionMode mode)
    {
        if (!QtApplication.invokeDelegate(mode).invoked)
            super.onActionModeStarted(mode);
    }
    public void super_onActionModeStarted(ActionMode mode)
    {
        super.onActionModeStarted(mode);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onAttachFragment(Fragment fragment)
    {
        if (!QtApplication.invokeDelegate(fragment).invoked)
            super.onAttachFragment(fragment);
    }
    public void super_onAttachFragment(Fragment fragment)
    {
        super.onAttachFragment(fragment);
    }
    //---------------------------------------------------------------------------

    @Override
    public View onCreateView(View parent, String name, Context context, AttributeSet attrs)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(parent, name, context, attrs);
        if (res.invoked)
            return (View)res.methodReturns;
        else
            return super.onCreateView(parent, name, context, attrs);
    }
    public View super_onCreateView(View parent, String name, Context context,
            AttributeSet attrs) {
        return super.onCreateView(parent, name, context, attrs);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onKeyShortcut(int keyCode, KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onKeyShortcut != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyShortcut, keyCode,event);
        else
            return super.onKeyShortcut(keyCode, event);
    }
    public boolean super_onKeyShortcut(int keyCode, KeyEvent event)
    {
        return super.onKeyShortcut(keyCode, event);
    }
    //---------------------------------------------------------------------------

    @Override
    public ActionMode onWindowStartingActionMode(Callback callback)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(callback);
        if (res.invoked)
            return (ActionMode)res.methodReturns;
        else
            return super.onWindowStartingActionMode(callback);
    }
    public ActionMode super_onWindowStartingActionMode(Callback callback)
    {
        return super.onWindowStartingActionMode(callback);
    }
    //---------------------------------------------------------------------------
//@ANDROID-11
    //////////////// Activity API 12 /////////////

//@ANDROID-12
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.dispatchGenericMotionEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchGenericMotionEvent, ev);
        else
            return super.dispatchGenericMotionEvent(ev);
    }
    public boolean super_dispatchGenericMotionEvent(MotionEvent event)
    {
        return super.dispatchGenericMotionEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onGenericMotionEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onGenericMotionEvent, event);
        else
            return super.onGenericMotionEvent(event);
    }
    public boolean super_onGenericMotionEvent(MotionEvent event)
    {
        return super.onGenericMotionEvent(event);
    }
    //---------------------------------------------------------------------------
//@ANDROID-12

}
