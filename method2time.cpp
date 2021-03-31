#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/core/utility.hpp>

using namespace std;
using namespace cv;

int main(int argc,char* argv[]){
    // Display image
    //Mat im_out = Mat(900,1300,0);
	float x;
	float y;
	ofstream table("time2.txt");
	table<< "x,y,runtime\n";
	
	//running for loop for each resolution
	for (x=150.0;x<=500.0;x=x+100.0){
	for (y=300.0;y<=1000.0;y=y+100.0){
	
	VideoCapture cap(argv[1]);
	if (!cap.isOpened()){
		cout<<"\nerror reading the video\n";
		return 0;
	}  

	Mat imgdiff;
	//Mat back_image = imread("empty.png", IMREAD_GRAYSCALE);
    Mat back_image = imread("empty.png");
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
    pts_dst.push_back(Point2f(0, y));//left bottom
    pts_dst.push_back(Point2f(x, y));//right bottom
    pts_dst.push_back(Point2f(x, 0));//right up

	Mat h = findHomography(pts_src, pts_dst);
    //warped image
    Mat warp_back;
    // Warp source image to destination based on homography
    warpPerspective(back_image, warp_back, h, back_image.size());
	TickMeter time;
	time.start();

	Mat back_crop = warp_back(Rect(0, 0, x, y));
	//imshow("bc", back_crop);
	int fm_count = 1;
	int rows = back_crop.rows;
	int cols = back_crop.cols;

	//cout<<rows<<endl;

	Mat f_crop;
	cout<<"executing\n";
	//new
	Ptr<BackgroundSubtractor> pBackSub;
	pBackSub = createBackgroundSubtractorMOG2();
	Mat fgmask;
	Mat frame;
	int qd = 0; //measure of queue density
	int dd = 0; //measure of dynamic density
	float error;
	float ms=0.0;

	while (true){
		bool next = cap.read(frame);
			if (next==false){
				cout<<"end of video\n";
				break;
			}
		if(fm_count%5 == 1 ){
			//cout<<"yes"<<endl;
			//warped image
    		Mat warp_frame;
    		warpPerspective(frame, warp_frame, h, back_image.size());
    		// Display image
			f_crop = warp_frame(Rect(0, 0, x, y));
			
			int total = (int) fm_count/5;
			
			pBackSub->apply(f_crop, fgmask, .04);
			imshow("fgmask", fgmask);
			if (fm_count > 5){
	    	for(int y=0; y<rows; y++)
			{
    	    for(int x=0; x<cols; x++)
        	{
	    	    Vec3b & fc = f_crop.at<Vec3b>(Point(x,y));
    		    Vec3b & bc = back_crop.at<Vec3b>(Point(x,y));
    	    	/*int c1,c2,c0;
    		    c0 = abs(fc[0]-bc[0]);
	    	    c1 = abs(fc[1] - bc[1]);
    		    c2 = abs(fc[2]-bc[2]);*/
        		if( abs(fc[2]-bc[2])> 60 || abs(fc[1]-bc[1]) > 60 || abs(fc[0]-bc[0]) > 60)
        		{
	            	/*fc[0] = 230;
    	        	fc[1] = 230;
        	    	fc[2] = 230;*/
        	    	qd++;
    	    	}
        		Vec3b & dc = fgmask.at<Vec3b>(Point(x,y));
        		if (dc[0] > 0){
        			dd++;
        		}
				
	    	}
    		}
			}
			//imshow("cropped", f_crop);
        	if (waitKey(10) == 27){
				cout << "Esc key is pressed by user. Stoppig the video" << endl;
				table.close();
				return 0;
  			}
  		}
		  else if(fm_count%5 == 3){
			//cout << fm_count -2 << ',' << qd/(x*y) << ',' << dd/(x*y) << endl;
  			//error =pow((qd/(x*y) -dd/(x*y)),2); 
			//ms = ms + error;
			qd=0; dd=0;
  		}
  		fm_count++;

	}
	//cout<<ms<<endl;
	time.stop();
	table << x << ','<< y <<',' << time.getTimeSec() << endl;
	cout << "Total time: "<< time.getTimeSec() << endl;
	cout<<"done\n";
	x+=50;
	}
	}
	table.close();
}	
