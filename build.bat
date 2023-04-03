set output=tiny


set TARGET_PLATFORM="SMS" 
REM set TARGET_PLATFORM="WINDOWS" 

if %TARGET_PLATFORM% == "WINDOWS" set compiler=gcc
if %TARGET_PLATFORM% == "SMS" set compiler=sdcc

set FLAGS=-D PLATFORM_SMS
if %TARGET_PLATFORM% == "WINDOWS" set FLAGS=-g -D PLATFORM_WINDOWS -fno-builtin -Wno-implicit-function-declaration

set mainentry=tiny


call clean %output%
:compile
@echo off
echo Building asset source files..
folder2c assets assets/assets
echo Compiling assets

if %errorlevel% NEQ 0 goto :EOF

echo Compiling Helpers..
if %TARGET_PLATFORM% == "SMS" goto COMPILE_HELPERS_SMS
%compiler% %FLAGS% -c libs/SMScompat.c 
goto COMPILE_ASSETS


:COMPILE_HELPERS_SMS
%compiler% %FLAGS% -c -mz80 libs/console.c
 

:COMPILE_ASSETS
echo Compiling assets and string functions
if %TARGET_PLATFORM% == "WINDOWS" goto COMPILE_HELPERS_WINDOWS
%compiler% %FLAGS% -c -mz80 libs/strings.c
%compiler% %FLAGS% -c -mz80 assets/assets.c
goto COMPILE_MAIN

:COMPILE_HELPERS_WINDOWS
%compiler% %FLAGS% -c libs/strings.c
%compiler% %FLAGS% -c assets/assets.c
%compiler% %FLAGS% -LC:\PDCurses\wincon -lcurses -c libs/SMScompat.c


:COMPILE_MAIN
echo Compiling %mainentry%..
if %TARGET_PLATFORM% == "SMS" %compiler% %FLAGS% -c -mz80 --peep-file peep-rules.txt %mainentry%.c
if %TARGET_PLATFORM% == "WINDOWS" %compiler% %FLAGS% -c %mainentry%.c
if %errorlevel% NEQ 0 goto :EOF
echo Linking..


if %TARGET_PLATFORM% == "SMS" %compiler% %FLAGS% -o %output%.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0/crt0_sms.rel %mainentry%.rel assets.rel console.rel strings.rel SMSlib.lib
REM if %TARGET_PLATFORM% == "WINDOWS" %compiler% %FLAGS% -LC:\PDCurses\wincon -lcurses -o %output% %mainentry%.o assets.o SMScompat.o strings.o 
if %TARGET_PLATFORM% == "WINDOWS" %compiler% %FLAGS% -o %output% %mainentry%.o assets.o SMScompat.o strings.o -lncurses 


if %errorlevel% NEQ 0 goto :EOF
echo Creating ROM..
if %TARGET_PLATFORM% == "SMS" ihx2sms %output%.ihx %output%.sms
echo Starting binary
if %TARGET_PLATFORM% == "SMS" call run %output%
if %TARGET_PLATFORM% == "WINDOWS"  %output%  
:exit
