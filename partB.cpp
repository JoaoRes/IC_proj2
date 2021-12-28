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
vector<int> codes_length;
SF_INFO sfinfo;

int readFile(char* inFile);
int predictor(int frames);
short folding(short residual);
short defolding(short n);
void calculateHistograms(int frames);
void encoder(int m, int frames);
void decoder(int m, int frames, char* s1);
void lossyCoding(int frames, int nbits);

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample01.wav";
    char* s1 = "xxxxxx.wav";

    // read wav File
    int frames = readFile(s);
    // frames = 1;
    
    // ----------- LOSSLESS ENCODER ---------------
    int m = predictor(frames);
    cout << "----------------     OPTIMAL M      ----------------" << endl;
    cout << "M -> " << m << endl;
    calculateHistograms(frames);
    encoder(m, frames);

    // ----------- LOSSLESS DECODING --------------
    decoder(m, frames, s1);

    
    //lossyCoding(frames, 4);

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

void calculateHistograms(int frames){
    map<short, int> H_residual;
    map<short, int> H_mono;
    map<short, int>::iterator it;

    for(int i=0; i<frames ; i++){
        H_residual[defolding(bufferResidual[i])]++;
        H_mono[bufferMono[i]]++;
    }

    double entr=0;
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
        entr+=-p*log(p);
    }
    MyFile.close();
    cout << "entropy -> " << entr << endl;
    
    cout << "----------------     RESIDUAL     ----------------" << endl;
    entr=0;
    p=0;
    ofstream MyFile1;
    MyFile1.open("histResidual.txt");
    for(it=H_residual.begin(); it!=H_residual.end();it++){
        // cout << it->first << " ";
        // for(int i=0 ; i<it->second ; i++){
        //     cout << "*";
        // }
        // cout << endl;

        std::string temp_first = std::to_string( it->first);
        std::string temp_second = std::to_string(it->second);
        MyFile1 << ""+string(temp_first)+"\t"+string(temp_second) <<endl;
        
        p=(double)H_residual[it->first]/(frames);
        entr+=-p*log(p);
    }
    MyFile1.close();
    cout << "entropy -> " << entr << endl;
}

void encoder(int m, int frames){
    Golomb g;
    BitStream b("", "encode_output.txt");
    string gCode;
    string wByte;

    for (int i=0 ; i<frames ; i++){
        gCode = g.encoder(bufferResidual[i], m);
        #ifdef _DEBUG
            cout << gCode << endl;
        #endif
        wByte.append(gCode);
        #ifdef _DEBUG
            cout << wByte << endl;
        #endif
        codes_length.push_back(gCode.length());
        #ifdef _DEBUG
            cout << codes_length.at(i) << endl;
        #endif
        while(wByte.length() >= 8){
            if(wByte.length() % 8 == 0){
                b.writeNBits(wByte.substr(0, wByte.length()));
                wByte="";
            }else if(wByte.length() > 8){
                b.writeNBits(wByte.substr(0, 8));
                wByte.erase(0,8);
            }
        }
    }
    
    while(wByte.length() != 8){
        wByte+="0";
    }
    b.writeNBits(wByte);
    b.close();
}

void decoder(int m, int frames, char* file){
    SNDFILE* outFile;
    sfinfo.channels=1;

    outFile=sf_open(file, SFM_WRITE, &sfinfo);
    short* buffer = (short*) malloc(frames*sizeof(short));
    BitStream b("encode_output.txt", "");
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
            fLinha = 3*buffer[i-1] + 3*buffer[i-2] + buffer[i-3];
        }else if(i==2){
            fLinha = 2*buffer[i-1] + buffer[i-2];
        }else if(i==1){
            fLinha = buffer[i-1];
        }else{
            fLinha=0;
        }
        buffer[i]=(short) n+fLinha;
        #ifdef _DEBUG
            cout << i << "/" << frames << ": code: " << code << " | nINT: " << nINT << " | n: " << n;
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


// for (int i=0 ; i<frames ; i++){
//         code = b.readNBits(codes_length.at(i));
//         nINT = g.decoder(code, m);
//         n = defolding(nINT);

//         if(i>2){
//             fLinha = 3*buffer[i-1] - 3*buffer[i-2] + buffer[i-3];
//         }else if(i==2){
//             fLinha = 2*buffer[i-1] - buffer[i-2];
//         }else if(i==1){
//             fLinha = buffer[i-1];
//         }else{
//             fLinha=0;
//         }
//         buffer[i]=n+fLinha;
//         // cout << i << "/" << frames << ": code: " << code << " | nINT: " << nINT << " | n: " << n;
//         // cout << " | DECODED -> " << buffer[i] << " | MONOBUFFER -> " << bufferMono[i] << endl;
// }

// void lossyCoding(int frames, int nbits){
//     // TODO calcular nbits otimo
    
//     int ptr[frames];
//     for(int i=0 ; i<frames ; i++){
//         ptr[i]=(bufferMono[i] >>  nbits) << nbits;
//     }
// }