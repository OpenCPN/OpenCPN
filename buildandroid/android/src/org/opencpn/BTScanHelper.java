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

@SuppressLint("NewApi")
public class BTScanHelper {
    // Debugging
    private static final String TAG = "BluetoothSPP";
    private static final boolean D = true;

    // Member fields
    private BluetoothAdapter mBtAdapter;
//    private ArrayAdapter<String> mPairedDevicesArrayAdapter;
    private ArrayList<String> mPairedDevicesArrayAdapter;
    private Set<BluetoothDevice> pairedDevices;
//    private Button scanButton;
    private Context m_context;

    private boolean m_receiverRegistered = false;

    public String getDiscoveredDevices(){
        String ret_str = "";;

        for(int i = 0 ; i < mPairedDevicesArrayAdapter.size() ; i++){
            ret_str = ret_str.concat(mPairedDevicesArrayAdapter.get(i));
            ret_str = ret_str.concat(";");
        }

        return ret_str;

    }

    public BTScanHelper(Context context) {
        Log.i("DEBUGGER_TAG", "start BTScanHelper ctor");
        m_context = context;

         // Initialize array adapters. One for already paired devices
        // and one for newly discovered devices
        //int layout_text = getIntent().getIntExtra("layout_text", R.layout.device_name);
        mPairedDevicesArrayAdapter = new ArrayList();

         // Register for broadcasts when a device is discovered
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        m_context.registerReceiver(mReceiver, filter);

        // Register for broadcasts when discovery has finished
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);

        if(!m_receiverRegistered){
            m_context.registerReceiver(mReceiver, filter);
            m_receiverRegistered = true;
        }


        // Get the local Bluetooth adapter
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();

        // Get a set of currently paired devices
        pairedDevices = mBtAdapter.getBondedDevices();

        // If there are paired devices, add each one to the ArrayAdapter
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                String paired = device.getName() + ";" + device.getAddress();
                Log.i("DEBUGGER_TAG", "paired");
                Log.i("DEBUGGER_TAG", paired);

                mPairedDevicesArrayAdapter.add(paired);
            }
        } else {
            String noDevices = "No devices found";
            mPairedDevicesArrayAdapter.add(noDevices);
        }
        Log.i("DEBUGGER_TAG", "end BTScanHelper ctor");

    }

    public void stopDiscovery() {
        if (D) Log.d(TAG, "stopDiscovery()");
        // Make sure we're not doing discovery anymore
        if (mBtAdapter != null) {
            mBtAdapter.cancelDiscovery();
        }

        // Unregister broadcast listeners
        if(m_receiverRegistered){
            m_context.unregisterReceiver(mReceiver);
            m_receiverRegistered = false;
        }
        mPairedDevicesArrayAdapter.set(0, "Finished");


    }

    // Start device discover with the BluetoothAdapter
    public void doDiscovery() {
        if (D) Log.d(TAG, "doDiscovery()");

        // Remove all element from the list
        mPairedDevicesArrayAdapter.clear();

        String scanning = "Scanning";
        mPairedDevicesArrayAdapter.add(scanning);

        // If there are paired devices, add each one to the ArrayAdapter
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                String paired = device.getName() + ";" + device.getAddress();
                Log.i("DEBUGGER_TAG", "pairedC");
                Log.i("DEBUGGER_TAG",  paired);

                mPairedDevicesArrayAdapter.add(paired);
            }
        } else {
            String strNoFound = "No devices found";
            mPairedDevicesArrayAdapter.add(strNoFound);
        }

         // If we're already discovering, stop it
        if (mBtAdapter.isDiscovering()) {
            mBtAdapter.cancelDiscovery();
        }

        // Request discover from BluetoothAdapter
        mBtAdapter.startDiscovery();

        if (D) Log.d(TAG, "doDiscovery() done");

    }


    // The BroadcastReceiver that listens for discovered devices
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            // When discovery finds a device
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                Log.i("DEBUGGER_TAG", "Action Found");

                // Get the BluetoothDevice object from the Intent
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                // If it's already paired, skip it, because it's been listed already
                if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                    String strNoFound = "No devices found";

                        if(mPairedDevicesArrayAdapter.get(1).equals(strNoFound)) {
                                mPairedDevicesArrayAdapter.remove(strNoFound);
                        }

                        String found = " ";
                        found = device.getName() + ";" + device.getAddress();


                        mPairedDevicesArrayAdapter.add(found);
                        Log.i("DEBUGGER_TAG", "found" + found);
                }

            // When discovery is finished, change the Activity title
            }
            else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                Log.i("DEBUGGER_TAG", "BTScan Action Finished");
                mPairedDevicesArrayAdapter.set(0, "Finished");

//                setProgressBarIndeterminateVisibility(false);
//                String strSelectDevice = getIntent().getStringExtra("select_device");
//                if(strSelectDevice == null)
//                        strSelectDevice = "Select a device to connect";
//                setTitle(strSelectDevice);
            }
        }
    };

}
