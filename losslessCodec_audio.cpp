#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include <map>
#include "BitStream.hh"
#include "Golomb.hh"
#include "GeneralFunctions.hh"

using namespace std;

short *bufferMono;
short *bufferResidual;
vector<int> codes_length;
GeneralFunctions gf;

int predictor(int frames);
int calculateHistograms(int frames);
void losslessEncoder(int m, int frames);
void losslessDecoder(int m, int frames, char* s1);

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample01.wav";
    char* s1 = "xxxxxx.wav";
    // args verification



    // read wav File
    int frames = gf.readFile(s, bufferMono);
    
    // ----------- LOSSLESS ENCODER ---------------
    int sum = predictor(frames, bufferMono, bufferResidual);
    int m = gf.calculateM(sum, frames);
    cout << "----------------     OPTIMAL M      ----------------" << endl;
    cout << "M -> " << m << endl;
    int entrMono = calculateHistograms(frames);
    losslessEncoder(m, frames);

    // ----------- LOSSLESS DECODING --------------
    losslessDecoder(m, frames, s1);

    return 0;
}

int predictor(int frames, short* bufferIn, short* bufferOut){
    bufferOut = (short * ) malloc(frames * sizeof(short));
    
    int sum=0;
    int fLinha;

    for(int i=0 ; i<frames ; i++){
        if(i>2){
            fLinha = 3*bufferIn[i-1] - 3*bufferIn[i-2] + bufferIn[i-3];
        }else if(i==2){
            fLinha = 2*bufferIn[i-1] - bufferIn[i-2];
        }else if(i==1){
            fLinha = bufferIn[i-1];
        }else{
            fLinha=0;
        }

        bufferOut[i] = gf.folding(bufferIn[i] - fLinha);
        sum+=bufferOut[i];

        #ifdef _DEBUG
            cout << "Residual -> " << bufferOut[i] << " || Fn -> " << bufferMono[i] << endl;
        #endif
    }

    return sum;
}