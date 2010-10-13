!ifndef ___EnumHKU___
!define ___EnumHKU___
 
!include LogicLib.nsh
!include WordFunc.nsh
 
!define TOKEN_QUERY             0x0008
!define TOKEN_ADJUST_PRIVILEGES 0x0020
 
!define SE_RESTORE_NAME         SeRestorePrivilege
 
!define SE_PRIVILEGE_ENABLED    0x00000002
 
!define HKEY_USERS              0x80000003

Var key
Var logged
Var EnumHKU_counter
Var SID
Var USERPROFILE

!macro _EnumHKU_AdjustTokens
 
StrCpy $R1 0
 
System::Call "kernel32::GetCurrentProcess() i .R0"
System::Call "advapi32::OpenProcessToken(i R0, i ${TOKEN_QUERY}|${TOKEN_ADJUST_PRIVILEGES}, *i R1R1) i .R0"
 
${If} $R0 != 0
  System::Call "advapi32::LookupPrivilegeValue(t n, t '${SE_RESTORE_NAME}', *l .R2) i .R0"
 
  ${If} $R0 != 0
    System::Call "*(i 1, l R2, i ${SE_PRIVILEGE_ENABLED}) i .R0"
    System::Call "advapi32::AdjustTokenPrivileges(i R1, i 0, i R0, i 0, i 0, i 0)"
    System::Free $R0
  ${EndIf}
 
  System::Call "kernel32::CloseHandle(i R1)"
${EndIf}
 
!macroend
 
!macro _EnumHKU_InvokeCallback CALLBACK SUBKEY
 
Push $0
Push $1
Push $R0
Push $R1
Push $R2
Push $R3
Push $R4
Push $R5
Push $R6
 
Push "${SUBKEY}"
Call "${CALLBACK}"

Pop $R6
Pop $R5
Pop $R4
Pop $R3 
Pop $R2
Pop $R1
Pop $R0
Pop $1
Pop $0
 
!macroend
 
!macro _EnumHKU_Load FILE CALLBACK SUBKEY
 
GetFullPathName /SHORT $R2 ${FILE}
System::Call "advapi32::RegLoadKeyW(i ${HKEY_USERS}, t '${SUBKEY}', t R2) i .R3"
MessageBox MB_OK "_EnumHKU_Load: $R2" ;#DEBUG
${If} $R3 == 0
  !insertmacro _EnumHKU_InvokeCallback ${CALLBACK} "${SUBKEY}"
  ;System::Call "advapi32::RegUnLoadKeyW(i ${HKEY_USERS}, t '${SUBKEY}')"
  IntOp $EnumHKU_counter $EnumHKU_counter + 1
${EndIf}
 
!macroend
 
!macro EnumHKU UN CALLBACK SUBKEY
 
Push $0
Push $1
StrCpy $EnumHKU_counter 0 
GetFunctionAddress $0 "${CALLBACK}"
StrCpy $1 "${SUBKEY}_$EnumHKU_counter"
StrCpy $key ${SUBKEY}
Call ${UN}_EnumHKU
 
Pop $1
Pop $0
 
!macroend
 
!define EnumHKU '!insertmacro EnumHKU ""'
!define un.EnumHKU '!insertmacro EnumHKU "un."'
 
!macro _EnumHKU UN
Function ${UN}_EnumHKU
 
Push $R0
Push $R1
Push $R2
Push $R3
Push $R4
Push $R5
Push $R6

# enumerate logged on users
StrCpy $R0 0
${Do}
  EnumRegKey $R1 HKU "" $R0
  ${If} $R1 != ""
	;DetailPrint "on - $R1" ;#DEBUG
	  StrLen $R4 $R1
	  ${If} $R4 > 8
	    StrCpy $R6 0
		${WordFind} "$R1" "_Classes" "E*" $R6
		IfErrors 0 cont
		
		ReadRegStr $R5 HKU "$R1\Volatile Environment" USERPROFILE
	    #ExpandEnvStrings $R5 $R5
	    StrCpy $USERPROFILE $R5
		StrCpy $logged "on"
		!insertmacro _EnumHKU_InvokeCallback $0 $R1
		StrCpy $USERPROFILE ""
		StrCpy $logged ""
	  ${EndIf}
	  cont:
	  IntOp $R0 $R0 + 1
  ${EndIf}
${LoopUntil} $R1 == ""


# enumerate logged off users
System::Call "kernel32::GetVersion() i .R0"
IntOp $R0 $R0 & 0x80000000
 
${If} $R0 == 0
  # nt
  !insertmacro _EnumHKU_AdjustTokens
 
  StrCpy $R0 0
  ${Do}
    EnumRegKey $R1 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList" $R0
    ${If} $R1 != ""
	  StrLen $R4 $R1
	  ${If} $R4 > 8
		  ClearErrors
		  ReadRegStr $R5 HKU "$R1\Volatile Environment" HOMEPATH
		  IfErrors 0 cont1
		  ReadRegStr $R5 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\$R1" ProfileImagePath
		  ExpandEnvStrings $R5 $R5
		  StrCpy $USERPROFILE $R5
		  StrCpy $logged "off"
		  !insertmacro _EnumHKU_Load "$R5\NTUSER.DAT" $0 $1
		  StrCpy $1 "$key_$EnumHKU_counter"
		  StrCpy $USERPROFILE ""
		  StrCpy $logged ""
	  ${EndIf}
      
	  cont1:
	  IntOp $R0 $R0 + 1
    ${EndIf}
  ${LoopUntil} $R1 == ""
${Else}
  # 9x
  ClearErrors
  FindFirst $R1 $R2 "$WINDIR\Profiles\*.*"
  ${Unless} ${Errors}
    ${Do}
      ${If} $R2 != "."
      ${AndIf} $R2 != ".."
        ${If} ${FileExists} "$WINDIR\Profiles\$R2\USER.DAT"
          !insertmacro _EnumHKU_Load "$WINDIR\Profiles\$R2\USER.DAT" $0 $1
        ${EndIf}
      ${EndIf}
      ClearErrors
      FindNext $R1 $R2
    ${LoopUntil} ${Errors}
    FindClose $R1
  ${EndUnless}
${Endif}

Pop $R6
Pop $R5
Pop $R4
Pop $R3
Pop $R2
Pop $R1
Pop $R0
FunctionEnd
!macroend
 
!insertmacro _EnumHKU ""
!insertmacro _EnumHKU "un."
!endif