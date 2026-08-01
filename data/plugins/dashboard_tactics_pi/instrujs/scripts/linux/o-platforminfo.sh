#!/usr/bin/env bash
# /* $Id: o-platforminfo.sh, v1.0 2019/11/30 VaderDarth Exp $ */
#
# Displaying information about the installed libraries and OpenCPN


echo ""
echo "+--------------------------------------------------------------------------+"
echo "!                                                                          !"
echo "!             o-platforminfo.sh of dashboard_tactics_pi                    !"
echo "!                  (looking for some utilities)                            !"
echo "+--------------------------------------------------------------------------+"
echo ""

READELFPATH=$(which readelf)
if [ -z "${READELFPATH}" ]
then
    echo "Cannot find 'readelf' from the path, attempting with:"
    READELFPATH="/usr/bin/readelf"
fi
echo "- '${READELFPATH}'"
if [ -f ${READELFPATH} ]
then
    echo "Found '${READELFPATH}(1)'."
else
    echo "Cannot find '${READELFPATH}(1)'."
    READELFPATH=""
fi
LDDPATH=$(which ldd)

if [ -z "${LDDPATH}" ]
then
    echo "Cannot find 'ldd(1)' from the path, attempting with:"
    LDDPATH="/usr/bin/ldd"
fi
echo "- '${LDDPATH}(1)'"
if [ -f ${LDDPATH} ]
then
    echo "Found '${LDDPATH}(1)'."
else
    echo "Cannot find '${LDDPATH}(1)'."
    LDDPATH=""
fi
APTPATH=$(which apt)
DPKGPATH=""
LISTCMD=""
if [ -z "${APTPATH}" ]
then
    echo "Cannot find 'apt(8)' from the path, attempting with:"
    APTPATH="/usr/bin/apt"
fi
echo "- '${APTPATH}(8)'"
if [ -f ${APTPATH} ]
then
    echo "Found '${APTPATH}(8)'."
    LISTCMD="${APTPATH} list --installed"
else
    echo "Cannot find '${APTPATH}(8)'. Attempting with 'dpkg(1)'"
    APTPATH=""
    DPKGPATH=$(which dpkg)
    if [ -f ${DPKGPATH} ]
    then
        echo "Found '${DPKGPATH}(1)'."
        LISTCMD="${DPKGPATH} --list"
    else
        echo "Cannot find '${DPKGPATH}(1)'. Will not search."
        DPKGPATH=""
    fi
fi

echo ""
echo "+--------------------------------------------------------------------------+"
echo "!                                                                          !"
echo "!                     Platform's Linux Distribution                        !"
echo "!                                                                          !"
echo "+--------------------------------------------------------------------------+"
echo ""

HOSTNAMECTL=$(which hostnamectl)
FILEOSRELS="/etc/os-release"
if [ -z "${HOSTNAMECTL}" ]
then
    if [ -f ${FILEOSRELS} ]
    then
        cat ${FILEOSRELS}
    else
        echo "Cannot find OS-info. Attempted with:"
        echo "- hostnamectl(1) utility not available"
        echo "- /etc/os-release file not found"
    fi
else
    ${HOSTNAMECTL}
fi

echo ""
echo "+--------------------------------------------------------------------------+"
echo "!                                                                          !"
echo "!                                OpenCPN                                   !"
echo "!                                                                          !"
echo "+--------------------------------------------------------------------------+"
echo ""

OCPNPATH=$(which opencpn)
if [ -z "${OCPNPATH}" ]
then
    echo "Cannot find 'opencpn' from the path, attempting with:"
    OCPNPATH="/usr/bin/opencpn"
fi
echo "- '${OCPNPATH}'"
if [ -f ${OCPNPATH} ]
then
    echo "Found '${OCPNPATH}' file."
else
    echo "Cannot find '${OCPNPATH}' file."
    OCPNPATH=""
fi

if [ \( ! -z "${OCPNPATH}" -a ! -z "${READELFPATH}" \) ]
then
    echo ""
    echo "     >>>>> Searching for the /build/release of OpenCPN's 'about.cpp' <<<<<"
    echo ""
    ${READELFPATH} -p .rodata "${OCPNPATH}" | grep "/build/" | grep "about.cpp"
    echo ""
    echo "     >>>>> Displaying OpenCPN's dependencies on wxWidgets libraries <<<<<"
    echo ""
    ${READELFPATH} -d "${OCPNPATH}" | grep "libwx"
    echo ""
    echo "     >>>>> Searching for wxWidgets GTK-version dependency of OpenCPN <<<<<"
    echo ""
    GTK2=$(${READELFPATH} -d "${OCPNPATH}" | grep "libwx" | grep "gtk2")
    if [ ! -z "${GTK2}" ]
    then
        echo "- Found GTK2 wxWidgets library dependencies"
    fi
    GTK3=$(${READELFPATH} -d "${OCPNPATH}" | grep "libwx" | grep "gtk3")
    if [ ! -z "${GTK3}" ]
    then
        echo "- Found GTK3 wxWidgets library dependencies"
        echo "  This means for >v5.1 that OpenCPN's Help/About is using wxWebView"
    fi
    if [ ! -z "${LDDPATH}" ]
    then
        echo ""
        echo "     >>>>> From where the wxWidgets libraries of OpenCPN are found <<<<<"
        echo ""
        ${LDDPATH} ${OCPNPATH} | grep "libwx"
    fi
fi

echo ""
echo "+--------------------------------------------------------------------------+"
echo "!                                                                          !"
echo "!             Installed and available wxWidgets libraries                  !"
echo "!                                                                          !"
echo "+--------------------------------------------------------------------------+"
echo ""


if [ ! -z "${LISTCMD}" ]
then
    ${LISTCMD} | grep libwx
    echo ""
    WEBVIEWLIB=$(${LISTCMD} | grep libwx | grep webview)
    if [ -z "$WEBVIEWLIB" ]
    then
        echo ""
        echo "     -----------------------------------------------------------------"
        echo "     ***  NOTE:  there is no wxWidgets WebView library installed.  ***"
        echo "     -----------------------------------------------------------------"
        echo ""
        if [ ! -z "${APTPATH}" ]
        then
            echo ""
            echo "     >>>>> Searching for available wxWidgets WebView libraries <<<<<"
            echo ""
            ${APTPATH} search libwx | grep web
            echo ""
            echo "     -----------------------------------------------------------------"
            echo "     ** NOTE: only if GTK version is the same as what OpenCPN"
            echo "              is using, there is a point to install the missing"
            echo "              webview library, with 'apt install' command:"
            echo ""
            echo "              If your OpenCPN installation is GTK2 based, it cannot"
            echo "              use GTK3 based libraries. Installing one will not help."
            echo ""
            echo "              It is not difficult to retrieve latest, stable wxWidgets"
            echo "              sources: https://www.wxwidgets.org/downloads/ and build:"
            echo "              https://wiki.wxwidgets.org/Compiling_and_getting_started"
            echo ""
            echo "              'sudo make install' will install wxWidgets libraries in"
            echo "              /usr/local/lib and not mix them with the distribution's"
            echo "              libraries: run 'sudo ldconfig' and this script again"
            echo "              to observe the result."
            echo ""
            echo "              ATTENTION: webkitgtk2 is no longer being updated and"
            echo "                         is considered a security risk."
            echo "                         Please consider this before using your own"
            echo "                         GTK2 based libwx_gtk2u_webview-3.0."
            echo "     -----------------------------------------------------------------"
            echo ""
        fi
    fi
fi

echo ""
echo "+--------------------------------------------------------------------------+"
echo "!                                                                          !"
echo "!          Installed libdashboard_tactics_pi.so shared libraries           !"
echo "!                                                                          !"
echo "+--------------------------------------------------------------------------+"
echo " NOTE: inspect the extracted information for OpenCPN/system compatibility:"
echo "  - more than one library installed? Check opencpn.log which is used ;"
echo "  - there should be no dynamic libraries which are not found from the system ;"
echo "  - The GTK-version shall match with the OpenCPN one ;"
echo "  - The wxWdigets version shall match ;"
echo "  - there should be no dynamic libraries which are not found from system ;"
echo "  - the OpenCPN API (see Help/about) should be higher than in the 'plugin_api'."
echo "-------------------------------------------------------------------------------"
echo ""

FILELIST="~/.local/lib/opecpn/libdashboard_tactics_pi.so"
FILELIST="${FILELIST} /usr/local/lib/opencpn/libdashboard_tactics_pi.so"
FILELIST="${FILELIST} /usr/lib/opencpn/libdashboard_tactics_pi.so"
for PLUGINSOFILE in ${FILELIST}
do
    if [ -f ${PLUGINSOFILE} ]
    then
        echo "Found: ${PLUGINSOFILE}"
        if [ ! -z "${READELFPATH}" ]
        then
            ${READELFPATH} -p .rodata "${PLUGINSOFILE}" | grep "plugin_id"
            ${READELFPATH} -p .rodata "${PLUGINSOFILE}" | grep "plugin_date"
            ${READELFPATH} -p .rodata "${PLUGINSOFILE}" | grep "plugin_api"
            echo ""
            echo "Displaying ${PLUGINSOFILE}'s dependencies on wxWidgets libraries:"
            echo ""
            ${READELFPATH} -d "${PLUGINSOFILE}" | grep "libwx"
            echo ""
            echo "Searching for wxWidgets GTK-version dependency of ${PLUGINSOFILE}:"
            echo ""
            GTK2=$(${READELFPATH} -d "${PLUGINSOFILE}" | grep "libwx" | grep "gtk2")
            if [ ! -z "${GTK2}" ]
            then
                echo "- Found GTK2 wxWidgets library dependencies"
            fi
            GTK3=$(${READELFPATH} -d "${PLUGINSOFILE}" | grep "libwx" | grep "gtk3")
            if [ ! -z "${GTK3}" ]
            then
                echo "- Found GTK3 wxWidgets library dependencies"
            fi
            if [ ! -z "${LDDPATH}" ]
            then
                echo ""
                echo "Displaying status of dynamic libraries links in ${PLUGINSOFILE}:"
                echo ""
                ${LDDPATH} ${PLUGINSOFILE}
            fi
        fi
    fi
done

echo ""
echo "+--------------------------------------------------------------------------+"
echo "!   DONE: scroll back now to read the analysis, or for a report run again: !"
echo "!                                                                          !"
echo "!            ./o-platforminfo.sh 2>&1 | gzip >/tmp/report.txt.gz           !"
echo "+--------------------------------------------------------------------------+"

exit 0



            
