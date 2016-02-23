/*
    Copyright (c) 2012-2013, BogDan Vatra <bogdan@kde.org>
    Contact: http://www.qt-project.org/legal

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
package org.opencpn;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.util.Log;

import org.opencpn.opencpn.R;

//@ANDROID-11


public class OCPNGRIBFragmentDisplay extends PreferenceFragment {

    public Preference ms52Options;
    public boolean mbs52 = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Log.i("DEBUGGER_TAG", "SettingsFragment charts!");

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences_grib_display);

        Bundle b = getArguments();
        if(b != null)
          mbs52 = b.getString("S52").equalsIgnoreCase("TRUE");


        ms52Options = findPreference("pref_s52Options");
        if(ms52Options != null)
            ms52Options.setEnabled(mbs52);

        m_frag = OCPNGRIBFragmentDisplay.this;
    }

    public void updateChartDirListView(){
        Log.i("DEBUGGER_TAG", "updateChartDirListView");

        Preference cfdp = findPreference("pref_ChartFiles");

        ((ChartFilesDialogPreference) cfdp).updateListView();
    }

    public String getSelected(){
        Preference cfdp = findPreference("pref_ChartFiles");

        return ((ChartFilesDialogPreference) cfdp).getSelected();

    }

    public static OCPNGRIBFragmentDisplay getFragment(){ return m_frag; }
    private static OCPNGRIBFragmentDisplay m_frag;



}
