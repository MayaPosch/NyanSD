/* !include "MUI.nsh"

!define MUI_ABORTWARNING # This will warn the user if he exits from the installer.

!insertmacro MUI_PAGE_WELCOME # Welcome to the installer page.
!insertmacro MUI_PAGE_DIRECTORY # In which folder to install page.
!insertmacro MUI_PAGE_INSTFILES # Installing page.
!insertmacro MUI_PAGE_FINISH # Finished installation page.

!insertmacro MUI_LANGUAGE "English"

Name "MQTTCute" # Name of the installer (usually the name of the application to install).
OutFile "MQTTCute_installer.exe" # Name of the installer's file.
InstallDir "$PROGRAMFILES\MQTTCute" # Default installing folder ($PROGRAMFILES is Program Files folder).
ShowInstDetails show # This will always show the installation details.

Section "MQTTCute" # In this section add your files or your folders.
  # Add your files with "File (Name of the file)", example: "File "$DESKTOP\MyApp.exe"" ($DESKTOP is Desktop folder); or add your folders always with "File (Name of the folder)\*", always add your folders with an asterisk, example: "File /r $DESKTOP\MyApp\*" (this will add its files and (with /r its subfolders)).
SectionEnd */

;NSIS Modern User Interface
;MQTTCute install script

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

	;Name and file
	Name "NyanSD"
	OutFile "NyanSD_installer_0.1-alpha.exe"

	;Default installation folder
	InstallDir "$PROGRAMFILES\NyanSD"

	;Get installation folder from registry if available
	InstallDirRegKey HKLM "Software\NyanSD" ""

	;Request application privileges for Windows Vista+
	RequestExecutionLevel admin

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING

;--------------------------------
;Pages

	;!insertmacro MUI_PAGE_LICENSE "LICENSE"
	;!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "NyanSD"
	SetOutPath $INSTDIR
	
	SetOverwrite ifnewer

	File "nyansdd.exe"
	File "nyansd-browse.exe"
	File /r *.dll
	
	; Request access to the 64-bit registry.
	SetRegView 64

	; Store installation folder
	WriteRegStr HKLM "Software\NyanSD" "" $INSTDIR
	
	; Create folder for service files.
	SetShellVarContext all
	CreateDirectory "$PROGRAMDATA\NyanSD\services"

	; Create uninstaller
	WriteUninstaller "$INSTDIR\uninstall.exe"
	
	; Register uninstaller
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD"	"DisplayName" "NyanSD" ;The Name shown in the dialog
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "Publisher" "Maya Posch/Nyanko"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "HelpLink" "https://github.com/MayaPosch/NyanSD"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "DisplayVersion" "0.1-alpha"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "NoModify" 1 ; The installers does not offer a possibility to modify the installation
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD" "NoRepair" 1 ; The installers does not offer a possibility to repair the installation
  
	; Create a shortcuts in the start menu programs directory.
	;CreateDirectory "$SMPROGRAMS\NyanSD"
    ;CreateShortCut "$SMPROGRAMS\NyanSD\NyanSD.lnk" "$INSTDIR\nyansd-browse.exe"
    ;CreateShortCut "$SMPROGRAMS\NyanSD\Uninstall NyanSD.lnk" "$INSTDIR\uninstall.exe"
	
	; Install service.
	SimpleSC::StopService "NyanSD" 1 30
	SimpleSC::RemoveService "NyanSD"
	SimpleSC::InstallService "NyanSD" "NyanSD" "16" "2" "$INSTDIR\nyansdd.exe" "" "" ""
	SimpleSC::SetServiceDescription "NyanSD" "Nyanko Service Discovery"
SectionEnd

;--------------------------------
;Descriptions

	;Language strings
	;LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."

	;Assign language strings to sections
	;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	; !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
	;!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "un.Uninstall Section"
	; Remove service.
	SimpleSC::StopService "NyanSD" 1 30
	SimpleSC::RemoveService "NyanSD"
	
	; Remove files.
	Delete "nyansdd.exe"
	Delete "nyansd-browse.exe"
	Delete $INSTDIR\*.dll

	Delete "$INSTDIR\uninstall.exe"

	RMDir /r "$INSTDIR"
	
	; Request access to the 64-bit registry.
	SetRegView 64

	DeleteRegKey /ifempty HKLM "Software\NyanSD"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NyanSD"
  
	Delete "$SMPROGRAMS\NyanSD\NyanSD.lnk"
    Delete "$SMPROGRAMS\NyanSD\Uninstall NyanSD.lnk"

SectionEnd