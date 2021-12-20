#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include "BitStream.hh"
#include "Golomb.hh"

using namespace std;

int main(int argc, char* argv[]){
    char* inFile;
    
    SNDFILE* audioFile;
    SF_INFO sfinfo;
    sfinfo.format=0;

    audioFile=sf_open(inFile, SFM_READ, &sfinfo);

    int numItems = (int) sfinfo.frames * sfinfo.channels;
    int* buffer = (int * ) malloc(numItems * sizeof(int));
    int* monoBuffer = (int *) malloc(sfinfo.frames * sizeof(int));
    int* bufferError = (int * ) malloc(numItems * sizeof(int));

    int cntData = sf_read_int(audioFile, buffer, numItems);

    for(int i=0; i<cntData ; i+=2){
        int avg=(buffer[i]+buffer[i+1])/2;
        monoBuffer[i/2]=avg;
    }

    for(int i=0 ; i<sfinfo.frames ; i++){
        int fLinha;
        if(i==0){
            fLinha=0;
        }else if(i==1){
            fLinha = monoBuffer[i-1];
        }else{
            fLinha = (monoBuffer[i-1] + monoBuffer[i-2]) / 2;
        }
        bufferError[i] = monoBuffer[i] - fLinha;
    }

    for(int a : *bufferError){
        cout << a << endl;
    }
}