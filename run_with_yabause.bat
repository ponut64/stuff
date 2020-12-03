@ECHO Off
SET EMULATOR_DIR=..\..\Emulators

if exist game.iso (
"%EMULATOR_DIR%\yabause\yabause.exe" -a -i game.cue
) else (
echo Please compile first !
)
