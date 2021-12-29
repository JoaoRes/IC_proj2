#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>
#include <map>
#include "BitStream.hh"
#include "Golomb.hh"
#include <chrono>
using namespace std::chrono;

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

int calculateHistograms(int frames){
    map<short, int> H_mono;
    map<short, int>::iterator it;

    for(int i=0; i<frames ; i++){
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

    return ceil(entrMono);
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

    int nBits = 16 - entrMono;// (int)ceil(entrMono);

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
    #ifdef _TIMES
        auto start = high_resolution_clock::now();
    #endif
    char s2[] = "zzz.wav";

    if(argc<1){
        printf("\nMissing arguments");
        exit(1);
    }
    // read wav File
    int frames = readFile(argv[1]);

    //calculating entropy
    #ifdef _TIMES
        auto hist_start = high_resolution_clock::now();
    #endif
    int entrMono = calculateHistograms(frames);
    #ifdef _TIMES
        auto hist_end = high_resolution_clock::now();
        auto lossyCoding_start = high_resolution_clock::now();
    #endif
    // ----------- LOSSY ENCODING ----------------
    lossyCoding(frames, entrMono);
    #ifdef _TIMES
        auto lossyCoding_end = high_resolution_clock::now();
        auto lossyDecoding_start = high_resolution_clock::now();
    #endif
    // ----------- LOSSY DECODING ------------------
    lossyDecoding(s2);
    #ifdef _TIMES
        auto lossyDecoding_end = high_resolution_clock::now();

        auto end = high_resolution_clock::now();
        auto hist_duration = duration_cast<milliseconds>(hist_end - hist_start);
        auto lossyCoding_duration = duration_cast<milliseconds>(lossyCoding_end - lossyCoding_start);
        auto lossyDecoding_duration = duration_cast<milliseconds>(lossyDecoding_end - lossyDecoding_start);
        auto total_duration = duration_cast<milliseconds>(end - start);
        cout << "Histogram Time = "<< (double)hist_duration.count()/1000 << " seconds | "<< hist_duration.count() << " milliseconds" << endl;
        cout << "Encoding Time = "<< (double)lossyCoding_duration.count()/1000 << " seconds | "<< lossyCoding_duration.count() << " milliseconds" << endl;
        cout << "Decoding Time = "<< (double)lossyDecoding_duration.count()/1000 << " seconds | "<< lossyDecoding_duration.count() << " milliseconds" << endl;
        cout << "Total Time = "<< (double)total_duration.count()/1000 << " seconds | "<< total_duration.count() << " milliseconds" << endl;
    #endif
    return 0;
}