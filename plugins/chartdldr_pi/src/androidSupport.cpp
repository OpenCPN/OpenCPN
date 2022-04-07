
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Android support utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "androidSupport.h"
#include <wx/tokenzr.h>

#include <QtAndroidExtras/QAndroidJniObject>
#include "qdebug.h"

extern int g_Android_SDK_Version;

extern JavaVM *java_vm;  // found in androidUtil.cpp, accidentally exported....
extern JNIEnv *jenv;

bool CheckPendingJNIException() {
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return true;

  if ((jenv)->ExceptionCheck() == JNI_TRUE) {
    // Handle exception here.
    (jenv)->ExceptionDescribe();  // writes to logcat
    (jenv)->ExceptionClear();

    return false;  // There was a pending exception, but cleared OK
                   // interesting discussion:
    // http://blog.httrack.com/blog/2013/08/23/catching-posix-signals-on-android/
  }

  return false;
}

wxString callActivityMethod_s4s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3,
                                wxString parm4) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  wxCharBuffer p3b = parm3.ToUTF8();
  jstring p3 = (jenv)->NewStringUTF(p3b.data());

  wxCharBuffer p4b = parm4.ToUTF8();
  jstring p4 = (jenv)->NewStringUTF(p4b.data());

  // const char *ts = (jenv)->GetStringUTFChars(p2, NULL);
  // qDebug() << "Test String p2" << ts;

  //  Call the desired method
  // qDebug() << "Calling method_s4s" << " (" << method << ")";

  QAndroidJniObject data = activity.callObjectMethod(
      method,
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
      "String;)Ljava/lang/String;",
      p1, p2, p3, p4);
  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);
  (jenv)->DeleteLocalRef(p3);
  (jenv)->DeleteLocalRef(p4);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s4s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_ss(const char *method, wxString parm) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  jstring p = (jenv)->NewStringUTF(parm.c_str());

  //  Call the desired method
  // qDebug() << "Calling method_ss";
  // qDebug() << method;

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;)Ljava/lang/String;", p);

  (jenv)->DeleteLocalRef(p);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_ss";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_vs(const char *method) {
  if (CheckPendingJNIException()) return _T("NOK");

  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "()Ljava/lang/String;");
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  if (s) {
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
      // qDebug() << "GetEnv failed.";
    } else {
      const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
      return_string = wxString(ret_string, wxConvUTF8);
    }
  }

  return return_string;
}

wxString callActivityMethod_s2s(const char *method, wxString parm1,
                                wxString parm2) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  jstring p1 = (jenv)->NewStringUTF(parm1.c_str());
  jstring p2 = (jenv)->NewStringUTF(parm2.c_str());

  //  Call the desired method
  // qDebug() << "Calling method_s2s" << " (" << method << ")";

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1,
      p2);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s2s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

bool AndroidUnzip(wxString zipFile, wxString destDir, int nStrip,
                  bool bRemoveZip) {
  qDebug() << "AndroidUnzip" << zipFile.mb_str() << destDir.mb_str();

  wxString ns;
  ns.Printf(_T("%d"), nStrip);

  wxString br = _T("0");
  if (bRemoveZip) br = _T("1");

  qDebug() << "br" << br.mb_str();

  wxString stat = callActivityMethod_s4s("unzipFile", zipFile, destDir, ns, br);

  if (wxNOT_FOUND == stat.Find(_T("OK"))) return false;

  qDebug() << "unzip start";

  bool bDone = false;
  while (!bDone) {
    wxMilliSleep(1000);
    // wxSafeYield(NULL, true);

    qDebug() << "unzip poll";

    wxString result = callActivityMethod_ss("getUnzipStatus", _T(""));
    if (wxNOT_FOUND != result.Find(_T("DONE"))) bDone = true;
  }
  qDebug() << "unzip done";

  return true;
}

wxString AndroidGetCacheDir() {
  wxString dirs = callActivityMethod_vs("getSystemDirs");
  qDebug() << "dirs: " << dirs.mb_str();

  wxString cacheDir;

  wxStringTokenizer tk(dirs, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();
    //        if(wxNOT_FOUND != token.Find(_T("EXTAPP")))
    //            g_bExternalApp = true;

    token = tk.GetNextToken();
    //        g_androidFilesDir = token;              // used for "home dir"
    token = tk.GetNextToken();
    //        g_androidCacheDir = token;
    token = tk.GetNextToken();
    //        g_androidExtFilesDir = token;           // used as PrivateDataDir,
    //        "/storage/emulated/0/Android/data/org.opencpn.opencpn/files"
    // if app has been moved to sdcard, this gives like (on Android 6)
    // /storage/2385-1BF8/Android/data/org.opencpn.opencpn/files

    token = tk.GetNextToken();
    cacheDir = token;

    //        token = tk.GetNextToken();
    //        g_androidExtStorageDir = token;
  }

  return cacheDir;
}

bool AndroidSecureCopyFile(wxString in, wxString out) {
  bool bret = true;

  wxString result = callActivityMethod_s2s("SecureFileCopy", in, out);

  if (wxNOT_FOUND == result.Find(_T("OK"))) bret = false;

  return bret;
}

bool b_androidBusyShown;
void androidShowBusyIcon() {
  if (b_androidBusyShown) return;

  // qDebug() << "ShowBusy";

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("showBusyCircle", "()Ljava/lang/String;");

  b_androidBusyShown = true;
}

void androidHideBusyIcon() {
  if (!b_androidBusyShown) return;

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("hideBusyCircle", "()Ljava/lang/String;");

  b_androidBusyShown = false;
}

void androidEnableRotation(void) {
  callActivityMethod_vs("EnableRotation");
}

void androidDisableRotation(void) {
  callActivityMethod_vs("DisableRotation");
}

int androidGetSDKVersion() {
  wxString deviceInfo = callActivityMethod_vs("getDeviceInfo");
  wxStringTokenizer tkz(deviceInfo, _T("\n"));
  while (tkz.HasMoreTokens()) {
    wxString s1 = tkz.GetNextToken();
    if (wxNOT_FOUND != s1.Find(_T("OS API Level"))) {
      int a = s1.Find(_T("{"));
      if (wxNOT_FOUND != a) {
        wxString b = s1.Mid(a + 1, 2);
        long SDK;
        b.ToLong(&SDK);
        g_Android_SDK_Version = SDK;
      }
    }
  }
  return g_Android_SDK_Version;
}

