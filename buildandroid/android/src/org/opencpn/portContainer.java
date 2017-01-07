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
public class portContainer{
        public UsbSerialPort port = null;
        private String friendlyName = "";
        public SerialInputOutputManager sioManager;
        public int baudRate = 4800;
        public int vendorID = 0;
        public int productID = 0;
        public boolean approved = false;

        public String prefString = "";

        public String getFriendlyName(){
            return friendlyName;
        }
        public void setFriendlyName( String name){
            friendlyName = name;
            updateIDS();
            updatePrefString();
        }

        public void updateFriendlyName(){
            String fName = "";
//            if((vendorID == UsbId.VENDOR_WEGMATT) && (productID == UsbId.WEGMATT_DAISY)){
//                fName = "AUSBSerial:dAISy";
//                prefString = "prefb_dAISy";
//            }
            if((vendorID == UsbId.VENDOR_PROLIFIC) && (productID == UsbId.PROLIFIC_PL2303)){
                fName = "AUSBSerial:Prolific_PL2303";
                prefString = "prefb_PL2303";
            }
            else if((vendorID == UsbId.VENDOR_FTDI) && (productID == UsbId.FTDI_FT232R)){
                fName = "AUSBSerial:FTDI_FT232R";
                prefString = "prefb_FT232R";
            }
            else if((vendorID == UsbId.VENDOR_FTDI) && (productID == UsbId.FTDI_FT231X)){
                fName = "AUSBSerial:FTDI_FT231X";
                prefString = "prefb_FT231X";
            }
            else{
                fName = String.format("AUSBSerial:USBDP-%04X/%04X", vendorID, productID);
                prefString = "prefb_USBDP";
            }

            friendlyName = fName;
        }

        private void updateIDS(){
            String fName = friendlyName;

//            if(fName.equals("AUSBSerial:dAISy")){
//                vendorID = UsbId.VENDOR_WEGMATT;
//                productID = UsbId.WEGMATT_DAISY;
//            }

            if(fName.equals("AUSBSerial:Prolific_PL2303")){
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

            else{
                if(fName.contains("USBDP-")){

                    String numString = fName.substring(fName.indexOf("USBDP-") + 6).trim();
                    String[] sarray = numString.split("/");
                    Log.d("OpenCPN", "parsing:" + numString + " " + sarray[0] + " " + sarray[1]);

                    try {
                        vendorID = Integer.parseInt(sarray[0], 16);
                    } catch(NumberFormatException e) {
                       System.out.println("parse value is not valid : " + e);
                    }

                    try {
                        productID = Integer.parseInt(sarray[1], 16);
                    } catch(NumberFormatException e) {
                       System.out.println("parse value is not valid : " + e);
                    }

                    Log.d("OpenCPN", "parse device " + String.format("%04X/%04X\n", vendorID, productID));
                }
            }


        }

        private void updatePrefString(){
            String fName = friendlyName;

//            if(fName.equals("AUSBSerial:dAISy")){
//                prefString = "prefb_dAISy";
//            }

            if(fName.equals("AUSBSerial:Prolific_PL2303")){
                prefString = "prefb_PL2303";
            }

            else if(fName.equals("AUSBSerial:FTDI_FT232R")){
                prefString = "prefb_FT232R";
            }

            else if(fName.equals("AUSBSerial:FTDI_FT231X")){
                prefString = "prefb_FT231X";
            }

            else{
                if(fName.contains("USBDP")){
                    prefString = "prefb_USBDP";
                }
            }
        }
};

