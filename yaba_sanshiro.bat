@ECHO Off
SET EMULATOR_DIR=..\..\Emulators

if exist sl_coff.iso (
"%EMULATOR_DIR%\sanshiro\yabasanshiro.exe" -a -i sl_coff.cue
) else (
echo Please compile first !
)