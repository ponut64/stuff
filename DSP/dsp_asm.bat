SET /P this=FILE: 
dsp-asm.exe %this%.asm %this%.bin
ECHO this %this%
PAUSE