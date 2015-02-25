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

public class OCPNGpsNmeaListener implements GpsStatus.NmeaListener{

    OCPNNativeLib mNativeLib;

    public OCPNGpsNmeaListener(OCPNNativeLib nativelib) {
        this.mNativeLib = nativelib;
    }


    @Override
    public void onNmeaReceived(long timestamp, String nmea) {
//        Log.i("DEBUGGER_TAG", "onNmeaReceived");
        Log.i("DEBUGGER_TAG", nmea);

        mNativeLib.processNMEA( nmea );
    }


}


