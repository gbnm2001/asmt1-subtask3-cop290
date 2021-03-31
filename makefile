method2time:
	g++ method2time.cpp -o method2time -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method2time trafficvideo.mp4
method2utility:
	g++ method2utility.cpp -o method2utility -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method2utility trafficvideo.mp4
method1utility:
	g++ method1utility.cpp -o method1utility -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method1utility trafficvideo.mp4
method1time:
	g++ method1time.cpp -o task1 -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method1 trafficvideo.mp4
method2:
	g++ method2time.cpp -o method2time -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method2time trafficvideo.mp4 && g++ method2utility.cpp -o method2utility -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method2utility trafficvideo.mp4
method1:
	g++ method1time.cpp -o method1time -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method1time trafficvideo.mp4 && g++ method1utility.cpp -o method1utility -pthread -std=c++11 `pkg-config --cflags --libs opencv` && ./method1utility trafficvideo.mp4
