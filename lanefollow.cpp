#include "vision.hpp"
using namespace dahun;
bool ctrl_c_pressed=false;
bool s_pressed=false;
void ctrlc(int){ctrl_c_pressed=true;}
int main(void) {
	Dxl mx;
	struct timeval start,end1;
	double time1;

	signal(SIGINT,ctrlc);

	
	string src = "nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=(int)640, height=(int)360, format=(string)NV12 ! \
    nvvidconv flip-method=0 ! video/x-raw, width=(int)640, height=(int)360, format=(string)BGRx ! videoconvert ! \
    video/x-raw, format=(string)BGR !appsink";
	

	if(!mx.open()){cout<<"dynamixel open error"<<endl; return -1;}


	VideoCapture cap(src,CAP_GSTREAMER);
	//VideoCapture cap("lanefollow_100rpm_cw.mp4");
	if (!cap.isOpened()){ cout << "Video error" << endl; return -1; }

	string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! h264parse ! \
			rtph264pay pt=96 ! udpsink host=192.168.0.11 port=8001 sync=false";
	VideoWriter writer(dst1, 0, (double)30, Size(WIDTH, HEIGTH), true);
	if(!writer.isOpened())  { cout << "Writer error" << endl; return -1;}

	string dst2 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! h264parse ! \
			rtph264pay pt=96 ! udpsink host=192.168.0.11 port=8002 sync=false";
	VideoWriter writer1(dst2, 0, (double)30, Size(WIDTH/2, HEIGTH/4), true);
	if(!writer.isOpened())  { cout << "Writer error" << endl; return -1;}

	string dst3 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! h264parse ! \
			rtph264pay pt=96 ! udpsink host=192.168.0.11 port=8003 sync=false";
	VideoWriter writer2(dst3, 0, (double)30, Size(WIDTH/2, HEIGTH/4), true);
	if(!writer.isOpened())  { cout << "Writer error" << endl; return -1;}

	if (!cap.isOpened()) {
		cerr << "Video open failed!" << endl;
		return 0;
	}

	Mat frame,Roi_L,Roi_R,frame1,Roi_L1,Roi_R1;
	Mat labels, stats, centroids;
	Point2d crnt_ptl,crnt_ptr; //roi 4/3 ~ roi 4/1
	Point2d prev_ptR(180,45),prev_ptL(90,45); //roi 4/3 ~ roi 4/1
	Point2d center_r(0,45),center_l(320,45); //roi 4/3 ~ roi 4/1
    
	
	int Rspeed = 0, Lspeed = 0;
	

	while (true) {
		gettimeofday(&start,NULL);
		if(mx.kbhit()){
			char c =mx.getch();
			if(c=='s')s_pressed=!s_pressed;
		}
		cap >> frame;
		writer<<frame; //원본영상 writer
        if (frame.empty())break;
		Image_Pretreatment(frame);
		Roi_L=frame(Rect(0,0,frame.cols/2,frame.rows));
		Roi_R=frame(Rect(frame.cols/2,0,frame.cols/2,frame.rows));
		Point2d cpt_L=find_object(labels, stats, centroids, Roi_L,prev_ptL, crnt_ptl);
		Point2d cpt_R=find_object(labels, stats, centroids, Roi_R,prev_ptR, crnt_ptr);
		int error_l=get_error(cpt_L,center_l);
		int error_R=get_error(cpt_R,center_r);
		int error_sum=error_l+error_R;
		set_speed(Lspeed, Rspeed, error_sum);
		if (ctrl_c_pressed) break;
		if(s_pressed) mx.setVelocity(Lspeed,Rspeed);
		else mx.setVelocity(0,0);
		//cvtColor(frame, frame, COLOR_GRAY2BGR);
		writer1<<Roi_L;
		writer2<<Roi_R;
		usleep(10*1000);
		gettimeofday(&end1,NULL);
		time1 = end1.tv_sec-start.tv_sec +(end1.tv_usec-start.tv_usec)/1000000.0;
		cout <<"error: "<<error_sum<<"     lvel: "<<Lspeed<<"    rvel:"<<Rspeed << "   time:" << time1 << endl;
		
	}
	destroyAllWindows();
	mx.close();
	return 0;
}