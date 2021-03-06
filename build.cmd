::!%windir%\System32\cmd.exe
@ECHO OFF

PUSHD "%~dp0\Nuclex.Support.Native"
CALL build.cmd
POPD

PUSHD "%~dp0\Nuclex.Storage.Native"
CALL build.cmd
POPD

PUSHD "%~dp0\Nuclex.Pixels.Native"
CALL build.cmd
POPD
