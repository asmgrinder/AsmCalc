# AsmCalc
AsmCalc is a free calculator written in assembly language. This is still pre-alpha version.

##License
Licensed under the MIT license, see LICENSE file for details.

Credits:
AsmCalc development and bugfixes by Victor Antoci.

Compilation:
One of the ways to compile sources is to use yasm for compilation and mingw for linking and resource compilation, though other linkers and resource compilers should do the job too.

Compilation: yasm-1.3.0-win32.exe -f win32 calc.asm
Resource compilation: windres.exe -i calc.rc -F pe-i386 --input-format=rc -o calc.res -O coff 
Linking: g++ -o calc.exe calc.obj calc.res -luser32 -lkernel32 -ladvapi32 -nostdlib -mwindows
