all: 
	pushd build && cl -Zi -EHsc ../main.cpp -Fe:Handmade.exe User32.lib Gdi32.lib && popd

run:
	./build/Handmade.exe