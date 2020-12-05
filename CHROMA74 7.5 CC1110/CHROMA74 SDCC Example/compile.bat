@echo off 
set project_name=CC1110
set xram_loc=0xF000

SDCC\bin\sdcc -c -V -mmcs51 --model-large --xram-loc %xram_loc% -I/  main.c

SDCC\bin\sdcc main.rel -V -mmcs51 --model-large --xram-loc %xram_loc% -I/ -o %project_name%.ihx

SDCC\bin\packihx %project_name%.ihx > %project_name%.hex

del %project_name%.lk
del %project_name%.map
del %project_name%.mem
del %project_name%.ihx

del *.asm
del *.lst
del *.rel
del *.rst
del *.sym