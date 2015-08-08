package org.opencpn;

public class OCPNNativeLib {

//  static {
//    System.loadLibrary("ndk_demo");
//  }

  /**
   * Adds two integers, returning their sum
   */
  public native int add( int v1, int v2 );

  /**
   * Returns Hello World string
   */
  public native String hello();

  public native int test();

  public native int processNMEA(String nmea_string);
  public native int processBTNMEA(String nmea_string);

  public native int onConfigChange();

  public native int onMenuKey();

  public native int onStart();
  public native int onStop();
  public native int onPause();
  public native int onResume();

  public native int invokeMenuItem( int item);
  public native int selectChartDisplay( int type, int family);
  public native int invokeCmdEventCmdString( int cmd_id, String s);
  public native String getVPCorners();

  public native int setDownloadStatus( int status, String url);
}
