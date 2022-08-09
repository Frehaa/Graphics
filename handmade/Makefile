COMPILER = cl
LIBS = User32.lib Gdi32.lib # Dsound.lib
FLAGS = -Zi -EHsc

all: main.cpp 
	pushd build && ${COMPILER} ${FLAGS} ../main.cpp -Fe:Handmade.exe ${LIBS} && popd

run:
	./build/Handmade.exe