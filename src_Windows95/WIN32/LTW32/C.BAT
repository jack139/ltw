@echo off
copy \user\gt\doc\win32\ltw32d\debug\ltw32d.dll debug
copy \user\gt\doc\win32\ltw32d\debug\ltw32d.map debug

copy \user\gt\doc\win32\lthook32\debug\lthook32.dll debug
copy \user\gt\doc\win32\lthook32\debug\lthook32.map debug

\user\gt\tool\mapsympe debug\ltw32.map
\user\gt\tool\mapsympe debug\ltw32d.map
\user\gt\tool\mapsympe debug\lthook32.map
move *.sym debug
