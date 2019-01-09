package org.opencpn;

import android.app.AlertDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.GpsStatus;
import android.location.GpsStatus.NmeaListener;
import android.os.Bundle;
import android.os.IBinder;
import android.os.HandlerThread;
import android.provider.Settings;
import android.util.Log;

import org.opencpn.OCPNNativeLib;
import org.opencpn.GPSServer;

public class OCPNGpsNmeaListener implements GpsStatus.NmeaListener{

    OCPNNativeLib mNativeLib;
    GPSServer mserver;

    public OCPNGpsNmeaListener(OCPNNativeLib nativelib, GPSServer server) {
        this.mNativeLib = nativelib;
        this.mserver = server;
    }


    @Override
    public void onNmeaReceived(long timestamp, String nmea) {
//        Log.i("DEBUGGER_TAG", "onNmeaReceived");
//        Log.i("DEBUGGER_TAG", nmea);

        // Reset the dog.
        if(null != mserver)
            mserver.m_watchDog = 0;

        mNativeLib.processNMEA( nmea );
    }


}


