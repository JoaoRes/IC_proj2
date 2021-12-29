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

    return ceil(entrMono);
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
    cout << "Nframes ="<<frames<<endl;
    Golomb g;
    BitStream b("", "lossy_encoded_output.txt");
    string entrMono_Unary;
    string binary_frames = bitset<32>(frames).to_string(); //to binary
    cout<<binary_frames<<"\n";
    #ifdef _DEBUG
        cout << "Nframes ="<<frames<<endl;
        cout<<binary_frames<<"\n";
        cout << "entropy ="<<entrMono<<endl;
    #endif

    int nBits = 16 -entrMono;// (int)ceil(entrMono);

    for(int i=0 ; i<=entrMono-1 ; i++) {
        entrMono_Unary +="1";
    }
    for(int i=entrMono ; i<16 ; i++) {
        entrMono_Unary +="0";
    }

    b.writeNBits(entrMono_Unary);
    b.writeNBits(binary_frames);
    short wValue;
    for (int i=0 ; i<frames ; i++){
        wValue = bufferMono[i];

        //cout << g.decToBinary(wValue) << endl;
        #ifdef _DEBUG
            string binary_before  ("");
            int mask0 = 1;
            for(int i = 0; i < 16; i++)
            {
                if((mask0&wValue) >= 1)
                    binary_before = "1"+binary_before;
                else
                    binary_before = "0"+binary_before;
                mask0<<=1;
            }
        #endif
        wValue = (wValue >> nBits) << nBits;
        #ifdef _DEBUG
                string binary_shifted  ("");
                mask0 = 1;
                for(int i = 0; i < 16; i++)
                {
                    if((mask0&wValue) >= 1)
                        binary_shifted = "1"+binary_shifted;
                    else
                        binary_shifted = "0"+binary_shifted;
                    mask0<<=1;
                }
        #endif
        string binary  ("");
        int mask = 1;
        mask<<=nBits;
        for(int i = nBits; i < 16; i++)
        {
            if((mask&wValue) >= 1)
                binary = "1"+binary;
            else
                binary = "0"+binary;
            mask<<=1;
        }

        #ifdef _DEBUG
            cout << "nbits= ="<<nBits<<endl;
            cout<< "binary_before=  "<<binary_before<<endl;
            cout << "binary_shifted= "<<binary_shifted<<endl;
            cout<<"binary ="<<binary<<endl;

        #endif
        //cout << g.decToBinary(wValue) << endl;
        b.writeNBits(binary);
    }

    b.close();
}

void lossyDecoding(char* file){
    BitStream b("lossy_encoded_output.txt", "");
    string entr_s = b.readNBits(16);
    string binary_frames = b.readNBits(32);
    int entropy = (int) entr_s.find('0');
    if (entropy == -1)
        entropy = 16;
    cout << "entropy ="<<entropy<<endl;
    int frames = stoi(binary_frames,0,2);
    #ifdef _DEBUG
        cout << "entropy ="<<entropy<<endl;
        cout << "binary_frames ="<<binary_frames<<endl;
        cout << "frames ="<<frames<<endl;
    #endif

    SNDFILE* outFile;
    sfinfo.channels=1;

    outFile=sf_open(file, SFM_WRITE, &sfinfo);
    short* buffer = (short*) malloc(frames*sizeof(short));

    string code;
    short n;

    for (int i=0 ; i<frames ; i++){
        code = b.readNBits(entropy);
        n = (short) stoul(code,0,2);

        n = n << (16-entropy);

        buffer[i]= n;

        #ifdef _DEBUG1
                cout << "entropy = "<<entropy<<" frames = "<< frames <<"/"<< i <<" code= "<< code << " n = "<<n<<endl;
        #endif
    }
    sf_write_short(outFile, buffer, frames);

    sf_close(outFile);
    b.close();



}

int main(int argc, char* argv[]){
    char* s = "AudioSampleFiles/sample01.wav";
    char* s1 = "xxxxxx.wav";
    char* s2 = "zzz.wav";

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
    lossyDecoding(s2);

    return 0;
}