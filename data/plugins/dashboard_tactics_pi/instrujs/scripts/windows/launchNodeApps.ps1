# /* $Id: launchDashNodeApps.ps1, v1.0 2019/11/30 VaderDarth Exp $ */
#
# Windows PowerShell script to launch Node.js based applications for DashT   #
##############################################################################
##### Without arguments #####
#
# Verify that either https://github.com/SignalK/signalk-server-windows or
# node.js package has been installed 
#
##### [service1] [service2] ... [serviceN] #####
#
# Enumerated node services, in free order. Of which services the following
# have dependencies with DashT InstruJS:
#
# For all services:
# - If a folder C:\signalk\nodejs exists, then priority given to it,
#   otherwise node.js global scope (global for user) is used
# - If node service is not installed, an installation attempt will take place
# signalk-server:
# - If a TCP port 3000 is listening, no action will be taken
# http-server:
# - If a TCP port 8080 is listening, no action will be taken 
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
# #####################
# Read in the arguments as space separated list
# #####################

$nodeServices = @()
for ( $i = 0; $i -lt $args.count; $i++ ) {
    $nodeServices += $args[$i]
}

#
function Get-InstalledSoftware {
    <#
    .SYNOPSIS
        Retrieves a list of all software installed
    .EXAMPLE
        Get-InstalledSoftware
        
        This example retrieves all software installed on the local computer
    .PARAMETER Name
        The software title you'd like to limit the query to.
    .PARAMETER Arch
        Eiher "x86" for 32-bit applications or "6432" for applications registering themselves in WOV6432 node 
    #>
    [OutputType([System.Management.Automation.PSObject])]
    [CmdletBinding()]
    param (
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$Name,
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$Arch
    )
 
    $retval = @()
    $UninstallKeys = @()
    if ($PSBoundParameters.ContainsKey('Arch')) {
        if ( $Arch -eq "x86" ) {
            $UninstallKeys += "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall"
        }
        else {
            $UninstallKeys += "HKLM:\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
        }
    }
    else {
        $UninstallKeys += "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall", "HKLM:\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
    }
    $null = New-PSDrive -Name HKU -PSProvider Registry -Root Registry::HKEY_USERS
    $UninstallKeys += Get-ChildItem HKU: -ErrorAction SilentlyContinue | Where-Object { $_.Name -match 'S-\d-\d+-(\d+-){1,14}\d+$' } | ForEach-Object { "HKU:\$($_.PSChildName)\Software\Microsoft\Windows\CurrentVersion\Uninstall" }
    if (-not $UninstallKeys) {
        $retval = "No software registry keys found"
    } else {
        foreach ($UninstallKey in $UninstallKeys) {
            if ($PSBoundParameters.ContainsKey('Name')) {
                $WhereBlock = { ($_.PSChildName -match '^{[A-Z0-9]{8}-([A-Z0-9]{4}-){3}[A-Z0-9]{12}}$') -and ($_.GetValue('DisplayName') -like "$Name*") }
            } else {
                $WhereBlock = { ($_.PSChildName -match '^{[A-Z0-9]{8}-([A-Z0-9]{4}-){3}[A-Z0-9]{12}}$') -and ($_.GetValue('DisplayName')) }
            }
            $gciParams = @{
                Path        = $UninstallKey
                ErrorAction = 'SilentlyContinue'
            }
            $selectProperties = @(
                @{n='GUID'; e={$_.PSChildName}}, 
                @{n='Name'; e={$_.GetValue('DisplayName')}}
            )
            $retval += Get-ChildItem @gciParams | Where $WhereBlock | Select-Object -Property $selectProperties

        }
    }
    return $retval
}

function LaunchNodeServicePs1 {
    <#
    .SYNOPSIS
        Launch Node.js service from the user's npm-directory using provided PS1-script 
    .EXAMPLE
        LaunchNodeServicePs1 -ServiceName "signalk-server" -NpmDir "C:\signalk\nodejs"
        
        This example launches the signal-server.ps1 script from the signalk-server-windows
    .PARAMETER ServiceName
        The node service name
    .PARAMETER NpmDir
        The directory for the service's PS1-startup script
    #>
    [OutputType([System.Management.Automation.PSObject])]
    [CmdletBinding()]
    param (
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$ServiceName,
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$NpmDir
    )
    $ps1exe = "C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe"
    $ps1argtmpl = "-ExecutionPolicy Bypass -File "

    $ps1path = "$NpmDir\$ServiceName.ps1"
    $ps1pathExists = Test-Path -Path $ps1path
    if ( $ps1pathExists -eq $False ) {
        echo "`n"
        echo "`n"
        echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -`n"
        echo "`n"
        echo "`n"
        echo "                  ******************************`n"
        echo "                  * ERRORS: - cannot continue. *`n"
        echo "                  ******************************`n"
        echo "`n"
        echo "`n"
        echo "- Could not find $nodeService startup-up script of Node.js`n"
        echo "  Searching from:`n"
        echo "$ps1path`n"
        echo "`n"
    }
    else {
        $ps1arg = "$psargtmpl $ps1path"
        Start-Process $ps1exe $ps1arg
    }
}

function Test-Port
{
    <#
    .SYNOPSIS
        Test-Path equivalent to check if TCP port exists and can be connected to
    .EXAMPLE
        Test-Port -Address 127.0.0.1 -Port 3000
        
        This example test if the (typical) port of Signal K node server is used
    .PARAMETER Address
        IP-address like 127.0.0.1
    .PARAMETER Port
        TCP port number to test
    .OUTPUTS
        System.Boolea $True if pot exist and can be connected, $False otherwise
    #>
    [OutputType([System.Management.Automation.PSObject])]
    [CmdletBinding()]
    [OutputType([System.Boolean])]
    param (
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$Address,
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$Port
    )
    $tcpClient = new-object Net.Sockets.TcpClient
    try {
        $tcpClient.Connect("$Address", $Port)
        $true
    }
    catch {
        $false
    }
    finally {
        $tcpClient.Dispose()
    }
}


# ####################
# Memorize the context
# ####################

$oldPreference = $ErrorActionPreference
$oldLocation = Get-Location



# Test code, useful only for debugging of the Get-InstalledSoftware() function
# $allInstalledGUIDs = Get-InstalledSoftware
# $allInstalledGUIDs = Get-InstalledSoftware -Arch "x86"
# $allInstalledGUIDs = Get-InstalledSoftware -Arch "6432"
# $allInstalledGUIDs | ForEach-Object { $_.GUID; $_.Name }
# $allInstalledGUIDs[0].GUID

# ################
# Priority node.js is given to https://github.com/SignalK/signalk-server-windows
# ################

$SkWindowsNodeJsDir = "C:\signalk\nodejs"
$SkWindowsNodeJsDirExists = Test-Path -Path $SkWindowsNodeJsDir
$SkWindowsNodeCliCmd = "$SkWindowsNodeJsDir\node.exe"
$SkWindowsNodeCliCmdExists = Test-Path -Path $SkWindowsNodeCliCmd
$SkWindowsNpmCliCmd = "$SkWindowsNodeJsDir\npm.cmd"
$SkWindowsNpmCliCmdExists = Test-Path -Path $SkWindowsNpmCliCmd
$SkWindowsNodeVersion = "n/a"
$SkWindowsNpmVersion = "n/a"


$SkWindows = $False
if ( ($SkWindowsNodeCliCmdExists -eq $True) -AND ($SkWindowsNpmCliCmdExists -eq $True) ) {
    $SkWindows = $True
    echo ""
    echo "https://github.com/SignalK/signalk-server-windows installation found from:"
    echo "$SkWindowsNodeJsDir"
    echo ""
    if ( -NOT $nodeServices ) {
        echo ""
        # NB: we cannot use dynamic strings below, like with Invoke-Expression cmdlet,
        #     we _must_ hard code it in order to pass security checks
        $SkWindowsNodeVersion = (C:\signalk\nodejs\node.exe --version) | Out-String
        echo "$SkWindowsNodeCliCmd --version returns: $SkWindowsNodeVersion"
        $SkWindowsNpmVersion = (C:\signalk\nodejs\npm.cmd --version) | Out-String
        echo "signalk-server-windows package manager:"
        echo "$SkWindowsNpmCliCmd --version returns: $SkWindowsNpmVersion"
        echo ""
    }
    else {
        echo "signalk-server-windows will be used in _priority_"
    }
}

# #######
# Node.js must be installed if no signal-server-windows
# #######

$NodeJsFolder = "nodejs"
$nodeCliCmd = "node.exe"
$nodeCliCmdExists = $False
$nodeVersion = "n/a"
$npmCliCmd = "npm.cmd"
$npmCliCmdExists = $False
$npmVersion = "n/a"

$nodeJsGuidObjArray = Get-InstalledSoftware -Name "Node.js"
if ( $nodeJsGuidObjArray ) {

    $ErrorActionPreference = ‘stop’
    try {if(Get-Command $nodeCliCmd){
            $nodeCliCmdExists = $True
            # NB: we cannot use dynamic strings below, like with Invoke-Expression cmdlet,
            #     we _must_ hard code it in order to pass security checks
            $nodeVersion = (node --version) | Out-String
        }
    }
    Catch {
        $nodeCliCmdExists = $False
    }
    Finally {
        $ErrorActionPreference = $oldPreference
    }
    $ErrorActionPreference = ‘stop’
    try {if(Get-Command $npmCliCmd){
            $npmCliCmdExists = $True
            # NB: we cannot use dynamic strings below, like with Invoke-Expression cmdlet,
            #     we _must_ hard code it in order to pass security checks
            $npmVersion = (npm --version) | Out-String
        }
    }
    Catch {
        $npmCliCmdExists = $False
    }
    Finally {
        $ErrorActionPreference = $oldPreference
    }

    if ( -NOT $nodeServices ) {
        echo ""
        if ( $SkWindows -eq $True ) {
            echo "Will use above signalk-server-windows instance for all services."
            echo "- Additionally, found following instances of Node.js:"
        }
        else {
            echo "Found following istallations of Node.js:"
        }
        echo ""
    `	ForEach ( $nodeJSGuidObj in $nodeJsGuidObjArray ) {
            echo "$($nodeJsGuidObj.GUID) $($nodeJsGuidObj.Name)"
        }
        echo ""
        if ( $nodeCliCmdExists -eq $True ) {
            echo "$nodeCliCmd --version returns: $nodeVersion"
        }
        if ( $npmCliCmdExists -eq $True ) {
            echo "Node.js package manager:"
            echo "$npmCliCmd --version returns: $npmVersion"
        }
        if ( ($nodeCliCmdExists -eq $False) -OR ($npmCliCmdExists -eq $False) ) {
            $PathString = [Environment]::GetEnvironmentVariable('PATH')
            $foundidx = ($PathString | Select-String $NodeJsFolder).Matches.Index
            echo "Cannot find $nodeCliCmd and/or $npmCliCmd, check your command PATH:"
            if ( -NOT $foundidx ) {
                echo "- there is no path with '$NodeJsFolder'"
                echo "  (if you have just installed Node.js, reboot your system and try again)"
            }
            else {
                echo "- there is a path with '$NodeJsFolder',please check that path being a valid one."
            }
            echo ""
            echo "$PathString"
            echo ""
        }
        echo ""
        sleep 10
        exit 0
    }
    else {
        echo ""
        echo "Node.js $nodeVersion"
        echo "npm $npmVersion"
    }
}
else {
    if ( -NOT $nodeServices ) {
        if ( $SkWindows -eq $True ) {
            echo "- No additional Node.js installation found from this system."
        }
        else {
            echo "No Node.js installation found from this system."
            sleep 10
            exit 0
        }
        echo ""
    }
    else {
        if ( $SkWindows -eq $False ) {
            echo ""
            echo ""
            echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
            echo ""
            echo ""
            echo "                  ******************************"
            echo "                  * ERRORS: - cannot continue. *"
            echo "                  ******************************"
            echo ""
            echo ""
            echo "- Could not find a Node.js installation. Services cannot be started."
            echo "  Redirecting to OpenCPN documentation for instructions:"
            echo ""
            echo "https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:supplementary_software:signalk:a3"
            echo ""
            Start "https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:supplementary_software:signalk:a3"
            echo ""
            if ( $npmDirExists -eq $True ) {
                echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
                echo ""
                echo ""
                echo "   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +"
                echo "   ! WARNING: - No Node.js installation but a npm data folder. !"
                echo "   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +"
                echo ""
                echo ""
                echo "- Perhaps a trace of a previous Node.js installation:"
                echo ""
                echo "$npmDir"
                echo ""
                echo "- Rename the folder, for backup before installing again Node.js"
                echo ""
            }
            sleep 10
            exit -1
        }
    }
}

# #############################
# OpenCPN and its DashT plug-in must be installed
# #############################


$DashTwebFolderExists = $False
$DashTwebFolderPath = "C:\Program Files (x86)\OpenCPN\plugins\dashboard_tactics_pi\data\instrujs\www"
$DashTwebFolderExists = Test-Path -Path $DashTwebFolderPath

if ( $DashTwebFolderExists -eq $False ) {
    echo ""
    echo ""
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
    echo ""
    echo ""
    echo "                  ******************************"
    echo "                  * ERRORS: - cannot continue. *"
    echo "                  ******************************"
    echo ""
    echo ""
    echo "- Could not find a DashT http:// data for InstruJS instruments."
    echo ""
    echo $DashTwebFolderPath
    echo ""
    echo "- Have you installed DashT v2.0 or superior?"
    echo ""
    echo "- Redirecting to the donwload page of the latest release:"
    echo ""
    echo "https://github.com/canne/dashboard_tactics_pi/releases/latest"
    echo ""
    Start "https://github.com/canne/dashboard_tactics_pi/releases/latest"
    echo ""
    sleep 10
    exit -1
}


Set-Location( $DashTwebFolderPath )


# ########################
# Ready to launch services, in given order. If not installed, attempt to install first
# ########################

$UserAppData = [Environment]::GetEnvironmentVariable('APPDATA')
$npmPS1Dir = "$UserAppData\npm"

ForEach ( $nodeService in $nodeServices ) {

    # Check if the ports of known applications are already listened
    $portTarget = 0
    $portInUse = $False
    if ( $nodeService -eq "signalk-server" ) {
        $portTarget = 3000
    }
    if ( $nodeService -eq "http-server" ) {
        $portTarget = 8080
    }
    if ( $portTarget -ne 0 ) {
        $portInUse = Test-Port -Address 127.0.0.1 -Port $portTarget
        if ( $portInUse -eq $True ) {
            echo "Service ${nodeService}: port $portTarget is already used and served, no action."
        }
    }

    if ( $portInUse -eq $False ) {

        # Ok to launch service, test first does it exists

        $serviceInstalled = $False

        $ErrorActionPreference = ‘stop’
        try {
            if ( $SkWindows -eq $True ) {
                if( Test-Path -Path "$SkWindowsNodeJsDir\${nodeService}.ps1" ){
                    $serviceInstalled = $True
                    $retval = LaunchNodeServicePs1 -ServiceName $nodeService -NpmDir $SkWindowsNodeJsDir
                }
                else {
                    throw "Node service $nodeService not found."
                }
            }
            else {
                if( Get-Command $nodeService ){
                    $serviceInstalled = $True
                    $retval = LaunchNodeServicePs1 -ServiceName $nodeService -NpmDir $npmPS1Dir
                }
                else {
                    throw "Node service $nodeService not found."
                }
            }
            if ( $retval ) {
                echo ""
                Write-Verbose ($retval)
                echo ""
                throw "Node service $nodeService launch failed."
            }
        }
        Catch {
            $serviceInstalled = $False
        }
        Finally {
            $ErrorActionPreference = $oldPreference
        }


        # In case no service available, we try to install it and then launch again

        if ( $serviceInstalled -eq $False ) {
            echo "Node service $nodeService not installed, attempting to install it:"
            echo ""
            $npmInstallAndScope = "install --force"
            if ( $SkWindows -eq $False ) {
                $npmInstallAndScope = "$npmInstallAndScope --global"
            }
            $npmInstallPermissionOption = ""
            $npmSpecificVersion = ""
            if ( $nodeService -eq "signalk-server" ) {
                $npmInstallPermissionOption = "--unsafe-perm"
                $npmSpecificVersion = "@1.28.0"
            }
            $npmCmdArgs = "$npmInstallAndScope $npmInstallPermissionOption $nodeService$npmSpecificVersion"
            echo $npmCmdArgs
            $useNpmCliCmd = $npmCliCmd
            if ( $SkWindows -eq $True ) {
                $useNpmCliCmd = $SkWindowsNpmCliCmd
                $restoreLocation = Get-Location
                Set-Location( $SkWindowsNodeJsDir )
            }

            $instprocess = (Start-Process -Wait $useNpmCliCmd $npmCmdArgs)

            if ( $SkWindows -eq $True ) {
                Set-Location( $restoreLocation )
            }
            $useNpmDir = $npmPS1Dir
            if ( $SkWindows -eq $True ) {
                $useNpmDir = $SkWindowsNodeJsDir
            }
            $retval = LaunchNodeServicePs1 -ServiceName $nodeService -NpmDir $useNpmDir
            if ( $retval ) {
                echo ""
                echo "Node service $nodeService installation and launch failed:"
                Write-Verbose ($retval)
                echo ""
            }
            else {
                $serviceInstalled = $True
            }
        }
        if ( $serviceInstalled -eq $True ) {
            if ( $nodeService -eq "signalk-server" ) {
                sleep 5
                Start "http://127.0.0.1:3000"
            }
            if ( $nodeService -eq "http-server" ) {
                sleep 3
                Start "http://127.0.0.1:8080"
            }
            echo "OK: $nodeService"
            echo ""
        }
    }
}

Set-Location( $oldLocation )
echo "Done."
sleep 3
exit 0
