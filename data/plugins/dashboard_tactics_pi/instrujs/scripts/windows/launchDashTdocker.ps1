# /* $Id: launchDashTdocker.ps1, v1.0 2019/11/30 VaderDarth Exp $ */
#
# Windows PowerShell script to launch Docker container services for DashT#
##############################################################################
##### without arguments, or with argument "start" #####
#
# Three services will be created on the local network of this computer
# ('127.0.0.1' or 'localhost'):
# - web:
#   nginx server, serving ports
#   80: serving instrujs HTML and *.js (Javascript) files to OpenCPN
#   8089: CORS proxy connecting *.js code with the database service
# - graphs:
#   Grafana for the visualization of the data - independently of OpenCPN
#   30000
# - db:
#   InfluxDB v2 time series database server with incoprated data visualization
#   9999
#
# If the above services are already running, no action is taken
#
##### with argument "stop" #####
#
# The above services will be stopped if they are running
#
##############################################################################
# This script is designed to run with -ExecutionPolicy Bypass switch on system
# where PowerShell execution policy is Restricted: system is not altered.
# For debugging, as Admin: Get-ExecutionPolicy -List
#                          <note the current execution policy>
#                          Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
#                          <debug...>
#                          Set-ExecutionPolicy <see above> -Scope CurrentUser
##############################################################################
#
$param = $args[0]

####### Prerequisites
$somethingMissing = $False
#
function Check_Program_Installed( $programName ) {
# see launchNodeApps.ps1 when GUID / DisplayName need to searched, here only by name
$x86_check = ((Get-ChildItem "HKLM:Software\Microsoft\Windows\CurrentVersion\Uninstall") |
Where-Object { $_."Name" -like "*$programName*" } ).Length -gt 0;
if(Test-Path 'HKLM:Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall')  
{
$x64_check = ((Get-ChildItem "HKLM:Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall") |
Where-Object { $_."Name" -like "*$programName*" } ).Length -gt 0;
}
return $x86_check -or $x64_check;
}
#
# Check if DashT shared files are made available for Docker
#
$dashTPublicAvailable = $False
$PublicFolder = [Environment]::GetEnvironmentVariable('PUBLIC')
if ( $PublicFolder -eq "" ) {
    $HomeFolder = [Environment]::GetEnvironmentVariable('HOME')
    if ( $HomeFolder -ne "" ) {
        $UserFolder = Split-Path $HomeFolder
        $PublicFolder = $UserFolder + "\Public"
    }
}
$PublicDashTFolderExist = $False
$OpenCPNDashTFolderExists = $False
$OpenCPNDashTFolderPath = "C:\Program Files (x86)\OpenCPN\plugins\dashboard_tactics_pi\data\instrujs"
$OpenCPNDashTFolderExists = Test-Path -Path $OpenCPNDashTFolderPath
if ( $OpenCPNDashTFolderExists -eq $True ) {
    if ( $PublicFolder -ne "" ) {
        $PublicDashFolderPath = $PublicFolder + "\DashT"
        $PublicDashTFolderExist = Test-Path -Path $PublicDashFolderPath
        if ( $PublicDashTFolderExist -eq $False ) {
            echo "First time start, preparing the shared folder for Docker..."
            Copy-Item -Path $OpenCPNDashTFolderPath -Destination $PublicDashFolderPath -Recurse
            $PublicDashTFolderExist = Test-Path -Path $PublicDashFolderPath
            if ( $PublicDashTFolderExist -eq $False ) {
                $somethingMissing = $True
            }
        }
        else {            
            echo "Updating shared folder for Docker... "
            Copy-Item -Path $OpenCPNDashTFolderPath -Destination $PublicDashFolderPath -Recurse -Force
        }
    }
}
#
# Check if Docker Desktop for Windows has been installed
#
$oldPreference = $ErrorActionPreference
$ErrorActionPreference = ‘stop’
$dockerName = "Docker Desktop"
$dockerOK = Check_Program_Installed($dockerName)
if ( $dockerOK -ne $True ) {
    $somethingMissing = $True
    }
#
# check if Docker CLI exists or is in the path
#
$dockerCliCmd = "docker.exe"
$dockerCliCmdExists = $False
try {if(Get-Command $dockerCliCmd){
    $dockerCliCmdExists = $True
    }
}
Catch {
    $dockerCliCmdExists = $False
}
Finally {
 $ErrorActionPreference = $oldPreference
}
if ( $dockerCliCmdExists -ne $True ) {
    $somethingMissing = $True
    }
#
# check if Docker proxy is running
#
$oldPreference = $ErrorActionPreference
$dockerProxyProcess = "com.docker.proxy"
$dockerProxyProcessExists = $False
try {if(Get-Process $dockerProxyProcess){
    $dockerProxyProcessExists = $True
    }
}
Catch {
    $dockerProxyProcessExists = $False
}
Finally {
 $ErrorActionPreference = $oldPreference
}
if ( $dockerProxyProcessExists -ne $True ) {
    $somethingMissing = $True
    }
#
# check if Docker Composer programs exists or is in the path
# Apparently works also on Docker Desktop 3.x distro, keep this
#
$composerCmd = "docker-compose.exe"
$composerCmdExists = $False
try {if(Get-Command $composerCmd){
    $composerCmdExists = $True
    }
}
Catch {
    $composerCmdExists = $False
}
Finally {
 $ErrorActionPreference = $oldPreference
}
if ( $composerCmdExists -ne $True ) {
    $somethingMissing = $True
    }
#
# Check if the docker-compose.yml configuration file is in expected location
#
$composerConfDir = "~\..\Public\DashT"
$composerConfFile = "docker-compose.yml"
$composerConfPath = "$composerConfDir\$composerConfFile"
$composerConfPathOK = Test-Path -Path $composerConfPath
if ( $composerConfPathOK -ne $True ) {
    $somethingMissing = $True
    }
#
# If an issue, give a report
#
if ( $somethingMissing -eq $True ) {
    echo ""
    echo ""
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
    echo ""
    echo ""
    echo "                      ******************************"
    echo "                      * ERRORS: - cannot continue. *"
    echo "                      ******************************"
    echo ""
    if ( $PublicDashTFolderExist -ne $True ) {
        echo "- Could not do first-run initialization by copying template from:"
        echo "  $OpenCPNDashTFolderPath"
        if ( $OpenCPNDashTFolderExists -eq $True ) {
            echo "  to"
            echo "  $PublicDashFolderPath"
            echo "- Check the paths and permissions."
        }
        else {
            echo "  The folder does not exist"
            echo "- Have you removed the dashboard_tactics_pi Plug-In or installed OpenCPN in a different path?"
        }
    }
    if ( $dockerOK -ne $True ) {
        echo "- Cannot find '$dockerName', please install first. Launching the website:"
        sleep 10
        Start "https://hub.docker.com/editions/community/docker-ce-desktop-windows/"
        }
    else {
        if ( $dockerProxyProcessExists -ne $True ) {
            echo "- Cannot find '$dockerProxyProcess' (Proxy) (while '$dockerName' is installed): probably not started."
            echo ""
            echo "  After starting, please wait the Docker Desktop Started notification before launching the containers."
            }
        if ( $dockerCliCmdExists -ne $True ) {
            echo "- Cannot find '$dockerCliCmd' (CLI) (while '$dockerName' is installed): probably not in the path."
            }
        if ( $composerCmdExists -ne $True ) {
            echo "- Cannot find '$composerCmd' (while '$dockerName' is installed): probably not in the path."
            }
        }
    if ( $composerConfPathOK -ne $True ) {
        echo "- Cannot find Docker Composer configuration file: '$composerConfPath'"
        }
    sleep 10
    exit -1
    }
#
####### Prerequisites PASS
#
$oldLocation = Get-Location
Set-Location( $composerConfDir )

#
# Ready to start or stop the composer services
#

$composerArgUp     = "up"
$composerArgInBgnd = "-d"
$composerArgDown   = "down"

if ( $param -ne "stop" ) {
    echo "Creating and starting composed DashT service containers..."
    $composerArg = "$composerArgUp $composerArgInBgnd"
    }
else {
    echo "Stopping and removing composed DashT service containers..."
    $composerArg = $composerArgDown
}

Start-Process -Wait $composerCmd $composerArg

# Set InfluxDB image container time is as the time obtained from the host
# This is Hyper-V sleep issue. Courtesy to https://dockr.ly/3eJNErk
# One just need to add "T" in the date to make it ISO compatible.

### NOTE: DEPENDENCY TO THE INFLUXDB OFFICIAL IMAGE NAME ###
$influxImgName = "influxdb:latest"
if ( $param -ne "stop" ) {
    sleep 3
    echo "Adjusting Hyper-V sleep affected date and time (UTC) on all InfluxDB containers..."
    echo "... in case your are using WSL2 and not Hyper-V time sync will not hurt ..."
    $datetime = Get-Date
    $dt = $datetime.ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ss')
    echo $dt
    $dockerCliCmdArgs = "run --net=host --ipc=host --uts=host --pid=host --security-opt=seccomp=unconfined --privileged --rm $influxImgName date -s $dt"
    Start-Process -Wait $dockerCliCmd $dockerCliCmdArgs
    sleep 3
    }

# Add Grafana plug-in for InfluxDB data query format (Flux) as data source.
# Grana stores this if local datavolume is used, but if volume is volatile,
# it will be lost. It is bettero to install it every time.

### NOTE: DEPENDENCY TO THE CONTAINER NAME IN docker-compose.yml ###
$grafanaContainerName = "dasht_grafana"
if ( $param -ne "stop" ) {
    ### NOTE: DEPENDENCY TO THE GRAFANA PLUG-IN LOCAL INSTALLATION DIRECTORY ###
    # If not bind but mount volume; or if no volume: installed always
    $grafanaLocalBindFluxPluginDescrFile = $PublicDashFolderPath + "\grafana\plugins\grafana-influxdb-flux-datasource\docker-compose.yml"
    $hasGrafanaLocalBindFluxPluginDescrFile = Test-Path -Path $grafanaLocalBindFluxPluginDescrFile
    if ( $hasGrafanaLocalBindFluxPluginDescrFile -eq $False ) {
        sleep 2
        echo "(re)Installing InfluxDB Flux as datasource plug-in into DashT Grafana service..."
        $dockerCliCmdArgs = "exec -it $grafanaContainerName grafana-cli plugins install grafana-influxdb-flux-datasource"
        Start-Process -Wait $dockerCliCmd $dockerCliCmdArgs
        sleep 3
       }
    }

# Done
Set-Location( $oldLocation )
echo "Done."
sleep 3
exit 0