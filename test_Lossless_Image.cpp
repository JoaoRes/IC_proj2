#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
#include "LosslessCodec.hh"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <chrono>
using namespace std::chrono;


using namespace std;
using namespace cv;


int main(int argc, char* argv[]){
    //Mat img = imread("imagens_PPM/lena.ppm");
        #ifdef _TIMES
            auto start = high_resolution_clock::now();
        #endif
        LosslessCodec llc;
        #ifdef _TIMES
            auto lossyCoding_start = high_resolution_clock::now();
        #endif
        int entropy = llc.encode(argv[1]);
        #ifdef _TIMES
            auto lossyCoding_end = high_resolution_clock::now();
            auto lossyDecoding_start = high_resolution_clock::now();
        #endif
        llc.decode("img.bin");
        #ifdef _TIMES
        auto lossyDecoding_end = high_resolution_clock::now();

        auto end = high_resolution_clock::now();
        auto lossyCoding_duration = duration_cast<milliseconds>(lossyCoding_end - lossyCoding_start);
        auto lossyDecoding_duration = duration_cast<milliseconds>(lossyDecoding_end - lossyDecoding_start);
        auto total_duration = duration_cast<milliseconds>(end - start);
        cout << "Encoding Time = "<< (double)lossyCoding_duration.count()/1000 << " seconds | "<< lossyCoding_duration.count() << " milliseconds" << endl;
        cout << "Decoding Time = "<< (double)lossyDecoding_duration.count()/1000 << " seconds | "<< lossyDecoding_duration.count() << " milliseconds" << endl;
        cout << "Total Time = "<< (double)total_duration.count()/1000 << " seconds | "<< total_duration.count() << " milliseconds" << endl;

        ofstream MyFile;
        MyFile.open(argv[2]);

        MyFile <<argv[1]<<";;:;"<< total_duration.count()<<"\t"<< lossyCoding_duration.count()<<"\t"<< lossyDecoding_duration.count()<<"\t"<<entropy<<"\t" <<endl;

        MyFile.close();

    #endif

}