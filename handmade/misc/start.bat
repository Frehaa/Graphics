@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
path=F:\Projects\Graphics\handmade\misc\;%path%
cd ..
devenv F:\Projects\Graphics\handmade\build\Handmade.exe
code .