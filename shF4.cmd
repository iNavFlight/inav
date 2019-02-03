pause
wait

rem
rem Based on the msys.bat file from the MSYS package 
rem   http://www.mingw.org/wiki/msys
rem

rem Shared directories for toolchains
set TOOLS_DIR=C:/devtools

rem this should let run MSYS shell on x64
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
  SET COMSPEC=%WINDIR%\SysWOW64\cmd.exe
)

rem some MSYS environment variables
if "x%MSYSTEM%" == "x" set MSYSTEM=MINGW32
if not "x%DISPLAY%" == "x" set DISPLAY=

set NOT_FOUND=
set PATH_DIRS=%TOOLS_DIR%\bin;%TOOLS_DIR%\gcc-arm-none-eabi-6-2017-q2-update\bin;%TOOLS_DIR%\eclipse;c:\ProgramData\Oracle\Java\javapath;C:\Ruby25-x64\bin
set MAKE_COMMAND=

call :which MSYSGIT       "C:\Git\bin"       git.exe

if "%NOT_FOUND%" == "" goto set_path

echo:
echo Some tools were not found in the PATH or expected location:
for %%f in (%NOT_FOUND%) do echo   %%f
echo You may want to install them and/or update paths in the %0 file.
echo:

rem --------------------------------------------------------------------------
rem Provide a clean environment for command line build. We remove the
rem msysGit cmd subdirectory as well, so no recursive sh call can occur.
rem --------------------------------------------------------------------------

:set_path
set PATH=%SYSTEMROOT%\system32;%SYSTEMROOT%
set PATH=%PATH_DIRS%;%PATH%
rem echo PATH: %PATH%

rem --------------------------------------------------------------------------
rem Start a shell.
rem Any shell script can be passed to it via command line of this batch file.
rem --------------------------------------------------------------------------

if not exist "%MSYSGIT%\bash.exe" goto no_bash
call "%MSYSGIT%\bash.exe" --login -i %*
goto :eof

:no_bash
echo Cannot find bash, exiting with error
pause

rem exit 1

rem --------------------------------------------------------------------------
rem Attempt to find executable in the directory given or in the PATH
rem --------------------------------------------------------------------------

:which
rem search in the directory given first
for %%F in (%2) do set FP=%%~F\%3
if exist "%FP%" goto found_directly

rem search in the PATH last
for %%F in (%3) do set FP=%%~$PATH:F
if exist "%FP%" goto found_in_path

:not_found
for %%F in (%2) do set FP=%%~F
rem echo %3: not found, expected in %FP%
set FP=
set NOT_FOUND=%NOT_FOUND% %3
goto set

:found_directly
for %%F in ("%FP%") do set FP=%%~dpsF
rem echo %3: found at: %FP%
goto set

:found_in_path
for %%F in ("%FP%") do set FP=%%~dpsF
rem echo %3: found in the PATH: %FP%

:set
rem set results regardless of was it found or not
set %1=%FP%
rem echo %1=%FP%
if "%FP%" == "" goto :eof
if not "%PATH_DIRS%" == "" set PATH_DIRS=%PATH_DIRS%;
set PATH_DIRS=%PATH_DIRS%%FP%
goto :eof
pause
wait
