@echo off

mkdir build
pushd build
cl /Zi /EHsc ../main.cpp User32.lib Gdi32.lib
popd