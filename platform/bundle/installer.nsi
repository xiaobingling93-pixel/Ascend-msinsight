!include MUI2.nsh
!include nsDialogs.nsh

!define PRODUCT_PUBLISHER "huawei"

Outfile "Ascend-Insight_7.0.RC1_win.exe"
InstallDir $PROGRAMFILES\ascend_insight
Caption "Ascend Insight Setup"
Name "Ascend Insight"

!define CURRENT_VERSION "7.0.RC1"

!define REGKEY "Software\huawei\ascend_insight"

!define MUI_FINISHPAGE_RUN  "$INSTDIR\ascend_insight.exe"

; Modern UI settings
!define MUI_ICON "resources\images\icons\mindstudio.ico"
; Set the uninstaller icon
!define MUI_UNICON "resources\images\icons\mindstudio.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
!define MUI_HEADERIMAGE_RIGHT

; Pages
!define MUI_UNPAGE_CUSTOMFUNCTION_PRE un.onCustomUninstallPage
!define MUI_UNPAGE_CUSTOMFUNCTION_LEAVE un.onCustomUninstallPageLeave

!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE "resources\license\hwEulaCommunity.txt"

!define MUI_PAGE_HEADER_TEXT "Choose Install Location"
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Merge custom uninstall page with confirm uninstall page
UninstPage custom un.onCustomUninstallPage un.onCustomUninstallPageLeave
!define MUI_UNPAGE_CONFIRM_TEXT "Your ascend_insight uninstall text here"
!insertmacro MUI_UNPAGE_INSTFILES

; Language
!insertmacro MUI_LANGUAGE "English"

Var RemoveCacheData
Function un.onCustomUninstallPage
    !insertmacro MUI_HEADER_TEXT "Whether To Delete Cache Files" "eg log,setting..."
    
    nsDialogs::Create 1018
    Pop $0
    
    ${NSD_CreateCheckbox} 0 30u 100% 10u "Remove cache data"
    Pop $1
    
    nsDialogs::Show
FunctionEnd

Function un.onCustomUninstallPageLeave
    ${NSD_GetState} $1 $0
    StrCpy $RemoveCacheData $0
FunctionEnd


Function .onInit
  nsProcessW::_FindProcess "ascend_insight.exe" $R0
  Pop $0
  ${If} $0 = "0" 
    MessageBox MB_OK "ascend_insight.exe is running. Please close it first."
    Abort
  ${EndIf}

  ReadRegStr $0 HKLM "${REGKEY}" "Version"

  StrCmp $0 "" NoPreviousInstallation
    MessageBox MB_YESNO|MB_ICONQUESTION "A previous version of ascend_insight ($0) is already installed. Do you want to update to version ${CURRENT_VERSION}?" IDYES NoPreviousInstallation
    Abort

  NoPreviousInstallation:

  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Install\ascend_insight" "InstallPath"
  StrCmp $0 "" +2 0
  StrCpy $INSTDIR $0
FunctionEnd

; Components
Section "ascend_insight" Secascend_insight
  SetOutPath $INSTDIR
  File /r "ascend_insight.exe"
  SetOutPath $INSTDIR\resources
  File /r "resources\*"
 
  SetOutPath $INSTDIR
  
  ; Create Start Menu shortcut
  CreateDirectory $SMPROGRAMS\ascend_insight
  CreateShortCut "$SMPROGRAMS\ascend_insight\ascend_insight.lnk" "$INSTDIR\ascend_insight.exe"
  
  ; Create Desktop shortcut
  CreateShortCut "$DESKTOP\Ascend Insight.lnk" "$INSTDIR\ascend_insight.exe"

  ; Add to control panel
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "DisplayName" "ascend_insight"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "DisplayVersion" "${CURRENT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "DisplayIcon" "$INSTDIR\ascend_insight.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "Publisher" "Huawei Technologies CO.,Ltd."

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight" "NoRepair" 1
  WriteUninstaller "Uninstall.exe"

  StrCpy $1 $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Install\ascend_insight" "InstallPath" $1
  
  ; 
  WriteRegStr HKLM "${REGKEY}" "Version" "${CURRENT_VERSION}"
 
SectionEnd

; Uninstaller
Section "Uninstall"
  nsProcessW::_FindProcess "ascend_insight.exe" $R0
  Pop $0
  ${If} $0 = "0" 
    MessageBox MB_OK "ascend_insight.exe is running. Please close it first."
    Abort
  ${EndIf}

  ; Remove files
  Delete $INSTDIR\ascend_insight.exe
  RMDir /r $INSTDIR\resources
    ${If} $RemoveCacheData == 1
        StrCpy $1 $APPDATA
        ; Remove User Cache
        RMDir /r $PROFILE\.ascend_insight
    ${EndIf}

  ; Remove Start Menu shortcut
  Delete $SMPROGRAMS\ascend_insight\ascend_insight.lnk
  RMDir $SMPROGRAMS\ascend_insight

  ; Remove Desktop shortcut
  Delete "$DESKTOP\Ascend Insight.lnk"
  
  ; Remove control panel entry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ascend_insight"
  
  

  ; Remove registry key
  DeleteRegKey HKLM "${REGKEY}"

  ; Remove installation directory
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r $INSTDIR
SectionEnd
