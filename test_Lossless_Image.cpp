#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
#include "LosslessCodec.hh"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;


int main(int argc, char* argv[]){
    //Mat img = imread("imagens_PPM/lena.ppm");

    LosslessCodec llc;

    llc.encode(argv[1]);

    llc.decode("img.bin");

}