CC=gcc
SRC=main.c playlist.c converter.c
CFLAGS=-DLIBGME064
INCLUDE=-I./ -I./../ #-I/usr/include/
LIBS=-L./ -lgme -lsndfile
TARGET=gm2wav

all : $(TARGET)

$(TARGET) : $(SRC)
	$(CC) $(SRC) $(LIBS) $(INCLUDE) $(CFLAGS) -o $(TARGET)
