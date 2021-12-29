#include <iostream>
#include <vector>
#include <sndfile.h>
#include "BitStream.hh"
#include "Golomb.hh"
#include "GeneralFunctions.hh"
#include <chrono>
using namespace std::chrono;
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
            //cout << "Residual -> " << bufferOut[i] << " || Fn -> " << bufferIn[i] << endl;
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
        fLinha = fLinha_calc(i, buffer);
        buffer[i]=(short) n+fLinha;
        #ifdef _DEBUG
            // cout << i << "/" << frames << "-> code: " << code << " | nINT(Residual): " << nINT << " | BUFFER_RESIDUAL: " << bufferResidual[i] << " | n(defolded): " << n;
            // cout << " | DECODED -> " << buffer[i] << " | MONOBUFFER -> " << bufferMono[i] << endl;
        #endif
    }

    sf_write_short(outFile, buffer, frames);

    sf_close(outFile);
    b.close();
}

int main(int argc, char* argv[]){
    #ifdef _TIMES
        auto start = high_resolution_clock::now();
    #endif
    char s1[] = "xxxxxx.wav";
    // args verification

    if(argc<1){
        printf("\nMissing arguments");
        exit(1);
    }

    SF_INFO sfinfo;

    // read wav File
    sfinfo = readFile(argv[1], sfinfo);
    int frames = sfinfo.frames;
    
    bufferResidual = (short * ) malloc(frames * sizeof(short));


    #ifdef _DEBUG
        cout << "frames: " << frames << endl;
    #endif
    // ----------- LOSSLESS ENCODER ---------------
    #ifdef _TIMES
        auto pred_start = high_resolution_clock::now();
    #endif
    int sum = predictor(frames, bufferMono, bufferResidual);
    #ifdef _TIMES
        auto pred_end = high_resolution_clock::now();
        auto calculateM_start = high_resolution_clock::now();
    #endif
    int m = gf.calculateM(sum, frames);
    cout << "----------------     OPTIMAL M      ----------------" << endl;
    cout << "M -> " << m << endl;
    #ifdef _TIMES
        auto calculateM_end = high_resolution_clock::now();
        auto hist_start = high_resolution_clock::now();
    #endif
    gf.calculateHistograms(frames, bufferMono, "MONO");
    gf.calculateHistograms(frames, bufferResidual, "RESIDUAL");
    #ifdef _TIMES
        auto hist_end = high_resolution_clock::now();
        auto losslessCoding_start = high_resolution_clock::now();
    #endif

    losslessEncoder(m, frames, bufferResidual);
    #ifdef _TIMES
        auto losslessCoding_end = high_resolution_clock::now();
        auto losslessDecoding_start = high_resolution_clock::now();
    #endif
    cout << "----------------  ENCODING COMPLETE  ----------------" << endl;

    // ----------- LOSSLESS DECODING --------------
    losslessDecoder(m, frames, s1, sfinfo);
    #ifdef _TIMES
        auto losslessDecoding_end = high_resolution_clock::now();
        auto end = high_resolution_clock::now();
        auto predictor_duration = duration_cast<milliseconds>(pred_end - pred_start);
        auto calculateM_duration = duration_cast<microseconds>(calculateM_end - calculateM_start);
        auto hist_duration = duration_cast<milliseconds>(hist_end - hist_start);
        auto losslessCoding_duration = duration_cast<milliseconds>(losslessCoding_end - losslessCoding_start);
        auto losslessDecoding_duration = duration_cast<milliseconds>(losslessDecoding_end - losslessDecoding_start);
        auto total_duration = duration_cast<milliseconds>(end - start);
        cout << "Predictor Time = "<< (double)predictor_duration.count()/1000 << " seconds | "<< predictor_duration.count() << " milliseconds" << endl;
        cout << "Calc M Time = "<< (double)calculateM_duration.count()/1000 << " miliseconds | "<< calculateM_duration.count() << " microseconds" << endl;
        cout << "Histogram Time = "<< (double)hist_duration.count()/1000 << " seconds | "<< hist_duration.count() << " milliseconds" << endl;
        cout << "Encoding Time = "<< (double)losslessCoding_duration.count()/1000 << " seconds | "<< losslessCoding_duration.count() << " milliseconds" << endl;
        cout << "Decoding Time = "<< (double)losslessDecoding_duration.count()/1000 << " seconds | "<< losslessDecoding_duration.count() << " milliseconds" << endl;
        cout << "Total Time = "<< (double)total_duration.count()/1000 << " seconds | "<< total_duration.count() << " milliseconds" << endl;

    ofstream MyFile;
    MyFile.open(argv[2]);

    MyFile <<argv[1]<<";;:;"<< total_duration.count()<<"\t"<< predictor_duration.count()<<"\t"<< hist_duration.count()<<"\t"
    << losslessCoding_duration.count()<<"\t"<< losslessDecoding_duration.count()<<"\t"<<ceil(m)<<"\t" <<endl;

    MyFile.close();

    #endif
    cout << "----------------  DECODING COMPLETE  ----------------" << endl;

    return 0;
}