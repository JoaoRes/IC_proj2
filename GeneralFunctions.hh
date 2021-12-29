#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sndfile.h>
#include <map>
// #include "BitStream.hh"
// #include "Golomb.hh"

class GeneralFunctions{
    public:
        SF_INFO readFile(char* inFile, short* bufferMono);
        short folding(short residual);
        short defolding(short n);
        int calculateM(int sum, int frames);
        int calculateHistograms(int frames, short* buffer, string type);
    private:
        SF_INFO sfinfo;
};

SF_INFO GeneralFunctions::readFile(char* inFile, short* bufferMonoChannel){
    SNDFILE* audioFile;
    sfinfo.format=0;

    audioFile=sf_open(inFile, SFM_READ, &sfinfo);

    int numItems = (int) sfinfo.frames * sfinfo.channels; 
    short* buffer = (short * ) malloc(numItems * sizeof(short));
    bufferMonoChannel = (short *) malloc(sfinfo.frames * sizeof(short));

    int cntData = sf_read_short(audioFile, buffer, numItems);

    for(int i=0; i<cntData ; i+=2){
        int avg=(buffer[i]+buffer[i+1])/2;
        bufferMonoChannel[i/2]= (short)avg;
    }

    for (int i=0 ; i<sfinfo.frames ; i++){
        cout << "BufferMono[" << i << "]: " << bufferMonoChannel[i] << endl;
    }

    sf_close(audioFile);
    return sfinfo;
}

short GeneralFunctions::folding(short residual){
    cout << "merda" << endl;
    if(residual >= 0) residual = residual*2;
    else residual = residual*(-2) - 1;
    return residual;
}

short GeneralFunctions::defolding(short n){
    if(n % 2 == 0){
        n /= 2;
    }else{
        n = (n + 1) / (-2);
    }
    return n;
}

int GeneralFunctions::calculateM(int sum, int frames){
    double mean = (double) sum/frames;
    int m = ceil(-1/log2(mean/(mean+1.0)));

    return m;
}

int GeneralFunctions::calculateHistograms(int frames, short* buffer, string type){
    map<short, int> H_buffer;
    map<short, int>::iterator it;

    for(int i=0; i<frames ; i++){
        H_buffer[buffer[i]]++;
    }

    double entr=0;
    double p=0;
    ofstream MyFile;
    string namefile = "hist"+type+".txt";
    MyFile.open("histMono.txt");


    cout << "----------------     " << type << "      ----------------" << endl;
    for(it=H_buffer.begin(); it!=H_buffer.end();it++){
        // cout << it->first << " ";
        // for(int i=0 ; i<it->second ; i++){
        //     cout << "*";
        // }
        // cout << endl;
        std::string temp_first = std::to_string( it->first);
        std::string temp_second = std::to_string(it->second);
        MyFile << ""+string(temp_first)+"\t"+string(temp_second) <<endl;
        
        p=(double)H_buffer[it->first]/(frames);
        entr+=-p*log(p);
    }
    MyFile.close();
    cout << "entropy -> " << entr << endl;

    return entr;
}