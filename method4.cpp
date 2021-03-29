#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include "pthread.h"
#include <string> 
using namespace std;
using namespace cv;

//make all the required variables as global variables
int no_threads = 7;

int QD[7];
int DD[7];
bool tfree[7];
Mat frames[7];

Mat imgdiff;
Mat h;
Mat back_crop;
Mat frame;
Mat frame1;Mat frame2;
Mat back_image;
/*Mat warp_frame1;
Mat warp_frame2;
Mat warp_frame3;
Mat warp_frame4;*/
int fm_count=1;
/*int qd1 =0;int qd2=0;int qd3=0;int qd4=0; //measure of queue density
int dd1 =0;int dd2=0;int dd3=0;int dd4=0;//measure of dynamic density
bool t1free = true; bool t2free = true; bool t3free=true; bool t4free=true;*/
bool doing = true;
ofstream table("m4.txt");
pthread_mutex_t* mutex1;

bool allow(){
    for (int i=0; i<no_threads; i++){
        
        if (tfree[i] ){return true;}
    }
    return false;

}

void* subtractor(void *id);


int main(int argc,char* argv[]){

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
    /*int rows = back_crop.rows;
    cols = back_crop.cols;
    cout<<rows<<','<<cols<<endl;*/
    
    table<< "time , queue density , dynamic density\n";
    string sr="shit\n";
    cout << "running\n";

    pthread_mutex_t mutex_var1;
    mutex1 = &mutex_var1;
    pthread_mutex_init(mutex1, NULL); 
    pthread_t threads[no_threads];

    //int IDS[no_threads];
    int IDs[no_threads];//so that each integer is a different object to pass to the threads

    for(int i=0; i<no_threads; i++){
        IDs[i]=i;
        tfree[i]=true;
        //cout<<i<<'\n';
        if(0!=pthread_create(&threads[i], NULL, subtractor, &IDs[i])){cout<<"error creating thread1\n";}
    }

    int fm_ass_counter=0;
    int table_ass_counter = 5*no_threads-1;
    int divider = 5*no_threads;
    int sub=5*no_threads-2;

    while (true){
        //cout<<"stuck\n";
        bool next = cap.read(frame);
            if (next==false){
                cout<<"end of video\n";
                doing=false;
                break;
            }
            if(fm_count % divider == fm_ass_counter + 1) {
                frames[fm_ass_counter/5] = frame;
                //cout<<"reading"<<fm_ass_counter/5<<'\n';
                fm_ass_counter +=5;
                
                if(fm_ass_counter == 5*no_threads){ fm_ass_counter=0;}
            }

            //else if(fm_count%10==6){frame2=frame;}
            //cout<<fm_count<<','<<table_ass_counter<<'\n';
            if(fm_count % divider == table_ass_counter){
                //cout<<"writing\n";
                if(table_ass_counter == divider-1){
                    table<< fm_count-sub <<','<<QD[0]/320000.0<<','<<DD[0]/320000.0<<'\n';
                    table_ass_counter = 4;
                    fm_count++;
                    //QD[0]=0;
                    //DD[0]=0;
                    continue;
                }
                table<<fm_count - sub<<',' << QD[(table_ass_counter+1)/5]/320000.0 << ','<<DD[(table_ass_counter+1)/5]/320000.0<<'\n';
                table_ass_counter+=5;
                //QD[(table_ass_counter+1)/5]=0;
                //DD[(table_ass_counter+1)/5]=0;
                //cout<<"stuck\n";
                //if(table_ass_counter==divider){table_ass_counter=0;}
            }
            fm_count++;
            //cout<<"done\n";

            /*if(fm_count%20==6){//19, do -18 for writing
            	table<<fm_count-5<<','<<qd1/320000.0<<','<<dd1/320000.0<<'\n';
            	qd1=0;dd1=0;
            }
            else if(fm_count%20==11 && fm_count>10){//4 
            	table<<fm_count-5<<','<<qd2/320000.0<<','<<dd2/320000.0<<'\n';
            	qd2=0;dd2=0;
            }
            else if(fm_count%20==16){//9
            	table<<fm_count-5<<','<<qd3/320000.0<<','<<dd3/320000.0<<'\n';
            	qd3=0;dd3=0;
            }
            else if(fm_count%20==1 &fm_count>20){//14
            	table<<fm_count-5<<','<<qd4/320000.0<<','<<dd4/320000.0<<'\n';
            	qd4=0;dd4=0;
            }*/

        
    }
    cout<<fm_count<<'\n';
    doing=false;
    for(int i =0; i<no_threads; i++){
        pthread_join(threads[i],NULL);
    }

    table.close();
    cout<<"done\n";
}   

//common resources - frame, fm_count, table, doing
void* subtractor(void *ID){
    int *Id = (int*) ID;
    int id = *Id;

    Mat f_crop;
    Mat fgmask;
    Mat warp_frame;
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();

    int qd,dd;
    //cout<<id<<'\n';
    //double para=0.04*no_threads;
    //switch((long)id)
    while (doing){
        if (fm_count % (5*no_threads) == (5*id+2)){
            //cout<<"Here"<<fm_count<<'\n';
            tfree[id]=false;
            //table<<fm_count-1;
            warpPerspective(frames[id], warp_frame, h, back_image.size());

            f_crop = warp_frame(Rect(0, 0, 400, 800));
            pBackSub->apply(f_crop, fgmask, 0.04);

            for(int y=0; y<800; y++){
            for(int x=0; x<400; x++){

                Vec3b & fc = f_crop.at<Vec3b>(Point(x,y));
                Vec3b & bc = back_crop.at<Vec3b>(Point(x,y));
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
            QD[id] = qd;
            DD[id] = dd;
            qd=0;
            dd=0;
            //pthread_mutex_lock(mutex1);
            //table << ',' << qd/320000.0 << ',' << dd/320000.0 << endl;
            //pthread_mutex_unlock(mutex1);
            //qd=0;dd=0;
            //cout<<"t1";
            tfree[id]=true;
        }
    }
   pthread_exit(NULL);
}
