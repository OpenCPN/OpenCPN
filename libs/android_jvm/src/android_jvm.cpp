/***************************************************************************
 *   Copyright (C) 2015-2023 by David S. Register                          *
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

/** \file android_jvm.cpp   Singleton Android Java JVM interface */
#include <string>
#include <wx/string.h>
#include <QString>

#include "android_jvm.h"


JavaVM* java_vm;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  java_vm = vm;
  return JNI_VERSION_1_6;
}

bool CheckPendingJNIException() {
  if (!java_vm) return false;

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

wxString callActivityMethod_vs(const char *method) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return "NOK";

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "()Ljava/lang/String;");
  if (CheckPendingJNIException()) return "NOK";

  jstring s = data.object<jstring>();
  // qDebug() << s;

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

wxString callActivityMethod_is(const char *method, int parm) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

  JNIEnv *jenv;
  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "(I)Ljava/lang/String;", parm);
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_iis(const char *method, int parm1, int parm2) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

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
      activity.callObjectMethod(method, "(II)Ljava/lang/String;", parm1, parm2);
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_ss(const char *method, wxString parm) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return "NOK";

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

wxString callActivityMethod_s2s(const char *method, const wxString parm1,
                                const wxString parm2) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

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

wxString callActivityMethod_s3s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

  JNIEnv *jenv;
  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return "NOK";

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  wxCharBuffer p3b = parm3.ToUTF8();
  jstring p3 = (jenv)->NewStringUTF(p3b.data());

  //  Call the desired method
  // qDebug() << "Calling method_s3s" << " (" << method << ")";

  QAndroidJniObject data =
      activity.callObjectMethod(method,
                                "(Ljava/lang/String;Ljava/lang/String;Ljava/"
                                "lang/String;)Ljava/lang/String;",
                                p1, p2, p3);
  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);
  (jenv)->DeleteLocalRef(p3);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s3s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_s4s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3,
                                wxString parm4) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

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

wxString callActivityMethod_s2s2i(const char *method, wxString parm1,
                                  wxString parm2, int parm3, int parm4) {
  if (!java_vm) return "NOK";
  if (CheckPendingJNIException()) return "NOK";

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  // qDebug() << "Calling method_s2s2i" << " (" << method << ")";
  // qDebug() << parm3 << parm4;

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;Ljava/lang/String;II)Ljava/lang/String;", p1,
      p2, parm3, parm4);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_ssi(const char *method, wxString parm1, int parm2) {
  if (!java_vm) return "NOK";

  if (CheckPendingJNIException()) return _T("NOK");

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;I)Ljava/lang/String;", p1, parm2);

  (jenv)->DeleteLocalRef(p1);

  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

void androidTerminate() { callActivityMethod_vs("terminateApp"); }
