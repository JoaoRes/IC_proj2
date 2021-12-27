#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class BitStream{
    public:
        BitStream(string inFile, string outFile);
        int readBit(void);
        string readNBits(int nBits);
        void writeBit(int bit);
        void writeNBits(string nBits);
        vector<int>getBuffer();
        void close();
    private:
        vector<int> buffer;
        fstream inFile;
        fstream outFile;
};

BitStream::BitStream(string inF, string outF){
    if(not inF.empty()) {
        inFile.open(inF);
        char c;
        while (inFile.get(c)) {
            for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
                buffer.push_back(((c >> i) & 1));
        }
    }else{
        cout << "No In File to open" << endl;
    }

    if(not outF.empty()){
        outFile.open(outF, fstream::out | fstream::app);
    }else{
        cout << "No out File to open" << endl;
    }
}

int BitStream::readBit(){
    if(inFile.is_open()){
        vector<int> aux;
        int bit=buffer.at(0);
        for(int i=1 ; i<(int)buffer.size() ; i++){
            aux.push_back(buffer[i]);
        }
        buffer=aux;
        return bit;
    }
    cout << "File not Open!!" << endl;
    return 1;
}

string BitStream::readNBits(int nBits){
    if(inFile.is_open()){
        vector<int>::const_iterator begin = buffer.begin()+nBits;
        vector<int>::const_iterator end = buffer.end();
        vector<int> aux(begin, end);
        string nbits_string="";

        for(int i=0 ; i<nBits ; i++)
            nbits_string += to_string(buffer[i]);

        buffer=aux;
        return nbits_string;

    }
    cout << "File not Open!!" << endl;
    return "";
}

// not 100%, not sure how
void BitStream::writeBit(int bit){
    if(outFile.is_open()){
        outFile << bit;
    }else{
        cout << "File not Open!!" << endl;
    }
}

void BitStream::writeNBits(string nBits){
    int bitcount = 0;
    char currentByte = 0;

    if(outFile.is_open()){
        for(int i=0 ; i<(int)nBits.length() ; i++){
            currentByte = currentByte << 1;
            currentByte |= nBits[i]-'0';
            bitcount++;
            if (bitcount == 8){
                outFile << currentByte;
                currentByte = 0;
                bitcount = 0;
            }
        }
    }else{
        cout << "No output file to write into!" << endl;
    }
}

void BitStream::close(void){
    //BitStream close method
    if(inFile.is_open()){
        inFile.close();
    }
    if(outFile.is_open()){
        outFile.close();
    }
}

vector<int> BitStream::getBuffer(){
    return buffer;
}
