SYSCONF_LINK = cl
CPPFLAGS     =
LDFLAGS      =
LIBS         = -lm

OUTDIR = ./out/
DESTDIR = ./
TARGET  = main.exe

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(DESTDIR)$(TARGET)

test:
	echo test

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -out $(DESTDIR)$(TARGET) $(OUTDIR)$(OBJECTS) $(LIBS)


$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) $(CFLAGS) $< -o $@
tgaimage.o: 
	cl -EHsc tgaimage.cpp
main.o: 
	cl -EHsc main.cpp tgaimage.cpp


clean:
	-rm -f main.o tgaimage.o
	-rm -f ./ma.exe
	-rm -f *.tga

