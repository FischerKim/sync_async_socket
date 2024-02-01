@echo off

set EXECUTABLE=dummy.exe
set NUM_INSTANCES=100

echo Launching %NUM_INSTANCES% instances of %EXECUTABLE%...

for /L %%i in (1,1,%NUM_INSTANCES%) do (
    start "Instance%%i" /B %EXECUTABLE% >NUL 2>&1
)

echo All instances launched.

pause