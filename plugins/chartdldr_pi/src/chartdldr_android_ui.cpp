/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_android_ui.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>

double g_androidDPmm = 4.0;

#ifdef __ANDROID__

#include "androidSupport.h"
#include "android_jvm.h"

#include <QtAndroidExtras/QAndroidJniObject>
#include <jni.h>
#include <wx/tokenzr.h>

void ChartDldrSetAndroidBackColor(wxWindow* ctrl, wxColour col) {
  static int depth = 0;
  if (depth == 0) {
    ctrl->SetBackgroundColour(col);
  }

  wxWindowList kids = ctrl->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();

    if (dynamic_cast<wxListBox*>(win))
      dynamic_cast<wxListBox*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxTextCtrl*>(win))
      dynamic_cast<wxTextCtrl*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxChoice*>(win))
      dynamic_cast<wxChoice*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxComboBox*>(win))
      dynamic_cast<wxComboBox*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxRadioButton*>(win))
      dynamic_cast<wxRadioButton*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxScrolledWindow*>(win))
      dynamic_cast<wxScrolledWindow*>(win)->SetBackgroundColour(col);
    else if (dynamic_cast<wxButton*>(win))
      dynamic_cast<wxButton*>(win)->SetBackgroundColour(col);

    if (win->GetChildren().GetCount() > 0) {
      depth++;
      ChartDldrSetAndroidBackColor(win, col);
      depth--;
    }
  }
}

bool ChartDldrInitAndroidDisplayMetrics() {
  g_androidDPmm = 4.0;

  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (!activity.isValid()) {
    return false;
  }

  QAndroidJniObject data =
      activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();
  JNIEnv* jenv;
  if (java_vm->GetEnv((void**)&jenv, JNI_VERSION_1_6) == JNI_OK) {
    const char* ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return_string.Replace(_T(","), _T("."));

  double density = 1.0;
  wxStringTokenizer tk(return_string, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();  // xdpi
    token = tk.GetNextToken();           // density
    token.ToDouble(&density);
  }

  const double ldpi = 160. * density;
  g_androidDPmm = ldpi / 25.4;
  return true;
}

#endif  // __ANDROID__
