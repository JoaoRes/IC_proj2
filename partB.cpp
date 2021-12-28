#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include <map>
#include "BitStream.hh"
#include "Golomb.hh"

using namespace std;

short *bufferMono;
short *bufferResidual;
short *bufferResidualdefolded;
vector<int> codes_length;
SF_INFO sfinfo;

int readFile(char* inFile);
int predictor(int frames);
short folding(short residual);
short defolding(short n);
int calculateHistograms(int frames);
void encoder(int m, int frames);
void decoder(int m, int frames, char* s1);
void lossyCoding(int frames, int entropy);
void lossyDecoding(int frames);

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample01.wav";
    char* s1 = "xxxxxx.wav";

    // read wav File
    int frames = readFile(s);
    //frames = 1;
    
    // ----------- LOSSLESS ENCODER ---------------
    // int m = predictor(frames);
    // cout << "----------------     OPTIMAL M      ----------------" << endl;
    // cout << "M -> " << m << endl;
    int entrMono = calculateHistograms(frames);
    // encoder(m, frames);

    // ----------- LOSSLESS DECODING --------------
    // decoder(m, frames, s1);

    // ----------- LOSSY ENCODING ----------------
    lossyCoding(frames, entrMono);

    // ----------- LOSSY DECODING ------------------
    lossyDecoding(frames);

    return 0;
}

int readFile(char* inFile){
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

int predictor(int frames){
    bufferResidual = (short * ) malloc(frames * sizeof(short));
    bufferResidualdefolded = (short * ) malloc(frames * sizeof(short));
    int sum=0;
    int fLinha;

    for(int i=0 ; i<frames ; i++){
        if(i>2){
            fLinha = 3*bufferMono[i-1] - 3*bufferMono[i-2] + bufferMono[i-3];
        }else if(i==2){
            fLinha = 2*bufferMono[i-1] - bufferMono[i-2];
        }else if(i==1){
            fLinha = bufferMono[i-1];
        }else{
            fLinha=0;
        }
        bufferResidualdefolded=bufferMono-fLinha;
        bufferResidual[i] = folding(bufferMono[i] - fLinha);
        sum+=bufferResidual[i];
        #ifdef _DEBUG
            cout << "Residual -> " << bufferResidual[i] << " || Fn -> " << bufferMono[i] << endl;
        #endif
    }
    
    double mean = (double) sum/frames;
    int m = ceil(-1/log2(mean/(mean+1.0)));
    
    return m;
}

int calculateHistograms(int frames){
    map<short, int> H_residual;
    map<short, int> H_mono;
    map<short, int>::iterator it;

    for(int i=0; i<frames ; i++){
        //H_residual[defolding(bufferResidual[i])]++;
        H_mono[bufferMono[i]]++;
    }

    double entrMono=0;
    double p=0;
    ofstream MyFile;
    MyFile.open("histMono.txt");


    cout << "----------------     MONO      ----------------" << endl;
    for(it=H_mono.begin(); it!=H_mono.end();it++){
        // cout << it->first << " ";
        // for(int i=0 ; i<it->second ; i++){
        //     cout << "*";
        // }
        // cout << endl;
        std::string temp_first = std::to_string( it->first);
        std::string temp_second = std::to_string(it->second);
        MyFile << ""+string(temp_first)+"\t"+string(temp_second) <<endl;
        
        p=(double)H_mono[it->first]/(frames);
        entrMono+=-p*log(p);
    }
    MyFile.close();
    cout << "entropy -> " << entrMono << endl;
    
    // cout << "----------------     RESIDUAL     ----------------" << endl;
    // int entr=0;
    // p=0;
    // ofstream MyFile1;
    // MyFile1.open("histResidual.txt");
    // for(it=H_residual.begin(); it!=H_residual.end();it++){
    //     // cout << it->first << " ";
    //     // for(int i=0 ; i<it->second ; i++){
    //     //     cout << "*";
    //     // }
    //     // cout << endl;

    //     std::string temp_first = std::to_string( it->first);
    //     std::string temp_second = std::to_string(it->second);
    //     MyFile1 << ""+string(temp_first)+"\t"+string(temp_second) <<endl;
        
    //     p=(double)H_residual[it->first]/(frames);
    //     entr+=-p*log(p);
    // }
    // MyFile1.close();
    // cout << "entropy -> " << entr << endl;

    return entrMono;
}

void encoder(int m, int frames){
    Golomb g;
    BitStream b("", "lossless_encoded_output.txt");
    string gCode;

    for (int i=0 ; i<frames ; i++){
        gCode = g.encoder(bufferResidual[i], m);
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

void decoder(int m, int frames, char* file){
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
        n = defolding(nINT);

        if(i>2){
            fLinha = 3*buffer[i-1] - 3*buffer[i-2] + buffer[i-3];
        }else if(i==2){
            fLinha = 2*buffer[i-1] - buffer[i-2];
        }else if(i==1){
            fLinha = buffer[i-1];
        }else{
            fLinha=0;
        }
        buffer[i]=(short) n+fLinha;
        #ifdef _DEBUG
            cout << i << "/" << frames << "-> code: " << code << " | nINT(Residual): " << nINT << " | BUFFER_RESIDUAL: " << bufferResidual[i] << " | n(defolded): " << n << " | Residual Defolded: " << bufferResidualdefolded[i];
            cout << " | DECODED -> " << buffer[i] << " | MONOBUFFER -> " << bufferMono[i] << endl;
        #endif
    }

    int rdData = sf_write_short(outFile, buffer, frames);

    sf_close(outFile);
    b.close();
}

short folding(short residual){
    if(residual >= 0) residual = residual*2;
    else residual = residual*(-2) - 1;

    return residual;
}

short defolding(short n){
    if(n % 2 == 0){
        n /= 2;
    }else{
        n = (n + 1) / (-2);
    }
    return n;
}

void lossyCoding(int frames, int entrMono){
    Golomb g;
    BitStream b("", "lossy_encoded_output.txt");
    string gCode;
    string wByte;
    string entrMono_Unary;

    int nBits = 16 - (int)ceil(entrMono);

    for(int i=0 ; i<=entrMono ; i++) {
        entrMono_Unary +="1";
    }
    for(int i=entrMono+1 ; i<16 ; i++) {
        entrMono_Unary +="0";
    }

    b.writeNBits(entrMono_Unary);

    short wValue;
    for (int i=0 ; i<frames ; i++){
        wValue = bufferMono[i];

        //cout << g.decToBinary(wValue) << endl;
        wValue >> nBits;

        //cout << g.decToBinary(wValue) << endl;
        b.writeNBits(g.decToBinary(wValue));
    }

    b.close();
}

void lossyDecoding(int frames){
    BitStream b("lossy_encoded_output.txt", "");
    string entr_s = b.readNBits(16);

    int entropy = (int) entr_s.find('0');

    SNDFILE* outFile;
    sfinfo.channels=1;

    outFile=sf_open(f, SFM_WRITE, &sfinfo);
    short* buffer = (short*) malloc(frames*sizeof(short));
    for()


}