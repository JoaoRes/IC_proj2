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

SF_INFO readFile(char* inFile, SF_INFO sfinfo){
    SNDFILE* audioFile;
    sfinfo.format=0;

    audioFile=sf_open(inFile, SFM_READ, &sfinfo);

    int numItems = (int) sfinfo.frames * sfinfo.channels; 
    short* buffer = (short * ) malloc(numItems * sizeof(short));
    bufferMono = (short * ) malloc(sfinfo.frames * sizeof(short));

    int cntData = sf_read_short(audioFile, buffer, numItems);

    for(int i=0; i<cntData ; i+=2){
        int avg=(buffer[i]+buffer[i+1])/2;
        bufferMono[i/2]= (short)avg;
    }

    sf_close(audioFile);
    return sfinfo;
}

int fLinha_calc(int i, short* buffer){
    int fLinha;
    if(i>2){
        fLinha = 3*buffer[i-1] - 3*buffer[i-2] + buffer[i-3];
    }else if(i==2){
        fLinha = 2*buffer[i-1] - buffer[i-2];
    }else if(i==1){
        fLinha = buffer[i-1];
    }else{
        fLinha = 0;
    }

    return fLinha;
}

int predictor(int frames, short* bufferIn, short* bufferOut){    
    int sum=0;
    int fLinha;
    short value;
    for(int i=0 ; i<frames ; i++){
        fLinha = fLinha_calc(i, bufferIn);

        value = bufferIn[i] - fLinha;

        bufferOut[i] = gf.folding(value);
        sum+=bufferOut[i];

        #ifdef _DEBUG
            cout << "Residual -> " << bufferOut[i] << " || Fn -> " << bufferIn[i] << endl;
        #endif
    }

    return sum;
}

void losslessEncoder(int m, int frames, short* buffer){
    Golomb g;
    BitStream b("", "lossless_encoded_output.txt");
    string gCode;

    for (int i=0 ; i<frames ; i++){
        gCode = g.encoder(buffer[i], m);
        #ifdef _DEBUG
            cout << "GOLOMB CODE -> " << gCode << endl;
        #endif
        codes_length.push_back(gCode.length());
        #ifdef _DEBUG
            cout << "GOLOMB CODE LENGTH -> " << codes_length.at(i) << endl;
        #endif
        b.writeNBits(gCode);
    }
    b.close();
}

void losslessDecoder(int m, int frames, char* file, SF_INFO sfinfo){
    SNDFILE* outFile;
    sfinfo.channels=1;

    outFile=sf_open(file, SFM_WRITE, &sfinfo);
    short* buffer = (short*) malloc(frames*sizeof(short));
    BitStream b("lossless_encoded_output.txt", "");
    Golomb g;
    string code;
    short nINT;
    short n;
    short fLinha;
    for (int i=0 ; i<frames ; i++){
        code = b.readNBits(codes_length.at(i));
        nINT = g.decoder(code, m);
        n = gf.defolding(nINT);
        fLinha_calc(i, buffer);
        buffer[i]=(short) n+fLinha;
        #ifdef _DEBUG
            // cout << i << "/" << frames << "-> code: " << code << " | nINT(Residual): " << nINT << " | BUFFER_RESIDUAL: " << bufferResidual[i] << " | n(defolded): " << n;
            // cout << " | DECODED -> " << buffer[i] << " | MONOBUFFER -> " << bufferMono[i] << endl;
        #endif
    }

    int rdData = sf_write_short(outFile, buffer, frames);

    sf_close(outFile);
    b.close();
}

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample01.wav";
    char* s1 = "xxxxxx.wav";
    // args verification


    SF_INFO sfinfo;

    // read wav File
    sfinfo = readFile(s, sfinfo);
    int frames = sfinfo.frames;
    
    bufferResidual = (short * ) malloc(frames * sizeof(short));


    #ifdef _DEBUG
        cout << "frames: " << frames << endl;
    #endif
    // ----------- LOSSLESS ENCODER ---------------
    int sum = predictor(frames, bufferMono, bufferResidual);

    int m = gf.calculateM(sum, frames);
    cout << "----------------     OPTIMAL M      ----------------" << endl;
    cout << "M -> " << m << endl;

    gf.calculateHistograms(frames, bufferMono, "MONO");
    gf.calculateHistograms(frames, bufferResidual, "RESIDUAL");

    losslessEncoder(m, frames, bufferResidual);

    // ----------- LOSSLESS DECODING --------------
    losslessDecoder(m, frames, s1, sfinfo);

    return 0;
}