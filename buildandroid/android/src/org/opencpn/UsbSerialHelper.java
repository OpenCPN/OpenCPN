/*
 * Copyright 2015 David S Register
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


package org.opencpn;

import java.util.Set;
import java.io.IOException;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
//import android.widget.AdapterView;
//import android.widget.ArrayAdapter;
//import android.widget.Button;
//import android.widget.ListView;
//import android.widget.TextView;
//import android.widget.AdapterView.OnItemClickListener;

import java.util.ArrayList;
import android.os.SystemClock;

import android.app.PendingIntent;
import android.content.Intent;

import java.util.HashMap;
import java.util.Iterator;

import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.driver.UsbId;
import com.hoho.android.usbserial.util.HexDump;
import android.os.AsyncTask;
import java.util.ArrayList;
import java.util.List;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.util.SerialInputOutputManager;


import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@SuppressLint("NewApi")
public class UsbSerialHelper {
    private UsbManager mUsbManager;
    private List<UsbSerialPort> mEntries = new ArrayList<UsbSerialPort>();
    private List<portContainer> mActivePorts = new ArrayList<portContainer>();
    private List<portContainer> mPendingPorts = new ArrayList<portContainer>();
    private List<portContainer> mDetectedPorts = new ArrayList<portContainer>();
    private ExecutorService mExecutor = null;
    private PendingIntent mPermissionIntent;

    private boolean usbConnected = false;

    private OCPNNativeLib nativeLib = null;

    private static final String ACTION_USB_PERMISSION =
        "com.android.example.USB_PERMISSION";

    private static final int ACTIVE = 0;
    private static final int PENDING = 1;
    private static final int DETECTED = 2;

    public static final int NOSCAN = 0;
    public static final int SCAN = 1;

    private static final boolean DEBUG = true;

    public UsbSerialHelper() {
    }

    private class portContainer{
        public UsbSerialPort port = null;
        public String friendlyName = "";
        public SerialInputOutputManager sioManager;
        public int baudRate = 4800;
        public int vendorID = 0;
        public int productID = 0;
        public boolean approved = false;

        private boolean updateFriendlyName( int vID, int pID){
            String fName = "";
            if((vID == UsbId.VENDOR_WEGMATT) && (pID == UsbId.WEGMATT_DAISY))
                fName = "AUSBSerial:dAISy";
            else if((vID == UsbId.VENDOR_PROLIFIC) && (pID == UsbId.PROLIFIC_PL2303))
                fName = "AUSBSerial:Prolific_PL2303";
            else if((vID == UsbId.VENDOR_FTDI) && (pID == UsbId.FTDI_FT232R))
                fName = "AUSBSerial:FTDI_FT232R";
            else if((vID == UsbId.VENDOR_FTDI) && (pID == UsbId.FTDI_FT231X))
                fName = "AUSBSerial:FTDI_FT231X";
            else if((vID == UsbId.VENDOR_MICROCHIP) && (pID == UsbId.MCP_000A))
                fName = "AUSBSerial:MCP_000A";
            else if((vID == UsbId.VENDOR_MICROCHIP) && (pID == UsbId.MCP_0205))
                fName = "AUSBSerial:MCP_0205";
            friendlyName = fName;
            return true;
        }

        private boolean updateIDS(){
            String fName = friendlyName;

            if(fName.equals("AUSBSerial:dAISy")){
                vendorID = UsbId.VENDOR_WEGMATT;
                productID = UsbId.WEGMATT_DAISY;
            }

            else if(fName.equals("AUSBSerial:Prolific_PL2303")){
                vendorID = UsbId.VENDOR_PROLIFIC;
                productID = UsbId.PROLIFIC_PL2303;
            }

            else if(fName.equals("AUSBSerial:FTDI_FT232R")){
                vendorID = UsbId.VENDOR_FTDI;
                productID = UsbId.FTDI_FT232R;
            }

            else if(fName.equals("AUSBSerial:FTDI_FT231X")){
                vendorID = UsbId.VENDOR_FTDI;
                productID = UsbId.FTDI_FT231X;
            }

            else if(fName.equals("AUSBSerial:MCP_000A")){
                vendorID = UsbId.VENDOR_MICROCHIP;
                productID = UsbId.MCP_000A;
            }

            else if(fName.equals("AUSBSerial:MCP_0205")){
                vendorID = UsbId.VENDOR_MICROCHIP;
                productID = UsbId.MCP_0205;
            }

            return true;
        }

    };


    public void initUSBSerial(Context context){
        if(null != context){
            mUsbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
            mExecutor = Executors.newSingleThreadExecutor();

            IntentFilter filter = new IntentFilter();
            filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
            context.registerReceiver(mUsbReceiver, filter);

            filter = new IntentFilter();
            filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
            context.registerReceiver(mUsbReceiver, filter);
        }

        nativeLib = OCPNNativeLib.getInstance();
    }

    public void deinitUSBSerial(Context context){
        if(null != context){
            try {
                context.unregisterReceiver(mUsbReceiver);
            } catch(IllegalArgumentException e) {
                if (e.getMessage().indexOf("Receiver not registered") >= 0) {
                        //here you know receiver is not registered, do what you need here
                } else {
                        //other exceptions
                        throw e;
                }
            }
        }
    }


    private portContainer findContainer( int listID, String friendlyName ){
        List<portContainer> list = mDetectedPorts;
        if(listID == ACTIVE)
            list = mActivePorts;
        else if(listID == PENDING)
            list = mPendingPorts;

        for(int i=0 ; i < list.size() ; i++){
            portContainer p = list.get(i);
            if(null != p){
                if(p.friendlyName.equals(friendlyName))
                    return p;
            }
        }
        return null;
    }


    private portContainer findContainer( int listID, int vID, int pID ){
        List<portContainer> list = mDetectedPorts;
        if(listID == ACTIVE)
            list = mActivePorts;
        else if(listID == PENDING)
            list = mPendingPorts;

        for(int i=0 ; i < list.size() ; i++){
            portContainer p = list.get(i);
            if(null != p){
                if((p.productID == pID) && (p.vendorID == vID))
                    return p;
            }
        }
        return null;
    }

    public String scanSerialPorts( final int parm ){
        String ret_str = "";

        if(parm == SCAN)
            refreshDeviceList();

        for(int i=0 ; i < mDetectedPorts.size() ; i++){
            portContainer cport = mDetectedPorts.get(i);

            if((null != cport) && (null != cport.port)){
                UsbSerialDriver driver = cport.port.getDriver();
                UsbDevice device = driver.getDevice();

                String title = String.format("Vendor %s Product %s",
                    HexDump.toHexString((short) device.getVendorId()),
                    HexDump.toHexString((short) device.getProductId()));

                String subtitle = driver.getClass().getSimpleName();
                if(DEBUG) Log.d("OpenCPN", "scanSerialPorts Port: " + title + " " + subtitle);

                String friendlyNameP = cport.friendlyName + ";";

                ret_str += friendlyNameP;
            }
        }


        if(DEBUG) Log.d("OpenCPN", "scanSerialPorts results:" + ret_str);

        return ret_str;

    }

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

        String messageNMEA = "";

        @Override
        public void onRunError(Exception e) {
            if(DEBUG) Log.d("OpenCPN", "Runner stopped.");
        }

        @Override
        public void onNewData(final byte[] data) {
//            final String message = "Read " + data.length + " bytes: \n" + HexDump.dumpHexString(data) + "\n\n";
//            if(DEBUG) Log.d("OpenCPN", "onNewData " + message);

            messageNMEA += new String(data);

            int eol = messageNMEA.indexOf(0x0a);
            if(-1 != eol){
                String smsg = messageNMEA.substring(0, eol);
                smsg = smsg.trim();
                smsg += "\n";

                if(DEBUG) Log.d("OpenCPN", "Formatted " + smsg);
                if(null != nativeLib)
                    nativeLib.processNMEA( smsg );
                if(eol < messageNMEA.length()){
                    String m1 = messageNMEA.substring(eol+1);
                    messageNMEA = m1;
                }
                else{
                    messageNMEA = "";
                }
            }


        }
    };

    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
         @Override
         public void onReceive(Context context, Intent intent) {

             String action = intent.getAction();

             if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                 if(DEBUG) Log.d("OpenCPN", "BroadcastReceiver ACTION_USB_DEVICE_ATTACHED");

                 UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                 if (device != null) {
                     if(DEBUG) Log.d("OpenCPN", "USB Attached: ");

                     UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
                     if(null != driver){
                        List<UsbSerialPort> portsList = driver.getPorts();
                        UsbSerialPort port = null;
                        if(null != portsList){
                            if(portsList.size() > 0)
                                port = portsList.get(0);

                            if(DEBUG) Log.d("OpenCPN", "USB Attached: Probe found port: " + port);

                            portContainer tentPort = new portContainer();
                            tentPort.friendlyName = "";
                            tentPort.port = port;
                            tentPort.sioManager = null;
                            tentPort.vendorID = device.getVendorId();
                            tentPort.productID = device.getProductId();
                            tentPort.updateFriendlyName(device.getVendorId(), device.getProductId());

                      //  Refresh the entry in the DETECTED list
                            portContainer p = findContainer( DETECTED, tentPort.friendlyName );
                            if(null != p)
                                mDetectedPorts.remove(p);
                            mDetectedPorts.add(tentPort);

                     //  Refresh the entry in the PENDING list
                            p = findContainer( PENDING, tentPort.friendlyName );
                            if(null != p)
                                mPendingPorts.remove(p);
                            mPendingPorts.add(tentPort);

                     // and start it
                            p = findContainer( PENDING, tentPort.friendlyName );
                            if(null != p)
                                startUSBSerialPort( p.friendlyName, p.baudRate );
                        }
                    }
                 }

                 usbConnected=true;
             }

             if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                 if(DEBUG) Log.d("OpenCPN", "BroadcastReceiver ACTION_USB_DEVICE_DETACHED");
                 UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                 if (device != null) {
                     if(DEBUG) Log.d("OpenCPN", "USB Detached: ");
                     portContainer p = findContainer( ACTIVE, device.getVendorId(), device.getProductId() );
                     if(null != p)
                        stopUSBSerialPort( p.friendlyName );

                 }
                 usbConnected=false;
             }

             if (ACTION_USB_PERMISSION.equals(action)) {
                 synchronized (this) {
                     if(DEBUG) Log.d("OpenCPN", "BroadcastReceiver ACTION_USB_PERMISSION");
                     UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                     if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                         if(device != null){
                             if(DEBUG) Log.d("OpenCPN", "permission approved for device " + device);
                             //call method to set up device communication
                             portContainer p = findContainer( PENDING, device.getVendorId(), device.getProductId() );
                             if(null != p){
                                 if(null == p.port){
                                     if(DEBUG) Log.d("OpenCPN", "...except port is null");

                                     // Probe for it
                                     UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
                                     List<UsbSerialPort> ports = driver.getPorts();
                                     UsbSerialPort port = null;
                                     if(ports.size() > 0)
                                        port = ports.get(0);

                                     if(DEBUG) Log.d("OpenCPN", "...so substituting from probe");
                                     p.port = port;

                                 }
                                 connectUSBSerialPort( p );
                             }
                         }
                     }
                     else {
                         if(DEBUG) Log.d("OpenCPN", "permission denied for device " + device);
                     }
                 }
             }
         }
     };






    private void refreshDeviceList() {

        new AsyncTask<Void, Void, List<UsbSerialPort>>() {
            @Override
            protected List<UsbSerialPort> doInBackground(Void... params) {
                if(DEBUG) Log.d("OpenCPN", "Refreshing device list ...");
//                SystemClock.sleep(1000);

                final List<UsbSerialDriver> drivers =
                        UsbSerialProber.getDefaultProber().findAllDrivers(mUsbManager);

                final List<UsbSerialPort> result = new ArrayList<UsbSerialPort>();
                for (final UsbSerialDriver driver : drivers) {
                    final List<UsbSerialPort> ports = driver.getPorts();
                    if(DEBUG) Log.d("OpenCPN", String.format("+ %s: %s port%s",
                            driver, Integer.valueOf(ports.size()), ports.size() == 1 ? "" : "s"));
                    result.addAll(ports);
                }

                return result;
            }

            @Override
            protected void onPostExecute(List<UsbSerialPort> result) {
                mEntries.clear();
                mEntries.addAll(result);
                if(DEBUG) Log.d("OpenCPN", "Done refreshing, " + mEntries.size() + " entries found.");

                mDetectedPorts.clear();
                for(int i=0 ; i < mEntries.size() ; i++){
                    UsbSerialPort port = mEntries.get(i);

                    if(null != port){
                        UsbSerialDriver driver = port.getDriver();
                        UsbDevice device = driver.getDevice();

                        String title = String.format("Vendor %s Product %s",
                            HexDump.toHexString((short) device.getVendorId()),
                            HexDump.toHexString((short) device.getProductId()));

                        String subtitle = driver.getClass().getSimpleName();
                        if(DEBUG) Log.d("OpenCPN", "refreshDeviceList Port: " + title + " " + subtitle);

                        portContainer container = new portContainer();
                        container.port = port;
                        container.sioManager = null;
                        container.vendorID = device.getVendorId();
                        container.productID = device.getProductId();
                        container.updateFriendlyName(device.getVendorId(),device.getProductId());

                        mDetectedPorts.add(container);

                    }
                }
            }
        }.execute((Void) null);
    }

    private void stopIoManager(portContainer port) {
        if(null != port){
            if (port.sioManager != null) {
                if(DEBUG) Log.i("OpenCPN", "Stopping io manager for port " + port.friendlyName);
                port.sioManager.stop();
                port.sioManager = null;
            }
        }
    }

    private void startIoManager(portContainer port) {
        if (port != null) {
            if(DEBUG) Log.i("OpenCPN", "Starting io manager for port "+ port.friendlyName );
            port.sioManager = new SerialInputOutputManager(port.port, mListener);
            mExecutor.submit(port.sioManager);
        }
    }

    private void onDeviceStateChange(portContainer port) {
        stopIoManager(port);
        startIoManager(port);
    }

    public String startUSBSerialPort( String friendlyName, int baudRate ){
        if(DEBUG) Log.d("OpenCPN", "Trying startUSBSerialPort for friendlyName=" + friendlyName);

        portContainer container = findContainer( ACTIVE, friendlyName );
        if(null != container){
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort port already running, OK.");
            return "OK";
        }




        // Find the port in the detected list, which will have been updated in the Broadcast receiver
        boolean bFound = false;
        UsbSerialPort port = null;

        container = findContainer( DETECTED, friendlyName );
        if(null != container){
            if(DEBUG) Log.i("OpenCPN", "startUSBSerialPort: starting friendly name :" + friendlyName);
            bFound = true;
            port = container.port;
            container.baudRate = baudRate;
            if(DEBUG) Log.i("OpenCPN", "startUSBSerialPort: port: " + port);
        }

        if(bFound && (null != port)){
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort adding port to PENDING list");

            portContainer p = findContainer( PENDING, friendlyName );
            if(null != p)
                mPendingPorts.remove(p);

            portContainer tentPort = new portContainer();
            tentPort.friendlyName = friendlyName;
            tentPort.port = port;
            tentPort.sioManager = null;
            tentPort.baudRate = baudRate;
            tentPort.updateIDS();
            mPendingPorts.add(tentPort);


            //  It seems that if one uses an intent in the manifest, then one does not need (nor want) to ask permissison
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort:  Assuming have permission...");
            p = findContainer( PENDING, friendlyName );
            if(null != p){
                if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort:   proceeding to connect");
                connectUSBSerialPort(p);
            }

/*
            if( mUsbManager.hasPermission(port.getDriver().getDevice())){
                if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort:  Have permission");
                p = findContainer( PENDING, friendlyName );
                if(null != p){
                    if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort:   proceeding to connect");
                    connectUSBSerialPort(p);
                }
            }
            else{
                if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort Request Permission,  port=" + port + " baud=" + String.valueOf(baudRate)
                + " VID:" + String.valueOf(port.getDriver().getDevice().getVendorId())
                + " PID:" + String.valueOf(port.getDriver().getDevice().getProductId()));

                mPermissionIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(ACTION_USB_PERMISSION), 0);
                IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
                mContext.registerReceiver(mUsbReceiver, filter);
                mUsbManager.requestPermission(port.getDriver().getDevice(), mPermissionIntent);
            }
*/


        }


        if(bFound && (null != port)){
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort OK");
            return "OK";
        }
        else if(null == port){
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort fail, port is null");
            return "portNULL";
        }
        else{
            if(DEBUG) Log.d("OpenCPN", "startUSBSerialPort fail, NameNotFound");
            return "NameNotFound";
        }
    }


    public String connectUSBSerialPort(portContainer container){
        UsbSerialPort port = container.port;
        if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort"  + " baud=" + String.valueOf(container.baudRate) );

        if(null != port){
            if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort,  port=" + port + " baud=" + String.valueOf(container.baudRate) );

                if(mUsbManager.hasPermission(port.getDriver().getDevice()))
                    if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort,  Permission OK" );
                else
                    if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort,  Permission DENIED" );


                UsbDeviceConnection connection = mUsbManager.openDevice(port.getDriver().getDevice());
                if (connection == null) {
                    if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort device failed " +  port.getDriver().getDevice().getDeviceName());
                    return "DEVICE_OPEN_FAIL";
                }

                try {
                    port.open(connection);
                    port.setParameters(container.baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

                    if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort: Driver Simple Name is " + port.getDriver().getClass().getSimpleName());
                    if(port.getDriver().getClass().getSimpleName().equalsIgnoreCase("CdcAcmSerialDriver")){
//                        if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort: setting DTR for CdcAcmSerialDriver");
//                        port.setDTR(true);
                    }

                } catch (IOException e) {
                    if(DEBUG) Log.e("OpenCPN", "Error setting up device: " + e.getMessage(), e);
                    try {
                        port.close();
                    } catch (IOException e2) {
                        // Ignore.
                    }

                    return "DEVICE_SETUP_ERROR";
                }

            onDeviceStateChange(container);

            //  shift the container into the correct list
            portContainer pc = new portContainer();
            pc.port = container.port;
            pc.friendlyName = container.friendlyName;
            pc.sioManager = container.sioManager;
            pc.baudRate = container.baudRate;
            pc.vendorID = container.vendorID;
            pc.productID = container.productID;
            pc.approved = container.approved;

            mPendingPorts.remove(container);
            mActivePorts.add(pc);

        }
        else{
            if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort port is NULL");

            //  Is this container already in the pending list?
            //  If so, don't add it again
            if(null == findContainer( PENDING, container.friendlyName )){
                if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort adding port to PENDING list");
                portContainer tentPort = new portContainer();
                tentPort.friendlyName = container.friendlyName;
                tentPort.port = null;
                tentPort.sioManager = null;
                tentPort.vendorID = container.vendorID;
                tentPort.productID = container.productID;

                mPendingPorts.add(tentPort);

            }
            else{
                if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort already in PENDING list");
            }

        }

        if(DEBUG) Log.d("OpenCPN", "connectUSBSerialPort OK" );

        return "OK";
    }


    public String stopUSBSerialPort( String friendlyName ){
        if(DEBUG) Log.d("OpenCPN", "stopUSBSerialPort,  friendlyName=" + friendlyName);

        String ret = "";

        // Find the port that maps to the friendly name

        // Find the port that maps to the friendly name
        boolean bFound = false;
        UsbSerialPort port = null;

        portContainer container = findContainer( ACTIVE, friendlyName );
        if(null != container){
            if(DEBUG) Log.i("OpenCPN", "stopUSBSerialPort: stopping friendly name :" + friendlyName);
            bFound = true;
            port = container.port;
        }
        stopIoManager(container);


        if(bFound && (null != port)){
            if(DEBUG) Log.d("OpenCPN", "Closing,  port=" + port);


            try {
                port.close();
            } catch (IOException e) {
                if(DEBUG) Log.e("OpenCPN", "Error closing device: " + e.getMessage(), e);
                try {
                    port.close();
                } catch (IOException e2) {
                    // Ignore.
                }

                ret = "_DEVICE_CLOSE_ERROR";
            }

        }

        portContainer pc = new portContainer();
        if(null != container){
        pc.port = container.port;
        pc.friendlyName = container.friendlyName;
        pc.sioManager = container.sioManager;
        pc.baudRate = container.baudRate;
        pc.vendorID = container.vendorID;
        pc.productID = container.productID;

        mActivePorts.remove(container);
        }

/*
        //  Is this container already in the pending list?
        //  If so, don't add it again
        if(null == findContainer( PENDING, friendlyName )){
            if(DEBUG) Log.d("OpenCPN", "stopUSBSerialPort adding port to PENDING list");
            pc.port = null;
            pc.sioManager = null;

            mPendingPorts.add(pc);

        }
        else{
            if(DEBUG) Log.d("OpenCPN", "stopUSBSerialPort already in PENDING list");
        }
*/
        if(bFound){
            if(DEBUG) Log.d("OpenCPN", "stopUSBSerialPort OK,  device: " + port.getClass().getSimpleName());

            return "OK" + ret;
        }
        else
            return "NameNotFound";
    }






}
