@ECHO Off

if exist game.iso (
"D:\Games\emuG\mednaffe_dev\mednafen.exe" game.cue
) else (
echo Please compile first !
)