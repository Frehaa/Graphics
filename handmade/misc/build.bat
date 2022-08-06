@echo off

mkdir F:\Projects\Graphics\handmade\build\
pushd F:\Projects\Graphics\handmade\build\
cl /Zi /EHsc ../main.cpp User32.lib Gdi32.lib
popd