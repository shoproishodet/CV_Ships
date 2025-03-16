//======================================================================================
int blobi = -1, TCOL = 0;
bool cup = false;// calibration needs z=0 >:(((
bool CAP_THREAD_BUSY = false;
int CAP_FRAME_READ_IPCAM_LIM = 5, CAP_IP_FRAMES = 0;
//======================================================================================
struct ExcludeBlobs{
	int me, sel;
	vec4f *box;
	ExcludeBlobs(){ CON.write(0, "----init ExcludeBlobs");me = 0;sel = -1;box = NULL; };
	void add(float x, float y){
		sel = me;me++;
		RESIZE(box, vec4f, me);
		box[sel].x = x, box[sel].y = y;
		box[sel].z = x, box[sel].w = y;//default size
	};
	void del(int i){
		me--;
		box[i] = box[me-1];
	};
	void save(const char *name){
		FILE *in = fopen(name, "wt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
		fprintf(in, "me: %i\n", me);
		for(int i = 0;i < me;i++){
			fprintf(in, "box: %f %f %f %f\n", box[i].x, box[i].y, box[i].z, box[i].w);
		}
		fclose(in);
	};
	void load(const char *name){
		me=0;
		FILE *in = fopen(name, "rt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
		char sline[256];
		int nmv;fgets(sline, 256, in);sscanf(sline, "me: %i\n", &nmv);
		//resize(nmv);
		for(int i = 0;i < nmv;i++){ 
			add(0, 0);fgets(sline, 256, in);sscanf(sline, "box: %f %f %f %f\n", &box[i].x, &box[i].y, &box[i].z, &box[i].w); 
			if(fabs(box[i].x) + fabs(box[i].y) + fabs(box[i].z) + fabs(box[i].w) < 0.001f) {
				nmv--;
				if(nmv<=0)break;
				i--;
				me--;
			}
		};
		CON.write(2, "EXBOX: loaded %i", me);
		fclose(in);
	};
	void check(int i){
		if(i < 0)return;
		float t;
		if(box[i].x > box[i].z){ t = box[i].x, box[i].x = box[i].z, box[i].z = t; };
		if(box[i].y > box[i].w){ t = box[i].y, box[i].y = box[i].w, box[i].w = t; };
	};
	void edit(float x, float y){
		if(key[K_DEL] && !lkey[K_DEL]){del(sel);return;}
		if((mouse[0] && lmouse[0]) || (mouse[1] && lmouse[1])){} else{
			if(lmouse[0] || lmouse[1])check(sel);
			sel = -1;
			for(int i = 0;i < me;i++){
				if(x > box[i].x && x<box[i].z && y>box[i].y && y < box[i].w){ sel = i; };
			};
		};
		if(mouse[0]){ if(sel == -1 && !lmouse[0])add(x, y); box[sel].x = x, box[sel].y = y; };
		if(mouse[1]){ if(sel == -1 && !lmouse[1])add(x, y); box[sel].z = x, box[sel].w = y; };
	};
	bool inside(float x, float y){
		for(int i = 0;i < me;i++){
			if(x > box[i].x && x<box[i].z && y>box[i].y && y < box[i].w){ return true; };
		};
		return false;
	};
	void Draw(){
		glLineWidth(4);
		for(int i = 0;i < me;i++){
			if(sel == i)glColor3f(1, 0, 0);else glColor3f(0, 1, 0);
			glBegin(GL_LINE_LOOP);
			glVertex3f(box[i].x, 0, box[i].y);glVertex3f(box[i].x, 0, box[i].w);glVertex3f(box[i].z, 0, box[i].w);glVertex3f(box[i].z, 0, box[i].y);
			glEnd();
		};
	};
};
ExcludeBlobs EX;
//======================================================================================
struct CvCap{
	int dev, w, h, f, c;//f=fps
	float border;
	Dword tid, frame_num, lframe_num;//,lid;
	int format;
	VideoCapture *cap;
	SCamera *cam;
	char *name, *dev_adr;
	//CvVideoWriter *writer;
	Mat capture_frame, gray_frame, TEMPLATE, lightmap;
	Mat filter_frame, gaussian_frame, balance_frame, threshold_frame;
	//bool DoSURF;
	byte filter;
	Rect box;
	//vec3i col_min,col_max;
	int lasIMGnchanels;
	//calibrate
	byte calibration;//0-none,1-process,2-done
	Mat distCoeffs;
	Mat intrinsic;
	//vector<vector<Point2f>> imagePoints;
	//off,connect,on,update,new_frame
	int state;//-1=off,0=conn,1=on   //,3-up,4-blolbs
	//bool read_cam,Opened, READY,READ;
	bool foundBLOB, new_frame, lnew_frame, _FORCED;
	volatile int loopthread;//PVS
	// Mutex IMGMUT;
	int m_id;
	//------------blobs array
	int numblobs, memblob, buf_numblobs, buf_memblob;
	vec3f *blob_pos, *buf_blob_pos;
	std::mutex blob_mutex;
	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------
	CvCap(){ init(); };
	void init(){
		border = 1.0f;
		memblob = buf_memblob = 0;
		numblobs = buf_numblobs = 0;
		blob_pos = NULL;buf_blob_pos = NULL;
		foundBLOB = false;
		state = -1;
		_FORCED = false;
		name = NULL;
		dev_adr = NULL;
		m_id = -1;
		new_frame = lnew_frame = false;
		//DoSURF=false; =lid
		filter = 0;
		tid = 0;
		dev = -1, w = 0, h = 0, f = 0, c = 0;
		cap = NULL;// new cv::VideoCapture();//
		cam = NULL;
		lasIMGnchanels = 0;
		//col_min.set(7,106,0);
		//col_max.set(7,106,255);
		//calib
		calibration = 0;//none
		distCoeffs = Mat::zeros(8, 1, CV_64F);
		intrinsic = Mat::eye(3, 3, CV_64F);
		intrinsic.at<double>(0, 0) = 1.0;//cameraMatrix.at<double>(0,0) = 1.0;// s.flag & CV_CALIB_FIX_ASPECT_RATIO )
		intrinsic.at<double>(1, 1) = 1.0;
		//LoadCalib("CamModel.yml");
		//writer=NULL;
		//cam=CAMM.add(5.0f,0.0f);img->Load("img_cam.params");//?? load
		//open(device,adr);
		frame_num = lframe_num = 0;
	};
	//-------------------------------------------------------------------------------------------------
	void open(int device = 0, const char *adr = NULL){
		dev = device;
		//dev_adr=adr;
		dev_adr = new char[64];strcpy(dev_adr, adr);
		if(dev >= 0){
			// startThread();
		} else{ CON.write(2, "can't open %i", device); };
	};
	//-------------------------------------------------------------------------------------------------
	~CvCap(){
		CON.write(2, "cap[%s] destructor", name);
		if(loopthread > 0)loopthread = false;
		while(loopthread != -1){ Sleep(10); };
	};//if(dev>=0 && cap)cap->~VideoCapture();cap=NULL;};
	//-------------------------------------------------------------------------------------------------
	void clear_lightmap(){
		w = (int) cap->get(CV_CAP_PROP_FRAME_WIDTH);
		h = (int) cap->get(CV_CAP_PROP_FRAME_HEIGHT);
		if(!w || !h)return;
		lightmap = Mat::zeros(h, w, CV_8UC1);
	};
	//-------------------------------------------------------------------------------------------------
	void ReOpen(){
		state = 0;if(!loopthread)return;
		///!CON.write(0, "ReOpen [%s] ", name);
		// LOG.write(0,"ReOpen [%s] ",name);
		// if(cap!=NULL){delete [] cap;cap=NULL;};//crash1 nw  || CAP_THREAD_BUSY
		 //while(CAP_THREAD_BUSY)Sleep(100);
		CAP_THREAD_BUSY = true;
		//if(dev_adr==NULL){
		 //if(cap==NULL)cap = new cv::VideoCapture(dev);else cap->open(dev);
		//}else{
		if(cap == NULL)cap = new cv::VideoCapture(dev_adr);else{ cap->open(dev_adr); };//crush 25.12 at esc  16.01.14 at esc ,fix    if(!loopthread)return;  cap->release();
	  // };
	   //if(cap==NULL){CON.write(0,"CV::no cap [%s]",name);return;};nw
	   //int mode=2048;
	   //while(mode>640 && cap->set(CV_CAP_PROP_FRAME_WIDTH,(double)mode)){mode-=2;};
	   //cap->set(CV_CAP_PROP_FRAME_HEIGHT,(double)768);
		w = (int) cap->get(CV_CAP_PROP_FRAME_WIDTH);
		h = (int) cap->get(CV_CAP_PROP_FRAME_HEIGHT);
		f = (int) cap->get(CV_CAP_PROP_FPS); // Opened=true;  LOG.write(1,"cam %i sleep 4sec",m_id);
		clear_lightmap();
		if(cap->isOpened()){ state = 1; } else{ 
            state = -1;
            ///!CON.write(1, "cam %i sleep 4sec", m_id);
            Sleep(4000); 
        };// read_cam=true;     CON.write(0,"CV::[%s] [%ix%i] at %i fps",name,w,h,f);
		CAP_THREAD_BUSY = false;
		//LoadCalib("CamModel.yml_ms");
	};
	//-------------------------------------------------------------------------------------------------
	static DWORD WINAPI StaticThreadStart(void *Param){
		CvCap *This = (CvCap*) Param;
		//CON.write(0,"CvCap [%s] static start thread",This->name);
		return This->ThreadStart();
	};
	DWORD ThreadStart(void){
		loopthread = true;
		ReOpen();
		float currentTime = 0, accumulator = 0;
		int TFPS = 0, MFPS = 0;
		//cstring ptx;
		bool initialized_gl_first_frame = false;
		float found_blob_timer = 0;
		while(loopthread){
			const float newTime = time();
			float deltaTime = newTime - currentTime;
			currentTime = newTime;
			accumulator += deltaTime;
			if(accumulator > 0.5)lnew_frame = false;
			//if(!cap || !cap->isOpened() || !cap->read(capture_frame)){READY=false;}else{READY=true;}; //Sleep(1000);continue;
			state = (cap && cap->isOpened() && cap->read(capture_frame));//state=0;//connect
			if(initialized_gl_first_frame && !_FORCED){// || TFPS>30){//1sec CAP_IP_FRAMES>CAP_FRAME_READ_IPCAM_LIM || TFPS>1){//skip frame     accumulator<1.5 && 
			 //Sleep(50);//if(TFPS>30)TFPS=30;//?
			 //if(!cap || !cap->isOpened() || !cap->read(capture_frame)){READY=false;}else{READY=true;}; //Sleep(1000);continue;
				if(accumulator < 5)continue;
			};
			//if(!READY){cam->lastTime=-1;read_cam=false;new_frame=false;Opened=false;ReOpen();continue;};//chush 2 times   16.01.14 (lightoff)
			if(!state){ lnew_frame = false;cam->lastTime = -1;new_frame = false;ReOpen();continue; };
			initialized_gl_first_frame = true;
			CAP_IP_FRAMES++;
			TFPS++;
			if(accumulator > 1){ accumulator = 0;MFPS = TFPS;TFPS = 0; };
			cam->lastTime = MFPS; //REFRESHES PER SECOND
			//ptx.print("WxHxF %ix%ix[%2i/%i]",w,h,TFPS,MFPS);
			f = MFPS;//fps
			//putText(capture_frame, ptx.text, cvPoint(0,10), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(200,200,250), 0.1, CV_AA);
			if(new_frame)continue;
			if(cam->con_adr[0] != 0 && cam->crop_border < 1.0f){
				float fw=capture_frame.size().width,fh=capture_frame.size().height;
				//w = capture_frame.cols;
				//h = capture_frame.rows;
				border = cam->crop_border;
				Mat crop = capture_frame(Rect((fw/2)*border, (fh/2)*border, fw-(fw)*border, fh-(fh)*border)).clone();
				cv::resize(crop, capture_frame,cv::Size(w,h));
				//crop.copyTo(capture_frame);
			}
			flip(capture_frame, capture_frame, 0);
			//cv::Rect2d rect = 
			cvtColor(capture_frame, gray_frame, CV_BGR2GRAY);
			gray_frame.copyTo(filter_frame);
			blob_mutex.lock();
			find_blobs();
			blob_mutex.unlock();
			if(foundBLOB)found_blob_timer = 2;else found_blob_timer -= deltaTime;
			_FORCED = (found_blob_timer > 0);
			//if((m_id+10)==100)CON.write(foundBLOB,"new_frame cam[%i] foundBLOB=%i",m_id+10,foundBLOB);//if(foundBLOB)
			new_frame = true; lnew_frame = true;
			Sleep(1);
			//read_cam=true;
		   //CON.write(1,"IMGMUT.lock()");
		   //IMGMUT.lock();
		   //capture_frame.copyTo(filter_frame);
		   //OnNewFrame();
		   //IMGMUT.unlock();  READY=
		};//while
		_FORCED = false;
		new_frame = false;
		state = -1;
		//read_cam=false;
		//Opened=false;
		MFPS = TFPS = 0;
		///!CON.write(0, "cap [%s] exit loop thread", name);
		loopthread = -1;
		return 1;
	};
	void startThread(){
		DWORD ThreadID;//COMPortThread=
		CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
	};// */
	//-------------------------------------------------------------------------------------------------
	void add_blob_pt(const vec3f &pt){
		if(!ShipDetector || !ShipDetector->Down)return;//if(!READY)return;
		//if(EX.me)CON.write(2,"BLOB: x=%f y=%f box(%f %f %f %f)",pt.x,pt.z, EX.box[0].x,EX.box[0].y,EX.box[0].z,EX.box[0].w);
		vec3f bp = intplane(cam->pos, cam->VRay(pt.x, pt.z), vec3f(0, cup ? 0 : 1, cup ? 1 : 0), vec3f(0, 0, 0));
		if(EX.inside(bp.x, bp.z)){ return; };
		if(memblob < numblobs + 1){
			memblob = 2 + numblobs;
			RESIZE(blob_pos, vec3f, memblob);
		};
		foundBLOB = true;//!!!!
		blob_pos[numblobs] = pt;
		numblobs++;
	};
	void BUFF_BLOBS(){
		buf_numblobs = numblobs;
		if(buf_memblob < numblobs){
			buf_memblob = numblobs;
			RESIZE(buf_blob_pos, vec3f, buf_memblob);//trhead not safe
		};
		for(int i = 0;i < numblobs;i++){ buf_blob_pos[i] = blob_pos[i]; };
	};
	//-------------------------------------------------------------------------------------------------
	//   union{int rows;int height;};   union{int cols;int width;};
	int cmul(int c){ return (c == 2 ? 3 : (c == 1 ? 1 : 1)); };//PVS same rasult
	int cmul2(int c){ return (c == 2 ? 1 : (c == 1 ? 2 : 1)); };
	vec3f get_blob_pos(int num){ //   || buf_resizing
		if(num >= buf_numblobs)return vec3f(0.0);
		return intplane(cam->pos, cam->VRay(buf_blob_pos[num].x, buf_blob_pos[num].z), vec3f(0, cup ? 0 : 1, cup ? 1 : 0), vec3f(0, 0, 0));
	};
	//--------------------------------------------------------------------------------------------------
	void find_blobs(){
		numblobs = 0;foundBLOB = false;
		//cv::blur(gray_frame, gray_frame, { 11, 11 });
		threshold(gray_frame, gray_frame, editT->val, 255, CV_THRESH_BINARY);
		vector<vector<Point> > contours;// cv::OutputArrayOfArrays
		contours.clear();//fix1[CE]? //PVS
		findContours(gray_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		if(contours.size() < 1)return;// fix2[CE]?
		if(contours.size() > 200)return;// sanity check
		//char *errcd=0,*desc=0,*fn=0;
		//int ln=0;
		//cvGetErrInfo(&errcd,&desc,&fn,&ln);
		//CON.write(0,"CV::%s, %s, %s, %i",errcd,desc,fn,ln);
		//CON.flush();
		//return;
		float ax = (float) gray_frame.cols / (float) WndW, ay = (float) gray_frame.rows / (float) WndH, p1A = 0, p2A = 0;
		for(int i = 0; i < contours.size(); i++){
			Rect rect = boundingRect(contours[i]);
			float area = rect.width*rect.height;
			if(area > 0 && area < (25*25)){
				float x = rect.x + rect.width*0.5, y = rect.y + rect.height*0.5;
				add_blob_pt(vec3f(x, 0, y));
				//drawContours( filter_frame, contours, i, Scalar(255,0,0), 1, 8, hierarchy, 0, Point() );
				//drawContours(filter_frame, contours, -1, Scalar::all(255), CV_FILLED);
				if(cam_lit->Down){
					//drawContours(lightmap, contours, -1, Scalar::all(50), CV_FILLED);
					cv::circle(lightmap,cv::Point(x, y),2,Scalar::all(255),1);
				} else{
					//drawContours(filter_frame, contours, -1, Scalar::all(255), 1);//CV_FILLED);
					cv::circle(filter_frame,cv::Point(x, y),2,Scalar::all(255),1);
				};
			};// area sane
		};/**/
		BUFF_BLOBS();
	};
	//-------------------------------------------------------------------------------------------------
	void GPU(){
		if(state < 1 || !new_frame)return;
		bool reinit = (tid == 0);
		if(lasIMGnchanels != filter_frame.channels()){
			reinit = 1;
			lasIMGnchanels = filter_frame.channels();
			switch(filter_frame.channels()){
				case 1:format = GL_LUMINANCE;break;
				case 3:format = GL_BGR;break;
				default:CON.write(0, "unknown format %i", filter_frame.channels());
			};
		} else{ if(w != filter_frame.cols || h != filter_frame.rows)reinit = 1; };
		if(reinit){
			glEnable(GL_TEXTURE_2D);
			if(tid)glDeleteTextures(1, &tid);
			glGenTextures(1, &tid);glBindTexture(GL_TEXTURE_2D, tid);
			w = filter_frame.cols;
			h = filter_frame.rows;
			//CON.write(0,"CvCapture::create gl tid=%i [%ix%i] %ichannels ",tid,w,h,filter_frame.channels());
			//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, format, GL_UNSIGNED_BYTE, cam_lit->Down ? lightmap.data : filter_frame.data);
		} else{
			//CON.write(0,"copy tex");
			//IMGMUT.lock();
			glBindTexture(GL_TEXTURE_2D, tid);
			//if(!Undistort_points->Down){
			 //undistort(filter_frame, capture_frame, intrinsic, distCoeffs);
			 //glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,format,GL_UNSIGNED_BYTE,capture_frame.data);
			//}else{
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, cam_lit->Down ? lightmap.data : filter_frame.data);//crash at run treshold 14.05.2014
		   //IMGMUT.unlock();
		   //};
		};
		new_frame = false;
	};
	//-------------------------------------------------------------------------------------------------
	void LoadCamParams(){
		if(!cam){ CON.write(2, "CV::LOAD CAM params::can't load - no cam");return; };
		distCoeffs.at<double>(0) = cam->dis[0];
		distCoeffs.at<double>(1) = cam->dis[1];
		distCoeffs.at<double>(2) = cam->dis[2];
		distCoeffs.at<double>(3) = cam->dis[3];
		distCoeffs.at<double>(4) = cam->dis[4];
		intrinsic.at<double>(0, 0) = cam->ox;
		intrinsic.at<double>(1, 1) = cam->oy;
		intrinsic.at<double>(0, 2) = cam->cx;
		intrinsic.at<double>(1, 2) = cam->cy;
	};
	void SetCVGL(Mat rvec, Mat tvec){
		Mat rotM = Mat(3, 3, CV_64FC1);
		Rodrigues(rvec, rotM);
		mat4 RRM(1);
		// RRM[0]=rotM.at<double>(0,0)*(b[0]->Down?-1:1);RRM[4]=rotM.at<double>(0,1)*(b[4]->Down?-1:1);RRM[ 8]=rotM.at<double>(0,2)*(b[ 8]->Down?-1:1);RRM[12]=tvec.at<double>(0)*(b[12]->Down?-1:1);
		// RRM[1]=rotM.at<double>(1,0)*(b[1]->Down?-1:1);RRM[5]=rotM.at<double>(1,1)*(b[5]->Down?-1:1);RRM[ 9]=rotM.at<double>(1,2)*(b[ 9]->Down?-1:1);RRM[13]=tvec.at<double>(1)*(b[13]->Down?-1:1);
		// RRM[2]=rotM.at<double>(2,0)*(b[2]->Down?-1:1);RRM[6]=rotM.at<double>(2,1)*(b[6]->Down?-1:1);RRM[10]=rotM.at<double>(2,2)*(b[10]->Down?-1:1);RRM[14]=tvec.at<double>(2)*(b[14]->Down?-1:1);
		RRM[0] = rotM.at<double>(0, 0);RRM[4] = rotM.at<double>(0, 1);RRM[8] = rotM.at<double>(0, 2);RRM[12] = tvec.at<double>(0);
		RRM[1] = rotM.at<double>(1, 0);RRM[5] = rotM.at<double>(1, 1);RRM[9] = rotM.at<double>(1, 2);RRM[13] = tvec.at<double>(1);
		RRM[2] = rotM.at<double>(2, 0);RRM[6] = rotM.at<double>(2, 1);RRM[10] = rotM.at<double>(2, 2);RRM[14] = tvec.at<double>(2);
		RRM.inverse();
		// RRM[0]*=(b[0]->Down?-1:1);RRM[4]*=(b[4]->Down?-1:1);RRM[ 8]*=(b[ 8]->Down?-1:1);RRM[12]*=(b[12]->Down?-1:1);
		// RRM[1]*=(b[1]->Down?-1:1);RRM[5]*=(b[5]->Down?-1:1);RRM[ 9]*=(b[ 9]->Down?-1:1);RRM[13]*=(b[13]->Down?-1:1);
		// RRM[2]*=(b[2]->Down?-1:1);RRM[6]*=(b[6]->Down?-1:1);RRM[10]*=(b[10]->Down?-1:1);RRM[14]*=(b[14]->Down?-1:1);
		cam->ang = RRM;//hist
		//cam->pos=RRM.T;//+cam->off; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! POS
		//cam->LookAt(intplane(cam->pos,cam->ang.dir(),vec3f(0,b[20]->Down?0:1,b[20]->Down?1:0),vec3f(0,0,0)),true);
		cam->LookAt(intplane(cam->pos, cam->ang.dir(), vec3f(0, 1, 0), vec3f(0, 0, 0)), true);
		cam->dis[0] = distCoeffs.at<double>(0);cam->dis[1] = distCoeffs.at<double>(1);cam->dis[2] = distCoeffs.at<double>(2);cam->dis[3] = distCoeffs.at<double>(3);cam->dis[4] = distCoeffs.at<double>(4);
		cam->setCV(filter_frame.cols, filter_frame.rows, intrinsic.at<double>(0, 0), intrinsic.at<double>(1, 1), intrinsic.at<double>(0, 2), intrinsic.at<double>(1, 2));
	};
};//end CvCap
//======================================================================================
struct CapManager{
	Dword mc, res;
	CvCap **cap;
	//int *ready;
	int sel, closest, realoc;
	Gizmo G;
	CapManager(){ init(); };
	~CapManager(){
		CON.write(0, "capm destructor");
		//for(int i=0;i<mc;i++){cap[i]->loopthread=false;Sleep(100);};//TMP
		for(int i = 0;i < mc;i++){ delete cap[i];cap[i] = NULL; };
	};
	void init(){ CON.write(0, "CAPM init");mc = 0;cap = NULL;sel = -1;closest = -1;G.init();realoc = false;res = 0; };//ready=NULL;}; 
	void reserve(int num){
		res = num;
		RESIZE(cap, CvCap*, num);
		for(int i = 0;i < num;i++){
			cap[i] = NULL;
		};
	};
	int add(const char *name, const vec3f &npos, const vec3f &ntarget){
		realoc = true;
		int id = mc;
		int nmc = mc + 1;
		if(res < nmc){ CON.write(2, "CAPM::RESERVED LESS (%i)->(%i)", res, nmc);RESIZE(cap, CvCap*, nmc); };
		//RESIZE(cap,CvCap*,nmc);//not safe;?  --- reserve
		//RESIZE(ready,int,mc);ready[id]=false;
		cap[id] = new CvCap();
		cap[id]->name = new char[64];strcpy(cap[id]->name, name);//cap[id]->name=name;
		cap[id]->cam = CAMM.add(npos, ntarget);
		//cap[id]->cam->name=name;
		cap[id]->cam->name = new char[64];strcpy(cap[id]->cam->name, name);//cap[id]->name=name;
		cap[id]->cam->Load(cap[id]->cam->name);cap[id]->LoadCamParams();//!
		cap[id]->cam->pos = npos;
		cap[id]->cam->LookAt(ntarget, true);cap[id]->cam->Rlim();
		cap[id]->m_id = id;
		//for(int i=0;i<mc;i++){
		 //CON.write(2,"cap[%i](%i)->cam(%i)->pos(%f %f %f) npos(%f %f %f)",i,cap[i]->m_id,cap[i]->cam->id,cap[i]->cam->pos.x,cap[i]->cam->pos.y,cap[i]->cam->pos.z,npos.x,npos.y,npos.z);
		//};
		mc = nmc;
		realoc = false;
		return id;
	};
	int get_blob_max(){
		if(realoc)return 0;
		int rez = 0;
		for(int i = 0;i < mc;i++){
			if(!cap[i] || cap[i]->state < 1 || cap[i]->buf_numblobs < 1 || !cap[i]->foundBLOB)continue;//    !cap[i]->read_cam
			rez += cap[i]->buf_numblobs;
		};
		return rez;
	};
	void get_blobs(vec3f *&blob_out, int max_b){//,int &bnum){
		int bnum = 0;
		for(int i = 0;i < mc;i++){
			if(!cap[i] || cap[i]->state < 1)continue;
			//if(!cap[i]->read_cam)continue;
			if(cap[i]->buf_numblobs < 1)continue;
			if(!cap[i]->foundBLOB)continue;
			SCamera *img = cap[i]->cam;
			img->rad = distplane(img->pos, img->ang.dir(), vec3f(0, cup ? 0 : 1, cup ? 1 : 0), vec3f(0, 0, 0));
			img->Update();
			//RESIZE(blob_out,vec3f,bnum+cap[i]->numblobs);
		  // vec3f get_blob_pos(int num){
			cap[i]->blob_mutex.lock();
			for(int j = 0;j < cap[i]->buf_numblobs;j++){//if(bnum+1>=8*4)break;   CON.write(1,"blob_out::bnum==max_b");
				if(bnum == max_b){ break; };
				blob_out[bnum++] = cap[i]->get_blob_pos(j);
				//intplane(img->pos,img->VRay(cap[i]->buf_blob_pos[j].x,cap[i]->buf_blob_pos[j].z),vec3f(0,cup?0:1,cup?1:0),vec3f(0,0,0));
			};//num blobs
			cap[i]->blob_mutex.unlock();
			//bnum+=cap[i]->numblobs;
		};//num cams
	};
	void activate_cams(const vec3f &pos){
		int idx[10], idx_num = 0;
		//------user activate cam in rad pos
		idx_num = dxf_cam_pos.circles_in_rad(pos, 100, idx);
		for(int i = 0;i < idx_num;i++){
			if(idx[i] >= 0 && idx[i] < mc && cap[idx[i]] && cap[idx[i]]->state == 1)cap[idx[i]]->_FORCED = true;
		};
		//----- ship activator
		for(int i = 0;i < SM.ms;i++){
			if(SM.ship[i].state == SS_RECYCLE)continue;
			idx_num = dxf_cam_pos.circles_in_rad(SM.ship[i].center, 100, idx);
			for(int j = 0;j < idx_num;j++){
				if(idx[j] >= 0 && idx[j] < mc && cap[idx[j]] && cap[idx[j]]->state == 1)cap[idx[j]]->_FORCED = true;
			};
		};
	};
	void draw(const ray &mdir){
		if(realoc)return;
		vec3f whl;
		vec3f IP = intplane(pl->msdir.p, pl->msdir.n, vec3f(0, 1, 0), vec3f(0, 0, 0));
		if(G.grappled == false)closest = dxf_cam_pos.closest_circle(IP);
		if(closest >= mc)closest = -1;
		activate_cams(IP);
		//--------------------------------------- draw cam shifts cad-prog
		if(DrawCams->Down){
			glDisable(GL_TEXTURE_2D);glDisable(GL_LIGHTING);
			glLineWidth(4);glColor3f(0, 1, 1);//cyan
			glBegin(GL_LINES);
			for(int i = 0;i < mc;i++){
				if(!cap[i] || !cap[i]->cam)continue; 
				int idx = cap[i]->m_id;
				int col = dxf_cam_pos.v[dxf_cam_pos.l[idx].x].x, row = dxf_cam_pos.v[dxf_cam_pos.l[idx].x].z;
				glVertex3f(col, 240, row);
				glVertex3fv(cap[i]->cam->pos);
			};
			glEnd();
		};
		//---------------------------------------render cam images
		int used_cams = 0, forced_cams = 0;
		for(int j = 0;j < 3;j++){
			for(int i = 0;i < mc;i++){
				if(realoc || !cap[i])continue;//!cap[i]->read_cam || !cap[i]->cam)continue;//crush 104 on ~50 at load cams
				if(j == 0 && cap[i]->_FORCED)continue;//do not draw forced at first loop
				if(j == 1 && !cap[i]->_FORCED)continue;//at second loop draw only forced

				if(j == 2 && i != closest)continue;//at thrid loop do not draw NOTselected
				if(j != 2 && i == closest)continue;//only at last loop draw only selected

				if(cap[i]->state == 1)used_cams++;
				if(cap[i]->_FORCED)forced_cams++;

				if(!DrawCams->Down){ cap[i]->new_frame = false;continue; };
				cap[i]->GPU();
				SCamera *img = cap[i]->cam;if(!img)continue;
				img->rad = distplane(img->pos, img->ang.dir(), vec3f(0, cup ? 0 : 1, cup ? 1 : 0), vec3f(0, 0, 0));
				img->Update();
				if(CamGizmo->Down && i == closest){
					mat4 RM(1);//        gizmo edit
					RM = img->ang;RM.T = img->pos;
					G.edit(RM, mdir);
					//img->ang=RM;
					img->pos = RM.T;
					G.Draw(mdir);
				};
				glColor3f(0, 1, 0);glLineWidth(1);
				if(cap[i]->_FORCED || cap[i]->lnew_frame)glLineWidth(4);
				if(cap[i]->foundBLOB)glLineWidth(6);
				if(cap[i]->_FORCED || cap[i]->lnew_frame)glColor3f(1, 0.4, 1);
				if(cap[i]->foundBLOB)glColor3f(0, 1, 1);
				if(cap[i]->state == -1)glColor3f(1, 0, 0);
				if(cap[i]->state == 0)glColor3f(1, 1, 0);
				img->DrawPlaneBox();
				glLineWidth(1);
				glColor3f(1, 1, 1);
				if(cap[i])PrTx.Add(vec3f(0, 0, 1), 0, pl->Project(img->Rpos + img->Rang.right() * 20), "%s(fps:%i)", cap[i]->name, cap[i]->f);
				if(cap[i]->state == 1 && ((cap[i]->_FORCED && key['z']) || DrawCamImg->Down)){
					glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glColor4f(1, 1, 1, 0.7);glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, cap[i]->tid);//cam_lit->Down?cap[i]->lid:
					img->glScreenQuadRay(vec3f(0, cup ? 0 : 1, cup ? 1 : 0), 0.0);
					img->glScreenQuad(img->Rmin);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
				};
			};//i
		};//j
		if(cam_stt)cam_stt->caption.print("total:%i active:%i forced:%i", mc, used_cams, forced_cams);
	};
	void save(){ for(int i = 0;i < mc;i++)cap[i]->cam->Save();CON.write(0, "CAPM::saved cam params"); };
	void load(){ for(int i = 0;i < mc;i++){ cap[i]->cam->Load();cap[i]->LoadCamParams();/*cap[i]->cam->G.init(); */};CON.write(0, "CAPM::loaded cam params"); };
	void play(int i){
		if(i < 0 || i >= mc)return;
		cap[i]->startThread();
	};
	void stop(int i){
		if(i < 0 || i >= mc)return;
		cap[i]->loopthread = false;
	};
};

CapManager CAPM;//constructor;
//======================================================================================
//-------------tools
void Set_TCOL(CPanel *p){ TCOL = p->val; };
//void Set_Wgt_track(CPanel *p){
// p->pos=clamp(p->pos,0.0,1.0);
// p->val=p->min+(p->pos*(p->max-p->min));
// p->caption.print("%f",p->val);
// for(int i=0;i<TM.mt;i++){
//  
//  setIdentity(TM.T[i].KF.KF->processNoiseCov, Scalar::all(C_track->val)); //adjust this for faster convergence - but higher noise
//  setIdentity(TM.T[i].KF.KF->measurementNoiseCov, Scalar::all(M_track->val));//1
//  setIdentity(TM.T[i].KF.KF->errorCovPost, Scalar::all(E_track->val));//0.1
//
// };
//};
void Set_Wgt_Redirect_1(CPanel *p){//p==edit p->param==track -> p->param->pos=(p->val-p->min)/(p->min+p->max);
	p->param->pos = clamp(p->param->pos, 0.0, 1.0);
	p->val = p->min + (p->param->pos*(p->min + p->max));// val1=value;
	p->caption.print("%f", p->val);
};
void Set_Wgt_Redirect_2(CPanel *p){//p==edit p->param==track -> p->param->pos=(p->val-p->min)/(p->min+p->max);
	p->param->pos = clamp(p->param->pos, 0.0, 1.0);
	p->val = p->min + (p->param->pos*(p->min + p->max));// val1=value;
	p->caption.print("%f", p->val);
	//CON.write(2,"set speed val=%f -= sel_ship==%i sid=%i",p->val,(int)SelShipEdit->val,SM.get_ship_idx_by_id((int)SelShipEdit->val));
	if(AllShip_cb->Down){
		for(int i = 0;i < SM.ms;i++){ SM.ship[i].set_speed(p->val / 100.0); };
		//CON.write(2,"set speed=%.3f to all ships ",p->val/100.0);
	} else{
		if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipIDX()].set_speed(p->val / 100.0);//{
		 //int sid=SM.get_ship_idx_by_id((int)SelShipEdit->val);//->speed=p->val;
		 //if(sid>=0){
		  //CON.write(2,"set speed=%.3f to ship %i with id=%i",p->val/100.0,sid,(int)SelShipEdit->val);
		 //};
		//};//set speed to ship[sel]
	};
};
void DEV_MODE_btn(CPanel *p){
	DEV_MODE = !DEV_MODE;
	MainPanel->Vis = DEV_MODE;
	EB2->Vis = !DEV_MODE;
	DM2->Vis = !DEV_MODE;
	CON.show = DEV_MODE;
	CON.write(2, "DEV_MODE=%i", DEV_MODE);
};
void fit_all_btn(CPanel *p){
	pl->pos.set(200, 700, -50);pl->target.set(200, 0, -50);pl->rad = 700;pl->Update();
	if(DEV_MODE)DEV_MODE_btn(NULL);
};
void Exit_btn_click(CPanel *p){
	if(applooprun == 255){ applooprun = 1;return; };
	applooprun = 255;CON.write(0, "Menu Exit - stop all ships");
	force_exit_time = time();
	CON.write(2, "ALL Ships - Stop");
	for(int i = 0;i < SM.ms;i++){//if(SM.ship[i].id<1)continue;
		SM.ship[i].set_action(SA_STOP);
	};
};
bool check_if_all_ships_are_stopped(){
	for(int i = 0;i < SM.ms;i++){
		if(SM.ship[i].state == SS_RECYCLE)continue;
		if(VLEN(SM.ship[i].dir_a) > 0.1)return false;
	};
	return true;
};
//------------ map
void Save_MAP_btn(CPanel *p){
	//for(int i = 1;i < MAX_CRV;i++){ TRAJECTORY[i].save(cstring("data\\curve%i.map", i)); };
	TrajMan.save();
	COLCRV.save();
	//BRD.save("data\\border.map");
};
void Load_MAP_btn(CPanel *p){
	//for(int i = 1;i < MAX_CRV;i++){ TRAJECTORY[i].load(cstring("data\\curve%i.map", i)); };
	TrajMan.load(0);
	// reset ship brain ?
	//------------------reassign closest curves
	//for(int i=0;i<SM.ms;i++){
	// if(i<MAX_SHIPS)SBR[1+i].reset();
	// if(SM.ship[i].state==SS_RECYCLE)continue;
	// SM.ship[i].crv_num_pos=-1;SM.ship[i].crv_pos=-1;SM.ship[i].crv_id=-1;
	//};
	//SM.ms=0;
	//TM.mt=0;
	//BRD.load("data\\border.map");
	//if(!COLCRV.mc)
	COLCRV.load();
	//--------------------
};//CRV1.load("data\\cur1.map");CRV2.load("data\\cur2.map");CRV3.load("data\\cur3.map");CRV4.load("data\\cur4.map");
//------------------- cam
void Load_Cam_Pparams_btn(CPanel *p){ CAPM.load(); };
void Save_Cam_Pparams_btn(CPanel *p){ CAPM.save(); };
void start_cam_btn(CPanel *p){
	if(p->Down){
		for(int i = 0;i < CAPM.mc;i++)CAPM.play(i);//startThread();
	} else{
		for(int i = 0;i < CAPM.mc;i++)CAPM.stop(i);//startThread();
	};
};
void start_camL_btn(CPanel *p){
	if(p->Down){
		for(int i = 70;i < CAPM.mc;i++)CAPM.play(i);//startThread();
	} else{
		for(int i = 70;i < CAPM.mc;i++)CAPM.stop(i);//startThread();
	};
};
void start_camR_btn(CPanel *p){
	if(p->Down){
		for(int i = 0;i < 70;i++)CAPM.play(i);//startThread();
	} else{
		for(int i = 0;i < 70;i++)CAPM.stop(i);//startThread();
	};
};
void cam_save_gizmo(CPanel *p){ CAPM.save(); };
void cam_load_gizmo(CPanel *p){ CAPM.load(); };
void cam_reset_pos(CPanel *p){
	for(int i = 0;i < CAPM.mc;i++){
		SCamera *img = CAPM.cap[i]->cam;
		float cam_pos_h = 240;////255;//226;
		int idx = CAPM.cap[i]->m_id;
		int col = dxf_cam_pos.v[dxf_cam_pos.l[idx].x].x, row = dxf_cam_pos.v[dxf_cam_pos.l[idx].x].z;
		img->pos = vec3f(col, cam_pos_h, row);
	};
	CON.write(0, "CAPM::reset all cams pos");
};
void Load_EXB_btn(CPanel *p){ EX.load("data\\exclusion.box"); };
void Save_EXB_btn(CPanel *p){ EX.save("data\\exclusion.box"); };
//------------------- ctrls
void start_end_edit_ctrl(CPanel *p){
	if(p->Down){
		//Ctrls.rebind();
		CON.write(1, "Edit Mode");
		DrawCtrls->Down = true;
		DrawCoils->Down = true;
	} else{
		Ctrls.save("data\\controllers.bind");
		CON.write(1, "Editing Saved");
	};
};
void sel_ctrl_edit_proc(CPanel *p){
	SelCtrl->val = atoi(SelCtrl->caption);
	CON.write(0, "selected controller:%f", SelCtrl->val);
	Ctrls.selc = (int) SelCtrl->val;
};
void add_ctrl_btn(CPanel *p){ Ctrls.add_ctrl(); };
void init_ctrls_btn(CPanel *p){
	if(p->Down){
		coilAPIbad = CoilsConnect();CON.write(0, "CoilsConnect();");
		if(coilAPIbad != 0){ CON.write(2, "Error CoilsConnect, return code %d\n", coilAPIbad); };
	} else{
		CoilsDisconnect();CON.write(0, "CoilsDisconnect();");
	};
};
void Off_all_coils_btn(CPanel *p){ dxf_coils.OFF_ALL(); };
//------------------------ marker
void IR_a(CPanel *p){ marker_send(AllShip_cb->Down ? MARKER_ALL : selShipID(), MARKER_FOUND); };//IR1=1 IR2=[0-1]
void IR_b(CPanel *p){ marker_send(AllShip_cb->Down ? MARKER_ALL : selShipID(), MARKER_DETECT); };// IR1 or IR2
void IR_c(CPanel *p){
	marker_send(AllShip_cb->Down ? MARKER_ALL : selShipID(), p->Down ? MARKERS_LIGHT_ON : MARKERS_LIGHT_OFF);
};// light ON/OFF
void IR_ct(CPanel *p){
	//ChargeTest->Down=
	CON.write(2, "Charge Testing is [%s]", ChargeTestbtn->Down ? "ON" : "OFF");
};
//void IR_d (CPanel *p){if(p->Down){marker_send(AllShip_cb->Down?MARKER_ALL:selShipID(),MARKER_FREQ+1);}else{marker_send(AllShip_cb->Down?MARKER_ALL:selShipID(),MARKER_FREQ+3);};};// FREQ 3/6
//void IR_dd(CPanel *p){if(p->Down){marker_send(AllShip_cb->Down?MARKER_ALL:selShipID(),MARKER_FREQ+5);}else{marker_send(AllShip_cb->Down?MARKER_ALL:selShipID(),MARKER_FREQ+7);};};// max FREQ 9/12
void IR_snd(CPanel *p){ marker_send(AllShip_cb->Down ? MARKER_ALL : selShipID(), MARKER_SOUND); };// sound
void IR_btr(CPanel *p){
	if(AllShip_cb->Down){
		for(int i = 0;i < SM.ms;i++){
			if(SM.ship[i].id < 1)continue;
			SM.ship[i].battery = 1;
		};
	} else{
		if(selShipIDX() >= 0)SM.ship[selShipIDX()].battery = 1;
	};
	//marker_send(AllShip_cb->Down?MARKER_ALL:(int)SelMarkEdit->val,MARKER_BATTERY);
};// battery
//-----------------------  ships
void save_anchors(const char *name){
	FILE *in = fopen(name, "wt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
	fprintf(in, "ms: %i\n", MAX_SHIPS);
	for(int i = 1;i <= MAX_SHIPS;i++)fprintf(in, "pos: %f %f ang: %f\n", ship_anchors[i].x, ship_anchors[i].y, ship_anchors[i].z);
	fclose(in);
};
void load_anchors(const char *name){
	FILE *in = fopen(name, "rt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
	char sline[256];
	int nms;fgets(sline, 256, in);sscanf(sline, "ms: %i\n", &nms);
	for(int i = 1;i <= nms;i++){ fgets(sline, 256, in);sscanf(sline, "pos: %f %f ang: %f\n", &ship_anchors[i].x, &ship_anchors[i].y, &ship_anchors[i].z); };
	fclose(in);
};
void Set_Ship_Anchor_Click(CPanel *p){
	if(p->Down){
		CON.write(2, "Edit anchor pos for ship %i", selShipIDX());
		load_anchors("data\\anchors.anc");
	} else{
		CON.write(2, "Save anchor pos for ship %i", selShipIDX());
		save_anchors("data\\anchors.anc");
	};
};
//--------------------------
void Ship_detecttor_btn(CPanel *p){
	marker_send(MARKER_ALL, MARKER_FOUND);
	// marker_send(MARKER_ALL,MARKER_LIGHT_OFF);
	marker_send(MARKER_ALL, MARKER_FREQ);
	marker_send(MARKER_ALL, MARKER_SOUND);
	ship_detection = false;
	if(p->Down){} else{ SM.ms = 0; };
};
void pop_down_state(CPanel *p){
	if(!p->Down)return;
	if(p != APilot)APilot->Down = false;
	if(p != SStop_btn)SStop_btn->Down = false;
	if(p != GTbase_btn)GTbase_btn->Down = false;
	if(p != SStop_btn)SStop_btn->Down = false;
};
void Send_ship_to_base(CPanel *p){
	//if(p->Down && EndbleBRD_cb->Down){EndbleBRD_cb->Down=false;};
	SM_reassign_anchors();
	if(AllShip_cb->Down){
		CON.write(0, "ALL Ships - GOTO base");
		for(int i = 0;i < SM.ms;i++){//if(SM.ship[i].id<1)continue;
			SM.ship[i].set_action(p->Down ? SA_GOTO_BASE : SA_STOP);
		};
	} else{
		//int sid=SM.get_ship_idx_by_id((int)SelShipEdit->val);
		if(p->Down){
			CON.write(0, "Ship %i - GOTO base", selShipIDX());
			if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipIDX()].set_action(SA_GOTO_BASE);
		} else{
			CON.write(0, "Ship %i - release", selShipIDX());
			if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipIDX()].set_action(SA_STOP);
		};
	};//!all
	pop_down_state(p);
};
void all_ship_action(bool act, bool ex_w_f = false){
	CON.write(act ? 0 : 2, "ALL Ships - %s", act ? "Autopilot" : "Stop");
	//&& (!ex_w_f || (ex_w_f && SM.ship[i].action!=SA_WAIT && SM.ship[i].action!=SA_FOLLOW))
	//for(int i = 1;i <= MAX_SHIPS;i++){ if(!ex_w_f || (ex_w_f && SBR[i].action != SA_WAIT && SBR[i].action != SA_FOLLOW)){ SBR[i].action = act ? SA_AUTOPILOT : SA_STOP; } };//?
	for(int i = 0;i < SM.ms;i++){//if(SM.ship[i].id<1)continue;
		if(ex_w_f && (SM.ship[i].action == SA_WAIT || SM.ship[i].action == SA_FOLLOW))continue;//PVS && -> ||
		SM.ship[i].set_action(act ? SA_AUTOPILOT : SA_STOP);
	};
	APilot->Down = true;
	pop_down_state(APilot);
};
void ship_autopilot_click(CPanel *p){
	//if(p->Down && !EndbleBRD_cb->Down){EndbleBRD_cb->Down=true;};
	if(AllShip_cb->Down){
		CON.write(0, "ALL Ships - Autopilot");
		for(int i = 0;i < SM.ms;i++){//if(SM.ship[i].id<1)continue;
			SM.ship[i].set_action(p->Down ? SA_AUTOPILOT : SA_STOP);
		};
	} else{
		//int sid=SM.get_ship_idx_by_id((int)SelShipEdit->val);
		if(p->Down){
			CON.write(0, "Ship %i - Autopilot", selShipIDX());
			if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipIDX()].set_action(SA_AUTOPILOT);
		} else{
			CON.write(2, "Ship %i - Stop", selShipIDX());
			if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipID()].set_action(SA_STOP);
		};
	};//!all
	pop_down_state(p);
};
void Stop_ship_click(CPanel *p){
	if(AllShip_cb->Down){
		CON.write(2, "ALL Ships - Stop");
		for(int i = 0;i < SM.ms;i++){//if(SM.ship[i].id<1)continue;
			SM.ship[i].set_action(SA_STOP);
		};
	} else{
		//int sid=SM.get_ship_idx_by_id((int)SelShipEdit->val);
		if(p->Down){
			CON.write(2, "Ship %i - Stop", selShipIDX());
			if(selShipIDX() >= 0 && selShipIDX() < SM.ms)SM.ship[selShipIDX()].set_action(SA_STOP);
		} else{
			CON.write(2, "Ship %i - Autopilot?", selShipIDX());
			//if(sid>=0)SM.ship[sid].action=SA_STOP;
		};
	};//!all
	pop_down_state(p);
};
//---------------------
void save_coff(const char *name){
	FILE *in = fopen(name, "wt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
	fprintf(in, "mc: %i\n", dxf_coils.ml);
	for(int i = 0;i < dxf_coils.ml;i++)fprintf(in, "pos: %f %f\n", dxf_coils.v[dxf_coils.l[i].x].x, dxf_coils.v[dxf_coils.l[i].x].z);
	fclose(in);
};
void load_coff(const char *name){
	FILE *in = fopen(name, "rt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
	char sline[256];
	int nms;fgets(sline, 256, in);sscanf(sline, "mc: %i\n", &nms);
	for(int i = 0;i < nms;i++){
		fgets(sline, 256, in);
		sscanf(sline, "pos: %f %f\n", &dxf_coils.v[dxf_coils.l[i].x].x, &dxf_coils.v[dxf_coils.l[i].x].z);
		dxf_coils.v[dxf_coils.l[i].y].x = dxf_coils.v[dxf_coils.l[i].x].x + 20;//fix radius
		dxf_coils.v[dxf_coils.l[i].y].z = dxf_coils.v[dxf_coils.l[i].x].z;
		MAP.v[i].x = dxf_coils.v[dxf_coils.l[i].x].x;
		MAP.v[i].y = dxf_coils.v[dxf_coils.l[i].x].z;
	};
	fclose(in);
};
void move_coil_center_click(CPanel *p){
	if(p->Down){
		load_coff("data\\coil.offsets");
	} else{
		save_coff("data\\coil.offsets");
	};
};
void draw_coils_btn_click(CPanel *p){
	if(p->Down){

	} else{

	}
}
//void ship_save_brain(CPanel *p){ ship_brain_save(); };
//void ship_load_brain(CPanel *p){ ship_brain_load(); };
void reset_lost_ships_click(CPanel *p){
	CON.write(2, "RESET all lost ships ms=%i", SM.ms);
	ship_detection = false;
	SM_clear_votes();
	for(int i = 0;i < SM.ms;i++){
		if(SM.ship[i].state == SS_LOST)SM.ship[i].state = SS_RECYCLE;//set lost to reuse
	};
};
//======================================================================================
void GUIResize(CUI *MNG){
	CON.write(0, "reinit UI");
	int y = 100, x = 0;
	MainPanel = UI.Add(UI_PANEL, 0, 0, WndW, 150, NULL, NULL, NULL, 0, 1, 0);
	StartCam = UI.Add(UI_BUTTON, 20, 60, 90, 40, MainPanel, "StartCams", start_cam_btn, 0, 1, 0);if(release)UI.L->Down = true;
	StartCamL = UI.Add(UI_BUTTON, 20, 60 + 41, 90, 20, MainPanel, "SC Left", start_camL_btn, 0, 1, 0);       UI.L->Down = true;//start_camL_btn(UI.L);//UI.L->use_OC(UI.L);
	StartCamR = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, 50 + 51, 90, 20, MainPanel, "SC Right", start_camR_btn, 0, 1, 0);if(release)UI.L->Down = true;
	ShipDetector = UI.Add(UI_BUTTON, StartCam->x + StartCam->w + 1, 60, 90, 40, MainPanel, "Detect Ships", Ship_detecttor_btn, 0, 1, 0);UI.L->Down = true;//UI.L->use_OC(UI.L);
	Ship_controll = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, 60, 90, 40, MainPanel, "Ship Controll", NULL, 0, 1, 0);UI.L->Down = true;
	ship_info_panel = UI.Add(UI_BUTTON, Ship_controll->x, 60 + 41, 90, 20, MainPanel, "Ship Info", NULL, 0, 1, 0);UI.L->Down = true;
	USB2COM3_p = UI.Add(UI_PANEL, StartCam->x, 60 + 41 + 21, 90, 20, MainPanel, "USB2COM3----", NULL, 0, 1, 0);
	W2C_p = UI.Add(UI_PANEL, UI.L->x + UI.L->w + 1, 60 + 41 + 21, 90, 20, MainPanel, "NET2CAN----", NULL, 0, 1, 0);

	//C_track=UI.Add(UI_TBAR_s,200,150,200,17,NULL,"0",NULL,0,1,0);       UI.L->Assign_UseAlt(Set_Wgt_track);UI.L->edit_set(UI_Ef,1,true,0,0.001);C_track->track_set_val(0.001);
	//M_track=UI.Add(UI_TBAR_s,200+200+5,150,200,17,NULL,"0",NULL,0,1,0); UI.L->Assign_UseAlt(Set_Wgt_track);UI.L->edit_set(UI_Ef,1,true,0,1);M_track->track_set_val(1);
	//E_track=UI.Add(UI_TBAR_s,200+400+10,150,200,17,NULL,"0",NULL,0,1,0);UI.L->Assign_UseAlt(Set_Wgt_track);UI.L->edit_set(UI_Ef,1,true,0,1);E_track->track_set_val(1);
	//--------------- CAD
	CPanel *CADG = UI.Add(UI_GROUP, 340, 5, 160, 135, MainPanel, "CAD", NULL, 0, 1, 0);y = 15;
	DrawBase = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Base layer", NULL, 0, 1, 0);UI.L->Down = true; y += 15;
	DrawConour = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Contour line", NULL, 0, 1, 0);UI.L->Down = true;debug_btr_cb = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 90, 17, CADG, "BtrDebug", NULL, 0, 1, 0);y += 15;
	DrawShore = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Shore line", NULL, 0, 1, 0);UI.L->Down = true;Ship_info_cb = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 90, 17, CADG, "ShipInfo", NULL, 0, 1, 0);y += 15;
	DrawCams = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Ceiling(cam)", NULL, 0, 1, 0);DrawCamImg = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 90, 17, CADG, "CamImgs", NULL, 0, 1, 0);y += 15;
	DrawCoils = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Floor(coils)", NULL, 0, 1, 0);UI.L->Down = true;DrawTrails = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 90, 17, CADG, "Trails", NULL, 0, 1, 0);UI.L->Down = true;y += 15;
	DrawCtrls = UI.Add(UI_CBOX, 5, y, 90, 17, CADG, "Floor(ctrls)", NULL, 0, 1, 0);UI.L->Down = false;y += 15;
	FitAll = UI.Add(UI_BUTTON, 5, y, 90, 30, CADG, "FIT ALL", fit_all_btn, 1, 1, 0);
	//---------------- cam
	CPanel *CAMG = UI.Add(UI_GROUP, CADG->x + CADG->w + 10, 5, 200, 135, MainPanel, "CAM", NULL, 0, 1, 0);y = 15;
	CamSMsel = UI.Add(UI_BUTTON, 5, y, 30, 17, CAMG, "Single", NULL, 0, 1, 0);//remove?
	UI.Add(UI_PANEL, UI.L->x + UI.L->w + 1, y, 60, 17, CAMG, "Selected:", NULL, 0, 1, 0);                                      //max_cam
	CPanel *SelCamEdit = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 30, 17, CAMG, "0", NULL, 0, 1, 0);UI.L->edit_set(UI_Ei, 1, true, 0, 104);y += 20;
	cam_stt = UI.Add(UI_PANEL, 5, y, 150, 17, CAMG, "state: total:104/on:0", NULL, 0, 1, 0);//y+=20;
	cam_lit = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 40, 17, CAMG, "lightmap", NULL, 0, 1, 0);y += 20;
	shp_trk = UI.Add(UI_PANEL, 5, y, 60, 17, CAMG, "Blobs:0", NULL, 0, 1, 0);
	UI.Add(UI_PANEL, UI.L->x + UI.L->w + 1, y, 60, 17, CAMG, "Treshold:", NULL, 0, 1, 0);
	editT = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 70, 17, CAMG, "0.0", NULL, 0, 1, 0);UI.L->Assign_UseAlt(Set_Wgt_Redirect_1);UI.L->edit_set(UI_Ef, 1, true, 0, 254);y += 20;
	UI.Add(UI_TBAR_s, 5, y, 190, 17, CAMG, NULL, NULL, 0, 1, 0);UI.L->param = editT;UI.L->param->param = UI.L;y += 20;//calback mouseon
	CamGizmo = UI.Add(UI_BUTTON, 5, y, 30, 17, CAMG, "Gizmo", NULL, 0, 1, 0);x = 5 + 35;
	CamHeightVal = UI.Add(UI_PANEL, x, y, 30, 17, CAMG, "H=255", NULL, 0, 1, 0);x += 35;
	CamSaveG = UI.Add(UI_BUTTON, x, y, 30, 17, CAMG, "Save", cam_save_gizmo, 1, 1, 0);x += 35;
	CamLoadG = UI.Add(UI_BUTTON, x, y, 30, 17, CAMG, "Load", cam_load_gizmo, 1, 1, 0);x += 35;
	CamResetPos = UI.Add(UI_BUTTON, x, y, 30, 17, CAMG, "Reset", cam_reset_pos, 1, 1, 0);x += 35;y += 20;
	//UI.Add(UI_BUTTON,x,y,30,17,CAMG,"llm",load_lightmaps_click,1,1,0);
	ExBLB_btn = UI.Add(UI_BUTTON, 5, y, 90, 17, CAMG, "ExcludeBlobsEdit", NULL, 0, 1, 0);
	UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 35, 17, CAMG, "Load", Load_EXB_btn, 1, 1, 0);
	UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 35, 17, CAMG, "Save", Save_EXB_btn, 1, 1, 0);
	//--------------- coil
	CPanel *COLG = UI.Add(UI_GROUP, CAMG->x + CAMG->w + 10, 5, 150, 135, MainPanel, "COIL", NULL, 0, 1, 0);y = 15;
	Controller_btn = UI.Add(UI_BUTTON, 5, y, 70, 20, COLG, "Init Ctrls", init_ctrls_btn, 0, 1, 0);
	CoilsOff = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 70, 20, COLG, "Off all coils", Off_all_coils_btn, 0, 1, 0);y += 20;
	UI.Add(UI_BUTTON, 5, y, 100, 17, COLG, "Sel controller:", sel_ctrl_edit_proc, 1, 1, 0);
	SelCtrl = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 30, 17, COLG, "0", NULL, 0, 1, 0);UI.L->edit_set(UI_Ei, 1, true, 0, 36);y += 20;
	ReBindCoils = UI.Add(UI_BUTTON, 5, y, 90, 20, COLG, "ReBindCoils", start_end_edit_ctrl, 0, 1, 0);
	UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 30, 20, COLG, "new", add_ctrl_btn, 1, 1, 0);y += 20;
	move_coil_center_btn = UI.Add(UI_BUTTON, 5, y, 90, 20, COLG, "MoveCoilCenter", move_coil_center_click, 0, 1, 0);//y += 20;
	draw_coils_btn = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 30, 20, COLG, "draw", draw_coils_btn_click, 0, 1, 0);draw_coils_btn->Down = true;y += 20;
	stacked_coil_cmd_info = UI.Add(UI_PANEL, 5, y, 110, 20, COLG, "stacked cmd num:0", NULL, 0, 1, 0);y += 20;
	debug_coils_cb = UI.Add(UI_CBOX, 5, y, 110, 20, COLG, "debug coils cmd", NULL, 0, 1, 0);
	//-------------- ship
	CPanel *SHPG = UI.Add(UI_GROUP, COLG->x + COLG->w + 10, 5, 200, 135, MainPanel, "SHIP", NULL, 0, 1, 0);y = 15;
	UI.Add(UI_PANEL, 5, y, 50, 17, SHPG, "Selected:", NULL, 0, 1, 0);
	SelShipEdit = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 20, 17, SHPG, "-2", NULL, 0, 1, 0);UI.L->edit_set(UI_Ei, 1, true, 0, MAX_SHIPS);UI.L->edit_set_val(1);//y+=20;
	AllShip_cb = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 30, 17, SHPG, "to all", NULL, 0, 1, 0);if(release){ UI.L->Down = true; };
	UI.Add(UI_PANEL, UI.L->x + UI.L->w + 1, y, 35, 17, SHPG, "Speed", NULL, 0, 1, 0);
	editS = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 40, 17, SHPG, "0.0", NULL, 0, 1, 0);UI.L->Assign_UseAlt(Set_Wgt_Redirect_2);UI.L->edit_set(UI_Ef, 1, true, 1, 100);y += 20;
	UI.Add(UI_TBAR_s, 5, y, 190, 17, SHPG, NULL, NULL, 0, 1, 0);UI.L->param = editS;UI.L->param->param = UI.L;y += 20;//calback mouseon
	APilot = UI.Add(UI_BUTTON, 5, y, 50, 17, SHPG, "Autopilot", ship_autopilot_click, 0, 1, 0);//if(release){UI.L->Down=true;};
	SStop_btn = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 30, 17, SHPG, "Stop", Stop_ship_click, 0, 1, 0);//y+=20;
	GTbase_btn = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 5, y, 60, 17, SHPG, "GOTO base", Send_ship_to_base, 0, 1, 0);//y+=20;
	SetShipBase_btn = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 45, 17, SHPG, "Set Base", Set_Ship_Anchor_Click, 0, 1, 0);
	y += 20;
	//-------------- ship trajectory map
	AddNewMapBtn = UI.Add(UI_BUTTON, 5, y, 60, 17, SHPG, "ADD curve", NULL, 0, 1, 0);									// HERE 10 is maximum curves in trajectory
//	SelMapEdit = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 20, 17, SHPG, "-2", NULL, 0, 1, 0);UI.L->edit_set(UI_Ei, 1, true, 1, 10);UI.L->edit_set_val(1);//y+=20;
	UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 35, 17, SHPG, "Load", Load_MAP_btn, 1, 1, 0);
	UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 35, 17, SHPG, "Save", Save_MAP_btn, 1, 1, 0);
	EditMap_btn = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 55, 17, SHPG, "Edit curve", NULL, 0, 1, 0);y += 20;
	//-------------- border
	EditBRD_btn = UI.Add(UI_BUTTON, 5, y, 50, 17, SHPG, "EditBorder", NULL, 0, 1, 0);
	EndbleBRD_cb = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 35, 17, SHPG, "collide", NULL, 0, 1, 0);UI.L->Down = true;
	Add_Collider_btn = UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 35, 17, SHPG, "Add", NULL, 0, 1, 0);
	y += 20;
	//UI.Add(UI_BUTTON, 5, y, 50, 17, SHPG, "SaveSBR", ship_save_brain, 1, 1, 0);
	//UI.Add(UI_BUTTON, UI.L->x + UI.L->w + 1, y, 50, 17, SHPG, "LoadSBR", ship_load_brain, 1, 1, 0);
	reset_lost_ships_btn = UI.Add(UI_BUTTON, UI.L->x + 10, y, 60, 17, SHPG, "Reset lost", reset_lost_ships_click, 1, 1, 0);
	//-------------- MARKER
	CPanel *MARKER = UI.Add(UI_GROUP, SHPG->x + SHPG->w + 10, 5, 130, 135, MainPanel, "MARKER", NULL, 0, 1, 0);y = 15;
	UI.Add(UI_PANEL, 5, y, 50, 17, MARKER, "Selected:", NULL, 0, 1, 0);
	SelMarkEdit = UI.Add(UI_EDIT, UI.L->x + UI.L->w + 1, y, 20, 17, MARKER, "-2", NULL, 0, 1, 0);UI.L->edit_set(UI_Ei, 1, true, 1, MAX_SHIPS);UI.L->edit_set_val(1);
	debug_marker_cb = UI.Add(UI_CBOX, UI.L->x + UI.L->w + 1, y, 40, 20, MARKER, "debug", NULL, 0, 1, 0);y += 20;
	UI.Add(UI_BUTTON, 5, y, 120, 17, MARKER, "NORMAL IR blink", IR_a, 1, 1, 0);y += 20;
	ChargeTestbtn = UI.Add(UI_BUTTON, 5, y, 120, 17, MARKER, "Charge Test ON/OFF", IR_ct, 0, 1, 0);y += 20;UI.L->Down = false;//UI.Add(UI_BUTTON,5,y,120,17,MARKER,"DETECT",IR_b,1,1,0);y+=20;
	UI.Add(UI_BUTTON, 5, y, 120, 17, MARKER, "Battery test NOW", IR_btr, 1, 1, 0);y += 20;
	LightONbtn = UI.Add(UI_BUTTON, 5, y, 120, 17, MARKER, "LIGHT ON/OFF", IR_c, 0, 1, 0);y += 20;LightONbtn->Down=true;UI.L->OnClick();
	//UI.Add(UI_BUTTON,5,y,60,17,MARKER,"FREQ 1/3",IR_d,0,1,0);UI.Add(UI_BUTTON,UI.L->x+UI.L->w+1,y,60,17,MARKER,"FREQ 5/7",IR_dd,0,1,0);y+=20;
	UI.Add(UI_BUTTON, 5, y, 120, 17, MARKER, "SOUND NOW", IR_snd, 1, 1, 0);y += 20;
	//--------------------------
	EB2 = UI.Add(UI_BUTTON, WndW - 95, 20, 90, 50, NULL, "Exit", Exit_btn_click, 0, 0, 0);
	DM2 = UI.Add(UI_BUTTON, WndW - 95, 80, 90, 50, NULL, "DEV MODE", DEV_MODE_btn, 1, 0, 0);
	UI.Add(UI_BUTTON, MARKER->x + MARKER->w + 10, 20, 90, 50, MainPanel, "Exit", Exit_btn_click, 0, 1, 0);
	UI.Add(UI_BUTTON, MARKER->x + MARKER->w + 10, 80, 90, 50, MainPanel, "DEV MODE", DEV_MODE_btn, 1, 1, 0);
	//--------------- trajectory presets
	TABLE = UI.Add(UI_GROUP, UI.L->x + UI.L->w + 10, 5, 130, 135, MainPanel, "TRAJECTORY", NULL, 0, 1, 0);y = 15;
	TrajMan.fill_up_traj_list_UI(TABLE, y);
	//editT->val=235;editT->param->pos=(editT->val-editT->min)/(editT->min+editT->max);editT->UseAlt();
	//editS->val=100;editS->param->pos=(editS->val-editS->min)/(editS->min+editS->max);editS->UseAlt();
	editT->edit_set_val(200);
	editS->edit_set_val(50);
	SelCtrl->edit_set_val(0);
	Load_MAP_btn(NULL);
	/*CRV1.load("data\\cur1.map");
	CRV2.load("data\\cur2.map");
	CRV3.load("data\\cur3.map");
	CRV4.load("data\\cur4.map");
	BRD.load("data\\border.map");*/
};
//======================================================================================