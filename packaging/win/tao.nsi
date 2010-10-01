; tao.nsi
;
; Tao NSIS installation script.

;--------------------------------

!include "MUI.nsh"

; The name of the installer
Name "Tao"

; The file to write
!ifndef OUTFILE
!define OUTFILE taosetup.exe
!endif
OutFile ${OUTFILE}

; The default installation directory
InstallDir $PROGRAMFILES\Taodyne\Tao

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Taodyne\Tao" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin


; Installer & uninstaller icons
!define MUI_ICON "tao_inst.ico"
!define MUI_UNICON "tao_uninst.ico"

; Installer & uninstaller bitmaps
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "inst_header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "uninst_header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "inst_welcome_finish.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "uninst_welcome_finish.bmp"

;--------------------------------

; Pages

  !insertmacro MUI_PAGE_WELCOME
;  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------

;Languages

  !insertmacro MUI_LANGUAGE "English"


;--------------------------------

; Installer sections

Section "Tao (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Files to package
  File /r "build_root\*.*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Taodyne\Tao "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao" "DisplayName" "Tao"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Tao"
  CreateShortCut "$SMPROGRAMS\Tao\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Tao\Tao.lnk" "$INSTDIR\Tao.exe" "" "$INSTDIR\Tao.exe" 0
  
SectionEnd

; Optional
Section "Register tao: links"

  push "tao"
  call RegisterURIScheme

SectionEnd


Function RegisterURIScheme

  Exch $0
  DetailPrint "Registering $0: links"
  DeleteRegKey HKCR "$0"
  WriteRegStr HKCR "$0" "" "URL:Tao link ($0:)"
  WriteRegStr HKCR "$0" "URL Protocol" ""
  WriteRegStr HKCR "$0\DefaultIcon" "" "$INSTDIR\Tao.exe"
  WriteRegStr HKCR "$0\shell" "" ""
  WriteRegStr HKCR "$0\shell\Open" "" ""
  WriteRegStr HKCR "$0\shell\Open\command" "" "$INSTDIR\Tao.exe $\"%1$\""
  Pop $R0

FunctionEnd


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Unregister Tao URIs
  push "tao"
  call un.UnregisterURIScheme

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao"
  DeleteRegKey HKLM SOFTWARE\Taodyne\Tao

  ; Remove application files and uninstaller
  Delete $INSTDIR\*.*

  ; Recursively remove Git files
  RMDir /r $INSTDIR\git

  ; Recursively remove fonts packaged with Tao
  RMDir /r $INSTDIR\fonts

  ; Remove shortcuts
  Delete "$SMPROGRAMS\Tao\*.*"
  RMDir  "$SMPROGRAMS\Tao"

  ; Remove installtion directory if empty (should be)
  RMDir "$INSTDIR"

SectionEnd


Function un.UnregisterUriScheme

  Exch $0
  DetailPrint "Unregistering $0: URI scheme"
  DeleteRegKey HKCR "$0"
  Pop $0

FunctionEnd
