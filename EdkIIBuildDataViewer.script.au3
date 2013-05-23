#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Icon=EdkIIBuildDataViewer\res\Intel.ico
#AutoIt3Wrapper_Outfile=EdkIIBuildDataViewer.script.exe
#AutoIt3Wrapper_Compression=4
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
;
; AutoIt Version: 3.0
; Language:       English
; Platform:       Windows 7
; Author:         Lee Hamel
;
; Script Function:
;   Open BIOS build log, write source list, then quit.
;
Dim $appTitle                     = "EDK II Build Data Viewer"
Dim $appExe                       = "EdkIIBuildDataViewer.exe"
Dim $buttonSelectBuildLogText     = "Select build log file"
Dim $selectInputBuildLogTitle     = "Select EDK II BIOS build log file"
Dim $buttonWriteSourceListText    = "Write source list"
Dim $selectWriteSourceListTitle   = "Input source list filename"
Dim $optionInfSourceListTitle     = "Include INF in list"
Dim $optionDoxygenSourceListTitle = "Select list format"
Dim $parseBuildLogFileTitle       = "Parsing build log file"

Dim $buildLog;							; input
Dim $sourceFileList;					; output
Dim $bSourceIncludeInf = False
Dim $bSourceUseDoxygenFormat = False

;
; Check for app running and active.
;
If WinExists($appTitle) == 0 Then
	MsgBox(16, "ERROR", $appTitle & " is not running", 2)
	Exit(1)
EndIf
If WinWaitActive($appTitle, "", 5) == 0 Then
	MsgBox(16, "ERROR", $appTitle & " is not active", 2)
	Exit(1)
EndIf

;
; Bring app to top.
;
WinActivate($appTitle)

;
; Parse command line parameters.
;
$argc = $CmdLine[0]
if $argc == 0 Then
	MsgBox(16, "ERROR", $appTitle & ": no command line specified", 2)
	Exit(1)
EndIf

Dim $DISPLAY_CMDLINE = 0
Dim $expectBuildLog = False
Dim $expectSourceFile = False
For $i = 1 To $CmdLine[0]
	If $DISPLAY_CMDLINE <> 0 Then
		MsgBox(0, "parameter" & $i, $CmdLine[$i], 2)
	EndIf

	If $expectBuildLog == True Then
		$buildLog = $CmdLine[$i]
		$expectBuildLog = False
	EndIf
	If $expectSourceFile == True Then
		$sourceFileList = $CmdLine[$i]
		$expectSourceFile = False
	EndIf
	If StringCompare($CmdLine[$i], "-l") == 0 Then
		$expectBuildLog = True
	EndIf
	If StringCompare($CmdLine[$i], "-s") == 0 Then
		$expectSourceFile = True
	EndIf
	If StringCompare($CmdLine[$i], "-d") == 0 Then
		$bSourceUseDoxygenFormat = True
	EndIf
	If StringCompare($CmdLine[$i], "-i") == 0 Then
		$bSourceIncludeInf = True
	EndIf
Next

If $DISPLAY_CMDLINE <> 0 Then
	MsgBox(0, "$buildLog", $buildLog, 2)
	MsgBox(0, "$sourceFileList", $sourceFileList, 2)
	MsgBox(0, "$bSourceUseDoxygenFormat", $bSourceUseDoxygenFormat, 2)
	MsgBox(0, "$bSourceIncludeInf", $bSourceIncludeInf, 2)
EndIf

;
; Wait a bit for app to load before scripting actions.
;
Sleep(5000)

;
; Select build log file to read.
;
$click = ControlClick($appTitle, "", $buttonSelectBuildLogText)
If $click == 0 Then
	MsgBox(16, "ERROR", $appTitle & " did not receive click on '" & $buttonSelectBuildLogText & "'", 2)
	Exit(1)
EndIf

Sleep(1000)
If ControlSetText($selectInputBuildLogTitle, "", "[CLASS:Edit; INSTANCE:1]", $buildLog) == 0 Then
	MsgBox(16, "ERROR", "Can't select build log file", 2)
	Exit(1)
EndIf

Sleep(1000)
$click = ControlClick($selectInputBuildLogTitle, "", "&Open")
If $click == 0 Then
	MsgBox(16, "ERROR", "Can't select build log file and Open", 2)
	Exit(1)
EndIf

;
; Wait for sources to be determined.
;
Sleep(1000)
While WinExists($parseBuildLogFileTitle) == 1
	Sleep(500)
WEnd

;
; Select source list file to write.
;
Sleep(1000)
$click = ControlClick($appTitle, "", $buttonWriteSourceListText)
If $click == 0 Then
	MsgBox(16, "ERROR", $appTitle & " did not receive click on '" & $buttonWriteSourceListText & "'", 2)
	Exit(1)
EndIf

Sleep(1000)
If ControlSetText($selectWriteSourceListTitle, "", "[CLASS:Edit; INSTANCE:1]", $sourceFileList) == 0 Then
	MsgBox(16, "ERROR", "Can't select source file list", 2)
	Exit(1)
EndIf

Sleep(1000)
$click = ControlClick($selectWriteSourceListTitle, "", "&Save")
If $click == 0 Then
	MsgBox(16, "ERROR", "Can't select source file list and Save", 2)
	Exit(1)
EndIf

;
; Select source list options.
;
Sleep(1000)
If $bSourceIncludeInf == False Then
	$click = ControlClick($optionInfSourceListTitle, "", "&No")
Else
	$click = ControlClick($optionInfSourceListTitle, "", "&Yes")
EndIf
Sleep(1000)
If $bSourceUseDoxygenFormat == False Then
	$click = ControlClick($optionDoxygenSourceListTitle, "", "&No")
Else
	$click = ControlClick($optionDoxygenSourceListTitle, "", "&Yes")
EndIf

;
; Wait a bit then quit.
;
Sleep(2000)
; Now quit by pressing Alt-F4
;Send("!{F4}")
;WinClose($appTitle, "")
;WinKill($appTitle, $appTitle)
ProcessClose($appExe)
Exit(0)
