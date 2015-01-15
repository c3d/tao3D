; tao.nsi
;
; Tao NSIS installation script.
;
; NOTE: Encoding for this file MUST BE ISO-8859-1, NOT UTF-8

;--------------------------------

!include "MUI2.nsh"
!include "version.nsh"

; The name of the installer
Name "${APP_NAME}"

; The file to write
!ifndef OUTFILE
!define OUTFILE taosetup.exe
!endif
OutFile ${OUTFILE}

; The default installation directory
InstallDir "$PROGRAMFILES\Taodyne\${APP_NAME}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Taodyne\${APP_NAME}" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Show all languages
!define MUI_LANGDLL_ALLLANGUAGES

; Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "Software\Taodyne\${APP_NAME}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------

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

; Request extra space for the title area
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES

;--------------------------------

; Pages

!insertmacro MUI_PAGE_WELCOME
;  !insertmacro MUI_PAGE_LICENSE "License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT $(create_dshortcut)
!define MUI_FINISHPAGE_RUN_FUNCTION "CreateDesktopShortcut"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT $(create_qlaunch)
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION "CreateQuickLaunchShortcut"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

;Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"

; Language strings - English
LangString sec_tao       ${LANG_ENGLISH} "${APP_NAME} (required)"
LangString sec_shortcuts ${LANG_ENGLISH} "Start Menu Shortcuts"
LangString sec_register  ${LANG_ENGLISH} "Register Tao links and .ddd extension"
LangString tao_document  ${LANG_ENGLISH} "Tao3D Document"
LangString reg_fileext   ${LANG_ENGLISH} "Registering $0 file extension"
LangString reg_uri       ${LANG_ENGLISH} "Registering $0: URI scheme"
LangString unreg_fileext ${LANG_ENGLISH} "Unregistering $0 file extension"
LangString unreg_uri     ${LANG_ENGLISH} "Unregistering $0: URI scheme"
LangString inst_path_invalid ${LANG_ENGLISH} "Install path is invalid. Some files will not be removed."
LangString create_dshortcut ${LANG_ENGLISH} "Create desktop shortcut"
LangString create_qlaunch ${LANG_ENGLISH} "Create quick launch shortcut"
LangString already_inst  ${LANG_ENGLISH} "${APP_NAME} is already installed.$\n$\nClick 'OK' to remove the previous version or 'Cancel' to cancel this upgrade."
LangString removing_prev ${LANG_ENGLISH}  "Removing previous installation..."

; Language strings - French
LangString sec_tao       ${LANG_FRENCH} "${APP_NAME} (requis)"
LangString sec_shortcuts ${LANG_FRENCH} "Raccourcis du menu Démarrer"
LangString sec_register  ${LANG_FRENCH} "Liens Tao et extension .ddd"
LangString tao_document  ${LANG_FRENCH} "Document Tao3D"
LangString reg_fileext   ${LANG_FRENCH} "Association de l'extension $0"
LangString reg_uri       ${LANG_FRENCH} "Association des URIs $0:"
LangString unreg_fileext ${LANG_FRENCH} "Suppression de l'extension $0"
LangString unreg_uri     ${LANG_FRENCH} "Suppression des URIs $0:"
LangString inst_path_invalid ${LANG_FRENCH} "Le chemin d'installation est invalide. Certains fichiers ne seront pas supprimés."
LangString create_dshortcut ${LANG_FRENCH} "Créer un raccourci sur le bureau"
LangString create_qlaunch ${LANG_FRENCH} "Créer un raccourci dans la barre de lancement rapide"
LangString already_inst  ${LANG_FRENCH} "${APP_NAME} est déjà installé.$\n$\nCliquez sur 'OK' pour désinstaller la version précédente ou sur 'Annuler' pour annuler cette mise à jour."
LangString removing_prev ${LANG_FRENCH}  "Suppression de la version précédente..."

; Makes the installer start faster
!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------

; Installer sections

; Uninstall previous version silently
; The "" makes the section hidden
Section "" SecUninstallPrevious

  Call UninstallPrevious

SectionEnd

Section $(sec_tao) SectionTao

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Files to package
  File /r "build_root\*.*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Taodyne\${APP_NAME}" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\\Tao.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "Taodyne"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section $(sec_shortcuts) SectionShortcuts

  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\Tao.exe" "" "$INSTDIR\Tao.exe" 0
  
SectionEnd

; Optional
Section $(sec_register) SectionRegister

  push "tao"
  call RegisterURIScheme
  push "taos"
  call RegisterURIScheme
  push ".ddd"
  call RegisterFileExtension
  call RefreshShellIcons

SectionEnd

; Language strings for components descriptions (shown when hovering the mouse over a component)

; English
LangString DESC_SectionTao       ${LANG_ENGLISH} "The main application. This component is required."
LangString DESC_SectionShortcuts ${LANG_ENGLISH} "Adds a Tao Presentation${MAYBE_PLAYER} entry to the start menu."
LangString DESC_SectionRegister  ${LANG_ENGLISH} "Associates the .ddd file extension to ${APP_NAME} and registers the tao:// and taos:// URI schemes."

; French
LangString DESC_SectionTao       ${LANG_FRENCH} "L'application principale. Ce composant est nécessaire."
LangString DESC_SectionShortcuts ${LANG_FRENCH} "Ajoute ${APP_NAME} au menu Démarrer."
LangString DESC_SectionRegister  ${LANG_FRENCH} "Associe l'extension .ddd à ${APP_NAME} et enregistre les schémas d'URI tao:// et taos://."

; Set the component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionTao} $(DESC_SectionTao)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionShortcuts} $(DESC_SectionShortcuts)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionRegister} $(DESC_SectionRegister)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function RegisterURIScheme

  Exch $0
  DetailPrint $(reg_uri)
  DeleteRegKey HKCR "$0"
  WriteRegStr HKCR "$0" "" "URL:Tao link ($0:)"
  WriteRegStr HKCR "$0" "URL Protocol" ""
  WriteRegStr HKCR "$0\DefaultIcon" "" "$INSTDIR\Tao.exe,1"
  WriteRegStr HKCR "$0\shell" "" ""
  WriteRegStr HKCR "$0\shell\Open" "" ""
  WriteRegStr HKCR "$0\shell\Open\command" "" "$INSTDIR\Tao.exe"
  WriteRegStr HKCR "$0\shell\Open\ddeexec" "" "[open($\"%1$\")]"
  WriteRegStr HKCR "$0\shell\Open\ddeexec\Application" "" "Tao"
  Pop $R0

FunctionEnd


Function RegisterFileExtension

  Exch $0
  DetailPrint $(reg_fileext)
  DeleteRegKey HKCR "$0"
  WriteRegStr HKCR "$0" "" "Tao3D.Document"
  DeleteRegKey HKCR "Tao3D.Document"
  WriteRegStr HKCR "Tao3D.Document" "" $(tao_document) 
  WriteRegStr HKCR "Tao3D.Document\DefaultIcon" "" "$INSTDIR\Tao.exe,1"
  WriteRegStr HKCR "Tao3D.Document\shell" "" ""
  WriteRegStr HKCR "Tao3D.Document\shell\Open" "" ""
  WriteRegStr HKCR "Tao3D.Document\shell\Open\command" "" "$INSTDIR\Tao.exe"
  WriteRegStr HKCR "Tao3D.Document\shell\Open\ddeexec" "" "[open($\"%1$\")]"
  WriteRegStr HKCR "Tao3D.Document\shell\Open\ddeexec\Application" "" "Tao"
  Pop $R0

FunctionEnd


!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0

Function RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
    (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

Function .onInit

  ; For installer to display language selection dialog
  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

Function CreateDesktopShortcut

  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\Tao.exe" "" "$INSTDIR\Tao.exe" 0

FunctionEnd

Function CreateQuickLaunchShortcut

  CreateShortCut "$QUICKLAUNCH\${APP_NAME}.lnk" "$INSTDIR\Tao.exe" "" "$INSTDIR\Tao.exe" 0

FunctionEnd

Function UninstallPrevious

  ; Check if program is already installed
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString"
  StrCmp $R0 "" done
  ReadRegStr $R1 HKLM "SOFTWARE\Taodyne\${APP_NAME}" "Install_Dir"

  ; Write "uninstalling" message
  DetailPrint $(removing_prev)

  ; Prevent ExecWait from overwriting our message
  SetDetailsPrint listonly

  ; /S runs the uninstaller silently
  ; _?= prevents the uninstaller from returning early (by default, the uninstaller
  ; copies itself into a temporary locations, starts the copy and returns immediately)
  ExecWait '$R0 /S _?=$R1'

  ; Restore default mode
  SetDetailsPrint both

  ; Since we used the _?= parameter, the uninstaller could not delete itself, so do it now
  ; FIXME: Do not hardcode uninstall.exe - but I could not find how to properly escape $R0
  Delete '$R1\uninstall.exe'
  RMDir '$R1'

done:
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
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
;  DeleteRegKey HKLM "SOFTWARE\Taodyne\${APP_NAME}"
  DeleteRegValue HKCU "Software\Taodyne\${APP_NAME}" "Installer Language"

  ; Remove shortcuts
  SetShellVarContext all
  Delete "$SMPROGRAMS\${APP_NAME}\*.*"
  RMDir  "$SMPROGRAMS\${APP_NAME}"

  ; Validate $INSTDIR before recursively deleting it (better safe than sorry)
  IfFileExists "$INSTDIR\*.*" 0 +2
  IfFileExists "$INSTDIR\Tao.exe" +3
    MessageBox MB_OK|MB_ICONSTOP $(inst_path_invalid)
      Abort

  ; Remove installation directory
  RMDir /r "$INSTDIR"

SectionEnd


Function un.UnregisterUriScheme

  Exch $0
  DetailPrint $(unreg_uri)
  DeleteRegKey HKCR "$0"
  Pop $0

FunctionEnd

Function un.UnregisterFileExtension

  Exch $0
  DetailPrint $(unreg_fileext)
  DeleteRegKey HKCR "$0"
  Pop $0

FunctionEnd

Function un.RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
    (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

Function un.onInit

  ; Uninstaller will automatically use the language that was selected during
  ; installation (no dialog selection dialog will be shown)
  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd
