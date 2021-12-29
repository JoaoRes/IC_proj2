#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include <map>
#include "BitStream.hh"
#include "Golomb.hh"

class GeneralFunctions{
    public:
        int readFile(char* inFile, short* bufferMono);
        int calculateM(int sum, int frames);
        short folding(short residual);
        short defolding(short n);
    private:
        SF_INFO sfinfo;
};

int GeneralFunctions::readFile(char* inFile, short* bufferMono){
    SNDFILE* audioFile;
    sfinfo.format=0;

    audioFile=sf_open(inFile, SFM_READ, &sfinfo);

    int numItems = (int) sfinfo.frames * sfinfo.channels; 
    short* buffer = (short * ) malloc(numItems * sizeof(short));
    bufferMono = (short *) malloc(sfinfo.frames * sizeof(short));

    int cntData = sf_read_short(audioFile, buffer, numItems);

    for(int i=0; i<cntData ; i+=2){
        int avg=(buffer[i]+buffer[i+1])/2;
        bufferMono[i/2]= (short)avg;
    }

    sf_close(audioFile);
    return sfinfo.frames;
}

int GeneralFunctions::calculateM(int sum, int frames){
    double mean = (double) sum/frames;
    int m = ceil(-1/log2(mean/(mean+1.0)));

    return m;
}