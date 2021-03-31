#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include "pthread.h"
#include <bits/stdc++.h>
using namespace std;
using namespace cv;

//make all the required variables as global variables
Mat imgdiff;
Mat h;
Mat back_crop;
Mat frame;
Mat warp_frame;

Mat back_image;

int fm_count=1;
//change the no_threads and tfree size to change the no of threads used
int no_threads = 8;

bool tfree[8];

bool doing = true;
ofstream table("m3.txt");
//pthread_mutex_t* mutex1;

class data{
public:
    int QD=0;
    int DD=0;
    int ID;
    int OFFSET;
    //int YSTART;
};

int max_offset;

bool all_free(void){
    for (int i=0;i<no_threads;i++){
        if(!tfree[i]){
            return false;
        }
    }
    return true;
}

void* subtractor(void* dat);


int main(int argc,char* argv[]){
    time_t start, end;
    time(&start);
    VideoCapture cap("trafficvideo.mp4");
    if (!cap.isOpened()){
        cout<<"\nerror reading the video\n";
        return 0;
    }
    back_image = imread("empty.png");
    if ( back_image.empty() ) 
    { 
        cout << "Error loading the image" << endl;
        return -1; 
    }
    vector<Point2f> pts_src;
    pts_src.push_back(Point2f(925, 220));//left up
    pts_src.push_back(Point2f(225, 1004));//left bottom
    pts_src.push_back(Point2f(1552, 1004));//right bottom
    pts_src.push_back(Point2f(1295, 220));//right up

    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(0, 0));//left up
    pts_dst.push_back(Point2f(0, 800));//left bottom
    pts_dst.push_back(Point2f(400, 800));//right bottom
    pts_dst.push_back(Point2f(400, 0));//right up

    h = findHomography(pts_src, pts_dst);
    //warped image
    Mat warp_back;

    warpPerspective(back_image, warp_back, h, back_image.size());
    
    back_crop = warp_back(Rect(0, 0, 400, 800));
    
    table<< "time , queue density , dynamic density\n";

    cout << "running\n";
    data args[no_threads];

    max_offset = 800/no_threads;
    int last_offset = 800 - (no_threads-1)*max_offset;

    pthread_t tids[no_threads];
    for (int i = 0; i < no_threads; i++) {
        args[i].ID = i;
        
        if(i<no_threads-1){
            args[i].OFFSET = max_offset;
        }
        else{args[i].OFFSET = last_offset;} 

        tfree[i]=true;

        if(0!=pthread_create(&tids[i], NULL, subtractor, &args[i]))
        {cout<<"error creating thread"<<i+1<<'\n';}
    }


    int qd=0;int dd=0;

    while (true){
        if(fm_count%5!=0 || all_free()) {
        bool next = cap.read(frame);
        //frame2=frame;
            if (next==false){
                cout<<"end of video\n";
                doing=false;
                break;
            }
        if(fm_count%5==1){
            warpPerspective(frame, warp_frame, h, back_image.size());
        }
        if(fm_count%5==0){
            for (int j=0; j<no_threads;j++){
                qd += args[j].QD;
                dd += args[j].DD;
            }
            table << fm_count-4<< ',' <<qd/320000.0<<','<<dd/320000.0<<'\n';
            qd=0;dd=0;
        }
        fm_count++;
        }
    }
    
    for(int i =0; i<no_threads; i++){
        pthread_join(tids[i],NULL);
    }
    
    table.close();
    cout<<"done\n";
    time(&end);
    cout<<"Execution Time = "<<(double)(end-start)<<"sec\n";
    
}   

//common resources - fm_count, doing, warp_frame
//dividing the frame by the y coordinate
void* subtractor(void* dat){
    Mat f_crop;
    Mat fgmask;
    
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();
    int dd,qd;
    data *d = (data*) dat;
    //pthread_mutex_lock(mutex1);
    int ystart = (d-> ID)*max_offset;
    int offset = d->OFFSET;
    
    while (doing ){
        if (fm_count%5==2){
            tfree[d->ID]=false;
            f_crop = warp_frame(Rect(0, ystart, 400, offset));
            pBackSub->apply(f_crop, fgmask, .04);

            for(int y=0; y<offset; y++){
            for(int x=0; x<400; x++){

                Vec3b & fc = f_crop.at<Vec3b>(Point(x,y));
                Vec3b & bc = back_crop.at<Vec3b>(Point(x,y + ystart));
                if( abs(fc[2]-bc[2])> 60 || abs(fc[1]-bc[1]) > 60 || abs(fc[0]-bc[0]) > 60)
                {
                    qd++;
                }
                Vec3b & dc = fgmask.at<Vec3b>(Point(x,y));
                if (dc[0] > 0){
                    dd++;
                }
            }
            }
            d->QD=qd;
            d->DD=dd;
            qd=0; dd=0;
            tfree[d->ID]=true;
        }
       //pthread_mutex_unlock(mutex1);
    }
   pthread_exit(NULL);
}
