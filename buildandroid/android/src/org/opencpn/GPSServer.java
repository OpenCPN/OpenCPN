package org.opencpn;

import android.app.AlertDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.HandlerThread;
import android.provider.Settings;
import android.util.Log;
import android.app.Activity;

import java.util.List;

import org.opencpn.OCPNGpsNmeaListener;
import org.opencpn.OCPNNativeLib;
import org.qtproject.qt5.android.bindings.QtActivity;



public class GPSServer extends Service implements LocationListener {

    private final static int GPS_OFF = 0;
    private final static int GPS_ON = 1;
    private final static int GPS_PROVIDER_AVAILABLE = 2;
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

    Location location; // location
    double latitude; // latitude
    double longitude; // longitude

    // The minimum distance to change Updates in meters
    private static final long MIN_DISTANCE_CHANGE_FOR_UPDATES = 1; // 1 meter

    // The minimum time between updates in milliseconds
    private static final long MIN_TIME_BW_UPDATES = 1000; // 1 second

    // Declaring a Location Manager
    protected LocationManager locationManager;

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

                            mNMEAListener = new OCPNGpsNmeaListener(mNativeLib);
                            locationManager.addNmeaListener (mNMEAListener);


                        }
                    });















//                    Log.i("DEBUGGER_TAG", "Requesting Updates");
//                    locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,1000,1, this);

//                    mNMEAListener = new OCPNGpsNmeaListener(mNativeLib);
//                    locationManager.addNmeaListener (mNMEAListener);

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
         final LocationManager mgr = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
         if ( mgr == null ) return false;
         final List<String> providers = mgr.getAllProviders();
         if ( providers == null ) return false;
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
                        location = locationManager
                                .getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                        if (location != null) {
                            latitude = location.getLatitude();
                            longitude = location.getLongitude();
                        }
                    }
                }
                // if GPS Enabled get lat/long using GPS Services
                if (isGPSEnabled) {
                    if (location == null) {
                        locationManager.requestLocationUpdates(
                                LocationManager.GPS_PROVIDER,
                                MIN_TIME_BW_UPDATES,
                                MIN_DISTANCE_CHANGE_FOR_UPDATES, this);
                        Log.d("GPS Enabled", "GPS Enabled");
                        if (locationManager != null) {
                            location = locationManager
                                    .getLastKnownLocation(LocationManager.GPS_PROVIDER);
                            if (location != null) {
                                latitude = location.getLatitude();
                                longitude = location.getLongitude();
                            }
                        }
                    }
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return location;
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
        if(location != null){
            latitude = location.getLatitude();
        }

        // return latitude
        return latitude;
    }

    /**
     * Function to get longitude
     * */
    public double getLongitude(){
        if(location != null){
            longitude = location.getLongitude();
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

    }

    @Override
    public void onProviderDisabled(String provider) {
        Log.i("DEBUGGER_TAG", "onProviderDisabled");

    }

    @Override
    public void onProviderEnabled(String provider) {
        Log.i("DEBUGGER_TAG", "onProviderDisabled");

    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        Log.i("DEBUGGER_TAG", "onStatusChanged");

    }


    @Override
    public IBinder onBind(Intent arg0) {
        return null;
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
