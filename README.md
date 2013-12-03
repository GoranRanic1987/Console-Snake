Console-Snake
=============
## Download Binaries

[Download Latest Version: v1.4.0](https://github.com/kevin-dong-nai-jia/Console-Snake/releases/tag/v1.4.0)

## Instruction

Exit: End

Restart: Home

Speed: PgUp / PgDn

Refresh / Redraw: F5

Pause / Resume: Space

Direction: Up / Dn / Lf / Rg

## Compile Command

mingw32-gcc.exe -c "Console-Snake.c" -o "Console-Snake.o"

mingw32-gcc.exe -o "Console-Snake.exe" "Console-Snake.o" "libgdi32.a" "libpthreadGC2.a"
