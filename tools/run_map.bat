SET /P this=MAP: 
SET /P xsz=XSIZE:
SET /P ysz=YSIZE:
obj2hmap.exe terrain.obj %this%.pgm %xsz% 255 %ysz% y 0.0 512.0 u8
PAUSE