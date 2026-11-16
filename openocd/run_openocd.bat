::C:\soft\OpenOCD-20250710-0.12.0\bin\openocd.exe -f stm32h7-toucan.cfg
C:\tools\OpenOCD-20250710-0.12.0\bin\openocd.exe -f stm32f4xx.cfg

:: start gdb:
::   target remote localhost:3333
::   add-symbol-file C:/Projects/motordrv/motordrv/bin/motordrv.elf
::   print variable_name
::   p/x variable_name
::    n - step-over function
::    finish - step-out function
::    s (or step) - nextline
::    thbreak _malloc_r    - one time hw breakpoint
::
::   To debug exception (no bt):
::        x/16xw $msp       (R0 R1 R2 R3
::                          R12 LR PC xPSR)       PC = is the exact crash site
::        list *0x0800282   to show code of PC