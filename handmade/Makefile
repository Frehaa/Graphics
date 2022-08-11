CC = cl
CL = LINK
LIBS = User32.lib Gdi32.lib # Dsound.lib
CFLAGS = -Zi -EHsc -c  # -W3
LFLAGS = /INCREMENTAL /MAP /DEBUG
OUT = build
TARGET = Handmade.exe
OBJECTS = ${OUT}/win32_platform.o ${OUT}/game.o
SOURCES = win32_platform.cpp game.cpp

${OUT}/${TARGET}: ${OBJECTS}
	${CL} ${LFLAGS} ${OUT}/win32_platform.o ${OUT}/game.o /OUT:${OUT}/${TARGET} ${LIBS}

$(OUT)/win32_platform.o: win32_platform.cpp
	${CC} ${CFLAGS} $? /Fo:$@ /Fd:$*.pdb

$(OUT)/game.o: game.cpp
	${CC} ${CFLAGS} $? /Fo:$@ /Fd:$*.pdb

run:
	${OUT}/Handmade.exe

clean:
	del *.obj
	del *.pdb
	del *.ilk
	del *.exe