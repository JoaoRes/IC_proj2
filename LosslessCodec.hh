#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
#include "Golomb.hh"
#include "BitStream.hh"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;

class LosslessCodec{
    public:
        void encode(string path);
        void decode();
    private:
        Mat yuv;
        void YUV420(Mat img, Mat* yuv_channels);
        Mat predictor(Mat img);
        int calculate_m(Mat matrix);
        double calculate_entropy(Mat y);

};

void LosslessCodec::YUV420(Mat img, Mat* yuv_channels){
    cvtColor(img, img, COLOR_RGB2YUV);
    // cvtColor(img, img, COLOR_RGB2YUV_I420);
    split(img, yuv_channels);

    Mat tmp_u (img.size().height/2, img.size().width/2, CV_8UC1);
    Mat tmp_v (img.size().height/2, img.size().width/2, CV_8UC1);

    int u_i = 0, v_i = 0;
    int u_j = 0, v_j = 0;
    for (int i = 0; i < img.size().height; i+=2) {
        for (int j = 0; j < img.size().width; j+=2) {
            tmp_u.at<uchar>(u_i, u_j++) = yuv_channels[1].at<uchar>(i,j);
            tmp_v.at<uchar>(v_i, v_j++) = yuv_channels[2].at<uchar>(i,j);
        }
        u_i++; v_i++;
        u_j = v_j = 0;
    }

    yuv_channels[1] = tmp_u;
    yuv_channels[2] = tmp_v;
}

Mat LosslessCodec::predictor(Mat img){
    Mat error (img.size().height, img.size().width, CV_8UC1);
    int a=0,b=0,c=0,x;
    for (int i=0; i < img.size().height ; i++){
        for( int j=0 ; j< img.size().width ; j++){
            if(i==0 and j==0){
                a = 0;
                b = 0;
                c = 0;
            }
            else if (i==0 and j!=0){
                b = 0;
                c = 0;
                a =(int) img.at<uchar>(i,j-1);
            }
            else if(i!=0 and j==0){
                a = 0;
                c = 0;
                b =(int) img.at<uchar>(i-1,j);
            }
            else{
                a =(int) img.at<uchar>(i,j-1);
                b =(int) img.at<uchar>(i-1,j);
                c =(int) img.at<uchar>(i-1,j-1);
            }
            
            if(c >= max(a,b)){
                x = min(a,b);
            }
            else if (c <= min(a,b)){
                x = max(a,b);
            }
            else{
                x = a+b-c;
            }

            error.at<uchar>(i,j) = img.at<uchar>(i,j) - (uchar) x ;
            
        }
    }

    return error;
}



void LosslessCodec::encode(string path){
    Mat img = imread(path);
    Mat channels[3];
    YUV420(img,channels);

    Mat error[3];
    for(int i =0 ; i<3 ; i++){
        error[i] = predictor(channels[i]);
    }

    Golomb g;
    BitStream bs = BitStream("","img.bin");
    int m, val;
    string bits;
    for(int k=0;k<3;k++){
        m=calculate_m(error[k]);
        cout << "M -> " << m << endl;


        for(int i=0;i<error[k].size().height;i++){
            for(int j=0; j<error[k].size().width;j++){
                val = error[k].at<uchar>(i,j);
                bits = g.encoder(val,m);
                bs.writeNBits(bits);
            }
        }
        
        cout << "size " << error[k].size().height * error[k].size().width << endl;
    }
    double media =0;
    media = (calculate_entropy(channels[0]) + calculate_entropy(channels[1])+ calculate_entropy(channels[2])) / 3;
    cout <<"MEDIA ENTROPIA -> "<< media << endl;
}

void LosslessCodec::decode(){

}

int LosslessCodec::calculate_m(Mat matrix){
    double media;
    int soma=0;

    for(int i=0;i<matrix.size().height;i++){
        for(int j=0; j<matrix.size().width;j++){
            soma = soma + (int) matrix.at<uchar>(i,j); 
        }
    }

    media = soma / (matrix.size().height * matrix.size().width);

    return ceil(-1/log2(media/(media+1)));
}

double LosslessCodec::calculate_entropy(Mat mat){
    int bins[256] = {0};
    int val;
    for(int i = 0; i < mat.size().height; i++) {
        for (int j = 0; j < mat.size().width; j++) {
            val = (int) mat.at<uchar>(i,j);           
            bins[val]++;
        }
    }

    double size = mat.size().width * mat.size().height;
    double entropy = 0;
    for(int i = 0; i < 256; i++) {
        if (bins[i] > 0) {
            entropy += (bins[i]/size) * (log(bins[i]/size));
        }
    }

    entropy = entropy *-1;

    return entropy;   
}

