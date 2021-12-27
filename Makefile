CC= g++
CFLAGS= -g -Wall -I/usr/include/opencv4
LFLAGS= pkg-config --libs opencv4

LosslessCodec.o: testopen.cpp 
    $(CC) $(CFLAGS) $(LFLAGS) $< -o $@

clean: 
    rm *.o