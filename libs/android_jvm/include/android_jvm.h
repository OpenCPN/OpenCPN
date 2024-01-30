 /**************************************************************************
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

/** \file android_jvm.h   Singleton Android Java JVM interface */

#ifndef ANDROID_JVM_H
#define ANDROID_JVM_H

#include <jni.h>

#include <QtAndroidExtras/QAndroidJniObject>

#include <wx/string.h>

/** Main thread java JVM. Worker threads needs to set up their own. */
extern JavaVM* java_vm;

/** Invoked by Android on start. */
jint JNI_OnLoad(JavaVM *vm, void *reserved);

bool CheckPendingJNIException();

wxString callActivityMethod_vs(const char *method);

wxString callActivityMethod_is(const char *method, int parm);
wxString callActivityMethod_iis(const char *method, int parm1, int parm2);
wxString callActivityMethod_ss(const char *method, wxString parm);
wxString callActivityMethod_s2s(const char *method, const wxString parm1,
                                const wxString parm2);
wxString callActivityMethod_s3s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3);
wxString callActivityMethod_s4s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3,
                                wxString parm4);
wxString callActivityMethod_s2s2i(const char *method, wxString parm1,
                                  wxString parm2, int parm3, int parm4);
wxString callActivityMethod_ssi(const char *method, wxString parm1, int parm2);

void androidTerminate();


#endif  // guard
