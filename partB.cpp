#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include "BitStream.hh"
#include "Golomb.hh"

using namespace std;

short *bufferMono;
short *bufferError;

int readFile(char* inFile);
int predictor(int frames);
void calculateHistograms(int frames);
void lossyCoding(int frames, int nbits);

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample03.wav";
    int frames = readFile(s);
    int m = predictor(frames);
    cout << "----------------     OPTIMAL M      ----------------" << endl;
    cout << "M -> " << m << endl;
    calculateHistograms(frames);
    lossyCoding(frames, 4);

    return 0;
}

int readFile(char* inFile){
    SNDFILE* audioFile;
    SF_INFO sfinfo;
    sfinfo.format=0;

    audioFile=sf_open(inFile, SFM_READ, &sfinfo);

    int numItems = (int) sfinfo.frames * sfinfo.channels;
    short* buffer = (short * ) malloc(numItems * sizeof(short));
    bufferMono = (short *) malloc(sfinfo.frames * sizeof(short));

    int cntData = sf_read_short(audioFile, buffer, numItems);

    for(int i=0; i<cntData ; i+=2){
        int avg=(buffer[i]+buffer[i+1])/2;
        bufferMono[i/2]= avg;
    }

    return sfinfo.frames;
}

int predictor(int frames){
    bufferError = (short * ) malloc(frames * sizeof(short));
    int sum=0;
    long long sum2=0;

    for(int i=0 ; i<frames ; i++){
        int fLinha;
        if(i==0){
            fLinha=0;
        }else if(i==1){
            fLinha = bufferMono[i-1];
        }else if (i==2){
            fLinha = 2*bufferMono[i-1] - bufferMono[i-2];
        }else{
            fLinha = 3*bufferMono[i-1] - 3*bufferMono[i-2] + bufferMono[i-3];
        }
        bufferError[i] = bufferMono[i] - fLinha;
        sum+=bufferError[i];
        //cout << "Residual -> " << bufferError[i] << " || Fn -> " << bufferMono[i] << endl;
    }
    
    double mean = (double) sum/frames;
    int m = log2(log(2) * (mean/sqrt(2)));
    
    return m;
}

void calculateHistograms(int frames){
    map<float, int> H_error;
    map<float, int> H_mono;
    map<float, int>::iterator it;

    for(int i=0; i<frames ; i++){
        H_error[bufferError[i]]++;
        H_mono[bufferMono[i]]++;
    }

    double entr=0;
    double p=0;

    cout << "----------------     MONO      ----------------" << endl;
    for(it=H_mono.begin(); it!=H_mono.end();it++){
        // cout << it->first << " ";
        // for(int i=0 ; i<it->second ; i++){
        //     cout << "*";
        // }
        // cout << endl;
        
        p=(double)H_mono[it->first]/(frames);
        entr+=-p*log(p);
    }

    cout << "entropy -> " << entr << endl;
    
    cout << "----------------     ERROR     ----------------" << endl;
    entr=0;
    p=0;
    for(it=H_error.begin(); it!=H_error.end();it++){
        // cout << it->first << " ";
        // for(int i=0 ; i<it->second ; i++){
        //     cout << "*";
        // }
        // cout << endl;
        
        p=(double)H_error[it->first]/(frames);
        entr+=-p*log(p);
    }

    cout << "entropy -> " << entr << endl;
}

void lossyCoding(int frames, int nbits){
    // TODO calcular nbits otimo
    
    int ptr[frames];
    for(int i=0 ; i<frames ; i++){
        ptr[i]=(bufferMono[i] >>  nbits) << nbits;
    }
}