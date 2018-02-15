# 3350 waterfall
# to compile your project, type make and press enter

CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGL

all: waterfall

lab1: waterfall.cpp
	g++ $(CFLAGS) waterfall.cpp \
	libggfonts.a -Wall $(LFLAGS) -o waterfall

clean:
	rm -f waterfall
	rm -f *.o

