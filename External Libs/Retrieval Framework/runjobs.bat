@echo off

:start

set pdcifilename=%1.bmp
set pdcimaskname=%1_mask.bmp
if exist %1.jpg set pdcifilename=%1.jpg
if exist %1.jpg set pdcimaskname=%1_mask.jpg
if exist %1.png set pdcifilename=%1.png
if exist %1.png set pdcimaskname=%1_mask.png

echo %pdcifilename% %pdcimaskname%
start /wait PDCI %pdcifilename% %pdcimaskname%
mkdir Images\Results\%1
move Images\Results\*.jpg Images\Results\%1
move Images\Results\*.png Images\Results\%1
move Images\Results\*.bmp Images\Results\%1

shift
if not "%1"=="" (
	goto start
)