@ECHO Off
SET COMPILER_DIR=..\..\Compiler
SET JO_ENGINE_SRC_DIR=../../jo_engine
SET PATH=%COMPILER_DIR%\Windows\Other Utilities;%PATH%

rm -f ./cd/0.bin
rm -f *.o
rm -f *.s
rm -f *.i
rm -f %JO_ENGINE_SRC_DIR%/*.o
rm -f ./game.bin
rm -f ./game.elf
rm -f ./game.map
rm -f ./game.iso
rm -f ./game.cue

ECHO Done.
