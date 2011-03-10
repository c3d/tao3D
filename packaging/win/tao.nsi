; tao.nsi
;
; Tao NSIS installation script.

;--------------------------------

!include "MUI.nsh"

; The name of the installer
Name "Tao Presentations"

; The file to write
!ifndef OUTFILE
!define OUTFILE taosetup.exe
!endif
OutFile ${OUTFILE}

; The default installation directory
InstallDir "$PROGRAMFILES\Taodyne\Tao Presentations"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Taodyne\Tao Presentations" "Install_Dir"

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

Section "Tao Presentations (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Files to package
  File /r "build_root\*.*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Taodyne\Tao Presentations" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "DisplayName" "Tao Presentations"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "DisplayIcon" "$INSTDIR\\Tao.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "Publisher" "Taodyne"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Tao Presentations"
  CreateShortCut "$SMPROGRAMS\Tao Presentations\Tao Presentations.lnk" "$INSTDIR\Tao.exe" "" "$INSTDIR\Tao.exe" 0
  
SectionEnd

; Optional
Section "Register tao: links and .ddd files"

  push "tao"
  call RegisterURIScheme
  push ".ddd"
  call RegisterFileExtension
  call RefreshShellIcons

SectionEnd


Function RegisterURIScheme

  Exch $0
  DetailPrint "Registering $0: links"
  DeleteRegKey HKCR "$0"
  WriteRegStr HKCR "$0" "" "URL:Tao link ($0:)"
  WriteRegStr HKCR "$0" "URL Protocol" ""
  WriteRegStr HKCR "$0\DefaultIcon" "" "$INSTDIR\Tao.exe,1"
  WriteRegStr HKCR "$0\shell" "" ""
  WriteRegStr HKCR "$0\shell\Open" "" ""
  WriteRegStr HKCR "$0\shell\Open\command" "" "$INSTDIR\Tao.exe $\"%1$\""
  Pop $R0

FunctionEnd


Function RegisterFileExtension

  Exch $0
  DetailPrint "Registering $0 files"
  DeleteRegKey HKCR "$0"
  WriteRegStr HKCR "$0" "" "TaoPresentations.Document"
  DeleteRegKey HKCR "TaoPresentations.Document"
  WriteRegStr HKCR "TaoPresentations.Document" "" "Tao Presentations Document"
  WriteRegStr HKCR "TaoPresentations.Document\DefaultIcon" "" "$INSTDIR\Tao.exe,1"
  WriteRegStr HKCR "TaoPresentations.Document\shell" "" ""
  WriteRegStr HKCR "TaoPresentations.Document\shell\Open" "" ""
  WriteRegStr HKCR "TaoPresentations.Document\shell\Open\command" "" "$INSTDIR\Tao.exe $\"%1$\""
  Pop $R0

FunctionEnd


!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0

Function RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
    (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Unregister Tao URIs
  push "tao"
  call un.UnregisterURIScheme

  ; Unregister .ddd files
  push ".ddd"
  call un.UnregisterFileExtension

  call un.RefreshShellIcons

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tao Presentations"
  DeleteRegKey HKLM "SOFTWARE\Taodyne\Tao Presentations"

  ; Remove application files and uninstaller
  Delete $INSTDIR\*.*

  ; Recursively remove Git files
  RMDir /r $INSTDIR\git

  ; Recursively remove fonts packaged with Tao
  RMDir /r $INSTDIR\fonts

  ; Recursively remove modules packaged with Tao, or installed subsequently
  RMDir /r $INSTDIR\modules

  ; Remove shortcuts
  SetShellVarContext all
  Delete "$SMPROGRAMS\Tao Presentations\*.*"
  RMDir  "$SMPROGRAMS\Tao Presentations"

  ; Remove installtion directory if empty (should be)
  RMDir "$INSTDIR"

SectionEnd


Function un.UnregisterUriScheme

  Exch $0
  DetailPrint "Unregistering $0: URI scheme"
  DeleteRegKey HKCR "$0"
  Pop $0

FunctionEnd

Function un.UnregisterFileExtension

  Exch $0
  DetailPrint "Unregistering $0 file extension"
  DeleteRegKey HKCR "$0"
  Pop $0

FunctionEnd

Function un.RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
    (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd
