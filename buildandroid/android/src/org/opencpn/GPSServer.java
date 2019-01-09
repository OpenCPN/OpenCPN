package org.opencpn;

import android.app.AlertDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.GpsStatus;
import android.location.GpsSatellite;

import android.os.Bundle;
import android.os.IBinder;
import android.os.HandlerThread;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;
import android.app.Activity;
import android.os.Handler;
import java.util.List;
import java.lang.Math;
import java.lang.Iterable;
import java.util.Iterator;

import org.opencpn.OCPNGpsNmeaListener;
import org.opencpn.OCPNNativeLib;
import org.qtproject.qt5.android.bindings.QtActivity;



public class GPSServer extends Service implements LocationListener {

    private final static int GPS_OFF = 0;
    private final static int GPS_ON = 1;
    public  final static int GPS_PROVIDER_AVAILABLE = 2;
    private final static int GPS_SHOWPREFERENCES = 3;

    private final Context mContext;
    private final Activity parent_activity;

    public String status_string;

    boolean isThreadStarted = false;
    HandlerThread mLocationHandlerThread;

    OCPNGpsNmeaListener mNMEAListener;
    OCPNNativeLib mNativeLib;

    // flag for GPS status
    boolean isGPSEnabled = false;

    // flag for network status
    boolean isNetworkEnabled = false;

    // flag for GPS status
    boolean canGetLocation = false;

    Location mLastLocation; // location
    double latitude; // latitude
    double longitude; // longitude
    float course;
    float speed;

    private GpsStatus mStatus;
    private MyListener mMyListener;
    long mLastLocationMillis;
    boolean isGPSFix = false;
    public int m_watchDog = 0;

    int m_tick;

    // The minimum distance to change Updates in meters
    private static final long MIN_DISTANCE_CHANGE_FOR_UPDATES = 1; // 1 meter

    // The minimum time between updates in milliseconds
    private static final long MIN_TIME_BW_UPDATES = 1000; // 1 second

    // Declaring a Location Manager
    protected LocationManager locationManager;

    private class MyListener implements GpsStatus.Listener {
        @Override
        public void onGpsStatusChanged(int event) {
//            Log.i("DEBUGGER_TAG", "StatusListener Event");

            if(null != locationManager){
                mStatus = locationManager.getGpsStatus(mStatus);
            }


            switch (event) {
                case GpsStatus.GPS_EVENT_STARTED:
                    Log.i("DEBUGGER_TAG", "GPS_EVENT_STARTED Event");
                    break;

                case GpsStatus.GPS_EVENT_STOPPED:
                    Log.i("DEBUGGER_TAG", "GPS_EVENT_STOPPED Event");
                    isGPSFix = false;
                    break;

                case GpsStatus.GPS_EVENT_FIRST_FIX:
                    Log.i("DEBUGGER_TAG", "GPS_EVENT_FIRST_FIX Event");
                    isGPSFix = true;
                    break;

                case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
//                    Log.i("DEBUGGER_TAG", "GPS_EVENT_SATELLITE_STATUS Event");

                        int nSatsUsed = 0;
                         // int maxSatellites = gpsStatus.getMaxSatellites();    // appears fixed at 255
                         Iterable<GpsSatellite>satellites = mStatus.getSatellites();
                         Iterator<GpsSatellite>satI = satellites.iterator();
                         while (satI.hasNext()) {
                             GpsSatellite satellite = satI.next();
//                             Log.i("DEBUGGER_TAG", "onGpsStatusChanged(): " + satellite.getPrn() + "," + satellite.usedInFix() + "," + satellite.getSnr() + "," + satellite.getAzimuth() + "," + satellite.getElevation());
                             if(satellite.usedInFix())
                             nSatsUsed++;
                         }

                    if(nSatsUsed < 3)
                        isGPSFix = false;

                    break;
            }
        }
    }

    public GPSServer(Context context, OCPNNativeLib nativelib, Activity activity) {
        this.mContext = context;
        this.mNativeLib = nativelib;
        this.parent_activity = activity;
//        getLocation();
    }

    public String doService( int parm )
    {
        String ret_string = "???";
        locationManager = (LocationManager) mContext.getSystemService(LOCATION_SERVICE);

        switch (parm){
            case GPS_OFF:
            Log.i("DEBUGGER_TAG", "GPS OFF");

            if(locationManager != null){
                if(isThreadStarted){
                    locationManager.removeUpdates(GPSServer.this);
                    locationManager.removeNmeaListener (mNMEAListener);
                    isThreadStarted = false;
                }
            }

            ret_string = "GPS_OFF OK";
            break;

            case GPS_ON:
                Log.i("DEBUGGER_TAG", "GPS ON");

                isGPSEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);

                if(isGPSEnabled){
                    Log.i("DEBUGGER_TAG", "GPS is Enabled");
                }
                else{
                    Log.i("DEBUGGER_TAG", "GPS is <<<<DISABLED>>>>");
                    ret_string = "GPS is disabled";
                    status_string = ret_string;
                    return ret_string;
                }

/*
                isNetworkEnabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
                if(isNetworkEnabled)
                    Log.i("DEBUGGER_TAG", "Network is Enabled");
                else
                    Log.i("DEBUGGER_TAG", "Network is <<<<DISABLED>>>>");
*/

                if(!isThreadStarted){

                    parent_activity.runOnUiThread(new Runnable()   {
                        LocationManager locationManager;
                        public void run()   {

                            locationManager = (LocationManager) mContext.getSystemService(LOCATION_SERVICE);
                            Log.i("DEBUGGER_TAG", "Requesting Updates");
                            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,1000,1, GPSServer.this);

                            mNMEAListener = new OCPNGpsNmeaListener(mNativeLib, GPSServer.this);
                            locationManager.addNmeaListener (mNMEAListener);

                            mMyListener = new MyListener();
                            locationManager.addGpsStatusListener(mMyListener);

                        }
                    });


                    HandlerThread hThread = new HandlerThread("HandlerThread");
                    hThread.start();
                    final Handler handler = new Handler(hThread.getLooper());


                    Runnable ticker = new Runnable() {
                        @Override
                        public void run() {
//                            Log.i("DEBUGGER_TAG", "Tick");

                            m_tick++;
                            m_watchDog++;

                            if(isGPSEnabled && (m_watchDog > 10)){
                                if(null != locationManager){
                                    mLastLocation = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
                                    if (mLastLocation != null) {
                                        latitude = mLastLocation.getLatitude();
                                        longitude = mLastLocation.getLongitude();
                                        course = mLastLocation.getBearing();
                                        speed = mLastLocation.getSpeed();
                                    }

                                    if(null != mNativeLib){
                                        String s = createRMC();
                                        mNativeLib.processNMEA( s );
                                    }
                                }
                            }

                            handler.postDelayed(this, 1000);
                            }
                        };

                    // Schedule the first execution
                    handler.postDelayed(ticker, 1000);


                    isThreadStarted = true;
                }

                ret_string = "GPS_ON OK";
                break;

            case GPS_PROVIDER_AVAILABLE:
            if(hasGPSDevice( mContext )){
                    ret_string = "YES";
                    Log.i("DEBUGGER_TAG", "Provider yes");
                }
                else{
                    ret_string = "NO";
                    Log.i("DEBUGGER_TAG", "Provider no");
                }

                break;

            case GPS_SHOWPREFERENCES:
                showSettingsAlert();
                break;

        }   // switch


        status_string = ret_string;
        return ret_string;
     }



     public boolean hasGPSDevice(Context context)
     {

 //        This code crashes unless run from the GUI thread, so is moved to the QtActivity initialization
 //        PackageManager packMan = getPackageManager();
 //        return packMan.hasSystemFeature(PackageManager.FEATURE_LOCATION_GPS);

    // This code produces false positive for some generic android tablets.
         final LocationManager mgr = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
         if ( mgr == null )
            return false;
         final List<String> providers = mgr.getAllProviders();
         if ( providers == null )
            return false;
         return providers.contains(LocationManager.GPS_PROVIDER);

     }


    public Location getLocation() {
        try {
            locationManager = (LocationManager) mContext
                    .getSystemService(LOCATION_SERVICE);

            // getting GPS status
            isGPSEnabled = locationManager
                    .isProviderEnabled(LocationManager.GPS_PROVIDER);

            // getting network status
            isNetworkEnabled = locationManager
                    .isProviderEnabled(LocationManager.NETWORK_PROVIDER);

            if (!isGPSEnabled && !isNetworkEnabled) {
                // no network provider is enabled
            } else {
                this.canGetLocation = true;
                // First get location from Network Provider
                if (isNetworkEnabled) {
                    locationManager.requestLocationUpdates(
                            LocationManager.NETWORK_PROVIDER,
                            MIN_TIME_BW_UPDATES,
                            MIN_DISTANCE_CHANGE_FOR_UPDATES, this);
                    Log.d("Network", "Network");
                    if (locationManager != null) {
                        mLastLocation = locationManager
                                .getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                        if (mLastLocation != null) {
                            latitude = mLastLocation.getLatitude();
                            longitude = mLastLocation.getLongitude();
                        }
                    }
                }
                // if GPS Enabled get lat/long using GPS Services
                if (isGPSEnabled) {
                    if (mLastLocation == null) {
                        locationManager.requestLocationUpdates(
                                LocationManager.GPS_PROVIDER,
                                MIN_TIME_BW_UPDATES,
                                MIN_DISTANCE_CHANGE_FOR_UPDATES, this);
                        Log.d("GPS Enabled", "GPS Enabled");
                        if (locationManager != null) {
                            mLastLocation = locationManager
                                    .getLastKnownLocation(LocationManager.GPS_PROVIDER);
                            if (mLastLocation != null) {
                                latitude = mLastLocation.getLatitude();
                                longitude = mLastLocation.getLongitude();
                            }
                        }
                    }
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return mLastLocation;
    }

    /**
     * Stop using GPS listener
     * Calling this function will stop using GPS in your app
     * */
    public void stopUsingGPS(){
        if(locationManager != null){
            locationManager.removeUpdates(GPSServer.this);
        }
    }

    /**
     * Function to get latitude
     * */
    public double getLatitude(){
        if(mLastLocation != null){
            latitude = mLastLocation.getLatitude();
        }

        // return latitude
        return latitude;
    }

    /**
     * Function to get longitude
     * */
    public double getLongitude(){
        if(mLastLocation != null){
            longitude = mLastLocation.getLongitude();
        }

        // return longitude
        return longitude;
    }

    /**
     * Function to check GPS/wifi enabled
     * @return boolean
     * */
    public boolean canGetLocation() {
        return this.canGetLocation;
    }

    /**
     * Function to show settings alert dialog
     * On pressing Settings button will lauch Settings Options
     * */
    public void showSettingsAlert(){
        AlertDialog.Builder alertDialog = new AlertDialog.Builder(this);

        // Setting Dialog Title
        alertDialog.setTitle("GPS is settings");

        // Setting Dialog Message
        alertDialog.setMessage("GPS is not enabled. Do you want to go to settings menu?");

        // On pressing Settings button
        alertDialog.setPositiveButton("Settings", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog,int which) {
                Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                mContext.startActivity(intent);
            }
        });

        // on pressing cancel button
        alertDialog.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
            dialog.cancel();
            }
        });

        // Showing Alert Message
        alertDialog.show();
    }

    @Override
    public void onLocationChanged(Location location) {
        Log.i("DEBUGGER_TAG", "onLocationChanged");
        if (location == null) return;

        mLastLocationMillis = SystemClock.elapsedRealtime();

        mLastLocation = location;
    }

    @Override
    public void onProviderDisabled(String provider) {
        Log.i("DEBUGGER_TAG", "onProviderDisabled " + provider);

    }

    @Override
    public void onProviderEnabled(String provider) {
        Log.i("DEBUGGER_TAG", "onProviderEnabled " + provider);

    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        Log.i("DEBUGGER_TAG", "onStatusChanged");

    }


    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    private String createRMC(){
        // Create an NMEA sentence
        String s = "$LCRMC,,";
        if(isGPSFix)
            s = s.concat("A,");
        else
            s = s.concat("V,");


        String slat = "";
        double ltt = latitude;
        if(latitude < 0)
            ltt = -latitude;

        double d0 = Math.floor(ltt);
        double d1 = ltt-d0;
        double d2 = Math.floor(d1 * 60);
        double d3 = (d1*60.) - d2;

        slat = slat.format("%.0f.%.0f,", (d0 * 100.) + d2, d3 * 10000);

        if(latitude > 0)
            slat = slat.concat("N,");
        else
            slat = slat.concat("S,");


        s = s.concat(slat);

        String slon = "";
        double lot = longitude;
        if(longitude < 0)
            lot = -longitude;

        d0 = Math.floor(lot);
        d1 = lot-d0;
        d2 = Math.floor(d1 * 60);
        d3 = (d1*60.) - d2;

        if(d0 < 100.)
            slon = "0";
        slon = slon.concat(slon.format("%.0f.%.0f,", (d0 * 100.) + d2, d3 * 10000));

        if(longitude > 0)
            slon = slon.concat("E,");
        else
            slon = slon.concat("W,");

        s = s.concat(slon);

        String sspeed = "";
        sspeed = sspeed.format("%.2f,", speed /.5144);
        s = s.concat(sspeed);

        String strack = "";
        strack = strack.format("%.0f,", course);
        s = s.concat(strack);

        s = s.concat(",,,");      // unused fields

        s = s.concat("*55");    // checksum

//        s = s.concat("\r\n");

        Log.i("DEBUGGER_TAG", s);

        return s;
    }
}



//GPSTracker gps = new GPSTracker(this);
//if(gps.canGetLocation()){ // gps enabled} // return boolean true/false

//Getting Latitude and Longitude
//gps.getLatitude(); // returns latitude
//gps.getLongitude(); // returns longitude

//Showing GPS Settings Alert Dialog
//gps.showSettingsAlert();

//Stop using GPS
//gps.stopUsingGPS();
