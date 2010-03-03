;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"



;--------------------------------
;General

  ;Name and file
  Name "vlc-record installer"
  OutFile "vlc-record-2.20-win-x86-setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\vlc-record"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\vlc-record" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

  SetCompressor /FINAL /SOLID lzma

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Russian"

;--------------------------------
;Installer Sections

Section "Install Section" SecInstall

  ;ADD YOUR OWN FILES HERE...
  SetOutPath "$INSTDIR"
  File "..\vlcdir\libvlc.dll"
  File "..\vlcdir\libvlccore.dll"
  File "..\vlcdir\axvlc.dll"
  File "..\vlcdir\npvlc.dll"
  File "release\vlc-record.exe"

  SetOutPath "$INSTDIR\plugins"
  File /r "..\vlcdir\plugins\*.dll"

  SetOutPath "$INSTDIR\language"
  File "*.qm"

  SetOutPath "$INSTDIR\modules"
  File /r modules\*.mod
  
  ;Store installation folder
  WriteRegStr HKCU "Software\vlc-record" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecInstall ${LANG_ENGLISH} "Install Section"
  LangString DESC_SecInstall ${LANG_GERMAN}  "Installationssektion"
  LangString DESC_SecInstall ${LANG_RUSSIAN} "Install Section"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecInstall} $(DESC_SecInstall)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\plugins\*.dll"
  Delete "$INSTDIR\modules\*.mod"
  Delete "$INSTDIR\language\*.qm"

  RMDir  "$INSTDIR\plugins"
  RMDir  "$INSTDIR\modules"
  RMDir  "$INSTDIR\language"

  Delete "$INSTDIR\libvlc.dll"
  Delete "$INSTDIR\libvlccore.dll"
  Delete "$INSTDIR\axvlc.dll"
  Delete "$INSTDIR\npvlc.dll"
  Delete "$INSTDIR\vlc-record.exe"


  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\vlc-record"

SectionEnd