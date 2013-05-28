@echo off
REM Batch file to create EDK II BIOS source list from build log.
cls
setlocal

:ParseArgs
if /I "%1"=="-h" goto Usage
if /I "%1"=="/h" goto Usage
if /I "%1"=="-?" goto Usage
if /I "%1"=="/?" goto Usage
goto check_parameters

:Usage
echo.
echo Create EDK II BIOS source list from build log.
echo.
echo Usage: [-h ^| /h ^| -? ^| /?] for help
echo        ^<BIOS workspace with trailing backslash^> ^<build log^> ^<BIOS source file list^>
goto done

:check_parameters
REM For help on %%~ commands, type "for /?" from Windows command prompt.
if "%1"=="" goto error1.0
if "%2"=="" goto error2.0
if "%3"=="" goto error3.0
if not [%~x1] == [] goto error1.1
if not exist %1%2 goto error1.2

REM Run EdkIIBuildDataViewer.exe so it can check command line switches.
EdkIIBuildDataViewer.exe -l %1%2 -s %1%3 -d
if %ERRORLEVEL% EQU 0 goto EdkIIBuildDataViewer.error1

REM Run EdkIIBuildDataViewer.exe so the automation script can interact with the UI.
start EdkIIBuildDataViewer.exe
REM Run AutoIt compiled automation script.
EdkIIBuildDataViewer.script.exe -l %1%2 -s %1%3 -d
goto done

:error1.0
echo ERROR: BIOS workspace not specified.
goto done

:error1.1
echo ERROR: BIOS workspace %1 not found.
goto done

:error1.2
echo ERROR: BIOS build log %1%2 not found.
goto done

:error2.0
echo ERROR: BIOS build log not specified.
goto done

:error3.0
echo ERROR: BIOS source list not specified.
goto done

:EdkIIBuildDataViewer.error1
echo ERROR: EdkIIBuildDataViewer command line switches are incorrect.

:done
endlocal
