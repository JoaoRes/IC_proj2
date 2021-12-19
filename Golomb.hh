#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
using namespace std;

class Golomb{
    public:
        Golomb(int m);
        void encoder(int num);
        void decoder(string code, int m);
        void decToBinary(int n);
    private:
        int m_;
        int q=0;
        int r=0;
};

Golomb::Golomb(int m){
    m_=m;
}

void Golomb::encoder(int num){
    bitset<64> r_binary;
    uint64_t numUnary = 0;
    string numBinary;

    q=floor(num/m_);
    r=num-q*m_;

    if(q == 0) {
        numUnary = 0;
    }
    else if (q>=1) {
        numUnary=1;
        for(int i = 0 ; i< q; i++) {
            numUnary= (numUnary*10)+1;
        }
        numUnary=numUnary-1;
    }

    // if m is power of 2
    if(ceil(log2(m_)) == floor(log2(m_))){
        int firstONE = 0;
        string auxStr = bitset<64>(r).to_string();
        for (int i=0; i<64; i++){
            if(auxStr[i]=='1' && firstONE==0){
                firstONE=1;
                numBinary+=auxStr[i];
            }
            else if (firstONE==1) {
                numBinary+=auxStr[i];
            }   
        }
    } else { // if m is not power of 2
        int b=ceil(log2(m_));
        int nBits;

        if(r<b){
            nBits=b-1;
        }
        else {
            r = r+pow(2, b);
            nBits = b;
        }

        int aux=0;
        string auxStr = bitset<64>(r).to_string();
        string reverse;

        for (int j=63; j>63-nBits; j--){
            reverse=auxStr[j]+reverse;
        }

        numBinary+=reverse;
    }
    
    cout << "unary " << numUnary  << " binary " << numBinary << endl;

}

void Golomb::decoder(string code, int m){
    int separation = (int) code.find('0');

    string unary_q = code.substr(0,separation);
    string binary_r = code.substr(separation+1);

    int q = sizeof(unary_q)/sizeof(char);

    cout << "q " << q << endl;

    cout << "unary " << unary_q << endl;

    cout << "binary " << binary_r << endl;

    
    
}
