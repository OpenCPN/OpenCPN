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
}
