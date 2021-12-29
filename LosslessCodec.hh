#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
#include <vector>
#include <fstream>
#include "Golomb.hh"
#include "BitStream.hh"
#include "GeneralFunctions.hh"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;
vector<int> codes_length;

class LosslessCodec{
    public:
        int encode(string path);
        void decode(string path);
    private:
        Mat yuv;
        void YUV420(Mat img, Mat* yuv_channels);
        Mat predictorEncode(Mat img);
        Mat predictorDecode(Mat img);
        int calculate_m(Mat matrix);
        double calculate_entropy(Mat y);

};

void LosslessCodec::YUV420(Mat img, Mat* yuv_channels){
    cvtColor(img, img, COLOR_RGB2YUV);
    // cvtColor(img, img, COLOR_RGB2YUV_I420);
    split(img, yuv_channels);
    #ifndef _NOSHOW
        imshow("img", yuv_channels[0]);
    #endif
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

Mat LosslessCodec::predictorEncode(Mat img){
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

            error.at<uchar>(i,j) = (int) img.at<uchar>(i,j) - x;
            
        }
    }

    return error;
}

Mat LosslessCodec::predictorDecode(Mat img){
    Mat original (img.size().height,img.size().width, CV_8UC1);
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
                a =(int) original.at<uchar>(i,j-1);
            }
            else if(i!=0 and j==0){
                a = 0;
                c = 0;
                b =(int) original.at<uchar>(i-1,j);
            }
            else{
                a =(int) original.at<uchar>(i,j-1);
                b =(int) original.at<uchar>(i-1,j);
                c =(int) original.at<uchar>(i-1,j-1);
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
            
            original.at<uchar>(i,j) = (int) img.at<uchar>(i,j) + x ;
            
        }
    }

    return original;
}


GeneralFunctions gf;
int LosslessCodec::encode(string path){
    
    Mat img = imread(path);
    Mat channels[3];
    YUV420(img,channels);

    Mat error[3];
    for(int i =0 ; i<3 ; i++){
        error[i] = predictorEncode(channels[i]);
    }

    Golomb g;
    BitStream bs = BitStream("","img.bin");
    int m[3], val;
    string bits;
    for( int k =0;k<3;k++){
        m[k]=calculate_m(error[k]);
        bits= g.decToBinary(m[k]);
        bs.writeNBits(bits);
        codes_length.push_back(bits.length());
        bits = g.decToBinary((int)error[k].size().height);
        bs.writeNBits(bits);
        codes_length.push_back(bits.length());
        bits = g.decToBinary((int)error[k].size().width);
        codes_length.push_back(bits.length());
        bs.writeNBits(bits);
    }

    for(int k=0;k<3;k++){
       

        for(int i=0;i<error[k].size().height;i++){
            for(int j=0; j<error[k].size().width;j++){
                val = error[k].at<uchar>(i,j);
                bits = g.encoder(val,m[k]);
                codes_length.push_back(bits.length());
                bs.writeNBits(bits);
            }
        }

    }

    bs.close();
    double media =0;
    media = (calculate_entropy(channels[0]) + calculate_entropy(channels[1])+ calculate_entropy(channels[2])) / 3;
    cout <<"MEDIA ENTROPIA -> "<< media << endl;
    return ceil(media);
}

void LosslessCodec::decode(String path){
    BitStream b (path , "");
    string code;
    Golomb g;
    short nINt;
    int m[3], altura[3], largura[3];
    int i=0;
    int k;

    for(k=0 ; k< 3; k++){
        code = b.readNBits(codes_length.at(i));
        m[k] = stoi(code,0,2);
        code = b.readNBits(codes_length.at(i+1));
        altura[k] = stoi(code,0,2);
        code = b.readNBits(codes_length.at(i+2));
        largura[k] = stoi(code,0,2);
        i+=3;
    }
    
    k=9;
    Mat y(altura[0], largura[0], CV_8UC1);
    Mat u(altura[1], largura[1], CV_8UC1);
    Mat v(altura[2], largura[2], CV_8UC1);


    for(int j = 0 ; j < altura[0] ; j++){
        for (int x =0 ; x < largura[0]; x++){
            code = b.readNBits(codes_length.at(k));
            nINt = g.decoder(code,m[0]);
            y.at<uchar>(j,x) = (uchar) nINt;
            k++;
        }
    }

    for(int j = 0 ; j < altura[1] ; j++){
        for (int x =0 ; x < largura[1]; x++){
            code = b.readNBits(codes_length.at(k));
            nINt = g.decoder(code,m[1]);
            u.at<uchar>(j,x) = (uchar) nINt;
            k++;
        }
    }

    for(int j = 0 ; j < altura[2] ; j++){
        for (int x =0 ; x < largura[2]; x++){
            code = b.readNBits(codes_length.at(k));
            nINt = g.decoder(code,m[2]);
            v.at<uchar>(j,x) = (uchar) nINt;
            k++;
        }
    }


    Mat original[3];
    original[0] = predictorDecode(y);
    original[1] = predictorDecode(u);
    original[2] = predictorDecode(v);

    Mat uup(altura[0], largura[0], CV_8UC1);
    Mat vup(altura[0], largura[0], CV_8UC1);

    int u_i = 0, v_i = 0;
    int u_j = 0, v_j = 0;
    for (int i = 0; i < altura[0]; i+=2) {
        for (int j = 0; j < largura[0]; j+=2) {
            uup.at<uchar>(i,j) = original[1].at<uchar>(u_i,u_j+1);
            uup.at<uchar>(i+1,j) = original[1].at<uchar>(u_i,u_j+1);
            uup.at<uchar>(i,j+1) = original[1].at<uchar>(u_i,u_j+1);
            uup.at<uchar>(i+1,j+1) = original[1].at<uchar>(u_i,u_j+1);
            vup.at<uchar>(i,j) = original[2].at<uchar>(v_i,v_j+1);
            vup.at<uchar>(i+1,j) = original[2].at<uchar>(v_i,v_j+1);
            vup.at<uchar>(i,j+1) = original[2].at<uchar>(v_i,v_j+1);
            vup.at<uchar>(i+1,j+1) = original[2].at<uchar>(v_i,v_j+1);
            u_j++;v_j++;
        }
        u_i++; v_i++;
        u_j = v_j = 0;
    }

    cout<< uup.size().height<< endl;

    Mat channels[3] = {original[0],uup,vup};
    Mat output;
    merge(channels,3,output);

    cvtColor(output,output, COLOR_YUV2RGB);
    #ifndef _NOSHOW
        imshow("output",output);
        imshow("u", uup);
        imshow("v",vup);
        waitKey(0);
    #endif


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

