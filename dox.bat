@echo off
REM Batch file to create doxygen output from EDK II BIOS source list.
cls
setlocal

REM doxygen cfg file should reference this environment variables with $(name)
set FILE_PATTERNS=*.c *.h *.asm *.asm16 *.inc *.S

:ParseArgs
if /I "%1"=="-h" goto Usage
if /I "%1"=="/h" goto Usage
if /I "%1"=="-?" goto Usage
if /I "%1"=="/?" goto Usage
goto check_parameters

:Usage
echo.
echo Generate BIOS doxygen files.
echo Current directory is used for location of BIOS source files.
echo.
echo Usage: [-h ^| /h ^| -? ^| /?] for help
echo        ^<BIOS platform^> ^<baseline name^> ^<BIOS workspace^> ^<doxygen cfg^> ^<doxygen output dir^> [BIOS source file list]
echo   If [BIOS source file list] isn't specified, then doxygen scans BIOS workspace for files that match: %FILE_PATTERNS%
echo   If [BIOS source file list] is    specified, then doxygen scans files in this list.
goto done

:check_parameters
REM For help on %%~ commands, type "for /?" from Windows command prompt.
if "%1"=="" goto error1.0
if "%2"=="" goto error2.0
if "%3"=="" goto error3.0
if not [%~x3] == [] goto error3.1
if "%4"=="" goto error4.0
if not exist %4 goto error4.1
if "%5"=="" goto error5.0
if not [%~x5] == [] goto error5.1
if not "%6"=="" if not exist %6 goto error6.0

set BIOS_PLATFORM=%1
set BIOS_BASELINE=%2
set BIOS_WORKSPACE=%3

set DOXYGEN_CFG=%4
set DOXYGEN_OUTPUT_DIR=%5\%BIOS_PLATFORM%
if not "%6"=="" (
  set BIOS_INPUT=%6
) else (
  set BIOS_INPUT=%BIOS_WORKSPACE%
)

REM doxygen cfg file should reference these environment variables with $(name)
set PROJECT_NAME=%BIOS_PLATFORM%
set PROJECT_NUMBER=%BIOS_BASELINE%
set PROJECT_BRIEF=%BIOS_PLATFORM% BIOS
set OUTPUT_DIRECTORY=%DOXYGEN_OUTPUT_DIR%
set INPUT=%BIOS_INPUT%
set EXCLUDE=*/Build/* *.inf
set EXCLUDE_PATTERNS=*/Build/* *.inf

echo BIOS platform        : %BIOS_PLATFORM%
echo BIOS baseline        : %BIOS_BASELINE%
echo BIOS workspace       : %BIOS_WORKSPACE%
echo doxygen config       : %DOXYGEN_CFG%
echo doxygen output dir   : %DOXYGEN_OUTPUT_DIR%
echo doxygen BIOS input   : %BIOS_INPUT%
echo.

set TEMP_PATH="%Path:;=";"%"
:check_doxygen_installed
FOR %%a IN (%TEMP_PATH%) DO if exist %%~a\doxygen.exe goto check_graphviz_installed
goto error_doxygen_install

:check_graphviz_installed
FOR %%a IN (%TEMP_PATH%) DO if exist %%~a\dot.exe if exist %%~a\gvgen.exe (
  set DOT_PATH=%%~a
  goto check_other
)
goto error_graphviz_install

:check_other
if not exist %DOXYGEN_CFG% goto error_doxygen_cfg

:step1
REM generate BIOS doxygen files
choice /t 3 /d y /m "Confirm: regenerate doxygen output for %1"
if %ERRORLEVEL% EQU 2 goto step2

echo.
echo Deleting old output in %DOXYGEN_OUTPUT_DIR%\html, please wait ...
rd /s /q %DOXYGEN_OUTPUT_DIR%\html

REM Override doxygen cfg settings with this statement if the cfg doesn't reference environment variables.
REM (type %DOXYGEN_CFG% & echo PROJECT_NAME=%BIOS_PLATFORM% & echo PROJECT_NUMBER=%BIOS_BASELINE% & echo INPUT=%BIOS_INPUT% & echo EXCLUDE=%BIOS_WORKSPACE%Build/* & echo OUTPUT_DIRECTORY=%DOXYGEN_OUTPUT_DIR% & echo DOT_PATH=%DOT_EXE_PATH%) | doxygen.exe -

REM Override doxygen cfg settings with this statement if the cfg does reference environment variables.
(type %DOXYGEN_CFG%) | doxygen.exe -

del %DOXYGEN_OUTPUT_DIR%\doxygen*.tmp

:step2
goto done

:error1.0
echo ERROR: BIOS platform not specified
goto done

:error2.0
echo ERROR: BIOS baseline not specified
goto done

:error3.0
echo ERROR: BIOS workspace not specified
goto done

:error3.1
echo ERROR: BIOS workspace not found, %3
goto done

:error4.0
echo ERROR: doxygen cfg not specified
goto done

:error4.1
echo ERROR: doxygen cfg not found, %4
goto done

:error5.0
echo ERROR: doxygen output dir not specified
goto done

:error5.1
echo ERROR: doxygen output dir not found, %5
goto done

:error6.0
echo ERROR: BIOS source file list not found, %6
goto done

:error_doxygen_install
echo ERROR: doxygen.exe not found in PATH, is doxygen installed?
goto done

:error_graphviz_install
echo ERROR: dot.exe not found in PATH, is graphviz installed?
goto done

:error_doxygen_cfg
echo ERROR: doxygen CFG file not found, %DOXYGEN_CFG%
goto done

:done
endlocal
