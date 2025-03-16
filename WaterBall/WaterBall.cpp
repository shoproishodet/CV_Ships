//======================================================================================
#include "CV249\CV249.h"
//======================================================================================
#include "..\inc\ENGINE.H"
#include "..\inc\bass\bass.hpp"
#pragma comment(lib,"bass.lib")
CBASS SND;
int snd_lost, snd_found, snd_low, snd_anomaly, snd_no_usb, snd_yes_usb, snd_no_n2c, snd_n2c_ok, snd_no_cam, snd_at_base, snd_no_move, snd_collision;
//======================================================================================
#include <mutex>
//======================================================================================
// NET MAC BC5FF4F20E9D
struct Timer{

	float lastTime, acc, step, frameTime, ms, fps;
	word frames;
	bool Ready;
	Timer(){ reset(0.5); };
	Timer(const float &new_step){ reset(new_step); };
	void reset(const float &new_step){ lastTime = time(), acc = 0.0f, Ready = false, step = new_step, frameTime = 0, frames = 0, ms = 0, fps = 0; };
	void getReady(){//protected	&& call once per frame loop
		float newTime = time();
		frameTime = newTime - lastTime;
		//if(frameTime<=0.0f)continue;
		//if(frameTime>0.25f)frameTime = 0.25f;//SOD
		lastTime = newTime;
		acc += frameTime;
		Ready = true;
	};
	bool once(){
		getReady();
		bool has = false;
		while(acc >= step){ acc -= step;has = 1; };
		//if(acc>=step){acc-=step;return true;};
		return has;//false;
	};
	bool loop(){
		if(!Ready)getReady();
		if(fabs(acc) >= step){ acc -= step;return true; };
		//const float render_dt=acc/step;
		frames++;
		ms += frameTime;if(ms > 1.){ ms -= 1.;fps = frames;frames = 0; };
		//fps=(fps*0.999+1./frameTime*0.001);
		Ready = false;
		return false;
	};
};
//======================================================================================      HASHES     http://www.cse.yorku.ca/~oz/hash.html
Dword hash_djb2(char *str){
	Dword hash = 5381;
	int c;
	while(c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
};
static Dword hash_sdbm(char *str){
	Dword hash = 0;
	int c;
	while(c = *str++) hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
};
//======================================================================================
struct Profiler{
	int mm, mmm, sub_pos;
	cstring *M;
	float *dt;
	Dword *H;
	Profiler() { sub_pos = mm = mmm = 0; M = NULL; dt = NULL; H = NULL; CON.write(0, "PROFILER constructor"); };
	~Profiler(){ save();CON.write(0, "PROFILER destructor"); };
	void save(const char *sname = 0){
		CON.write(0, "PROFILER save()");
		FILE *in = fopen(sname ? sname : "crush.stack", "wt");if(!in){ CON.write(2, "can't write [%s]", sname ? sname : "crush.stack");return; };
		fprintf(in, "cs size: %i\n", mm);
		for(int i = 0;i < mm;i++){ fprintf(in, "%f:[%s]\n", dt[i], M[i].text); };
		fclose(in);
	};
	//void reset(){mm=0;lt=ft;push("---------reset frame");ft=::time();};
	void push(const char *text){
		if(mm + 1 >= mmm){
			mmm = mm + 10;
			RESIZE(M, cstring, mmm);
			RESIZE(dt, float, mmm);
			RESIZE(H, Dword, mmm);
			for(int i = mm;i < mmm;i++){ M[i].init();H[i] = 0; };
		};
		M[mm].print("%s", text);
		dt[mm] = ::time();
		mm++;
	};
	int enter(Dword h, char *s){
		int idx = -1;for(int i = 0;i < mm;i++){ if(h == H[i]){ idx = i;break; }; };
		sub_pos++;
		if(idx != -1){//second cycle

		} else{//first
			idx = mm - 1;
			push(cstring("%i:[%s]", idx, s).text);
			H[idx] = h;
		};
		return idx;
	};
	void exit(int idx){
		if(idx < 0)return;
		dt[idx] = ::time() - dt[idx];
		sub_pos--;
	};
};
//---------------------
Profiler PF;
//---------------------
struct AProfiler{
	Dword h;
	int id;
	AProfiler(char *n){ h = hash_djb2(n);id = PF.enter(h, n); };
	~AProfiler(){ PF.exit(id); };
};
//---------------------
void PROFILER_LOOP(){
	return;
	CON.write(0, "enter profiler loop");
	bool hang = false;
	while(1){
		Sleep(500);
		if(::time()){//-PF.ft>10){
			if(!hang){
				CON.write(2, "app hang 10sec - frame time = %f - PF.save", ::time());//-PF.ft);
				//PF.save();
				hang = true;
			};
		} else{
			hang = false;
		};
	};
	CON.write(1, "profiler exit loop");
};
Thread ProfilerThread(&PROFILER_LOOP);
//======================================================================================
bool release = false, DEV_MODE = true;
void check_release(){
	FILE *in = fopen("data\\rele.ase", "rt");
	release = (in != NULL);
	CON.write(2, "mode == %i", release);
	if(in)fclose(in);
};
//======================================================================================
#include "coilAPI\\coilapi.h"
#pragma comment(lib,"coilAPI\\coilapi.lib")
int coilAPIbad = 1;
int coilAPIreinit = 0;
//======================================================================================
enum EMODE{
	COIL = 1,
	CTRL,
	CAM,
	SHIP
};
int MODE = 0;
//menu
CPanel *StartCam = NULL, *StartCamL = NULL, *StartCamR = NULL, *ShipDetector = NULL, *Controller_btn = NULL, *USB2COM3_p = NULL, *W2C_p = NULL;
//CAD
CPanel *DrawBase = NULL, *DrawConour = NULL, *DrawShore = NULL, *CamSMsel = NULL, *CamGizmo = NULL, *CamSaveG = NULL, *CamLoadG = NULL;
CPanel *DrawCamImg = NULL;
//CAM
CPanel *CamHeightVal = NULL, *CamResetPos = NULL;
//SHIP
CPanel *EditMap_btn = NULL, *APilot = NULL, *SetShipBase_btn = NULL, *SStop_btn = NULL, *GTbase_btn = NULL;//  *CRV1_btn=NULL,*CRV2_btn=NULL,
CPanel *editT = NULL, *editS = NULL;//,*save_map=NULL,*load_map=NULL;
CPanel *SelMarkEdit = NULL, *SelShipEdit = NULL, *SelMapEdit = NULL;
CPanel *EditBRD_btn = NULL, *EndbleBRD_cb = NULL, *ExBLB_btn = NULL, *Ship_info_cb = NULL, *AllShip_cb = NULL, *reset_lost_ships_btn = NULL;
//COIL
CPanel *CoilsOff = NULL, *DrawCoils = NULL, *DrawTrails = NULL, *DrawCtrls = NULL, *DrawCams = NULL, *FitAll = NULL;
CPanel *cam_stt = NULL, *shp_trk = NULL, *ReBindCoils = NULL, *SelCtrl = NULL, *cam_lit = NULL, *move_coil_center_btn = NULL, *draw_coils_btn = NULL, *debug_coils_cb = NULL, *stacked_coil_cmd_info = NULL;
//map
CPanel *Add_Collider_btn = NULL, *AddNewMapBtn = NULL, *ChargeTestbtn = NULL;
//MARKER
CPanel *debug_marker_cb = NULL, *debug_btr_cb = NULL, *LightONbtn = NULL;
//TABLE
CPanel *TABLE = NULL, *TABLE1 = NULL, *TABLE2 = NULL, *TABLE3 = NULL, *TABLE4 = NULL, *TABLE5 = NULL, *TABLE6 = NULL, *TABLE7 = NULL;
CPanel *MainPanel = NULL, *EB2 = NULL, *DM2 = NULL;
CPanel *Ship_controll = NULL, *ship_info_panel = NULL;
//CPanel *C_track,*M_track,*E_track;
float force_exit_time = 0;
int selShipIDX(){ return SelShipEdit ? (int) SelShipEdit->val : -1; };
int selShipID(){ return SelMarkEdit ? (int) SelMarkEdit->val : -1; };
int selMapID(){ return SelMapEdit ? (int) SelMapEdit->val : -1; };
//======================================================================================
#include "ComPort.h"
COMPort CPT;//(1);//"COM3");//ship led controller
#include "..\inc\http_post.h"
//======================================================================================
bool W2C_LOOP = true;
//HANDLE w2c_sem_handle;
//std::mutex w2c_sem_handle;
struct EVENTOS{
	HANDLE e;
	EVENTOS(){ init(); };
	~EVENTOS(){ CloseHandle(e); };
	void init(){ e = CreateEvent(0, 0, 0, 0); };
	void wait(){ WaitForSingleObject(e, INFINITE); };
	void set(){ SetEvent(e); };
};
EVENTOS w2c_event;
//======================================================================================
bool reize_CS = false;
struct CoilSyncro{
	int ms, ac, cps, acps;
	vec3i *stt;//0-on/off,1-coil+coil,2-power
	CoilSyncro(){ ms = ac = cps = acps = 0;stt = NULL; };
	bool off(){
		for(int i = 0;i < ms;i++){
			if(stt[i][0] == 0)continue;
			return false;
		};
		return true;
	};
	void push(const int &num, const byte &pw){//cps++;
		for(int i = 0;i < ms;i++){ if(stt[i][0] != 0 && stt[i][1] == num){ stt[i][2] = pw;return; }; };//renew coil state
		int idx = -1;
		for(int i = 0;i < ms;i++){ if(stt[i][0] == 0){ idx = i;break; }; };//get unused idx
		if(idx == -1){ reize_CS = true;idx = ms;RESIZE(stt, vec3i, (ms + 1));ms++;reize_CS = false; };//add new to stack, get new idx   crush at load 15.11.2014  +3 18.11.2014 on load
		stt[idx][0] = 1;//ac++;//set on
		stt[idx][1] = num;
		stt[idx][2] = pw;
		w2c_event.set();
		//w2c_sem_handle.unlock();
	};
	//int getPWR(int cid){
	// //float speed=editS->val/100.0f;
	// for(int i=0;i<ms;i++){
	//  if(cid==stt[i][1]){
	//    //return int(clamp(stt[i][2]*speed,0,100));//(int)(((float)stt[i][2])*speed);//new_power
	//   return stt[i][2];
	//  };
	// };
	// return 0;
	//};
	void sync(){
		if(coilAPIbad)return;
		//float speed=editS->val/100.0f; int(clamp(stt[i][2]*speed,0,100))
		ac = 0;
		for(int i = 0;i < ms;i++){ if(stt[i][0])ac++; };if(ac == 0)return;
		//stacked_coil_cmd_info->caption.print("stacked cmd num:%i",ac);
		for(int i = 0;i < ms;i++){
			if(reize_CS)return;
			if(stt[i][0] && coilAPIreinit == 0){
				int controller = 1 + stt[i][1] / 32;         //(k*32-1)+(i-1)
				int coil = 1 + stt[i][1] % 32;
				//int new_power=int(clamp(stt[i][2]*speed,0,100));//(int)(((float)stt[i][2])*speed);
				// CON.write(0,"SEND::cc[%i:%i]-pwr=%i(%i*%.3f) stacked[%i]", controller, coil, new_power,stt[i][2],speed,ms);
				int new_power = stt[i][2];
				stt[i][0] = 0;//ac--;//set off
				//stacked_coil_cmd_info->caption.print("stacked cmd num:%i",ac);
				//if(controller<1 || coil<1)CON.write(2,"-------------------- ctrl coil < 1");
				if(debug_coils_cb->Down)CON.write(2, "COIL_SEND::ctrl=%i,coil=%i,pwr=%i", controller, coil, min(max(new_power, 0), 100));
				CoilsSetCoilPower(controller, coil, min(max(new_power, 0), 100));cps++;
				if(min(max(new_power, 0), 100) > 0)acps++;
				//Sleep(100);
			};
		};
	};
};
CoilSyncro CS;
bool coils_off_at_end = false;
//======================================================================================
void SEND_W2C_LOOP(){
	CON.write(1, "W2C::enter loop %i", W2C_LOOP);
	float currentTime = 0, accumulator = 0, wtime = 0;;
	while(W2C_LOOP){
		//CON.write(0,"loop %i",W2C_SEND_CMD);
		const float newTime = time();
		float deltaTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += deltaTime;
		if(accumulator > 1){
			accumulator = 0;
			stacked_coil_cmd_info->caption.print("cps:%i, acps=%i", CS.cps, CS.acps);
			CS.cps = 0;CS.acps = 0;wtime = 0;
			stacked_coil_cmd_info->w = stacked_coil_cmd_info->Parent->w - 10;
		};
		if(reize_CS == false)CS.sync();
		if(CS.ac == 0){
			float ltm = ::time();
			if(W2C_LOOP)w2c_event.wait();
			//Sleep(100);
			wtime += ::time() - ltm;
		};
		//CON.write(1,"W2C LOOP");
	};
	CON.write(1, "W2C::exit loop");
};
Thread W2Cthread(&SEND_W2C_LOOP);
//======================================================================================
Timer ShipTrail(0.1), coilTimer(30.0);
CamManager CAMM;
SCamera *pl = NULL;
int afk_min = 7;
//======================================================================================
struct KalmanFilter{
	cv::KalmanFilter *KF;
	vec3f kest, kpred, lkest, kdir;
	void init(float x, float y){
		KF = new cv::KalmanFilter(4, 2, 0);
		reinit(x, y);
	};
	void reinit(float x, float y){
		setIdentity(KF->transitionMatrix);
		KF->transitionMatrix = *(Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
		KF->statePre.setTo(0);
		KF->statePre.at<float>(0, 0) = x;
		KF->statePre.at<float>(1, 0) = y;
		KF->statePost.setTo(0);
		KF->statePost.at<float>(0, 0) = x;
		KF->statePost.at<float>(1, 0) = y;              //1-a lot of noise - 0.0..1-less noise
		setIdentity(KF->measurementMatrix);         //0.000 000 5 -was ; 0.00000001
		setIdentity(KF->processNoiseCov, Scalar::all(0.0000001)); //adjust this for faster convergence - but higher noise
		setIdentity(KF->measurementNoiseCov, Scalar::all(0.5));//1
		setIdentity(KF->errorCovPost, Scalar::all(0.1));//0.1
		kest = 0.0;
		kpred = 0.0;
		lkest = 0.0;
		kdir = 0.0;
	};
	void predict(){
		Mat prediction = KF->predict();
		//Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
		//KF->statePre.copyTo(KF->statePost);
		//KF->errorCovPre.copyTo(KF->errorCovPost);
		kpred.set(prediction.at<float>(0), 0, prediction.at<float>(1));
		// kdir.set(prediction.at<float>(0),0,prediction.at<float>(1));
	};
	void correct(const vec3f &in){
		cv::Mat measurement;
		measurement = Mat::zeros(2, 1, CV_32F);
		measurement.at<float>(0) = in.x;
		measurement.at<float>(1) = in.z;
		Mat estimated = KF->correct(measurement);
		kest.set(estimated.at<float>(0), 0, estimated.at<float>(1));
	};
	/*//vec3f get(const float &dt){
	// return kpred;
	//};
	 KF->transitionMatrix.at<float>(0,2) = dt;
	 KF->transitionMatrix.at<float>(1,3) = dt;
	 Mat prediction = KF->predict();
	 //Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
	 kpred.set(prediction.at<float>(0),0,prediction.at<float>(1));
	 return kpred;
	};
	void update(const float &dt,const vec3f &pt){
	 KF->transitionMatrix.at<float>(0,2) = dt;
	 KF->transitionMatrix.at<float>(1,3) = dt;
	 Mat prediction = KF->predict();
	 //Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
	 kpred.set(prediction.at<float>(0),0,prediction.at<float>(1));
	 //Point measPt(measurement(0),measurement(1));
	 cv::Mat measurement;
	 measurement = Mat::zeros(2, 1, CV_32F);
	 measurement.at<float>(0)=pt.x;
	 measurement.at<float>(1)=pt.z;
	 Mat estimated = KF->correct(measurement);
	 lkest=kest;
	 kest.set(estimated.at<float>(0),0,estimated.at<float>(1));
	 kdir=lerp(kdir,kest-lkest,0.4);
	};// */
};
//======================================================================================
struct Trail{
	Dword mv, id, lim, ff, lff, fnum, lnum, lfnum, llnum, num_rate, event, levent, mode;
	float Ltime, Ftime, Ttime, ltime, LLtime, LFtime, max_on_time, blinking, vtime;
	vec3f *v, vel, pvel_pt;
	bool KFInit;
	::KalmanFilter KF;
	void init(int nid){
		id = nid;
		lim = 800;v = NULL;RESIZE(v, vec3f, lim);
		KF.init(0, 0);
		restart();
	};
	void restart(){
		num_rate = event = levent = mode = fnum = lnum = lfnum = llnum = ff = lff = 0;
		mv = 1;
		Ltime = Ftime = ltime = Ttime = LLtime = LFtime = vtime = ::time();
		KFInit = false;
		vel = pvel_pt = 0.0f;
		max_on_time = blinking = 0;
	};
	float timeout(){ return (::time() - ltime); };
	bool lost(){ return timeout() > 10; };//10sec-lost
	void add(const vec3f &pt){
		if(!KFInit){ KF.reinit(pt.x, pt.z);KFInit = true; };
		//if(::time()-ltime>0.1){
		 //vel=(pvel_pt-pt)/((::time()-ltime)*100);
		 //pvel_pt=pt;
		ltime = ::time();
		//KF.predict();
		KF.correct(pt);
		// };
		 //v[0]=KF.kpred;//pt
		ff = lff = 1;
		num_rate++;
	};
	void push(){
		KF.predict();
		v[0] = KF.kpred;//KF.kpred;// 
		//----------- get vel
		if(::time() - vtime > 0.1){
			//KF.correct(KF.kpred);//KF.kpred);
			vtime = ::time();
			vel = (v[0] - pvel_pt)*0.2 + vel * 0.8;
			pvel_pt = v[0];
		};
		//-----------
		if(mv > 1 && fVLEN2(v[0] - v[1]) < 2)return;
		if(mv < lim)mv++;
		if(mv > 1)for(int i = mv - 1;i > 0;i--){ v[i] = v[i - 1]; };//CON.write(0,"v[%i]<-v[%i]",i,i-1);};//shift tail back , i>=0 not work!!!
	};
	void update(){
		push();
		//--------- identify
		event = 0;
		if(ff){ fnum++;if(lnum){ LLtime = Ltime;Ltime = ::time() - Ttime;Ttime = ::time();llnum = lnum;lnum = 0;levent = 1;event = 1; };max_on_time = max(::time() - Ttime, max_on_time); };//found state
		if(!ff){ lnum++;if(fnum){ LFtime = Ftime;Ftime = ::time() - Ttime;Ttime = ::time();lfnum = fnum;fnum = 0;levent = 2;event = 2; }; };//lost state
		blinking = ff * FPS.dt + blinking * (1.0f - FPS.dt);
		ff = 0;
		//mode=(LLtime-LFtime<0.5);//dif
	};
	void draw(){
		update();
		glLineWidth(4);glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);    //glColor3fv(lerp(vec3f(0,1,0),vec3f(1,0,0),(VLEN(v[i]-v[i+1])-minc)/(maxc-minc)));
		if(DrawTrails->Down){glBegin(GL_LINE_STRIP);for(Dword i = 0;i < mv;i++){ glVertex3fv(v[i]); };glEnd();}
		if(Ship_info_cb->Down)PrTx.Add(vec3f(0, 0, 0), 1, pl->Project(v[0]), "  pt[%i] lnum[%i](%.3f) fnum[%i](%.3f) max(%.1f) blink(%.3f)", id, lnum, Ltime, fnum, Ftime, max_on_time, blinking);
	};
};
//======================================================================================
struct TrailManager{
	word mt;
	Trail *T;
	float frad;
	void init(){ mt = 0;T = NULL;frad = 30; };//50
	void push(const vec3f &p){
		int nid = -1;
		for(int i = 0;i < mt;i++){ if(T[i].lost()){ nid = i;break; }; };//reuse lost
		if(nid == -1){
			nid = mt;mt++;RESIZE(T, Trail, mt);
			T[nid].init(nid);
		} else T[nid].restart();  //crush on load 19.08.2014
		T[nid].add(p);
	};
	int closest(const vec3f &pt){
		int cnum = -1;
		float clen = frad;
		int p1 = -1;
		for(int i = 0;i < mt;i++){
			if(T[i].lost())continue;//find closest
			float len = fVLEN2(T[i].v[0] - pt);
			if(len > frad)continue;//too far
			if(len < clen){
				//bool skip=false;
				//if(p1==-1)p1=i;else{
				// glLineWidth(6);                                            
				// glColor3f(0,1,1);glBegin(GL_LINES);glVertex3fv(T[i].v[0]);glVertex3fv(T[p1].v[0]);//dir
				// vec3f dir=(T[i].v[0]-T[p1].v[0])*0.5, tan=dir.rotY(PI*0.5);
				// glVertex3fv(T[p1].v[0]+dir+tan);glVertex3fv(T[p1].v[0]+dir-tan);
				// glEnd();
				// skip=(len>VLEN(dir));
				//};
				cnum = i;clen = len;//if(!skip){};
			};//active
		};
		return cnum;
	};
	void update(int num, const vec3f &pt){//num==closest
		if(num < 0 || num >= mt || T[num].lost()){ push(pt); } else{ T[num].add(pt); };
	};
	void draw(){
		for(int i = 0;i < mt;i++){
			if(T[i].lost())continue;
			T[i].draw();
		};
	};
	vec3f pos(int n){
		if(n<0 || n>mt)return vec3f(0, 0, 0);
		return T[n].v[0];
	};
	vec3f dir(int n){//vel=mov/sec?
		if(n<0 || n>mt || T[n].mv < 2)return vec3f(0, 0, 0);
		//vec3f sdir(0.0);
		//int end=3;if(end>T[n].mv-1)end=T[n].mv-1;
		//for(int i=0;i<end;i++){sdir+=T[n].v[i]-T[n].v[i+1];};
		return T[n].vel;//T[n].v[0]-T[n].v[1];//sdir/end;
	};
	float moved_distance(int n){
		if(n<0 || n>mt || T[n].mv < 2)return -1;
		int end = min(10, T[n].mv - 1);
		vec3f sdir(0.0);
		for(int i = 0;i < end;i++){ sdir += T[n].v[i] - T[n].v[i + 1]; };
		return VLEN(sdir);
	};
};
//======================================================================================
TrailManager TM;
//=====================================================================================
void SEND_com_command2(const int &num, const int &pwr);//FD
int CTRL_get_adr(int in_all_coils_num);//FD
#include "dxf.hpp"
shape MAP;
DXF dxf_base, dxf_contour, dxf_cam_centers, dxf_relief, dxf_shore;
DXF dxf_coils, dxf_cam_pos;
//======================================================================================
void glDrawCircle(const vec3f &N, const vec3f &B, const vec3f &v, byte seg = 40, int gl_mode = GL_LINE_LOOP){
	if(seg < 5){ CON.write(2, "glDrawCircle::seg<5");return; };
	glBegin(gl_mode);
	for(byte i = 0;i < seg;i++){
		float ang = (float) i / seg * TWOPI;
		glVertex3fv(v + N * sin(ang) + B * cos(ang)); //CRUSH:16.09.2014 +4
	};
	glEnd();
}; //crush ????? 11.09.2014 at last ship detected  +1 16.09.2014
void glDrawCircle2d(const vec3f &v, byte seg = 40, int gl_mode = GL_LINE_LOOP){
	glBegin(gl_mode);
	for(byte i = 0;i < seg;i++){ float ang = (float) i / seg * TWOPI;glVertex2f(v.x + sin(ang)*v.z, v.y + cos(ang)*v.z); };
	glEnd();
};
void DrawShip(const vec3f &sp1, const vec3f &sp2, int mode = GL_TRIANGLE_FAN, const float &scale = 1){
	vec3f cr((sp2 - sp1)*0.6);//offset 0(back)..1(nose)
	float cs = fVLEN2(sp1 - sp2)*0.7*scale;//directional scale
	glDrawCircle(norm(cross(norm(sp1 - sp2), vec3f(0, 1, 0)))*cs*0.25, norm(sp1 - sp2)*cs, sp1 + cr, 5, mode);//draw ship
};
bool PinsideShip(const vec3f &p, vec3f sp1, vec3f sp2){
	vec3f cr((sp2 - sp1)*0.6);//offset 0(back)..1(nose)
	float cs = fVLEN2(sp1 - sp2)*0.7;//directional scale
	vec3f N = norm(cross(norm(sp1 - sp2), vec3f(0, 1, 0)))*cs*0.25;
	vec3f B = norm(sp1 - sp2)*cs;
	vec3f v = sp1 + cr;
	vec3f SEV, LSEV = v + N * sin((float) 4 / 5 * TWOPI) + B * cos((float) 4 / 5 * TWOPI);
	bool inside = false;
	for(int i = 0;i < 5;i++){
		float ang = (float) i / 5 * TWOPI;SEV = v + N * sin(ang) + B * cos(ang);
		if((LSEV.z < p.z && SEV.z >= p.z || SEV.z < p.z && LSEV.z >= p.z) && (LSEV.x <= p.x || SEV.x <= p.x)){
			inside ^= (LSEV.x + (p.z - LSEV.z) / (SEV.z - LSEV.z)*(SEV.x - LSEV.x) < p.x);
		};
		LSEV = SEV;
	};
	return inside;
};
bool PinsidePie(const vec3f &TP, const vec3f &p, const vec3f &d, float r, float pyr){
	vec3f tpd = p - TP;
	if(VLEN(tpd) > r)return false;
	vec3f LSEV = p, SEV = p + d.rotY(-30 * D2R)*r;
	bool inside = false;
	if((LSEV.z < TP.z && SEV.z >= TP.z || SEV.z < TP.z && LSEV.z >= TP.z) && (LSEV.x <= TP.x || SEV.x <= TP.x)){ inside ^= (LSEV.x + (TP.z - LSEV.z) / (SEV.z - LSEV.z)*(SEV.x - LSEV.x) < TP.x); };LSEV = SEV;
	SEV = p + d * r;
	if((LSEV.z < TP.z && SEV.z >= TP.z || SEV.z < TP.z && LSEV.z >= TP.z) && (LSEV.x <= TP.x || SEV.x <= TP.x)){ inside ^= (LSEV.x + (TP.z - LSEV.z) / (SEV.z - LSEV.z)*(SEV.x - LSEV.x) < TP.x); };LSEV = SEV;
	SEV = p + d + d.rotY(30 * D2R)*r;
	if((LSEV.z < TP.z && SEV.z >= TP.z || SEV.z < TP.z && LSEV.z >= TP.z) && (LSEV.x <= TP.x || SEV.x <= TP.x)){ inside ^= (LSEV.x + (TP.z - LSEV.z) / (SEV.z - LSEV.z)*(SEV.x - LSEV.x) < TP.x); };LSEV = SEV;
	SEV = p;
	if((LSEV.z < TP.z && SEV.z >= TP.z || SEV.z < TP.z && LSEV.z >= TP.z) && (LSEV.x <= TP.x || SEV.x <= TP.x)){ inside ^= (LSEV.x + (TP.z - LSEV.z) / (SEV.z - LSEV.z)*(SEV.x - LSEV.x) < TP.x); };LSEV = SEV;
	return inside;
	//tpd.norm();
	//float ang=tpd.yaw_angle();//radians z,x,y-up
	//return (d.yaw_angle()-pyr>ang && d.yaw_angle()+pyr<ang);
};
//=====================================================================================
//=====================================================================================
struct Curve{
	Dword mv, mem;
		vec3f *v;
		vec3f *t[2];
	int mip;// max int points
		vec3f *ip;
		float *ip_np;
	int sel_vert, sel_type, selc, active, lactive, num_ships;
	bool loop, highlight;
	float cptime, clen, opt_dist;

	Curve(Dword n = 0){ init();if(n)resize(n); };
	~Curve(){ DEL_(v);DEL_(t[0]);DEL_(t[1]); };
	void init(){ active = lactive = -1;mv = mem = 0;v = NULL;t[0] = t[1] = NULL;sel_vert = -1;sel_type = 1;loop = true;selc = -1;cptime = 0;num_ships = 0;clen = 1;opt_dist = 1;mip = 0;ip = NULL;ip_np = NULL; };
	void resize(const Dword &nmv){
		if(nmv < mv){ mv = nmv;return; }; //crush 21.10.2014 cmap
		if(mem < nmv){
			mem = nmv;
			RESIZE(v, vec3f, mem);
			RESIZE(t[0], vec3f, mem);RESIZE(t[1], vec3f, mem);
		};
		mv = nmv;
	};
	void save(const char *name){//if(mv<2)return;
		FILE *in = fopen(name, "wt");if(!in){ CON.write(2, "can't open [%s]", name);return; };
		fprintf(in, "mv: %i\n", mv);
		for(int i = 0;i < mv;i++)fprintf(in, "pos: %f %f %f t: %f %f %f\n", v[i].x, 0.0f, v[i].z, t[0][i].x, t[0][i].y, t[0][i].z);
		fclose(in);
	};
	bool load(const char *name){
		CON.write(0, "load [%s]", name);
		init();
		FILE *in = fopen(name, "rt");if(!in){ CON.write(2, "can't open [%s]", name);return false; };
		char sline[256];
		int nmv;fgets(sline, 256, in);sscanf(sline, "mv: %i\n", &nmv);
		resize(nmv);
		for(int i = 0;i < mv;i++){ fgets(sline, 256, in);sscanf(sline, "pos: %f %f %f t: %f %f %f\n", &v[i].x, &v[i].y, &v[i].z, &t[0][i].x, &t[0][i].y, &t[0][i].z); v[i].y = 0; };
		fclose(in);
		calc_clen();
		return true;
	};
	void add(const vec3f &pos){
		Dword n = mv;
		resize(mv + 1);
		v[n] = pos;sel_vert = n;
		autoTan(n - 1);
	};
	void Ins(int n, const vec3f &pos){
		Dword i = mv;
		resize(mv + 1);
		for(;i > n;i--){
			v[i] = v[i - 1];
			t[0][i] = t[0][i - 1];
			t[1][i] = t[1][i - 1];
		};
		v[n] = pos;sel_vert = n;
		autoTan(n);
	};
	void Del(int n){
		if(n < 0)return;
		mv--;
		memmove(v[n], v[n + 1], sizeof(vec3f)*(mv - n));
		memmove(t[0][n], t[0][n + 1], sizeof(vec3f)*(mv - n));
		memmove(t[1][n], t[1][n + 1], sizeof(vec3f)*(mv - n));
		autoTan(n);sel_vert = n;
	};
	void reflectTan(int n, int n2, bool first){//4autoTan
		vec3f N = normal(v[n], v[n2], v[n2] + t[first][n2]);
		t[first][n] = reflect(cross(N, v[n] - v[n2]).norm2(), t[first][n2]);
		t[!first][n] = -t[first][n];
	};
	void autoTan(int n, bool rec = 1){
		if(mv < 2)return;//mv-1
		int next = n + 1, prew = n - 1;
		if(loop){ if(next >= mv)next = 0;if(prew < 0)prew = mv - 1; };
		if(next < mv && prew >= 0){
			vec3f tan = norm(v[prew] - v[next]);///3;//2.5f;
			t[0][n] = tan * VLEN(v[n] - v[prew]) / 3;//VLEN(v[prew]-v[next])/6;//
			t[1][n] = -tan * VLEN(v[n] - v[next]) / 3;//VLEN(v[prew]-v[next])/6;//
			t[0][n] = (t[0][n] - t[1][n])*0.5;t[1][n] = -t[0][n];
			if(rec){ autoTan(next, 0);autoTan(prew, 0); };
		};
		if(loop)return;
		if(next == mv){ reflectTan(n, n - 1, 1);if(rec)autoTan(prew, 0); };//move self at end
		if(prew < 0){ reflectTan(n, n + 1, 0);if(rec)autoTan(next, 0); };// move self at 0
		if(next + 1 == mv)reflectTan(n + 1, n, 1);//move nb at end
		if(n == 1)reflectTan(n - 1, n, 0);//move nb at 0
	};
	vec3f ClosestSegment(const ray &dir, float &minrad, int *num = NULL){
		if(mv < 1)return vec3f(0.0);
		int n = -1;
		float dist = minrad;
		vec3f rez(0.0);
		for(Dword i = 0;i < mv;i++){
			Dword j = (i + 1) % mv;
			ray seg(v[i], v[j] - v[i]);
			vec3f pip = dir.ip(seg);//calcPlane(cross(cross(dir,v[j]-v[i]),v[j]-v[i]).norm2(),v[i]).intPlane(plpos,dir);
			vec3f CP = seg.closest(pip, true);//ClosestPointOnLine(CP,v[i],v[j],pip);
			float rad = fVLEN2(CP - pip);
			if(dist > rad){ rez = CP;dist = rad;n = j; };
		};
		if(num)*num = n;
		return rez;
	};
	vec3f closest_point(const vec3f &pt, int *num = NULL){
		if(mv < 2)return vec3f(0.0);
		int n = -1;
		float dist = 9999;
		vec3f rez(0.0);
		for(Dword i = 0;i < mv;i++){
			Dword j = (i + 1) % mv;
			ray seg(v[i], v[j] - v[i]);  //crush 21.10.2014 - eqd mouse reassign
			vec3f CP = seg.closest(pt, true);//ClosestPointOnLine(CP,v[i],v[j],pip);
			float rad = fVLEN2(CP - pt);
			if(dist > rad){ rez = CP;dist = rad;n = j; };
		};
		if(num)*num = n;
		return rez;
	};
	vec3f closest_point_mem(const vec3f &pt, int &num){
		if(mv < 2)return vec3f(0.0);
		int n = -1;
		float dist = 9999;
		vec3f rez(0.0);
		if(num == -1){//find closest
			for(Dword i = 0;i < mv;i++){
				Dword j = (i + 1) % mv;
				ray seg(v[i], v[j] - v[i]);
				vec3f CP = seg.closest(pt + norm(seg.n) * 20, true);
				float rad = fVLEN2(CP - (pt + norm(seg.n) * 20));
				if(dist > rad){ rez = CP;dist = rad;n = j; };
			};
		} else{
			//check current seg
			int cur = num - 1, next = num;if(cur < 0)cur = mv - 1;
			ray seg(v[cur], v[next] - v[cur]);
			vec3f CP = seg.closest((pt + norm(seg.n) * 20), true);
			float rad = fVLEN2(CP - (pt + norm(seg.n) * 20));
			if(dist > rad){ rez = CP;dist = rad;n = next; };
			//check next seg
			cur = num, next = (num + 1) % mv;
			seg.set(v[cur], v[next] - v[cur]);
			CP = seg.closest((pt + norm(seg.n) * 20), true);
			rad = fVLEN2(CP - (pt + norm(seg.n) * 20));
			if(dist > rad){ rez = CP;dist = rad;n = next; };
			//if(n != num){ CON.write(LOG_ONLY, "CRV::from [%i] to [%i] max[%i]", num, n, mv); };
		};
		num = n;
		return rez;
	};
	vec3f closest_point_mem_pos(const vec3f &pt, float &num_pos){
		if(mv < 2)return vec3f(0.0);
		float dist = 9999;
		vec3f rez(0.0);
		if(num_pos < 0 || num_pos>=mv){//find closest	 
			loop0i(mv){
				int cur = i - 1, next = i;if(cur < 0)cur = mv - 1;//Dword j=(i+1)%mv;
				ray seg(v[cur], v[next] - v[cur]);seg.normalize();
				float frac = seg.closest_dist_len(pt + seg.n * 20);
				vec3f CP = seg.at(frac);
				float rad = fVLEN2(CP - (pt + seg.n * 20));
				if(dist > rad){ rez = CP;dist = rad;num_pos = cur + clamp(frac / seg.len, 0.0, 0.999); };
			};
		} else{
			int num = (1 + (int) floor(num_pos)) % mv;
			//check prew seg
			int cur = num - 1, next = num;if(cur < 0)cur = mv - 1;
			ray seg(v[cur], v[next] - v[cur]);seg.normalize();
			float frac = seg.closest_dist_len(pt + seg.n * 20);
			vec3f CP = seg.at(frac);
			float rad = fVLEN2(CP - (pt + seg.n * 20));
			if(dist > rad){ rez = CP;dist = rad;num_pos = cur + clamp(frac / seg.len, 0.0, 0.999); };
			//check next seg
			cur = num, next = (num + 1) % mv;
			seg.set(v[cur], v[next] - v[cur]);seg.normalize();
			frac = seg.closest_dist_len(pt + seg.n * 20);
			CP = seg.at(frac);
			rad = fVLEN2(CP - (pt + seg.n * 20));
			if(dist > rad){ rez = CP;dist = rad;num_pos = cur + clamp(frac / seg.len, 0.0, 0.999); };
			//if(floor(num_pos)!=num){CON.write(LOG_ONLY,"CRV::from [%i] to [%i] max[%i]",num,floor(num_pos),mv);};
		};
		return rez;
	};
	vec3f get_point(float num_pos){
		int num = floor(num_pos);
		//int cur=num-1, next=num;if(cur<0)cur=mv-1;
		int cur = num, next = (num + 1) % mv;
		return lerp(v[cur], v[next], num_pos - num);
		// ray seg(v[cur],v[next]-v[cur]);seg.normalize();
		 //return seg.at((num_pos-cur)*seg.len);
	};
	float dist(float np1, float np2){//return 0;
		if(np2 < 0 || np1 < 0){ return clen;CON.write(2, "Curve::dist::not init"); };
		if(np2 >= mv){ CON.write(2, "Curve::dist::DEAD LOOP");return clen; };
		int n1 = floor(np1), n2 = floor(np2);
		float length = -(np1 - n1)*VLEN(v[n1] - v[(n1 + 1) % mv]) + (np2 - n2)*VLEN(v[n2] - v[(n2 + 1) % mv]);
		int i = n1, j;
		while(i != n2){
			j = (i + 1) % mv;
			length += fVLEN2(v[i] - v[j]);
			i = j;
		};
		//CON.write(0,"DIST %f %i to %i, mv=%i", length, n1,n2,mv);
		return length;
	};
	float dist_to_nearest_ip(float ship_n_pos, int &cip_id){
		float min_ip_dist = clen;cip_id = -1;
		loop0i(mip){
			float cd = dist(ship_n_pos, ip_np[i]);
			if(min_ip_dist > cd){ 
				min_ip_dist = cd;cip_id = i;
			};
		};
		return min_ip_dist;
	};
	//vec3f div_point(int dpn,int *cpn=NULL){if(mv<2)return vec3f(0.0);//fix
	// float ld3=clen/num_ships;
	// //float cl=fmod(::time()*5,ld3);//clen
	// vec3f LP=v[mv-1],rez=v[0];//crush 13.10.2014
	// float cl=clen-fmod(::time()*5,clen);
	// int cnt=0;
	// while(cl>ld3){cl-=ld3;cnt++;};
	// for(int i=0;i<mv;i++){
	//  float dl=VLEN(v[i]-LP);
	//  //mcl=fmod(cl,l3d);
	//  while(cl+dl>ld3){//mcl+dl>l3d
	//   float frac=(cl+dl-ld3)/VLEN(v[i]-LP);//frac to next mcl
	//   rez=lerp(LP,v[i],1.0-frac);
	//   if(cpn)*cpn=i;
	//   if(cnt==dpn)return rez;
	//   dl=(cl+dl-ld3);//mcl
	//   cl=0;//? cl+=dl;mcl=fmod(cl,l3d);
	//   //cnt++;if(cnt>=num_ships)cnt-=num_ships;
	//   cnt=(cnt+1)%num_ships;
	//  };
	//  LP=v[i];
	//  cl+=dl;
	// };
	// return rez;
	//};
	int ClosestVertex(const ray &dir, float &mindist){
		int n = -1;
		for(Dword i = 0;i < mv;i++){
			float dist = dir.angle(v[i]);//dot(norm(v[i]-dir.p),dir.n);//VLEN(plane.intPlane(plpos,norm(v[i]-plpos))-proj_pos);
			if(dist > mindist){ n = i;mindist = dist; };
		};
		return n;
	};
	int ClosestTangent(const ray &dir, float &mindist){
		int n = -1;
		for(Dword i = 0;i < mv;i++){
			float dist = dir.angle(v[i] + t[0][i]);
			if(dist > mindist){ n = i;mindist = dist; };
			dist = dir.angle(v[i] + t[1][i]);
			if(dist > mindist){ n = i + mv;mindist = dist; };//[1]=n+mv;
		};
		return n;
	};
	void calc_opt_dist(){
		//if(clen<0.5)CON.write(2,"clen<0.5 WHA??");
		if(num_ships < 1)opt_dist = clen;else opt_dist = clen / num_ships;
	};
	void calc_clen(){
		if(mv < 2)return;//loop
		clen = 0;
		vec3f LP(v[mv - 1]);
		for(int i = 0;i < mv;i++){ clen += VLEN(v[i] - LP);LP = v[i]; };
		calc_opt_dist();
		calc_self_intersections();
	};
	void Draw(vec3f color = vec3f(0, 0, 1)){
		//CON.write(0,"draw curve %i",mv);
		if(!mv)return;
		calc_opt_dist();
		////---------------------------------------------------- test equadiv points
		//glColor3fv(color);
		//glPointSize(10);
		////get curve length
		//float total_len=0;
		//vec3f LP(v[mv-1]);
		//for(int i=0;i<mv;i++){total_len+=VLEN(v[i]-LP);LP=v[i];};
		//clen=total_len;
		//float div=num_ships?num_ships:10;
		//float ld3=total_len/div;
		////draw equidist points
		//glBegin(GL_POINTS);
		//float cl=ld3-fmod(::time()*5,ld3);
		//LP=v[mv-1];
		//int cnt=0;
		//for(int i=0;i<mv;i++){
		// float dl=VLEN(v[i]-LP);
		// while(cl+dl>ld3){
		//  dl=cl+dl-ld3;
		//  glVertex3fv(lerp(LP,v[i], 1.0-(dl/VLEN(v[i]-LP)) ));
		//  //PrTx.Add(vec3f(1,0,0),1,pl->Project(lerp(LP,v[i], 1.0-(dl/VLEN(v[i]-LP)) )),"%i:[%.2f]/(%.1f)",i,dl,ld3);
		//  cl=0;
		// };
		// cl+=dl;
		// LP=v[i];
		//};
		//glEnd();
		//-----------------------------------------------------
		glLineWidth(highlight ? 10 : 6);
		glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);
		glColor3fv(color);glBegin(GL_LINE_STRIP);
		glVertex3fv(v[mv - 1]);
		for(Dword i = 0;i < mv;i++){ glVertex3fv(v[i]); };
		glEnd();
		glPointSize(10);
		glBegin(GL_POINTS);
		loop0i(mip){
			glVertex3fv(ip[i]);
		};
		glEnd();
		glPointSize(1);

		if(EditBRD_btn->Down || EditMap_btn->Down){
			for(Dword i = 0;i < mv;i++){
				PrTx.Add(vec3f(0, 1, 0), 0, pl->Project(v[i]), "[%i]", 1 + i);
				//glLineWidth(selc==i?4:1);
				//glColor3f(0,0,1);// tangents
				//float rad=VLEN(t[0][i]);
				//coil_rad=rad;
				//glBegin(GL_LINES);glVertex3fv(v[i]);glVertex3fv(v[i]+t[0][i]);glEnd();
				//glBegin(GL_POINTS);glVertex3fv(v[i]+t[0][i]);glEnd();//glVertex3fv(v[i]+t[1][i]);glVertex3fv(v[i]);glVertex3fv(v[i]+t[1][i]); */
				//float r=VLEN(t[0][i]);
				//DrawCircle(vec3f(0,0,r),vec3f(r,0,0),v[i]);
			};
			glLineWidth(1);
			if(sel_vert >= 0){
				glColor3f(1, 0, 0);
				float r = 40;
				glDrawCircle(vec3f(0, 0, r), vec3f(r, 0, 0), v[sel_vert]);
				glPointSize(10);
				glBegin(GL_POINTS);
				glVertex3fv(sel_type == 1 ? v[sel_vert] : v[sel_vert] + t[0][sel_vert]);
				glEnd();
				glPointSize(1);
			};
		};// EditBRD_btn->Down || EditMap_btn->Down
		highlight = false;
		//glPopMatrix();
	};
	bool edit(const ray &dir, const vec4f &Plane){
		// sel_vert=-1;
		if(!(mouse[0] && lmouse[0])){//hoverselect tangents & verts
			float mindist = 0.999f;
			sel_vert = ClosestVertex(dir, mindist);//sel_type=1;
			//if(sel_vert<0){sel_vert=ClosestTangent(dir,mindist);if(sel_vert>=0)sel_type=2;};
		};
		//if(sel_type==2 && sel_vert>=0 && mouse[0] && lmouse[0]){//move tangents controll
		// bool second=0;
		// int ssv=sel_vert;
		// //if(sel_vert>=mv){ssv=sel_vert-mv;second=true;};//0 or 1
		// vec3f nt=intPlane(dir,Plane)-v[ssv];//t[second][ssv]
		// for(int i=0;i<mv;i++)t[0][i]=nt;
		//};
		//if(sel_type!=1)return;
		if(sel_vert >= 0){//there are closest vertex              +vec3f(0,1,0);
			if(mouse[0] && lmouse[0]){ v[sel_vert] = intPlane(dir, Plane);calc_clen();return true; };//autoTan(sel_vert);
		} else{//no closest vetexies
			if(mouseclick[0]){//add/ins new vert   +vec3f(0,1,0)
				if(mv < 2){ add(intPlane(dir, Plane));calc_clen();return true; } else{
					float mindist = 40;//------------TUNE IT
					int n = -1;
					vec3f pz = ClosestSegment(dir, mindist, &n);
					if(n >= 0 && n < mv){ Ins(n, intPlane(dir, Plane));calc_clen();return true; };//else{   mv>1 &&  +vec3f(0,1,0)
				};//mv>1
			};//mouseclick[0]
		};
		if(key[K_DEL] && !lkey[K_DEL]){
			if(sel_vert >= 0){
				if(mv == 2)mv = 0;else{ Del(sel_vert);sel_vert = -1;calc_clen(); };
				return true;
			};
		};
		return false;
	};
	bool inside(const vec3f &p){
		bool inside = false;
		for(int i = 0;i < mv;i++){
			int j = (i + 1) % mv;
			if((v[i].z < p.z && v[j].z >= p.z || v[j].z < p.z && v[i].z >= p.z) && (v[i].x <= p.x || v[j].x <= p.x)){
				inside ^= (v[i].x + (p.z - v[i].z) / (v[j].z - v[i].z)*(v[j].x - v[i].x) < p.x);
			};
		};
		return inside;
	};
	void add_sip(float nump1, float nump2){
		//CON.write(2,"add_sip %f %f",nump1,nump2);
		RESIZE(ip, vec3f, (mip + 2));
		RESIZE(ip_np, float, (mip + 2));
		ip[mip] = get_point(nump1);ip_np[mip] = nump1;mip++;
		ip[mip] = get_point(nump2);ip_np[mip] = nump2;mip++;
	};
	void calc_self_intersections(){
		//CON.write(2,"calc sips");
		mip = 0;
		float ss, tt;
		loop0i(mv){
			int ii = (i + 1) % mv;
			//ray segi(v[i],v[ii]-v[i]);//segi.normalize();
			loop0j(mv){
				if(j == i || j == ii)continue;
				int jj = (j + 1) % mv;if(jj == i)continue;
				//ray segj(v[j],v[(jj)%mv]-v[j]);//segj.normalize();
				if(SegIntersects_st(vec2f(v[i].x, v[i].z), vec2f(v[ii].x, v[ii].z), vec2f(v[j].x, v[j].z), vec2f(v[jj].x, v[jj].z), ss, tt)){ add_sip(i + ss, j + tt); };
			};
		};
	};
};
//=====================================================================================
//#define MAX_CRV 5
//Curve TRAJECTORY[MAX_CRV];//max_ships ;CRV1,CRV2,CRV3,CRV4,  1,2,3,4
//=====================================================================================
void trajectory_preset_loader(CPanel *caller);//FD
//=====================================================================================
struct trajectory_manager{
	int num, cur_list_id;
		Curve *traj;
	vector<char*> list;
	Curve fallback;
	void init(){
		//load default TrajMan.get()
		num = 0;
		cur_list_id = 0;
		traj = nullptr;
		fallback.init();
		get_traj_list();
		load(0);// default if there is
	}
	bool isValid(int crv_id){
		return (crv_id>0 && crv_id<num);
	}
	Curve *get(int crv_id){
		//if(traj==nullptr) return null
		if(isValid(crv_id) == false) {CON.write(0,"crv_id invalid %i %i",crv_id,num);DebugBreak();	}
		return &traj[crv_id];
	}
	bool load(int index){
		if(index >= list.size())return false;
		cur_list_id = index;
		CON.write(0,"load trajectory %s",list[index]);
		cstring name("data//%s", list[index]);
		FILE *in = fopen(name.text, "rt");if(!in){ CON.write(2, "trajectory_manager::load()::can't open [%s]", name.text);return false; };
		char sline[256];
		fgets(sline, 256, in);sscanf(sline, "num trajectories: %i\n", &num);
		//CON.write(0,"read %i trajs",num);
		num++;
		traj = new Curve[10];// 10 is maximum trajectories on pool
		traj[0].init();
		for(int i = 1;i < num;i++) {
			fgets(sline, 256, in);
			int vnum;sscanf(sline, "num verts: %i\n", &vnum);
			//CON.write(0, "read %i verts", vnum);
			traj[i].init();
			loop0j(vnum){
				fgets(sline, 256, in);
				float x,y;sscanf(sline, "vert: %f %f\n", &x,&y);
				//CON.write(0, "vert %f %f", x,y);
				traj[i].add(vec3f(x, 0, y));
			}
			traj[i].calc_clen();
		}
		fclose(in);
		return true;
	}
	void save(){
		cstring name("data//%s", list[cur_list_id]);
		FILE *in = fopen(name.text, "wt");if(!in){ CON.write(2, "trajectory_manager::save()::can't open [%s]", name.text);return; };
		int cnt = num - 1;
		loopi(1,num) if(traj[i].mv==0) cnt--;
		fprintf(in, "num trajectories: %i\n", cnt);
		for(int i = 1;i < num;i++) {
			if(traj[i].mv==0) continue;
			fprintf(in, "num verts: %i\n", traj[i].mv);
			loop0j(traj[i].mv){
				fprintf(in, "vert: %f %f\n", traj[i].v[j].x, traj[i].v[j].z);// y==0 always
			}
		}
		fclose(in);
	}
	void get_traj_list(){
		// query all *.traj files in \data
		list.clear();
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(".//data//*.traj\0", &fd);
		if(hFind != INVALID_HANDLE_VALUE){
			do{ // skip default folder . and ..
				int slen = strlen(fd.cFileName);
				if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && slen && fd.cFileName[slen-1]=='j'){
					char *text = new char[slen+1];
					memcpy(text, fd.cFileName, slen);
					text[slen] = '\0';
					list.push_back(text);
				}
			} while(::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
	}
	void fill_up_traj_list_UI(CPanel *parent, int &y){
		get_traj_list();
		cstring str;
		loop0i(list.size()){
			str.print("load %s", list[i]);
			CPanel *item = UI.Add(UI_BUTTON, 5, y, 115, 17, parent, str.text, trajectory_preset_loader, 1, 1, 0);
			item->mul = i;
			y += 20;
		}
	}
};
trajectory_manager TrajMan;
//=====================================================================================
struct CurveManager{
	word mc;
	Curve *C;
	CurveManager(){ mc = 0;C = NULL; };
	void add(){ mc++;RESIZE(C, Curve, mc);C[mc - 1].init(); };
	void Draw(){ for(int i = 0;i < mc;i++)C[i].Draw(vec3f(1, 0, 1)); };
	void load(){//const char *name){//
		CON.write(0, "start load cmaps");
		//for(int i=0;i<mc;i++)C[i].mv=0;
		mc = 0;
		int i = 0, skip_add = 0;
		while(i < 11){
			if(0 == skip_add)add();
			C[mc - 1].load(cstring("data\\curve%i.cmap", i));
			i++;
			if(C[mc - 1].mv < 2){ skip_add = 1;continue; };//mc--;
			skip_add = 0;
		};
		if(C[mc - 1].mv < 2)mc--;
	};
	void save(){
		int cnt = 0;           // if(C[i].mv<2)continue;
		for(int i = 0;i < mc;i++){ C[i].save(cstring("data\\curve%i.cmap", cnt));cnt++; };
	};
	void edit(const ray &dir, const vec4f &Plane){
		//int cl=-1;
		//for(int i=0;i<mc;i++){
		// //get closest  selMapEdit->val
		//};
		//if(cl==-1)return;
		if(Add_Collider_btn->Down){
			Add_Collider_btn->Down = false;
			add();
		};
		for(int i = 0;i < mc;i++){
			C[i].edit(dir, Plane);
		};
	};
	bool collide(const vec3f &S, const vec3f &T, vec3f &CCP, vec3f &CCN){
		bool intersection = false;
		vec3f cip = T, CP, D = norm(T - S);
		vec2f CP2;
		float min_rad = fVLEN2(T - S), crad = 20;
		for(int k = 0;k < mc;k++){
			if(C[k].mv < 2)continue;
			for(int i = 0;i < C[k].mv;i++){
				//-------get intersection point 
				Dword j = (i + 1) % C[k].mv;
				vec3f BN = norm(cross(C[k].v[j] - C[k].v[i], vec3f(0, 1, 0)))*crad;
				if(SegIntersects(vec2f(C[k].v[i].x + BN.x, C[k].v[i].z + BN.z), vec2f(C[k].v[j].x + BN.x, C[k].v[j].z + BN.z), vec2f(S.x, S.z), vec2f(T.x, T.z), CP2)){
					CP.set(CP2.x, 0, CP2.y);
					float rad = fVLEN2(CP - S);//+norm(BN)*20
					if(rad < min_rad){ min_rad = rad;CCP = CP;CCN = BN;intersection = true; };//glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(BRD.v[i]+CN);glVertex3fv(BRD.v[j]+CN);glEnd();};
				};
				//connect segs
				//get prew b
				int pi = i - 1;if(pi < 0)pi = C[k].mv - 1;
				int pj = i;
				vec3f pBN = norm(cross(C[k].v[pj] - C[k].v[pi], vec3f(0, 1, 0)))*crad;
				if(SegIntersects(vec2f(C[k].v[pj].x + pBN.x, C[k].v[pj].z + pBN.z), vec2f(C[k].v[i].x + BN.x, C[k].v[i].z + BN.z), vec2f(S.x, S.z), vec2f(T.x, T.z), CP2)){
					CP.set(CP2.x, 0, CP2.y);
					float rad = fVLEN2(CP - S);//+norm(BN)*20
					if(rad < min_rad){ min_rad = rad;CCP = CP;CCN = norm(BN + pBN)*crad;intersection = true; };//glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(BRD.v[i]+CN);glVertex3fv(BRD.v[j]+CN);glEnd();};
				};
				////----------get closest point in border
				float si = SphereInt(S, D, vec4f(C[k].v[i], crad));if(si <= 0)continue;
				//float si=RayUSphere((S-BRD.v[i])/crad,norm(T-S)/crad);if(si<=0)continue;
				if(si < min_rad){ min_rad = si;CCP = S + D * si;CCP.y = 0;CCN = (CCP - C[k].v[i]);CCN.y = 0;CCN = norm(CCN)*crad;intersection = true; };//glDrawCircle(vec3f(crad,0,0),vec3f(0,0,crad),BRD.v[i],40);print_SphereInt(S,D,vec4f(BRD.v[i],crad));};
			};//i
		};//k
		return intersection;
	};
};
CurveManager COLCRV;//collision Curve
//=====================================================================================
bool LineSide(const vec3f &c, const vec3f &a, const vec3f &b){//return isLeft(v[a],v[b],v[c]);
	vec3f N(b.z - a.z, 0, a.x - b.x);// vec3f N;N.normal(v[a],v[b],viewer);
	return dot(N, c) > dot(N, a);
};
float ccw(const vec3f &p1, const vec3f &p2, const vec3f &p3){ return (p2.x - p1.x)*(p3.z - p1.z) - (p2.z - p1.z)*(p3.x - p1.x); }
void convex_hull(const vec3f *points, int n, vec3f *&hull, int &out_hullsize){
	//vec3f *hull;
	int i, t, k = 0;
	//hull = *out_hull;
	for(i = 0;i < n;++i){// lower hull
		while(k >= 2 && ccw(hull[k - 2], hull[k - 1], points[i]) <= 0) --k;
		hull[k++] = points[i];
	};
	for(i = n - 2, t = k + 1;i >= 0;--i){// upper hull
		while(k >= t && ccw(hull[k - 2], hull[k - 1], points[i]) <= 0) --k;
		hull[k++] = points[i];
	};
	//*out_hull = hull;
	out_hullsize = k;
};
//=====================================================================================
struct Controllers{
	int mc, selc;
	int *mcc, *cstt;//max coils, controller state
	float *off_time;
	int **cn;//coil number
	int *hulln;
	vec3f **hull;//controller zone
	Controllers(){
		mc = 0;selc = -1;if(SelCtrl)SelCtrl->edit_set_val(selc);mcc = NULL;cn = NULL;hull = NULL;hulln = NULL;off_time = NULL;
		cstt = NULL;
	};
	int get_adr(int in_all_coils_num){
		int cer = -1, cil = -1;
		for(int i = 0;i < mc;i++){
			for(int j = 0;j < mcc[i];j++){
				if(cn[i][j] == in_all_coils_num){ cer = i, cil = j;break; };
			};
			if(cer != -1 || cil != -1)break;
		};
		return cer * 32 + cil;//(cer+1)*32+(cil+1);
	};
	void add_ctrl(){
		int nnum = mc + 1;
		RESIZE(cn, int*, nnum);
		RESIZE(cstt, int, nnum);
		RESIZE(off_time, float, nnum);
		RESIZE(hull, vec3f*, nnum);RESIZE(hulln, int, nnum);
		RESIZE(mcc, int, nnum);
		for(int i = mc;i < nnum;i++){ mcc[i] = 0;cn[i] = NULL;cstt[i] = 0;hulln[i] = 0;hull[i] = NULL;off_time[i] = ::time(); };
		mc = nnum;
		selc = mc - 1;if(SelCtrl)SelCtrl->edit_set_val(selc);
		CON.write(0, "CTRL::ADD controller(%i) total(%i)", selc, mc);
	};
	void add_coil(int ctrl_idx, int coil_num){
		if(ctrl_idx < 0 || ctrl_idx >= mc)return;
		//dupe
		for(int i = 0;i < mcc[ctrl_idx];i++){
			if(cn[ctrl_idx][i] == coil_num){ CON.write(2, "skip dupe");return; };
		};
		int nnum = 1 + mcc[ctrl_idx];
		RESIZE(cn[ctrl_idx], int, nnum);
		cn[ctrl_idx][nnum - 1] = coil_num;
		mcc[ctrl_idx] = nnum;
		//CON.write(0,"CTRL %i::ADD COIL_idx(%i) total(%i)",ctrl_idx,coil_num,nnum);
	};
	void backspace(){
		if(selc < 0 || selc >= mc)return;
		if(mcc[selc] > 0)mcc[selc]--;
	};
	void rebind(){
		if(selc < 0){
			if(mc > 0)selc = 0;else{ add_ctrl(); };
		};
		mcc[selc] = 0;
	};
	void draw(){
		if(!draw_coils_btn->Down)return;
		//if(!DrawCtrls || !DrawCtrls->Down)return;
		//bool selected=if(selc<0 || selc>=mc)return;
		glLineWidth(3);
		for(int k = 0;k < mc;k++){
			//case ERR_POWER_ALARM:       Ctrls.set_cstt(adr,2);CON.write(0,"(2)power alarm on controller %d", adr);break;
			//case ERR_POWER_ALARM_CLEAR: Ctrls.set_cstt(adr,3);CON.write(0,"(3)power alarm clear on controller %d", adr);break;
			//case ERR_CONTROLLER_OFFLINE:Ctrls.set_cstt(adr,0);CON.write(0,"(0)controller %d OFFLINE", adr);break;
			//case ERR_CONTROLLER_ONLINE: Ctrls.set_cstt(adr,1);CON.write(0,"(1)controller %d ONLINE", adr);break;
			//default:                    Ctrls.set_cstt(adr,4);CON.write(0,"(4)unknown event code %d", event);break;
		 //-----------------draw powered coils
		 //for(int i=0;i<mcc[k];i++){
		 // //int pw=CS.getPWR(k*32+i);if(pw<1)continue;
		 // int pw=dxf_coils.get_power(cn[k][i]);if(pw<1)continue;
		 // PrTx.Add(vec3f(0,1,0),0,pl->Project(dxf_coils.center(cn[k][i]))+vec3f(0,0,5),"[%i:%i]pwr=%i",k+1,i+1,pw);
		 //};
		 //-----------------
			if(cstt[k] == 0)glColor3f(1, 0, 0);else//OFFLINE
				if(cstt[k] == 1){ if(!DrawCtrls || !DrawCtrls->Down)continue;glColor3f(selc == k, 0, 1); } else//ONLINE
					if(cstt[k] == 2){ glColor3f(0, 1, 1); } else//ALARM  if(!DrawCtrls || !DrawCtrls->Down)continue;
						if(cstt[k] == 3){ if(!DrawCtrls || !DrawCtrls->Down)continue;glColor3f(1, 1, 0); } else//ALARMC   
							if(cstt[k] == 4)glColor3f(1, 0, 1);//UNKNOWN
						//glColor3f(selc==k,0,1);
			glBegin(GL_LINE_STRIP);
			for(int i = 0;i < mcc[k];i++){ glVertex3fv(dxf_coils.center(cn[k][i])); };
			glEnd();
			glBegin(GL_LINE_LOOP);//STRIP);
			for(int i = 0;i < hulln[k];i++){ glVertex3fv(hull[k][i]); };
			glEnd();
		};
		//glColor3f(selc==k,0,1);
	   //for(int k=0;k<mc;k++){//if(selc!=k)continue;
	   // for(int i=0;i<mcc[k];i++){
	   //  //glVertex3fv(dxf_coils.center(cn[k][i]));
	   //  PrTx.Add(pl->Project(dxf_coils.center(cn[k][i])),"[%i:%i]",k+1,i+1);
	   // };
	   //};

		glLineWidth(1);
	};
	void DrawCCname(int cidx){//return ;//if(cidx<0 || cidx>=dxf_coils.ml)return;
		for(int k = 0;k < mc;k++){//if(selc!=k)continue;
			for(int i = 0;i < mcc[k];i++){
				if(cidx != cn[k][i])continue;
				//glVertex3fv(dxf_coils.center(cn[k][i]));                                                          // CS.getPWR(k*32+i)
				PrTx.Add(vec3f(0, 1, 0), 0, pl->Project(dxf_coils.center(cn[k][i])) + vec3f(0, 0, 5), "[%i:%i]pwr=%i", k + 1, i + 1, dxf_coils.get_power(cn[k][i]));
				//CON.write(1,"draw %i:%i - %i",k+1,i+1,cidx);
			};
		};
		//if(inc>=0 && inc<dxf_coils.ml){PrTx.Add(pl->Project(dxf_coils.v[dxf_coils.l[inc].x]+vec3f(0,0,5)),"[%i]",inc+1);};
	};
	void load(const char *name){
		FILE *in = fopen(name, "rt");if(!in){ CON.write(1, "NO controller bindings found");return; };
		char sline[256];
		int cnum = 0, clnum = 0;
		while(fgets(sline, 256, in) != NULL){
			if(sscanf(sline, "Controller %i, coils %i\n", &cnum, &clnum) == 2){
				add_ctrl();
				int cidx = 0;
				for(int i = 0;i < clnum;i++){
					fgets(sline, 256, in);sscanf(sline, "%i\n", &cidx);
					add_coil(mc - 1, cidx);
				};
				Convex(mc - 1);
			};
		};
		fclose(in);
		CON.write(0, "LOADED %i controllers", mc);
	};
	void save(const char *name){
		FILE *in = fopen(name, "wt");if(!in){ CON.write(1, "NO controller bindings can be saved");return; };
		for(int i = 0;i < mc;i++){
			fprintf(in, "Controller %i, coils %i\n", i, mcc[i]);
			for(int j = 0;j < mcc[i];j++){
				fprintf(in, "%i\n", cn[i][j]);
			};
		};
		fclose(in);
		CON.write(0, "SAVED %i controllers", mc);
	};
	void set_cstt(int adr, int stt){
		if(adr <= 0 || adr > mc)return;
		cstt[adr - 1] = stt;
		if(stt == 0){ CON.write(2, "controller %i goes offline, on_time=%.0fsec %0.3fms", adr, ::time() - off_time[adr - 1], ::time() - off_time[adr - 1]);off_time[adr - 1] = ::time(); };//controller %d OFFLINE
		if(stt == 1){ CON.write(2, "controller %i goes online, off_time=%.0fsec %0.3fms", adr, ::time() - off_time[adr - 1], ::time() - off_time[adr - 1]);off_time[adr - 1] = ::time(); };//controller %d ONLINE
		//if(stt==2){CON.write(2,"controller %i KZ",adr);};//controller %d короткое замыкание
	};
	void Convex(int k){
		vec3f *PT = new vec3f[mcc[k]];
		for(int i = 0;i < mcc[k];i++){ PT[i] = dxf_coils.center(cn[k][i]); };
		hull[k] = new vec3f[mcc[k] * 2];
		convex_hull(PT, mcc[k], hull[k], hulln[k]);
		delete[] PT;
	};
};
Controllers Ctrls;
int CTRL_get_adr(int in_all_coils_num){ return Ctrls.get_adr(in_all_coils_num); };
void example_callback(int adr, int event, void *data) {
	switch(event) {
		case ERR_POWER_ALARM:       Ctrls.set_cstt(adr, 2);CON.write(2, "(2)power alarm on controller %d", adr);break;//
		case ERR_POWER_ALARM_CLEAR: Ctrls.set_cstt(adr, 3);CON.write(0, "(3)power alarm clear on controller %d", adr);break;//
		case ERR_CONTROLLER_OFFLINE:Ctrls.set_cstt(adr, 0);CON.write(2, "(0)controller %d OFFLINE", adr);break;
		case ERR_CONTROLLER_ONLINE: Ctrls.set_cstt(adr, 1);CON.write(0, "(1)controller %d ONLINE", adr);break;
		case ERR_CONV_REINIT_STARTED:coilAPIreinit = 1;CON.write(1, "(5)converter re-init started");break;
		case ERR_CONV_REINIT_DONE:   coilAPIreinit = 0;CON.write(1, "(6)converter re-init done");break;
		default:                    Ctrls.set_cstt(adr, 4);CON.write(2, "(4)unknown event code %d", event);break;
	}
}
//======================================================================================
void SEND_com_command2(const int &num, const int &pwr){
	if(coils_off_at_end)return;
	//int controller=1+num/32;
	//int coil=num%32;
	if(CoilsOff->Down)return;//pwr>0 && 
	int nnum = Ctrls.get_adr(num);if(nnum < 0)return;
	CS.push(nnum, pwr);
	//CoilsSetCoilPower(controller, coil, pwr);
	//1 COM_str.print("c3 %i %i %i\r\n",controller,coil,pwr);
	//queue.push(&COM_str);
	//CON.write(1,"SEND %s",str.text);
	//1 CPT.writePort_b(COM_str.text);
	//COM_SEND_CMD=true;
};
//===================================================================================== actions
#define SA_STOP      0
#define SA_GOTO_BASE 1
#define SA_AUTOPILOT 2
#define SA_GOTO      3
#define SA_FOLLOW    4
#define SA_WAIT      5
char *action2str(int i){
	switch(i){
		case 0:return "Остановлен";
		case 1:return "На базу";
		case 2:return "По траектории";
		case 3:return "Перемещение";
		case 4:return "Следование";
		case 5:return "Ожидание";
	};
	return "нет";
};
//blink
#define LM_CONST 1
#define LM_BLINK 2
//states
#define SS_RECYCLE -2
#define SS_LOST    -1
#define SS_UNDEF    0
#define SS_OK       1
//movement
#define SM_STUCK   -1
#define SM_NONE     0
#define SM_MOVING   1
char *state2str(int i){
	switch(i){
		case -3:return "RECYCLE";
		case -2:return "LOST";
		case -1:return "UNDEF";
		case 1:return "MOVING";
		case 2:return "STUCK";
	};
	return "неизвестно";
};
//char *id2name(int i){
//	switch(i){
//		case 1:return "Торговое судно 1";//, от петра до конца 18  46см 
//		case 2:return "фрегат Россия\n 1724 32-пуш";//  59см 
//		case 3:return "линкор Гавриил\n 1713 66-пуш";// 82см 
//		case 4:return "Евстафий Плакида\n 1763 66-пуш";// 74см 
//		case 5:return "Торговое судно 2";// Флейт  , купцов Баженных 1724    55см 
//		case 6:return "яхта Диана";//Анны Иоановны  ххсм 
//		case 7:return "Торговое судно 3";//55см
//	};
//	return "неопределён";
//};
//=====================================================================================
struct ship_brain{
	float speed, charge;
	int action, crv_id, follow_id;
	vec3f target;//if action SA_GOTO only
	bool initialized;
	ship_brain(){ reset(); };//CON.write(2,"ship_brain::contructor");};   ok
	void reset(){ initialized = false;speed = 0.5;action = SA_AUTOPILOT;charge = 0;target = 0;follow_id = 0;crv_id = -1; };
};
//ship_brain SBR[MAX_SHIPS + 1];
//=====================================================================================
//=====================================================================================
struct ship_config{
	int num;
	struct config{
		
		cstring name;
		int can_id;
		int length_cm;
		int nose_off, tail_off;
		cstring icon;
		Texture tex;
		ship_brain brain;

		config() { tex.Init(); }
		~config(){}
		bool load(const char *cname){
			CON.write(0, "load ship config [%s]", cname);
			cstring fname("data//%s", cname);
			FILE *in = fopen(fname.text, "rt");if(!in){ CON.write(2, "ship_config::load()::can't open [%s]", fname.text);return false; };
			//---------- read russian text
			char sline[256];memset(sline, 0, 256);
			name.init();
			name.resize(256);
			fgets(sline, 256, in);memcpy( name.text, sline + 5, 256 - 5);//sscanf(sline, "name: %s\n", name.text);
			name.size = 0;
			loop0i(256){
				if(name.text[i] == '\n')break;
				if(name.text[i]=='\\' && name.text[i+1]=='n'){
					name.text[i]=' ';
					i++;name.size++;
					name.text[i]='\n';
				}
				name.size++;
			}
			name.text[name.size] = '\0';
			//-------------
			CON.write(0, "read ship name [%s]", name.text);
			fgets(sline, 256, in);sscanf(sline, "can_id:%i\n", &can_id);
			CON.write(0, "read ship can_id [%i]", can_id);
			fgets(sline, 256, in);sscanf(sline, "length_cm:%i\n", &length_cm);
			CON.write(0, "read [%i]", length_cm);
			fgets(sline, 256, in);sscanf(sline, "nose_magnet_offset_cm:%i\n", &nose_off);
			CON.write(0, "read [%i]", nose_off);
			fgets(sline, 256, in);sscanf(sline, "tail_magnet_offset_cm:%i\n", &tail_off);
			CON.write(0, "read [%i]", tail_off);
			icon.init();
			icon.resize(256);
			fgets(sline, 256, in);sscanf(sline, "icon:%s\n", icon.text);
			CON.write(0, "read [%s]", icon.text);
			fgets(sline, 256, in);sscanf(sline, "speed:%f\n", &brain.speed);
			CON.write(0, "read [%f]", brain.speed);
			fgets(sline, 256, in);sscanf(sline, "traj_curve:%i\n", &brain.crv_id);
			CON.write(0, "read [%i]", brain.speed);
			fclose(in);
			return true;
		}
		void load_Texture(){
			tex.loadJPG(icon.text, GL_REPEAT, 0, 0);
		}
	};
	vector<char*> list;// temp for read config
	config *cfg;
	Texture Unknown;
	ship_config(){
		// find all *.ship in data and load
		Unknown.Init();
		list.clear();
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(".//data//*.ship\0", &fd);
		if(hFind != INVALID_HANDLE_VALUE){
			do{ // skip default folder . and ..
				int slen = strlen(fd.cFileName);
				if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && slen && fd.cFileName[slen - 1] == 'p'){
					char *text = new char[slen + 1];
					memcpy(text, fd.cFileName, slen);
					text[slen] = '\0';
					list.push_back(text);
				}
			} while(::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
		// load them
		num = list.size();
		cfg = new config[num];
		loop0i(num) cfg[i].load(list[i]);
		// sort them so 'can_id' same as 'index' to cfg[] -> search in get(can_id) instead

	}
	config *get(int can_id)const{
		loop0i(num){
			if(can_id==cfg[i].can_id)return &cfg[i];
		}
		//if none found? new unknow ship setup magnet offsets as?
		return nullptr;
	}
	~ship_config(){
		delete[] cfg;
	}
	void load_textures(){
		Unknown.loadJPG("data\\ship_unknown.jpg", GL_REPEAT, 0, 0);
		loop0i(num){
			cfg[i].load_Texture();
		}
	}

};
ship_config ship_configs;// load ship configs in constructor
//=====================================================================================
vec3f get_pos(const vec3f &pos_ang, float len = 50, bool first = true){//anchor->sp1,sp2
	if(first){
		return vec3f(pos_ang.x, 0, pos_ang.z);
	} else{
		//vec3f dir(cos(pos_ang.y)*len,0,sin(pos_ang.y)*len);
		//return vec3f(pos_ang.x+dir.x,0,pos_ang.z+dir.z);
		return vec3f(pos_ang.x - cos(pos_ang.y)*len, 0, pos_ang.z - sin(pos_ang.y)*len);
	};
};
vec3f set_pos(const vec3f &a, const vec3f &b){//sp1,sp2->anchor
	return vec3f(a.x, atan2(a.z - b.z, a.x - b.x), a.z);
	//ship_anchors[selShipIDX()].set(IP.x,IP.z,ship_anchors[selShipIDX()].z);//set pos
	//ship_anchors[selShipIDX()].z=atan2(IP.z-ship_anchors[selShipIDX()].y,IP.x-ship_anchors[selShipIDX()].x);//set ang
};
//=====================================================================================
#define MAX_SHIPS 7
vec3f ship_anchors[MAX_SHIPS + 1];//pos x,y, z=angle  start from 1,2,3,4
const float safety_rad = 100;
void SM_reassign_anchors();//FD
vec3f ship_anchor_pos(int i, bool second = 0){
	if(i <= 0 || i > MAX_SHIPS){ return vec3f(0, 0, 0);CON.write(2, "anchor not set for ship.id=%i", i); };
	if(second){
		return vec3f(ship_anchors[i].x, 0, ship_anchors[i].y);
	} else{
		vec3f dir(cos(ship_anchors[i].z) * 50, sin(ship_anchors[i].z) * 50, 0);
		return vec3f(ship_anchors[i].x + dir.x, 0, ship_anchors[i].y + dir.y);
	};
};
bool ship_detection = false;
int ship_to_detect = -1;//,got_ship_respond_id=-1;//,reset_lost_ship=-1;
bool SM_has_id(int id);//FD
void assign_ship_id_by_votes(int nid);//FD
void SM_clear_votes();//FD
int SM_num_blinker();//FD
int next_ship_id(int sid){
	switch(sid){
		case 1:return 2;
		case 2:return 3;
		case 3:return 4;
		case 4:return 5;
		case 5:return 6;
		case 6:return 7;
		case 7:return 1;
	};
	return 1;
};
void ID_SHIP(){
	CON.write(0, "ID_SHIP::START");
	ship_detection = false;
	while(1){
		if(!ship_detection){ Sleep(2000);continue; };
		int lstd = ship_to_detect;                                                        //(1-loop-pass) == all ships are found                       
		while(SM_has_id(ship_to_detect)){ ship_to_detect = next_ship_id(ship_to_detect);if(lstd == ship_to_detect){ ship_detection = false;CON.write(2, "DETECT_END::ALL ships found");break; }; };
		if(!ship_detection)continue;
		//Sleep(2000);//wait 2 sec
		//if(SM_num_blinker()>1)CON.write(2,"BLINKED %i SHIPS",SM_num_blinker());
		//marker_send(MARKER_ALL,MARKER_FREQ);
		SM_clear_votes();
		marker_send(ship_to_detect, MARKER_DETECT);CON.write(0, "ID_SHIP::Search ship[?] id=%i for 10sec", ship_to_detect);
		Sleep(6000);//wait 8 sec
		//using votes assign ship id
		marker_send(MARKER_ALL, MARKER_FOUND);//put back
		assign_ship_id_by_votes(ship_to_detect);
		//if(!ship_detection || lstd!=ship_to_detect){CON.write(0,"ID_SHIP::ship %i are found",lstd);}else{ship_to_detect=next_ship_id(ship_to_detect);};
		Sleep(2000);//2sec to find last ship
		ship_to_detect = next_ship_id(ship_to_detect);
		//SM_clear_votes();
	};
	ship_detection = false;
	CON.write(0, "ID_SHIP::END");
};
Thread ship_id_thread(&ID_SHIP);
int get_closest_unused_anchor(const int &idx);
//=====================================================================================
void UIRead_ship_state2(int i);//FD
//=====================================================================================
//=====================================================================================
struct CoilMover{
	int ccoil[10], lccoil[10], usable;
	void backup(){ for(int i = 0;i < 10;i++){ lccoil[i] = ccoil[i];ccoil[i] = -1; }; };
	void  clear(){ usable = 0;for(int i = 0;i < 10;i++){ ccoil[i] = lccoil[i] = -1; }; };
	void exclude(int *fidx){
		for(int i = 0;i < 10;i++){
			if(fidx[i] < 0)continue;
			for(int j = 0;j < 10;j++){ if(ccoil[j] == fidx[i]){ ccoil[j] = -1;usable--; }; };
		};
	};                                  // =0.5      =true
	void off(){
		usable = 0;exclude(lccoil);
		for(int i = 0;i < 10;i++){
			if(ccoil[i] >= 0)dxf_coils.set_power(ccoil[i], 0.0f);
			if(lccoil[i] >= 0)dxf_coils.set_power(lccoil[i], 0.0f);
			ccoil[i] = lccoil[i] = -1;
		};
	};
	void off_used(){//turn off any lost coils index
		for(int i = 0;i < 10;i++){
			if(lccoil[i] < 0)continue;//last
			bool off = true;
			for(int j = 0;j < 10;j++){//current
				if(lccoil[i] == ccoil[j]){ off = false;break; };//used coil then skip offing
			};
			if(off){ dxf_coils.set_power(lccoil[i], 0.0f); };//auto off
		};
	};//off_cc
	void fill_coils_idx(vec3f pos){ usable = dxf_coils.circles_in_rad(pos, 40, ccoil); };
	void move(vec3f pos, vec3f dir, vec3f t_dir, float speed, float ang_lim, bool noback){
		for(int i = 0;i < 10;i++){
			if(ccoil[i] < 0)continue;
			vec3f n2c_dir = norm(dxf_coils.center(ccoil[i]) - pos);
			float ang = dot(n2c_dir, t_dir);
			//PrTx.Add(pl->Project(dxf_coils.center(nose_cc[i])+vec3f(0,0,9))," ang=%.2f dot(n2c,dir)=%.2f",ang,dot(n2c_dir,dir));
			if(ang > ang_lim && (!noback || (noback && dot(n2c_dir, dir) > -0.6))){
				dxf_coils.set_power(ccoil[i], (ang * 2 - 1.0)*speed);
			} else{ dxf_coils.set_power(ccoil[i], 0.0f); };
		};
		//off_lost();
	};
	void move2(vec3f pos, vec3f sideV, float speed, float ang_lim){
		for(int i = 0;i < 10;i++){
			if(ccoil[i] < 0)continue;
			vec3f n2c_dir = norm(dxf_coils.center(ccoil[i]) - pos);
			float ang = dot(n2c_dir, sideV);
			//PrTx.Add(pl->Project(dxf_coils.center(nose_cc[i])+vec3f(0,0,9))," ang=%.2f dot(n2c,dir)=%.2f",ang,dot(n2c_dir,dir));
			if(ang > ang_lim){
				dxf_coils.set_power(ccoil[i], (ang * 2 - 1.0)*speed);
			} else{ dxf_coils.set_power(ccoil[i], 0.0f); };
		};
		//off_lost();
	};
	void draw(const vec3f &pt){//gl_lines already
		for(int i = 0;i < 10;i++){
			if(ccoil[i] < 0)continue;
			glVertex3fv(pt);glVertex3fv(dxf_coils.center(ccoil[i]));
		};
	};
};
//void SM_excluder(CoilMover *CM,int sidx);//FD
//void SM_line_excluder(CoilMover *CMn,CoilMover *CMb,int idx,bool draw_exclude);//FD
void SM_circle_excluder(const int &idx);//FD
int get_closest_curve(int idx, vec3f sp1);//FD
//int get_closest_unused_curve(int idx, vec3f sp1);//FD
void stop_near_ships_on_lost(int lidx);//FD
bool CollideShips(int idx, float rad, const vec3f &S, const vec3f &T, vec3f &CCP, vec3f &CCN);/*FD*/
//=====================================================================================
struct Ship{			// id==can_id, idx==index

	int nose_id, back_id, id, idx, lid, crv_pos, nose_anchor, ass_anchor, vote, battery, battery_phase, n_led_event, b_led_event, crv_id, follow_id, anchor_id;
	float len, powerA, powerB, speed, timer, btr_timer, battery_charge, target_speed, crv_len_to_next;
	//int target;      //dir=ship orientation     //last known points
	vec3f dir_a, dir_b, dir, t_dir, target, tmp_target, sideV, sp1, sp2, center, acc_a, acc_b, anchor;
	float n_off, a_off, n_pwr, a_pwr, ltime, closest_ship_dist, crv_num_pos, closest_cip;//,lost_time;
	int closest_ship_relation, closest_cip_id;
	CoilMover CMn, CMb;
	int LED_MODE, action, state, next_action, motion;
	bool lost_rimended;
	ship_config::config *cfg;

	float timer_time(){ return ::time() - timer; };
	void timer_reset(){ timer = ::time(); };
	void init(int a, int b, int nidx){
		cfg = nullptr;
		idx = nidx;
		CMn.clear();
		CMb.clear();                                                        //or load ?
		CON.write(0, "init ship [%i %i]", a, b);//action=SA_STOP;next_action=SA_AUTOPILOT;id=-1;// id=SHIP_NONE;
		action = SA_AUTOPILOT;next_action = SA_AUTOPILOT;id = -1;// id=SHIP_NONE;
		nose_anchor = -1, ass_anchor = -1;
		n_off = a_off = 0.0f;dir_a = 0;dir_b = 0;dir = t_dir = target = 0;len = 0;nose_id = a;back_id = b;//target=-1;
		crv_pos = -1;powerA = 1.0, powerB = 1.0;speed = 0.5;
		if(a >= 0 && b >= 0){
			dir = TM.pos(a) - TM.pos(b);len = VLEN(dir);dir /= len;TM.T[nose_id].num_rate = TM.T[back_id].num_rate = 0;
			anchor = set_pos(TM.pos(a), TM.pos(b));
		};//identify();
		LED_MODE = 0;ltime = 0;lost_rimended = 0;
		state = SS_UNDEF;lid = -1;vote = 0;acc_a = acc_b = 0.0f;
		timer_reset();
		battery = 0;btr_timer = ::time();battery_charge = 0;
		crv_id = -1;
		follow_id = anchor_id = 0;
		target_speed = 0.5;
		closest_ship_dist = 999;
		closest_ship_relation = 0;
		crv_num_pos = -1;
		motion = SM_NONE;
		crv_len_to_next = 1;
		closest_cip = 999;
		closest_cip_id = -1;

		if(GTbase_btn && GTbase_btn->Down) {action=SA_GOTO_BASE;next_action=SA_GOTO_BASE;}
	};
	void off_coils_on_lost(){//not working ?
		CMn.backup();CMn.off();
		CMb.backup();CMb.off();
		//for(int i=0;i<10;i++){lnose_cc[i]=nose_cc[i];nose_cc[i]=-1;lass_cc[i]=ass_cc[i];ass_cc[i]=-1;};//nose & ass backup
		//for(int i=0;i<10;i++){
		// if(lnose_cc[i]>=0)dxf_coils.set_power(lnose_cc[i],0.0f);
		// if(lass_cc[i]>=0)dxf_coils.set_power(lass_cc[i],0.0f);
		//};
	};
	bool tottaly_lost_reminder(){
		if(ltime == 0.0)return false;
		float dt = ::time() - ltime;
		if(dt > 10){ ltime = ::time();lost_rimended = 0; }//restart every 10 sec
		bool llr = lost_rimended;
		lost_rimended = dt > 5.0f;
		return lost_rimended != llr;//alert after 5 sec
	};
	void lost(){
		CON.write(2, "LOST ship[%i].id=%i - off ship coils, alert", idx, id);
		//if(id>0){//if(id>=0){
		// //float dist=VLEN(ship_anchor_pos(id,0)-sp1);if(dist>50) // test with closest anchor
		// //SND.Play(snd_lost);//delay it for qset?
		// //lost_time=::time();
		//};
		stop_near_ships_on_lost(idx);//stop others
		//marker_send(id,MARKER_LIGHT_OFF);//off light
		marker_send(id, MARKER_FOUND);//set normal mode
		off_coils_on_lost();
		//set_table(id, idx, "-lost");
		int tid = id;
		init(-1, -1, idx);
		ltime = ::time();
		lid = tid;
		if(lid > 0)state = SS_LOST;//only if id is legal?
		cfg = nullptr;
	};
	vec3f nose_coil_(){
		int p1 = nose_id, p2 = back_id;
		vec3f pos = TM.pos(p1);
		return pos + norm(pos - TM.pos(p2))*n_off;
	};
	vec3f back_coil_(){
		int p1 = nose_id, p2 = back_id;
		vec3f pos = TM.pos(p2);
		return pos + norm(pos - TM.pos(p1))*a_off;
	};
	void write_charge_result(float measure, float perc, float volts){
		FILE *in = fopen(cstring("data\\ship_id%i.charge", id), "at");
		if(!in){ CON.write(2, "can't open [data\\ship_id%i.charge]", id);return; };
		fseek(in, 0, SEEK_END);
		int size = ftell(in);
		if(size < 10)fprintf(in, "|full date| |time h:m:s| |measurements| |percents| |voltage|\n");
		SYSTEMTIME obj;
		GetLocalTime(&obj);//   GetSystemTime
		//fprintf(cn,"(%id.%im.%iy %ih:%im:%is)::LOG_ONLY::%3i:%3i::%s\r\n", obj.wDay,obj.wMonth,obj.wYear, obj.wHour,obj.wMinute,obj.wSecond,tm/1000,tm%1000, tmtx);
		fprintf(in, "%id.%im.%iy %ih:%im:%is %2i%2i%2i %.1f %.1f %.1f\n", obj.wDay, obj.wMonth, obj.wYear, obj.wHour, obj.wMinute, obj.wSecond, obj.wHour, obj.wMinute, obj.wSecond, measure, perc, volts);
		fclose(in);
	};
	bool set_nose(){
		if(TM.T[nose_id].lost() || TM.T[back_id].lost()){ lost();return false; };//del
		dir = TM.pos(nose_id) - TM.pos(back_id);
		len = VLEN(dir);if(len < 20 || len>150){ lost();return false; };
		dir /= len;
		//------------- set nose 4 294 967 295 (0xffffffff)
		if(TM.T[nose_id].num_rate < 4294967000){
			if(TM.T[nose_id].num_rate < TM.T[back_id].num_rate){
				int tmp = nose_id;nose_id = back_id;back_id = tmp;
				anchor = set_pos(TM.pos(nose_id), TM.pos(back_id));
			};//swap
		} else{
			CON.write(0, "uint overflow - reset");
			TM.T[nose_id].num_rate = TM.T[back_id].num_rate = 0;
		};
		//--------- ship id detector blink
		//if(TM.T[nose_id].blinking<0.9){
		// if(TM.T[nose_id].levent!=TM.T[back_id].levent)LED_MODE=LM_BLINK;
		//}else{
		// LED_MODE=LM_CONST;
		//};
		//if(TM.T[nose_id].num_rate>500000){TM.T[nose_id].num_rate-=TM.T[back_id].num_rate;TM.T[back_id].num_rate=0;CON.write(2,"nose::refresh count");};
		//if(TM.T[nose_id].mv<TM.T[back_id].mv){int tmp=nose_id;nose_id=back_id;back_id=tmp;};
		//if(TM.T[nose_id].rate<TM.T[back_id].rate){int tmp=nose_id;nose_id=back_id;back_id=tmp;};//swap
		//identify();
		//if(TM.T[nose_id].mode==LM_CONST || TM.T[back_id].mode==LM_CONST)LED_MODE=LM_CONST;//const mode
		//if(TM.T[nose_id].mode==LM_BLINK && TM.T[back_id].mode==LM_BLINK)LED_MODE=LM_BLINK;//blink mode
		//////if(TM.T[nose_id].blink>0.85 || TM.T[back_id].blink>0.85)LED_MODE=LM_CONST;//const mode
		//////if(TM.T[nose_id].blink<0.85 && TM.T[back_id].blink<0.85)LED_MODE=LM_BLINK;//blink mode
		//LED_MODE=(TM.T[nose_id].blinking<0.85 && TM.T[nose_id].levent!=TM.T[back_id].levent)?LM_BLINK:LM_CONST;
		//-----battery charge detection
		if(id > 0){//------------------------------------------------------------------------------charge detection
			bool draw_dbg_msg = (debug_btr_cb->Down && id == selShipID());
			if(ChargeTestbtn->Down && battery == 0 && ship_detection == false && ::time() - btr_timer > 10 * 60){
				battery = 1;CON.write(0, "ship[%i].id=%i Start battery test", idx, id);
			};
			if(battery == 1){
				battery = 2;btr_timer = ::time();
				marker_send(id, MARKER_BATTERY);battery_phase = 1;
				//marker_send(id,MARKER_LIGHT_OFF);
				marker_send(id, MARKER_SOUND);
			};
			if(battery == 2){
				float passed_time = ::time() - btr_timer;battery_charge = passed_time;
				if(draw_dbg_msg){
					if(TM.T[nose_id].event == 1){ CON.write(0, "ship[%i].id=%i +++nose_found::|%.1f| Ltime(%f)", idx, id, passed_time, TM.T[nose_id].Ltime); };
					if(TM.T[nose_id].event == 2){ CON.write(0, "ship[%i].id=%i ---nose_lost::|%.1f| Ftime(%f)", idx, id, passed_time, TM.T[nose_id].Ftime); };
					if(TM.T[back_id].event == 1){ CON.write(0, "ship[%i].id=%i +++back_found::|%.1f| Ltime(%f)", idx, id, passed_time, TM.T[back_id].Ltime); };
					if(TM.T[back_id].event == 2){ CON.write(0, "ship[%i].id=%i ---back_lost::|%.1f| Ftime(%f)", idx, id, passed_time, TM.T[back_id].Ftime); };
				};
				//if(TM.T[nose_id].event==2 && battery_phase==3)battery_charge=passed_time;//update charge value on nose_led_lost
				int lphase = battery_phase;
				if(TM.T[nose_id].event == 1 && TM.T[nose_id].Ltime > 1.0f){ battery_phase++;if(draw_dbg_msg)CON.write(0, "ship[%i].id=%i MARKER nose", idx, id); };//->3
				if(TM.T[back_id].event == 1 && TM.T[back_id].Ltime > 1.0f){ battery_phase++;if(draw_dbg_msg)CON.write(0, "ship[%i].id=%i MARKER back", idx, id); };//->3   if(draw_dbg_msg)
				if(lphase != 3 && battery_phase == 3){ CON.write(0, "ship[%i].id=%i start battery test timer", idx, id); };//start timer after both led off then on    btr_timer=::time();
				if(passed_time > 10 && battery_phase < 3){ CON.write(2, "ship[%i].id=%i battery test falied (%.1f phase=%i)", idx, id, passed_time, battery_phase);battery = 0; };//marker_send(id,MARKER_LIGHT_ON);};
				//if(battery_phase==5 && TM.T[nose_id].Ftime>1.5f && TM.T[back_id].Ftime>1.5f){//5->6
				// battery_phase++;if(draw_dbg_msg)CON.write(0,"ship[%i].id=%i MARKER nose",idx,id);
				//};
				if(passed_time > 30 || battery_phase == 5){//battery=3;//battery_charge+=70;
					float charge_percent = battery_charge * 100.0f / 30.0f, battery_volts = (battery_charge + 70.0f) / 100.0f*(id == 1 || id == 5 || id == 6 ? 4.2f : 8.4f);
					if(passed_time < 14)SND.Play(snd_low);//snd_low_energy
					if(battery_phase == 5){
						CON.write(0, "ship[%i].id=%i END battery test, result=%.1f - PWR=%.1f (%.1fV)", idx, id, battery_charge, charge_percent, battery_volts);
						//set_table(id, idx, cstring("PWR=%.1f%% (%.1fV)", charge_percent, battery_volts));
						if(id<0 || id>MAX_SHIPS)CON.write(2, "ship[%i] id out of range %i", idx, id);
						//SBR[id].charge = charge_percent;
					} else{
						if(battery_phase == 4)CON.write(1, "ship[%i].id=%i END battery test, FUZY %.1f - PWR=%.1f (%.1fV)", idx, id, battery_charge, charge_percent, battery_volts);
						else CON.write(2, "ship[%i].id=%i END battery test, FALIED %.1f - PWR=%.1f (%.1fV)", idx, id, battery_charge, charge_percent, battery_volts);
					};
					if(battery_phase > 3)write_charge_result(battery_charge, charge_percent, battery_volts);
					btr_timer = ::time();
					battery = 0;//marker_send(id,MARKER_LIGHT_ON);
					marker_send(id, MARKER_SOUND);
				};
			};
		};//------------------------------------------------------------------------------------------id
		//-----
		dir_a = TM.dir(nose_id);
		dir_b = TM.dir(back_id);
		acc_a = nose_coil_() - sp1;
		acc_b = back_coil_() - sp2;
		sp1 = nose_coil_();
		sp2 = back_coil_();
		center = (sp1 + sp2)*0.5;
		//if(id>0 && LED_MODE==LM_BLINK){marker_send(id,MARKER_FOUND);CON.write(2,"SM::bring ship %i to found state",id);};
		return true;
	};
	void set_action(int act){
		if(act != SA_WAIT && act != SA_FOLLOW){ follow_id = 0;if(id > 0 && cfg)cfg->brain.follow_id = 0; };
		if(action == SA_AUTOPILOT && act == SA_GOTO)next_action = SA_AUTOPILOT;
		if(action == SA_STOP){ CMn.off();CMb.off(); };
		if(act == SA_AUTOPILOT){ crv_pos = -1;crv_num_pos = -1; };
		if(act == SA_WAIT)next_action = action;
		if(act == SA_STOP){ anchor = set_pos(sp1, sp2); };
		//if(act==SA_GOTO_BASE && action!=SA_GOTO_BASE) next_action = action;
		action = act;if(id > 0 && cfg)cfg->brain.action = act;
		//set_table(id,idx,cstring("PWR=%.1f%s (%.1fV)",charge_percent,"%",battery_volts));
	};
	void set_speed(float spd){ target_speed = spd;if(id > 0 && cfg)cfg->brain.speed = spd; };
	void set_traj_crv(int c_id){//,int c_id_pt = -1){
		if(TrajMan.isValid(c_id) == false) return;
		crv_id = c_id;
		if(cfg) cfg->brain.crv_id = c_id;
		//set closest traj pt
		GOTO(TrajMan.get(c_id)->closest_point(sp1)); // TESTIT
		//GOTO(TrajMan.get(crv_id)->closest_point_mem(sp1, crv_pos));// closest curve + curve_pos
	}
	void GOTO(const vec3f &pos){//--------------------------------------------------------- TODO: off collision near base
		target = tmp_target = pos;if(id > 0 && cfg)cfg->brain.target = pos;
		if(action == SA_GOTO_BASE && id > 0 && fVLEN2(ship_anchor_pos(id) - sp1) > 120){} else{
			vec3f CN;                                                    //|| (id>0 && CollideShips(idx,70,sp1,pos,tmp_target,CN))
			if(!key['c'] && (COLCRV.collide(sp1, pos, tmp_target, CN))){ tmp_target += CN; };//!
		};
		t_dir = norm(tmp_target - sp1);
		vec3f N = norm(cross(t_dir, dir));
		sideV = cross(N, dir);
	};
	void update_CC(){
		if(!Ship_controll || !Ship_controll->Down)return;//if(id<=SHIP_NONE)return; // const vec3f &Tdir,const vec3f &Vdir
		CMn.backup();//for(int i=0;i<10;i++){lnose_cc[i]=nose_cc[i];nose_cc[i]=-1;};//nose backup
		CMb.backup();//for(int i=0;i<10;i++){lass_cc[i]=ass_cc[i];ass_cc[i]=-1;};//ass backup
		CMn.fill_coils_idx(sp1);//SM_excluder(&CMn,idx);
		CMb.fill_coils_idx(sp2);CMb.exclude(CMn.ccoil);//SM_excluder(&CMb,idx);
		bool gotopt = false;
		if(action == SA_GOTO_BASE){//  && id>0
			if(!anchor_id)SM_reassign_anchors();//anchor_id=get_closest_unused_anchor(idx);
			gotopt = VLEN(ship_anchor_pos(anchor_id) - sp1) > 120;//> 120;
			if(gotopt) speed=0.5f;//{ 
			//bool nearb = VLEN(ship_anchor_pos(anchor_id) - sp1) <120;//> 120;
			//if(nearb) speed=0.3f;//{ 
			GOTO(ship_anchor_pos(anchor_id));
			//};//speed=1;
		};
		if(action == SA_GOTO){            //next - stop|auto|follow
			if(VLEN(target - sp1) < 30)set_action(next_action);//when arrived to target   ?SA_AUTOPILOT:SA_STOP
			GOTO(target);
		};
		//-----------------------!!!!!!!!!!!!!!   CHANGE t_dir
		//SM_line_excluder(&CMn,&CMb,idx,false);
		SM_circle_excluder(idx);
		bool slow_down = false;                                   //no need to turn          //  || (closest_ship_dist<100)  ??
		  //1: i follow j
		  //2: collision X_X
		  //3: ok
		  //4: j follow i
		if((action != SA_GOTO && !gotopt && VLEN(dir_a) > target_speed + 0.3 && dot(dir, t_dir) > 0.5))slow_down = true;
		if(action != SA_GOTO && !gotopt && (closest_ship_dist < 150 && (closest_ship_relation == 2 || closest_ship_relation == 1)))slow_down = true;
		if(slow_down == false && (action == SA_AUTOPILOT || gotopt || action == SA_GOTO || action == SA_FOLLOW)){//CON.write(2,"SA_GOTO %i",id);
		 //----------------------- speed code
		 //if(id>0){
			if(VLEN(target - sp1) > 100){ speed = 1.0f; } else{
				float smula = 0;
				if(crv_id > 0 && action == SA_AUTOPILOT){
					// float ds=(float)closest_ship_dist/(float)CRV[crv_id].opt_dist/(float)CRV[crv_id].num_ships;
					// smul=ds;//speed=clamp(speed*ds,0.1f,9.0f);
					smula = (((float) crv_len_to_next / (float) TrajMan.get(crv_id)->opt_dist) - 1.0)*0.25;
				};
				//CON.write(0,"%f < %f + %f",VLEN(dir_a),target_speed , smula);
				if(VLEN(dir_a) < (target_speed + smula)){
					speed = clamp(speed + 0.0001, 0.0f, 1.0f);
				} else{
					speed = clamp(speed - 0.0001, 0.0f, 1.0f);
				};
			};

			if(timer_time() > 20.0 && TM.moved_distance(nose_id) < 10.0f && VLEN(dir_a) < 0.1f){
				if(motion != SM_STUCK)motion = SM_STUCK;
				if(timer_time() > 40.0){ timer_reset(); } else{
					timer_reset();CON.write(2, "SHIP[%i].id=%i NOT MOVING", idx, id);SND.Play(snd_no_move);
				};
			} else{
				if(timer_time() > 20.0){ motion = SM_MOVING; };
			};
			// };//id
			 //-------------------------
			if(dot(dir, t_dir) < 0.5){//-------------turn ship at target or tmp_target                       -PI/5                -PI/5
				float ship_angle = atan2(dir.z, dir.x);if(LineSide(tmp_target, sp2, sp1))ship_angle += PI / 2;else ship_angle -= PI / 2; //if(LineSide(target,sp2,sp1))right else left
				vec3f hpa(cos(ship_angle), 0, sin(ship_angle));
				CMn.move(sp1, dir, -hpa, max(0.35, speed), 0.5, true);
				CMb.move(sp2, dir, hpa, max(0.35, speed), 0.5, true);
			} else{
				//------------------------------move to target
				CMn.move(sp1, dir, t_dir, speed, 0.5, true);
				if(dot(dir, t_dir) < 0.5){//-----turn back to target
					float cross_force_scale = VLEN(cross(dir, t_dir));//*0.8;
					CMb.move2(sp2, sideV, speed*cross_force_scale, 0.7);
				} else CMb.off();//ass
			};//----------------------------------end move
		   //};//target
		};//SA_GOTO
		if(action == SA_GOTO_BASE){//CON.write(2,"SA_ANCHOR %i %i",nose_anchor,ass_anchor);    && id>0
			if(!gotopt){//very close SND.Play(snd_at_base);
				//-----------back
				float dist = VLEN(ship_anchor_pos(anchor_id, 1) - sp2);
				speed = dist > 20 ? 0.25 : dist / 20 * 0.25;//VLEN(dir_a)/5;
				if(VLEN(dir_b) > 0.5)CMb.move(sp2, dir, -norm(dir_b), 0.5, 0.5, false);else  CMb.move(sp2, dir, norm(ship_anchor_pos(anchor_id, 1) - sp2), speed, 0.5, false);
				//-----------nose
				dist = VLEN(ship_anchor_pos(anchor_id, 0) - sp1);
				speed = dist > 20 ? 0.25 : dist / 20 * 0.25;
				//target=ship_anchor_pos(id,0);
				//t_dir=norm(target-sp1);
				//GOTO(ship_anchor_pos(id,0));
				if(VLEN(dir_a) > 0.5)CMn.move(sp1, dir, -norm(dir_a), 0.5, 0.5, false);else CMn.move(sp1, dir, norm(ship_anchor_pos(anchor_id, 0) - sp1), speed, 0.5, false);
			};
		};
		if(slow_down || action == SA_STOP){
			CMn.move(sp1, dir, -norm(dir_a), VLEN(dir_a)*0.8, 0.5, false);
			CMb.move(sp2, dir, -norm(dir_b), VLEN(dir_b)*0.8, 0.5, false);
		};
		//if(action==SA_STOP){//&& id>0
		// CMn.move(sp1,dir,norm(get_pos(anchor,50,true )-sp1),clamp(VLEN(get_pos(anchor,50,true )-sp1)/50,0.01,1),0.5,false);
		// CMb.move(sp2,dir,norm(get_pos(anchor,50,false)-sp2),clamp(VLEN(get_pos(anchor,50,false)-sp2)/50,0.01,1),0.5,false);
		//};
		//------------------------------------------
		CMn.off_used();
		CMb.off_used();
	};
	void DrawHelpersTarget(){
		if(action == SA_STOP){//&& id>0
			glColor4f(0, 1, 1, 0.5);
			DrawShip(get_pos(anchor, 50, true), get_pos(anchor, 50, false), GL_LINE_LOOP);
		};
		if(action == SA_AUTOPILOT || action == SA_GOTO_BASE || action == SA_GOTO || action == SA_FOLLOW){
			//--------------------------------------draw target
			glPointSize(10);
			glLineWidth(2);
			vec3f S = sp1, T = target, R = T, CN;
			int tryes = 0;
			glColor3f(0, 1, 1);                                  //|| CollideShips(idx,70,S,T,R,CN)
			while(tryes < 50 && (COLCRV.collide(S, T, R, CN))){
				tryes++;//------------------------------------======================================-------------------
				//glColor3f(1,0,0);glBegin(GL_POINTS);glVertex3fv(R);glEnd();
				glBegin(GL_LINES);glVertex3fv(S);glVertex3fv(R + CN);glEnd();//glVertex3fv(R);glVertex3fv(T);
				S = R + CN;S.y = 0;
			};
			//if(tryes)glBegin(GL_LINES);glVertex3fv(S);glVertex3fv(T);glEnd();
			if(LineSide(target, sp2, sp1)){ glColor3f(0, 0, 1); } else{ glColor3f(1, 0, 0); };//set right(red) or left(blue) color
			glBegin(GL_LINES);                                                      //       -PI/5                -PI/5
			 //float ship_angle=atan2(dir.z,dir.x);if(LineSide(target,sp2,sp1))ship_angle+=PI/2;else ship_angle-=PI/2; //if(LineSide(target,sp2,sp1))right else left
			 //vec3f hpa(cos(ship_angle),0,sin(ship_angle));
			 //glVertex3fv(sp1);glVertex3fv(sp1-hpa*5);
			 //glVertex3fv(sp2);glVertex3fv(sp2+hpa*5);
			glVertex3fv(target);glVertex3fv(sp1 - t_dir * len);//target-nose_coil
			glVertex3fv(tmp_target);glVertex3fv(sp1 - t_dir * len);
			if(DEV_MODE){
				glVertex3fv(sp1);glVertex3fv(sp1 + t_dir * VLEN(sp1 - target));
				glVertex3fv(sp2);glVertex3fv(sp2 + sideV * VLEN(cross(dir, t_dir)) * 40);//40);//ass-sideV
				CMn.draw(sp1);
				CMb.draw(sp2);
			};
			glEnd();
		};//---------------------------------------  end draw SA_GOTO
	};
	void setup(int num, bool skeak = true){
		crv_pos = -1;crv_num_pos = -1;
		n_off = -1, a_off = -5;
		id = -1;
		float div = 1.0 / 2.0;
		cfg = ship_configs.get(num);
		if(cfg != nullptr){
			id = num;
			n_off = cfg->nose_off * -div;
			a_off = cfg->tail_off * -div;
		}
		//switch(num){
		//	case 1:id = 1;n_off = -15 * div, a_off = -4 * div;break;//46  торговое судно, от петра до конца 18
		//	case 2:id = 2;n_off = -16 * div, a_off = -8 * div;break;//59  фрегат Россия 1724 32-пуш  
		//	case 3:id = 3;n_off = -27 * div, a_off = -16 * div;break;//82  линейный Гавриил 1713 66-пуш
		//	case 4:id = 4;n_off = -25 * div, a_off = -13 * div;break;//74  Евстафий Плакида 1763 66-пуш 
		//	case 5:id = 5;n_off = -22 * div, a_off = -12 * div;break;//55  Флейт торговый, купцов Баженных 1724
		//	case 6:id = 6;n_off = -17 * div, a_off = -8 * div;break;//47  яхта Диана
		//	case 7:id = 7;n_off = -19 * div, a_off = -7 * div;break;//55  тороговое 3
		//};
		if(id < 1){ CON.write(2, "false setup for ship[%i]", idx);return; };
		//if(skeak)SND.Play(snd_found);
		//marker_send(id,MARKER_LIGHT_ON);
		marker_send(id, MARKER_FOUND);
		marker_send(id, MARKER_SOUND);
		//marker_send(id,MARKER_FREQ);
		//set_action(SA_AUTOPILOT);
		read_sbr();
		if(GTbase_btn && GTbase_btn->Down) {action=SA_GOTO_BASE;next_action=SA_GOTO_BASE;}
		//set_table(id, idx, " PWR=? ");
		//UIRead_ship_state(idx);
	};
	void read_sbr(){
		if(id > 0){
			if(cfg!=nullptr){
				action = cfg->brain.action;
				target_speed = cfg->brain.speed;
				target = cfg->brain.target;
				crv_id = cfg->brain.crv_id;
				////crv_pt_id=SM??get_crv_pt_id(ship[i].crv_id,i);
				follow_id = cfg->brain.follow_id;
			}
		} else{
			action = SA_AUTOPILOT;
			speed = 0.5;
		};
		CON.write(0, "read from SBR[%i] |speed=%f crv_id=%i follow_id=%i action=%s|", id, speed, crv_id, follow_id, action2str(action));// 
	};
	void follow(int fid){
		follow_id = fid;if(id > 0 && cfg)cfg->brain.follow_id = fid;
		set_action(SA_FOLLOW);
		CON.write(0, "ship.id=%i will follow %i", id, fid);
	};
};
//=====================================================================================
//=====================================================================================
const int INF = 1000 * 1000 * 1000;
//-------------------------------------------------------------------------------------
struct VALG2{
	int w, h, memh, memw;
	float **m, *u, *v, *mins;
	int *links, *visited, *res;
	bool swap;
	VALG2(){
		m = NULL, u = v = mins = NULL;
		res = links = visited = NULL;
		w = h = memh = memw = 0;
		swap = false;
	};
	~VALG2(){
		loop0i(h)DEL_(m[i]);
		DEL_(m);
		DEL_(u);DEL_(v);DEL_(mins);
		DEL_(links);DEL_(visited);DEL_(res);
	};
	void prep(int nw, int nh){
		swap = (nw < nh);
		if(swap){ int t = nw;nw = nh;nh = t; };
		bool resz = false;
		if(nh > memh){
			memh = nh;
			RESIZE(m, float*, memh);loopi(h, memh)m[i] = NULL;resz = true;
			RESIZE(u, float, memh);
		};
		if(nw > memw){
			memw = nw;
			resz = true;
			RESIZE(v, float, memw);RESIZE(mins, float, memw);
			RESIZE(res, int, memw);RESIZE(links, int, memw);RESIZE(visited, int, memw);
		};
		if(resz)loop0i(memh){ RESIZE(m[i], float, memw); };
		w = nw, h = nh;
		loop0i(w){
			res[i] = -1;
			v[i] = 0.0;
			//loop0j(h)m[j][i]=VLEN(a[i]-b[j]);
		};//loop(w)
	};
	void solve(){//w>=h  int nw,vec2f *a,int nh,vec2f *b
		loop0i(h){
			u[i] = 0.0;
			loop0k(w) links[k] = -1, mins[k] = INF, visited[k] = 0;
			int markedI = i, markedJ = -1, j;
			while(markedI != -1){
				j = -1;
				loop0k(w)if(!visited[k]){
					if(m[markedI][k] - u[markedI] - v[k] < mins[k]){
						mins[k] = m[markedI][k] - u[markedI] - v[k];
						links[k] = markedJ;
					};
					if(j == -1 || mins[k] < mins[j]) j = k;
				};
				int delta = mins[j];
				loop0k(w) if(visited[k]){ u[res[k]] += delta;v[k] -= delta; } else{ mins[k] -= delta; }
				u[i] += delta;
				visited[j] = 1;
				markedJ = j;
				markedI = res[j];
			};//while()
			for(; links[j] != -1; j = links[j]) res[j] = res[links[j]];
			res[j] = i;
		};//loop(h)
	};
	////-------------------------------------------------------------------------------------------- USAGE::VALG2
};
VALG2 VAL;
//=====================================================================================
struct ShipMan{
	float flen;
	int sel;
	word ms;
		Ship *ship;
	ShipMan(){ init(); };
	void init(){ ms = 0;ship = NULL;flen = 30;sel = -1; };
	bool used_pt(int a){//if(TM.T[a].lost())return false;
		for(int i = 0;i < ms;i++){                                     //points used - if not to recycle or lost state
			if((ship[i].nose_id == a || ship[i].back_id == a) && ship[i].state != SS_RECYCLE && ship[i].state != SS_LOST){ return true; };
		};                                                             //>=SS_UNDEF
		return false;
	};
	int add_get(int a, int b){
		float sl = VLEN(TM.pos(a) - TM.pos(b));if(sl > 110 || sl < 25){ return -1; };//bad size
		//reuse
		for(int i = 0;i < ms;i++){//if(ship[i].id>0)continue;
			if(ship[i].state == SS_RECYCLE){ CON.write(0, "SHIP::REUSE %i", i);ship[i].init(a, b, i);return i; };
		};
		//add
		CON.write(0, "SM::add ship %i", ms);
		ms++;
		RESIZE(ship, Ship, ms);
		ship[ms - 1].init(a, b, ms - 1);
		return ms - 1;
	};
	bool grap(int num){ return (sel == num && mouse[0]); };
	bool in_pie(int sid, vec3f p, vec3f d, float r, float pyr){
		for(int i = 0;i < ms;i++){
			if(ship[i].state == SS_RECYCLE || i == sid)continue;
			if(PinsidePie(ship[i].sp1, p, d, r, pyr*D2R))return true;
			if(PinsidePie(ship[i].sp2, p, d, r, pyr*D2R))return true;
		};
		return false;
	};
	void draw_collision(int i){//if(!DEV_MODE)return; i moves about j
		Ship *s = &ship[i];if(!s)return;
		glLineWidth(3);glPointSize(10);glColor3f(1, 0, 1);
		//float search_len=90;
		//glDrawCircle(vec3f(0,0,150),vec3f(150,0,0),(s->sp1+s->sp2)*0.5);
		glDrawCircle(vec3f(0, 0, safety_rad), vec3f(safety_rad, 0, 0), (s->sp1 + s->sp2)*0.5);
		//--------------------- direction pie
		vec3f tdir = norm(s->t_dir);
		bool can_collide = in_pie(i, s->sp1, tdir, safety_rad, 30);
		glColor4f(can_collide, 0, !can_collide, 0.5);
		glDisable(GL_CULL_FACE);
		glBegin(can_collide ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
		glVertex3fv(s->sp1);glVertex3fv(s->sp1 + tdir.rotY(30 * D2R)*safety_rad);
		glVertex3fv(s->sp1 + tdir * safety_rad);
		glVertex3fv(s->sp1 + tdir.rotY(-30 * D2R)*safety_rad);
		glEnd();
		//if(can_collide)
		return;
		//---------------------
		vec3f S1(s->center), D1(s->dir), S2, D2;
		for(int j = 0;j < ms;j++){
			if(ship[j].state == SS_RECYCLE || j == i || ship[j].len < 10)continue;// || ship[j].id<1
			S2 = ship[j].center, D2 = ship[j].dir;
			if(VLEN(S1 - S2) < 150){
				//vec3f CN=norm(S1-S2),CT=norm(cross(CN,vec3f(0,1,0)));
				//vec3f T1=dot(D1,CT)<0?S1+CT*90:S1-CT*90, T2=dot(D2,-CT)<0?S2-CT*90:S2+CT*90;
				glColor3f(1, 0, 1);glBegin(GL_LINES);glVertex3fv(S1);glVertex3fv(S2);glEnd();// glVertex3fv(S1);glVertex3fv(T2);glVertex3fv(S2);glVertex3fv(T1);
				//glColor3f(1,0,1);glBegin(GL_POINTS);glVertex3fv(T1);glVertex3fv(T2); glEnd();//CN tan points
				//glColor3f(1,0,0.5);glBegin(GL_POINTS);glVertex3fv(S2+norm(s->sideV)*90);glVertex3fv(S2-norm(s->sideV)*90); glEnd();//dir tan points
				//if(dot(CN,D1)<0)s->GOTO(S1+reflect(CN,D1)*150);
				//if(dot(-CN,D2)<0)ship[j].GOTO(S2+reflect(-CN,D2)*150);
				//IP
				//float ang=angle(D1,D2,vec3f(0,1,0))*R2D;//-PI/2;
				////if(key['r'] && s->id==1)CON.write(2,"angle=%.3f dot=%.3f",ang,dot(D1,D2));// ipd=%.3f",ipd);
				// ray r1(S1,D1), r2(S2,D2);
				// vec3f IP=r1.ip(r2);
				//if(ang>30 && ang<150 && dot(IP-S1,D1)>0 && dot(IP-S2,D2)>0){// && ipd>0
				// float d1=VLEN(IP-S1), d2=VLEN(IP-S2);
				// if(d1<d2)glColor3f(1,0,0);else glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(S1);glVertex3fv(IP);glEnd();
				// if(d1>d2)glColor3f(1,0,0);else glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(S2);glVertex3fv(IP);glEnd();
				//};
				////------------draw vel intersection point
				//ray r1(CBn,norm(ship[j].dir)), r2(TM.pos(ship[i].nose_id),norm(ship[i].dir));
				//vec3f IP=r1.ip(r2);
				//glColor3f(1,0,0);
				//glBegin(GL_LINES);glVertex3fv(r1.p);glVertex3fv(IP);glEnd();
				//glBegin(GL_LINES);glVertex3fv(r2.p);glVertex3fv(IP);glEnd();
				//---------------------------
				//ottalkivanie 
				//glBegin(GL_LINES);glVertex3fv(s->sp1);glVertex3fv(s->sp1+CN*50);glEnd();
				//glBegin(GL_LINES);glVertex3fv(ship[j].sp1);glVertex3fv(ship[j].sp1-CN*50);glEnd();
				//reflect (not work properly)
				//glColor3f(0,1,0);
				// glBegin(GL_LINES);glVertex3fv(TM.pos(ship[i].nose_id));glVertex3fv(TM.pos(ship[i].nose_id)+reflect( CN,ship[i].dir)*50);glEnd();
				// glBegin(GL_LINES);glVertex3fv(TM.pos(ship[j].nose_id));glVertex3fv(TM.pos(ship[j].nose_id)+reflect(-CN,ship[j].dir)*50);glEnd();
				//ship[i].GOTO(TM.pos(ship[i].nose_id)+CN*50);       t_dir-CN*dot(CN,t_dir)   V+=N*-dot(N,V);
				 //if(ship[j].action!=SA_STOP)ship[j].GOTO(ship[j].sp1-CN*50);//MOVE AWAY J
				// vec3f ND(norm(ship[j].t_dir));
				// if(ship[j].action!=SA_STOP)ship[j].GOTO(ship[j].sp1 + (ND+CN*-dot(CN,ND))*50);//SLIDE AWAY J
				//float angj=atan2(ship[j].dir.z,ship[j].dir.x);//angle(norm(CA-CB),norm(ship[j].dir),vec3f(0,1,0));
				//float angjtoi=NormDeg(atan2(CN.z, CN.x)*R2D-atan2(CDIR.z, CDIR.x)*R2D)*D2R;//angle from j to i
				//float angjtoi2=atan2(CN.z, CN.x);
				//vec3f RV1=CB+vec3f(cos(angjtoi2+30*D2R),0,sin(angjtoi2+30*D2R))*90,
				//      RV2=CB+vec3f(cos(angjtoi2       ),0,sin(angjtoi2       ))*90,
				//      RV3=CB+vec3f(cos(angjtoi2-30*D2R),0,sin(angjtoi2-30*D2R))*90;//RotateAroundVector(vec3f(60,0,0),CB,ang+PI/2);//*60;
				//PrTx.Add(pl->Project(CB+vec3f(0,0,9)),"----angle=[%.0f]----",angjtoi*R2D);
				//if(ang>0.5){
				 //glColor3f(0,1,0);glBegin(GL_LINES);glVertex3fv(s->nose_coil());glVertex3fv(s->nose_coil()+s->dir*30);glEnd();//infront
				//};
												//y-red           //n-blue
				//if(fabs(angjtoi*R2D)<30){glColor3f(1,0,0);//else glColor3f(0,0,1);//i infront j
				// if(ship[j].action!=SA_ANCHOR)ship[j].GOTO(TM.pos(ship[j].nose_id)-CN*50);//MOVE AWAY J
				// glBegin(GL_LINES);//cone
				//  glVertex3fv(RV1);glVertex3fv(RV2);
				//  glVertex3fv(RV2);glVertex3fv(RV3);
				//  glVertex3fv(CB);glVertex3fv(RV1);
				//  glVertex3fv(CB);glVertex3fv(RV3);
				// glEnd();
				//};
				//float angi=atan2(ship[i].dir.z,ship[i].dir.x);
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA+vec3f(cos(angi+PI  ),0,sin(angi+PI  ))*90);glEnd();//CB to CA tail
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA+vec3f(cos(angi     ),0,sin(angi     ))*90);glEnd();//CB to CA nose
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA+vec3f(cos(angi+PI/2),0,sin(angi+PI/2))*60);glEnd();//CB to CA left side
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA+vec3f(cos(angi-PI/2),0,sin(angi-PI/2))*60);glEnd();//CB to CA left side
				//left & right
				//vec3f sideV=cross(CN,vec3f(0,1,0)) * (dot(sideV,ship[i].dir)<0?1:-1);// side * dot - turn to back
				//glColor3f(1,0,0);glBegin(GL_LINES);glVertex3fv(CA);glVertex3fv(CA+sideV*90); glEnd();//back
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA+sideV*90);glEnd();//to back
				//glBegin(GL_LINES);glVertex3fv(CBn);glVertex3fv(CA-sideV*90);glEnd();//to nose
			   //ship[j].GOTO(CA+sideV*90);
				int sr = ship[i].closest_ship_relation;
				PrTx.Add(vec3f(0.0), 1, pl->Project(lerp(S1, S2, 0.3)), "[%s]",
					sr == 0 ? "none" :
					sr == 1 ? cstring("ahead %i -slow_down", ship[j].id).text :
					sr == 2 ? "collision X_X -waiting" : sr == 3 ? "ok" :
					sr == 4 ? cstring("behind %i", ship[j].id).text :
					sr == 5 ? "O_o -moving" : "unknown");
			};
		};
	};
	void clear_all_votes(){ for(int i = 0;i < ms;i++){ ship[i].vote = 0;ship[i].n_led_event = 0, ship[i].b_led_event = 0; }; };
	void reassign_anchors(){
		int cnt = 0;loop0i(ms){ if(ship[i].state >= SS_UNDEF){ cnt++; }; };
		VAL.prep(MAX_SHIPS, cnt);
		loop0i(MAX_SHIPS){
			cnt = 0;
			loop0j(ms)if(ship[j].state >= SS_UNDEF){
				if(VAL.swap){
					VAL.m[i][cnt++] = VLEN(ship_anchor_pos(1 + i) - ship[j].sp1);
				} else{//--------------- common
					VAL.m[cnt++][i] = VLEN(ship_anchor_pos(1 + i) - ship[j].sp1);
				};
			};
		};//loop(w)
		VAL.solve();
		bool rep = key['r'];
		if(rep){
			CON.write(2, "VAL:w=%i,h=%i,swap=%i", VAL.w, VAL.h, VAL.swap);
			loop0i(VAL.w)CON.write(2, "res[%i]=%i", i, VAL.res[i]);
		};
		if(VAL.swap){
			cnt = 0;
			loop0i(ms){
				if(ship[i].state < SS_UNDEF)continue;
				loop0j(MAX_SHIPS)if(VAL.res[i] != -1)ship[cnt++].anchor_id = 1 + VAL.res[i];
			};
		} else{//--------------- common
			loop0i(MAX_SHIPS){//w
				cnt = -1;
				int cnt2 = 0;
				loop0j(ms){
					if(ship[j].state < SS_UNDEF)continue;
					if(cnt2 == VAL.res[i]){ cnt = j;break; };
					cnt2++;
				};//h
				if(cnt != -1)ship[cnt].anchor_id = 1 + i;
			};
		};//else swap
		//-----------report
	};
	void Update(const vec3f &IP){
		//----------- update curve num ships
		for(int i = 1;i < TrajMan.num;i++){ TrajMan.get(i)->num_ships = 0; };
		for(int i = 0;i < ms;i++){
			if(ship[i].state < SS_UNDEF)continue;
			if(ship[i].crv_id > 0 && ship[i].crv_id < TrajMan.num){ TrajMan.get(ship[i].crv_id)->num_ships++; };//ship[i].crv_len_to_next=CRV[ship[i].crv_id].clen;
			//loop0j(ms){if(ship[j].state<SS_UNDEF || j==i)continue;
			 //if(CRV[ship[i].crv_id].distance(CRV[ship[i].crv_id].closest_point_mem(ship[i].sp1,ship[i].crv_pos)))
			//};
		};
		//----------------------------------
		bool warn_collision = false;
		for(int i = 0;i < ms;i++){
			Ship &shp = ship[i];
			if(ship[i].state == SS_RECYCLE)continue;
			ship[i].closest_ship_dist = 999;
			int cs = -1;
			for(int j = 0;j < ms;j++){
				if(i != j && ship[j].state != SS_RECYCLE && ship[i].closest_ship_dist > VLEN(ship[i].center - ship[j].center)){
					//do not warn if same ship
					if(ship[i].id > 0 && ship[i].id == ship[j].lid && ship[j].id == -1)continue;
					if(ship[j].id > 0 && ship[j].id == ship[i].lid && ship[i].id == -1)continue;
					ship[i].closest_ship_dist = VLEN(ship[i].center - ship[j].center);cs = j;
				};
			};
			if(cs != -1){
				float aa = VLEN(ship[i].sp1 - ship[cs].sp1),
					ab = VLEN(ship[i].sp1 - ship[cs].sp2),
					ba = VLEN(ship[i].sp2 - ship[cs].sp1),
					bb = VLEN(ship[i].sp2 - ship[cs].sp2);
				ship[i].closest_ship_relation = min(aa, ab) < min(bb, ba) ? (ab < aa ? 1 : ship[i].id < ship[cs].id ? 2 : 5) : (bb < ba ? 3 : 4);
				ship[i].closest_ship_relation = in_pie(i, ship[i].sp1, ship[i].t_dir, safety_rad, 30) ? 2 : 0;
				//1: i follow j
				//2: collision X_X
				//3: ok
				//4: j follow i
				//5: pass         //warn time?                                 CON.write(2,"%i:warn_collision=true",i);
				if((aa < 40 || ab < 40 || bb < 40 || ba < 40) && ship[i].tottaly_lost_reminder()){ warn_collision = true;CON.write(2, "ship[%i].%i::warn_collision=true", i, ship[i].id); };
			};
			//----------------return moving near ships after lost one
			if(ship[i].action == SA_WAIT){
				bool ready = true;for(int j = 0;j < ms;j++){ if(i != j && ship[j].id < 1 && ship[j].state != SS_RECYCLE){ ready = false;break; }; };
				if(ready){ clear_all_votes();ship[i].set_action(ship[i].next_action);CON.write(0, "ALL SHIPs found:: ship[%i].id=%i - continue [%s]", i, ship[i].id, action2str(ship[i].action)); };
			};                               //next - auto|follow
			//---------------if ONE-lost and ONE-undef and rad < 100 - qset
			//if(ship[i].state==SS_UNDEF){
			// int num_lost=0,lid=-1,num_undef=0,lsh=0;
			// for(int j=0;j<ms;j++){if(ship[j].state==SS_LOST){num_lost++;lid=ship[j].lid;lsh=j;};if(ship[j].state==SS_UNDEF){num_undef++;};};
			// if(num_lost==1 && num_undef==1 && lid>0){//  && VLEN(ship[i].sp1-ship[lid].sp1)<150
			//  num_lost=0;
			//  for(int j=0;j<ms;j++){if(ship[j].id==lid){num_lost++;};};//no lid id already
			//  if(!num_lost){
			//   CON.write(2,"QSET:: ship[%i].lid=%i -> ship[%i].lid=%i",lsh,lid, i,ship[i].lid);
			//   ship[i].setup(lid,false);clear_all_votes();ship_detection=false;CON.write(2,"DETECT_END::Qset"); //ship[i].follow_id=ship[lsh].follow_id;  fixed
			//  };
			// };
			//};
			//---------------if follower is lost - wait
			if(ship[i].action == SA_FOLLOW){
				if(ship[i].follow_id < 1)ship[i].action = SA_AUTOPILOT;else{
					bool flost = true;for(int j = 0;j < ms;j++){ if(i != j && ship[j].id > 0 && ship[j].state != SS_RECYCLE && ship[i].follow_id == ship[j].id){ flost = false;break; }; };
					if(flost){ ship[i].set_action(SA_WAIT);CON.write(2, "LOST FOLLOWED SHIP:: ship[%i].id=%i - wait for followed ship.id[%i]", i, ship[i].id, ship[i].follow_id); };
				};
			};
			//--------------start votes
			if(ship[i].id < 1 && !ship_detection){ clear_all_votes();ship_detection = true;CON.write(0, "REstart detection for ship[%i]", i); };//start search, clear votes
			//if(ship_detection && ship[i].LED_MODE==LM_BLINK){ship[i].vote++;};//blinked_ship=i;bs_num++;};//count votes to prevent wrong id
			if(ship_detection && TM.T[ship[i].nose_id].blinking < 0.9){//---------------------------voter
				if(ship[i].n_led_event != TM.T[ship[i].nose_id].levent && ship[i].b_led_event != TM.T[ship[i].back_id].levent){
					if(ship[i].n_led_event)ship[i].vote++;//if not zero count votes
					ship[i].n_led_event = TM.T[ship[i].nose_id].levent, ship[i].b_led_event = TM.T[ship[i].back_id].levent;//copy state
				};
			};
			if(shp.len <= 0)continue;

			if(grap(i) && lmouse[0]){ 
				UIRead_ship_state2(i);
			};
			bool pin = PinsideShip(pl->msdir.p, ship[i].sp1, ship[i].sp2);
			if(sel == i && !mouse[0] && lmouse[0] && !pin){//---------- set follower ship
				for(int j = 0;j < ms;j++){
					if(ship[j].id < 1 || j == i)continue;//follow only detected ships   loop follow?
					if(PinsideShip(IP, ship[j].sp1, ship[j].sp2)){
						int fid = ship[j].id, res = 0;                                              //warning loop
						for(int k = 0;k < ms;k++){ if(ship[k].follow_id == fid && k != j && k != i){ fid = ship[k].id;if(res < 10)k = 0;res++; }; };
						ship[i].follow(fid);
					};//ship[i].follow(ship[j].id);};//
				};
			};//----------
			if(grap(i) && !pin){
				ship[i].action = SA_GOTO;ship[i].GOTO(IP);ship[i].crv_num_pos = -1;ship[i].crv_pos = -1;//ship[i].crv_pt_id=-1;
				//---------- set closest curve to mouse 
				int cc = -1;
				float cdist = 20;
				vec3f last_ccp;
				for(int j = 1;j < TrajMan.num;j++){// MAX_CRV
					vec3f ccp = TrajMan.get(j)->closest_point(IP);
					float cd = VLEN(ccp - IP);
					if(cd < cdist){ cdist = cd;cc = j; last_ccp=ccp;};
				};
				ship[i].set_traj_crv(cc);
				if(cc > 0){
					TrajMan.get(cc)->highlight = true;
					ship[i].next_action = SA_AUTOPILOT;
					ship[i].GOTO(last_ccp);
					//TrajMan.get(ship[i].crv_id)->closest_point_mem(last_ccp, ship[i].crv_pos);//   set new curve pos, closest to mouse
				} else{ ship[i].next_action = SA_STOP; };// CON.write(2,"use curve %i",cc);
			} else{
				if(ship[i].action == SA_AUTOPILOT){//follow curve    && ship[i].id>0
					if(TrajMan.isValid(ship[i].crv_id) == false){
						CON.write(0,"ship[i].crv_id = %i",ship[i].crv_id, TrajMan.num);
						ship[i].set_traj_crv(get_closest_curve(i, ship[i].sp1));
						//for(int j=0;j<ms;j++){if(ship[j].id<1 || j==i || ship[j].crv_id<1)continue;//same curve autofollower
						// if(ship[j].crv_id==ship[i].crv_id && ship[j].action==SA_AUTOPILOT){
						//  int fid=ship[j].id, res=0;                                              //warning loop
						//  for(int k=0;k<ms;k++){if(ship[k].follow_id==fid && k!=j && k!=i){fid=ship[k].id;if(res<10)k=0;res++;};};
						//  ship[i].follow(fid);
						//  break;
						// };
						//};
					};            //recheck
					if(TrajMan.isValid(ship[i].crv_id))
						ship[i].GOTO(TrajMan.get(ship[i].crv_id)->closest_point_mem(ship[i].sp1, ship[i].crv_pos));
				};//end autopilot
				if(ship[i].action == SA_FOLLOW && ship[i].id > 0){
					int fid = get_ship_idx_by_id(ship[i].follow_id);      //50
					if(fid >= 0)ship[i].GOTO(ship[fid].sp2 - ship[fid].dir * 30);
				};//end follow
			};
			ship[i].update_CC();
		};
		if(warn_collision)SND.Play(snd_collision);
	};
	void Draw(){// DrawInfo();
		if(!ms)return;
		glDisable(GL_TEXTURE_2D);
		int pin = mouse[0] ? sel : -1;
		for(int i = 0;i < ms;i++){
			if(ship[i].state == SS_RECYCLE)continue;
			if(ship[i].state == SS_LOST || !ship[i].set_nose()){
				glColor3f(1, 0, 0);glLineWidth(4);//red       DRAW RIPPLES of lost ship
				DrawShip(ship[i].sp1, ship[i].sp2, GL_LINE_LOOP);
				glLineWidth(2);
				for(int j = 0;j < 5;j++) DrawShip(ship[i].sp1, ship[i].sp2, GL_LINE_LOOP, 1 + fmodf((::time() - ship[i].ltime) + (j * 2), 10));
				for(int j = 0;j < ms;j++){
					if((ship[j].id == ship[i].lid || ship[i].lid < 1) && ship[i].state != SS_RECYCLE){
						ship[i].state = SS_RECYCLE;clear_all_votes();ship_detection = false;CON.write(2, "DETECT_END");CON.write(1, "RELEASE %i lid=%i", i, ship[i].lid);
					};
				};//remove alarm message
			  //if nothing helps then alert LOST SHIP
				if(ship[i].state == SS_LOST && ship[i].tottaly_lost_reminder()){ SND.Play(snd_lost);CON.write(2, "SHIP[%i].%i still not found", i, ship[i].lid); };
				//if(ship[i].timer_time()>10){ship[i].timer_reset();SND.Play(snd_lost);CON.write(2,"SHIP[%i].%i still not found",i,ship[i].lid);};
				continue;
			};//get len & dir - if lost - reset
			if(ship[i].vote > 0){
				glColor3f(0, 1, 0);glLineWidth(1);
				for(int j = 0;j < 5;j++) DrawShip(ship[i].sp1, ship[i].sp2, GL_LINE_LOOP, 1 + 10 - fmodf((::time() - ship[i].ltime) + (j * 2), 10));
			};
			if(!lmouse[0] && (PinsideShip(pl->msdir.p, ship[i].sp1, ship[i].sp2) || grap(i)))pin = i;
			draw_collision(i);
			if(ship[i].id < 1)glColor4f((pin == i ? 1 : 0.5), 0, 0, 0.5);
			else glColor4f((selShipID() == ship[i].id ? (pin == i ? 1 : 0.5) : 0), (pin == i ? 1 : 0.5), 0, 0.5);//, 0.7);
			DrawShip(ship[i].sp1, ship[i].sp2);
			//------------- blocker
			if(DEV_MODE && SA_GOTO_BASE == ship[i].action){
				glColor3f(1, 0, 0);
				glBegin(GL_LINES);
				//------------------------- gotobase
				reassign_anchors();
				glVertex3fv(ship[i].sp1);glVertex3fv(ship_anchor_pos(ship[i].anchor_id));
				//SM_line_excluder(&ship[i].CMn,&ship[i].CMb,i,true);
				for(int j = 0;j < ms;j++){
					if(i == j || ship[j].state == SS_RECYCLE)continue;
					vec3f c1(ship[i].center), c2(ship[j].center);
					vec3f dir(c2 - c1);
					if(VLEN(dir) > 100)continue;dir = norm(dir);
					vec3f tan = norm(cross(dir, vec3f(0, 1, 0)));
					glVertex3fv(c1 + tan * 60 + dir * 10);glVertex3fv(c1 - tan * 60 + dir * 10);
					glVertex3fv(c1 - tan * 60 + dir * 10);glVertex3fv(c1 + dir * 50);
					if(LineSide(c1 + dir * 50, c1 + tan * 60 + dir * 10, c1 - tan * 60 + dir * 10))glColor3f(1, 0, 0);else glColor3f(0, 0, 1);
					glVertex3fv(c1);glVertex3fv(c1 + dir * 50);
				};
				glEnd();
			};
			//------------- end blocker
			if(ship[i].motion == SM_STUCK){
				glColor3f(1, 0.5, 0);
				for(int j = 0;j < 5;j++) DrawShip(ship[i].sp1, ship[i].sp2, GL_LINE_LOOP, 1 + fmodf((::time() - ship[i].ltime) + (j * 2), 10));
			};
			ship[i].DrawHelpersTarget();
			//force vector
			//PrTx.Add(pl->Project(a+vec3f(0,0,5)),"dot(dir,target)=%f",dot(ship[i].dir_a,ship[i].t_dir));
			//PrTx.Add(pl->Project(a+vec3f(0,0,5)),"(%i)[%i]",TM.T[p1].id,TM.T[p1].rate);
			//PrTx.Add(pl->Project(b+vec3f(0,0,5)),"(%i)[%i]",TM.T[p2].id,TM.T[p2].rate);
			glLineWidth(4);//FORCE VECTORS
			glColor3f(1, 0, 0);
			glBegin(GL_LINES);
			glVertex3fv(ship[i].sp1);glVertex3fv(ship[i].sp1 + ship[i].dir_a * 10);
			glVertex3fv(ship[i].sp2);glVertex3fv(ship[i].sp2 + ship[i].dir_b * 10);
			glColor3f(1, 0, 1);
			glVertex3fv(ship[i].sp1);glVertex3fv(ship[i].sp1 + ship[i].acc_a * 10);
			glVertex3fv(ship[i].sp2);glVertex3fv(ship[i].sp2 + ship[i].acc_b * 10);
			// glColor3f(1,0,1);
			  //glVertex3fv(TM.T[p1].kest);glVertex3fv(vec3f(0,0,0));
			  //glVertex3fv(TM.T[p1].kpred);glVertex3fv(vec3f(0,0,0));
			 //glVertex3fv(a);glVertex3fv(a+dir_n(i)*10);
			glEnd();
			int closest_sid = -1, eq_ip = -1;
			glBegin(GL_POINTS);
			glColor3f(1, 0, 0);//RED
			if(TrajMan.isValid(ship[i].crv_id) == true){//-------------------
				vec3f TP = TrajMan.get(ship[i].crv_id)->closest_point_mem_pos(ship[i].sp1, ship[i].crv_num_pos);
				//glVertex3fv(TP);
				glVertex3fv(TrajMan.get(ship[i].crv_id)->get_point(ship[i].crv_num_pos));
				//---------- get len to next CRV IP
				ship[i].closest_cip = TrajMan.get(ship[i].crv_id)->dist_to_nearest_ip(ship[i].crv_num_pos, ship[i].closest_cip_id);
				//---------- get len to next SHIP                   //ship size
				float min_nsl = TrajMan.get(ship[i].crv_id)->clen, min_ip_dist = 100;
				loop0j(ms)if(j != i && ship[j].state >= SS_UNDEF && ship[i].crv_id == ship[j].crv_id){
					// j - ship on same curve as i
					float c_ip_d = fabs(ship[i].closest_cip - ship[j].closest_cip);
					if(c_ip_d < min_ip_dist && ship[i].closest_cip_id >= 0 && ship[j].closest_cip_id >= 0 && ship[i].closest_cip_id != ship[j].closest_cip_id){
						if(VLEN(TrajMan.get(ship[i].crv_id)->ip[ship[i].closest_cip_id] 
							  - TrajMan.get(ship[j].crv_id)->ip[ship[j].closest_cip_id]) < 1.0f){
							min_ip_dist = c_ip_d;eq_ip = j;
						};
					};
					//----
					float nsl = TrajMan.get(ship[i].crv_id)->dist(ship[i].crv_num_pos, ship[j].crv_num_pos);
					//CON.write(0,"%i) nsl=%f clen=%f closest_sid=%i",i, nsl, TrajMan.get(ship[i].crv_id)->clen,j);
					if(nsl < min_nsl){
						min_nsl = nsl;closest_sid = j; 
					};
				};                                                                                                                        //(((float)min_nsl/ (float)CRV[ship[i].crv_id].opt_dist)-1.)*0.25
				ship[i].crv_len_to_next = min_nsl;                                                                                                                                             // (float)CRV[ship[i].crv_id].num_ships
				PrTx.Add(vec3f(0.0), 0, pl->Project(ship[i].sp1 + norm(ship[i].t_dir) * 50), "dist %.1f to %i [%.1f to ip]", min_nsl, closest_sid>=0 ? ship[closest_sid].id : 0, ship[i].closest_cip);
			};//----------------------------------------
			glVertex3fv(ship[i].sp1);
			glVertex3fv(ship[i].sp2);
			glEnd();
			//--------------- ship collision
			if(eq_ip != -1 && ship[i].action == SA_AUTOPILOT){
				bool slowed_i = ship[i].closest_cip < ship[eq_ip].closest_cip;
				if(slowed_i){ ship[eq_ip].speed = 0.01;/*eq_ip-slow_down*/ } else{ ship[i].speed = 0.01;/*i-slow_down*/ };
				//glColor3f(1, 0.4f, 0);
				glLineWidth(4);
				glBegin(GL_LINES);
				glColor3f(slowed_i, 0, !slowed_i);glVertex3fv(ship[i].sp1);
				glColor3f(!slowed_i, 0, slowed_i);glVertex3fv(ship[eq_ip].sp1);
				glEnd();
			};
			if(DEV_MODE){
				if(Ship_info_cb->Down){
					PrTx.Add(vec3f(0, 0, 0), 1, pl->Project(ship[i].center), "%i:id=%i len=[%.1f] CRV[%i] vel_a(%.2f)acc_a(%.2f) vel_b(%.2f)acc_b(%.2f)", i, ship[i].id, ship[i].len, ship[i].crv_pos, VLEN(ship[i].dir_a), VLEN(ship[i].acc_a), VLEN(ship[i].dir_b), VLEN(ship[i].acc_b));
					if(ship[i].nose_id >= 0 && ship[i].back_id >= 0)PrTx.Add(vec3f(0, 0, 0), 1, pl->Project(ship[i].center - vec3f(0, 0, 20)), "nose_rate=(%i) back_rate=(%i) votes=%i", TM.T[ship[i].nose_id].num_rate, TM.T[ship[i].back_id].num_rate, ship[i].vote);
				}//else PrTx.Add(vec3f(0.0),1,pl->Project(ship[i].center),"[id=%i] vel=%.2f ACT=%s D=%.2f T=%.2f",ship[i].id, VLEN(ship[i].dir_a),action2str(ship[i].action),TM.moved_distance(ship[i].nose_id),ship[i].timer_time());
			} else{
				PrTx.Add(vec3f(0.0), 1, pl->Project(ship[i].center), "%i", ship[i].id);//, action2str(ship[i].action));  :%s
			};
		};
		sel = pin;
		//sel=tsel;if(!DEV_MODE)
	};
	void DrawInfo2d(){
		if(!ship_info_panel || !ship_info_panel->Down)return;
		int nums = 0, ssi[15];for(int i = 0;i < ms;i++){ if(ship[i].state == SS_RECYCLE)continue;ssi[nums] = i;nums++; };
		//sort ships by x
		if(nums > 1){
			for(int i = 0;i < nums - 1;i++){
				for(int j = i + 1;j < nums;j++){
					if(ship[ssi[i]].center.x > ship[ssi[j]].center.x){
						int t = ssi[i];ssi[i] = ssi[j];ssi[j] = t;
					};
				};
			};
		};
		float  width = (float) WndW / (float) nums, brd = 5, height = min(WndH / 6.0f, width*(2.0 / 3.0) - brd);//CON.write(2,"w=%f h=%f - uh=%f",width,WndH/6.0f,height);
		float iw = height * (3.0 / 2.0)*0.5;//,ih=height;
		glEnable(GL_BLEND);glDisable(GL_CULL_FACE); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for(int i = 0;i < nums;i++){//if(ship[i].state==SS_RECYCLE)continue;
			float cx = width * i + width * 0.5;
			//--------- mouse in test
			bool inm = (mx > cx - iw && mx<cx + iw && my>WndH - height + 5 && my < WndH - 5);
			glDisable(GL_TEXTURE_2D);// glDisable(GL_DEPTH_TEST);   glDisable(GL_BLEND);
			//glColor3f(0,0,0.3);
			//glBegin(GL_QUADS);
			// glVertex3fv(pl->UnProj(width*i+brd,height+5));glVertex3fv(pl->UnProj(width*(i+1)-brd,height+5));
			// glVertex3fv(pl->UnProj(width*(i+1)-brd,5));glVertex3fv(pl->UnProj(width*i+brd,5));
			//glEnd();
			//---------line to ship 2d->3d
			glColor3f(0, 0, 0);
			glLineWidth(inm ? 3 : 1);
			glBegin(GL_LINES);
			glVertex3fv(pl->Project(ship[ssi[i]].center));glVertex3fv(vec3f(cx, WndH - height, 0));
			glEnd();
			if(inm){
				glColor3f(0, 1, 0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(cx - iw, WndH - height + 5);
				glVertex2f(cx + iw, WndH - height + 5);
				glVertex2f(cx + iw, WndH - 5);
				glVertex2f(cx - iw, WndH - 5);
				glEnd();
			};
			//ship_pic[ship[ssi[i]].id>0?ship[ssi[i]].id:0].on();
			//int tid = ship[ssi[i]].id;if(tid < 1)tid = ship[ssi[i]].lid;
			int tex_id = ship_configs.Unknown.id;
			if(ship[ssi[i]].cfg!=nullptr) tex_id = ship[ssi[i]].cfg->tex.id;
			glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, tex_id);//ship_pic[tid > 0 ? tid : 0].id);
			if(ship[ssi[i]].id > 0)glColor4f(1, 1, 1, 0.8);else glColor4f(1, 0, 0, 0.8);
			glBegin(GL_QUADS);//bind ship image
			glTexCoord2f(0, 1);glVertex2f(cx - iw, WndH - height + 5);
			glTexCoord2f(1, 1);glVertex2f(cx + iw, WndH - height + 5);
			glTexCoord2f(1, 0);glVertex2f(cx + iw, WndH - 5);
			glTexCoord2f(0, 0);glVertex2f(cx - iw, WndH - 5);
			glEnd();
			//----------text
			BIGfont.set_tex();//BIGfont // font
			glColor3f(0, 0, 0);
			//BIGfont.Print(cx, WndH - height + 20, true, cstring("Заряд - %.1f", SBR[ship[ssi[i]].id].charge));
			BIGfont.Print(cx, WndH - height + 20, true, cstring("скорость - %.2f", VLEN(ship[ssi[i]].dir_a)));
			BIGfont.Print(cx, WndH - height + 40, true, cstring("сила - %.2f", ship[ssi[i]].speed * 100));
			if(ship[ssi[i]].crv_id > 0){
				float smula = (((float) ship[ssi[i]].crv_len_to_next / (float) TrajMan.get(ship[ssi[i]].crv_id)->opt_dist) - 1.0)*0.25;
				//CON.write(0,"crv = %f",ship[ssi[i]].crv_len_to_next);
				//CON.write(0,"id = %i crv_id = %i",ssi[i],ship[ssi[i]].crv_id);
				//CON.write(0,"opt = %f",TrajMan.get(ship[ssi[i]].crv_id)->opt_dist);
				//CON.write(0,"((%f/%f)-1)*0.25 = %f",ship[ssi[i]].crv_len_to_next, TrajMan.get(ship[ssi[i]].crv_id)->opt_dist, smula);
				//BIGfont.Print(cx, WndH - height + 60, true, cstring("near_shp_dst - %.2f", ship[ssi[i]].closest_ship_dist));
				//BIGfont.Print(cx, WndH - height + 80, true, cstring("closest_cip - %.2f", ship[ssi[i]].closest_cip));
				BIGfont.Print(cx, WndH - height + 60, true, cstring("траект - %i", ship[ssi[i]].crv_id));
				//BIGfont.Print(cx, WndH - height + 100, true, cstring("opt - %.2f", TrajMan.get(ship[ssi[i]].crv_id)->opt_dist));
				//BIGfont.Print(cx, WndH - height + 120, true, cstring("cshp - %i", TrajMan.get(ship[ssi[i]].crv_id)->num_ships));
				//BIGfont.Print(cx, WndH - height + 120, true, cstring("cpos - %.2f", ship[ssi[i]].crv_num_pos));
				//BIGfont.Print(cx, WndH - height + 140, true, cstring("cip - %i", ship[ssi[i]].closest_cip_id));
			}
			if(ship[ssi[i]].vote)BIGfont.Print(cx, WndH - height + 80, true, cstring("голос - %i", ship[ssi[i]].vote));
			//if(inm)BIGfont.Print(cx, WndH - height * 0.5, true, id2name(ship[ssi[i]].id));
			const ship_config::config *c = ship[ssi[i]].cfg;
			if(inm)BIGfont.Print(cx, WndH - height * 0.5, true, (c == nullptr) ? "неопределён" : c->name.text);
			BIGfont.Print(cx, WndH - 20, true, cstring("%i:%s", ship[ssi[i]].id, action2str(ship[ssi[i]].action)));
		};
		glDisable(GL_BLEND);glDisable(GL_TEXTURE_2D);
	};
	bool has(int mid){
		if(mid == -1)return false;//??
		for(int i = 0;i < ms;i++){
			if(mid == ship[i].id)return true;
		};
		return false;
	};
	int get_ship_idx_by_id(int qid){
		for(int i = 0;i < ms;i++){
			if(ship[i].state == SS_RECYCLE)continue;
			if(ship[i].id == qid)return i;
		};
		return -1;
	};
};
//=====================================================================================
ShipMan SM;
void trajectory_preset_loader(CPanel *caller){
	TrajMan.load(caller->mul);
}
//=====================================================================================
void SM_reassign_anchors(){ SM.reassign_anchors(); };
void UIRead_ship_state2(int i){
	if(i < 0 || i>=SM.ms)return;
	Ship &shp = SM.ship[i];
	SelMarkEdit->edit_set_val(shp.id);
	SelShipEdit->edit_set_val(shp.idx);
	if(SelMapEdit)SelMapEdit->edit_set_val(shp.crv_id);
	editS->edit_set_val(shp.target_speed * 100);//CON.write(2,"UIRead_ship(%i)_state speed=%.0f",i,SM.ship[i].speed*100);
	//APilot->Down=SM.ship[i].action==SA_GOTO;
};
void SM_circle_excluder(const int &idx){
	Ship *s = &SM.ship[idx];if(!s)return;
	vec3f S1(s->center), S2, CN;
	for(int i = 0;i < SM.ms;i++){
		if(idx == i || SM.ship[i].state == SS_RECYCLE)continue;
		S2 = (SM.ship[i].center);//, CN=S1-S2;
		//if(VLEN(CN)>100)continue;
		for(int k = 0;k < 10;k++){
			if(s->CMn.ccoil[k] < 0)continue;
			vec3f cc = dxf_coils.center(s->CMn.ccoil[k]);
			if(fVLEN2(S1 - cc) < safety_rad && fVLEN2(S2 - cc) < safety_rad && dot(norm(cc - s->sp1), norm(S2 - s->sp1)) > 0)s->CMn.ccoil[k] = -1;
		};//kn
		for(int k = 0;k < 10;k++){
			if(s->CMb.ccoil[k] < 0)continue;
			vec3f cc = dxf_coils.center(s->CMb.ccoil[k]);
			if(fVLEN2(S1 - cc) < safety_rad && fVLEN2(S2 - cc) < safety_rad && dot(norm(cc - s->sp2), norm(S2 - s->sp2)) > 0)s->CMb.ccoil[k] = -1;
		};//kb
	};//j
};
int get_closest_curve(int idx, vec3f sp1){
	int cc = -1;
	float cdist = 9999;
	for(int i = 1;i < TrajMan.num;i++){
		if(TrajMan.get(i)->mv < 2)continue;//-----------
		vec3f ccp = TrajMan.get(i)->closest_point(sp1);
		float cd = fVLEN2(ccp - sp1);
		if(cd < cdist){ cdist = cd;cc = i; };//closest unused
	};
	if(cc == -1){ CON.write(2, "ship_get_closest_curve::can't find curve"); } else CON.write(0, "ship_get_closest_curve:: use curve %i", cc);
	return cc;
};
bool SM_has_id(int id){
	for(int i = 0;i < SM.ms;i++){
		if(SM.ship[i].state == SS_RECYCLE)continue;
		if(SM.ship[i].id == id)return true;
	};
	return false;
};
int SM_num_blinker(){
	int numb = 0;
	for(int i = 0;i < SM.ms;i++){
		if(SM.ship[i].state == SS_RECYCLE)continue;
		if(SM.ship[i].LED_MODE == LM_BLINK)numb++;
	};
	return numb;
};
void all_ship_action(bool act, bool ex_w_f);//FD  hpp
void assign_ship_id_by_votes(int nid){
	int max_v = 0, max_vn = -1;
	for(int i = 0;i < SM.ms;i++){
		if(SM.ship[i].state == SS_RECYCLE)continue;
		CON.write(0, "DETECT:: ship(%i) has %i votes", i, SM.ship[i].vote);
		if(SM.ship[i].vote > max_v){ max_v = SM.ship[i].vote;max_vn = i; };
	};
	if(max_v > 6){
		SM.ship[max_vn].setup(nid);
		ship_detection = false;
		CON.write(0, "DETECT_END::BLINK::setup ship(%i) id=ship_to_detect(%i) has %i votes", max_vn, ship_to_detect, max_v);
		//if all detected - go to autopilot
		bool ready = true;for(int i = 0;i < SM.ms;i++){ if(SM.ship[i].id < 1 && SM.ship[i].state != SS_RECYCLE){ ready = false;break; }; };
		if(ready && !(GTbase_btn && GTbase_btn->Down)){ all_ship_action(true, true);CON.write(0, "no undetected ships - turn on autopilot"); }; // set_ships_closest_curves();
	};
	SM.clear_all_votes();
};
void SM_clear_votes(){ SM.clear_all_votes(); };
void SM_load_sbr(){ for(int i = 0;i < SM.ms;i++){ SM.ship[i].read_sbr(); }; };
void stop_near_ships_on_lost(int lidx){
	for(int i = 0;i < SM.ms;i++){
		if(lidx == i || SM.ship[i].state == SS_RECYCLE)continue;
		if(VLEN(SM.ship[i].center - SM.ship[lidx].center) < 150){
			CON.write(2, "Near lost ship:: stop ship[%i]", i);
			SM.ship[i].set_action(SA_WAIT);
		};
	};
};
bool CollideShips(int idx, float rad, const vec3f &S, const vec3f &T, vec3f &CCP, vec3f &CCN){
	bool intersection = false;
	vec3f cip = T, CP, D = norm(T - S);
	vec2f CP2;
	float min_rad = fVLEN2(T - S), crad = 20;
	for(int k = 0;k < SM.ms;k++){
		if(idx == k || SM.ship[k].state == SS_RECYCLE)continue;
		int seg = 5;
		for(byte i = 0;i < seg;i++){
			//-------get intersection point 
			Dword j = (i + 1) % seg;
			float ang = (float) i / seg * TWOPI;
			vec3f vi = SM.ship[k].center + vec3f(0, 0, rad)*sin(ang) + vec3f(rad, 0, 0)*cos(ang);
			ang = (float) j / seg * TWOPI;
			vec3f vj = SM.ship[k].center + vec3f(0, 0, rad)*sin(ang) + vec3f(rad, 0, 0)*cos(ang);
			vec3f BN = norm(cross(vj - vi, vec3f(0, -1, 0)))*crad;
			if(SegIntersects(vec2f(vi.x + BN.x, vi.z + BN.z), vec2f(vj.x + BN.x, vj.z + BN.z), vec2f(S.x, S.z), vec2f(T.x, T.z), CP2)){
				CP.set(CP2.x, 0, CP2.y);
				float rad = fVLEN2(CP - S);//+norm(BN)*20
				if(rad < min_rad){ min_rad = rad;CCP = CP;CCN = BN;intersection = true; };//glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(BRD.v[i]+CN);glVertex3fv(BRD.v[j]+CN);glEnd();};
			};
			//connect segs  //get prew b
			int pi = i - 1;if(pi < 0)pi = seg - 1;
			ang = (float) pi / seg * TWOPI;
			vec3f vpi = SM.ship[k].center + vec3f(0, 0, rad)*sin(ang) + vec3f(rad, 0, 0)*cos(ang);
			vec3f pBN = norm(cross(vi - vpi, vec3f(0, -1, 0)))*crad;
			if(SegIntersects(vec2f(vi.x + pBN.x, vi.z + pBN.z), vec2f(vi.x + BN.x, vi.z + BN.z), vec2f(S.x, S.z), vec2f(T.x, T.z), CP2)){
				CP.set(CP2.x, 0, CP2.y);
				float rad = fVLEN2(CP - S);//+norm(BN)*20
				if(rad < min_rad){ min_rad = rad;CCP = CP;CCN = norm(BN + pBN)*crad;intersection = true; };//glColor3f(0,0,1);glBegin(GL_LINES);glVertex3fv(BRD.v[i]+CN);glVertex3fv(BRD.v[j]+CN);glEnd();};
			};
			////----------get closest point in border
			float si = SphereInt(S, D, vec4f(vi, crad));if(si <= 0)continue;
			if(si < min_rad){ min_rad = si;CCP = S + D * si;CCP.y = 0;CCN = (CCP - vi);CCN.y = 0;CCN = norm(CCN)*crad;intersection = true; };//glDrawCircle(vec3f(crad,0,0),vec3f(0,0,crad),BRD.v[i],40);print_SphereInt(S,D,vec4f(BRD.v[i],crad));};
		};//i,seg
	};//k,ms
	return intersection;
};
//======================================================================================
int mw = 5 - 1, mh = 4 - 1;
bool CAM_LOOP = false;
int max_cam = 105;
//======================================================================================
//void coil_init(){
//  coilAPIbad= CoilsInit(example_callback);if(coilAPIbad!=0){CON.write(2,"Error init Coilapi, return code %d\n", coilAPIbad);};
//  coilAPIbad= CoilsConnect();             if(coilAPIbad!=0){CON.write(2,"Error CoilsConnect, return code %d\n", coilAPIbad);};
//};
void coil_close(){ CoilsDisconnect();CoilsExit();CON.write(0, "CoilsDisconnect();CoilsExit();"); };
//======================================================================================
#include "WaterBall.hpp"
#include "process.hpp"
#include "render.hpp"
//======================================================================================
void INIT_CAM_LOOP(){
	// while(!CAM_LOOP){Sleep(2000);};
	CON.write(1, "CAM::enter loop %i", CAM_LOOP);
	cstring astr, anstr;
	float cam_pos_h = 240;//255;//240;//255;//226;//2.399==240 - IR-cross left - 28.06.2014
	int start_ip = 10, off = 0;
	int num = max_cam;//64;//104/2;//dxf_cam_pos.mv;
	CON.write(0, "init cameras %i(%i)", num, dxf_cam_pos.ml);
	CAPM.reserve(num);
	for(int i = 0;i < num;i++){
		//name
		astr.print("http://admin:video@192.168.1.%i/video/mjpg.cgi?something.mjpeg\0", start_ip + i + off);
		//astr.print("rtsp://admin:12345@192.168.0.%i/?something.mjpeg\0",start_ip+i+off);
		anstr.print("cam_%i", start_ip + i + off);
		//pos
		int col = dxf_cam_pos.v[dxf_cam_pos.l[i + off].x].x, row = dxf_cam_pos.v[dxf_cam_pos.l[i + off].x].z;
		int c1 = CAPM.add(anstr.text, vec3f(col, cam_pos_h, row), vec3f(col, 0, row));
		//CON.write(1,"c1==idx %i==%i",c1,dxf_cam_pos.l[i].x);  2==4
		CAPM.cap[c1]->open(i + 1 + off, astr.text);
		//---------- set cam default params
		CAPM.cap[c1]->cam->Load("DL_DCS-930L");
		CAPM.cap[c1]->cam->pos = vec3f(col, cam_pos_h, row);
		mat4 RM;RM.RT(vec3f(-PI * 1.5, 37 * D2R, 0), 0.0);//-PI*1.5
		CAPM.cap[c1]->cam->ang = RM;
		//CAPM.cap[c1]->cam->Rlim();
		CAPM.cap[c1]->cam->LookAt(vec3f(col, 0, row), 1);
		CAPM.cap[c1]->cam->Rlim();
		CAPM.cap[c1]->cam->Update();
		//---------
		CAPM.cap[c1]->cam->Load();//load changed params
		if(CAPM.cap[c1]->cam->con_adr[0] != 0){
			strcpy(CAPM.cap[c1]->dev_adr, CAPM.cap[c1]->cam->con_adr);
			CON.write(1, "------ debug ----- CAM:: loaded [%s]", CAPM.cap[c1]->dev_adr);
		}
	};
	CON.write(1, "CAM::exit loop %i", CAM_LOOP);
	Sleep(2000);
	CamHeightVal->edit_set_val(CAPM.cap[0]->cam->pos.y);
	if(release) StartCam->OnClick(); else StartCamL->OnClick();
	Controller_btn->Down = true;Controller_btn->OnClick();//init_ctrls_btn(NULL);
	fit_all_btn(NULL);
	//if(DEV_MODE)DEV_MODE_btn(NULL);
	UpdateWindow(hWnd);SetForegroundWindow(hWnd);SetFocus(hWnd);Active = true;
};
Thread CAMthread(&INIT_CAM_LOOP);
Thread LOGthread(&LOG_WRITER_LOOP);
//======================================================================================
int Process_WCMD(const char *buf, int len) {// implementation of forward declaration from "net.h"
	if (len < 4) return 0;
	int rpos = 0;// buf read pos

	// check cmd header
	DWORD32 *header = (DWORD32*)buf; rpos += 4;
	if (*header != WCMD_REQUEST) return 0;

	// get wcmd
	if (rpos >= len)return 0;
	byte *wcmd = (byte*)(buf + rpos); rpos++;
	if (*wcmd > NUM_WCMDS) { 
		CON.write(0, "GOT UNKNOWN CMD [%i]", buf + rpos);
		cstring answer;// common part for all answers
		answer.init();

		// header
		int wpos = answer.size; answer.resize(answer.size + 4);// buf write pos
		(*(DWORD32*)(answer.text + wpos)) = WCMD_ANSWER;
		wpos = answer.size; answer.resize(answer.size + 4);
		(*(byte*)(answer.text + wpos+0)) = 'U';
		(*(byte*)(answer.text + wpos+1)) = 'N';
		(*(byte*)(answer.text + wpos+2)) = 'K';
		(*(byte*)(answer.text + wpos+3)) = 'N';
		// do send answer
		int rez = send(NET.ttg, answer.text, answer.size, 0);
		CON.write(0, "ANSWER SENT size = %i", rez);

		return 0; 
	}
	CON.write(0, "GOT CMD %i [%s]", *wcmd, WCMD_S[*wcmd]);

	cstring answer;// common part for all answers
	answer.init();

	// header
	int wpos = answer.size; answer.resize(answer.size + 4);// buf write pos
	(*(DWORD32*)(answer.text + wpos)) = WCMD_ANSWER;

	// wcmd
	wpos = answer.size; answer.resize(answer.size + 1);
	(*(byte*)(answer.text + wpos)) = *wcmd;

	switch (*wcmd) {
		case PING:
		{
			wpos = answer.size; answer.resize(answer.size + 4);
			(*(byte*)(answer.text + wpos+0)) = 'P';
			(*(byte*)(answer.text + wpos+1)) = 'O';
			(*(byte*)(answer.text + wpos+2)) = 'N';
			(*(byte*)(answer.text + wpos+3)) = 'G';
			break;
		}
		// ships
		case SHIPS_INFO:
		{
			// ship_num
			wpos = answer.size; answer.resize(answer.size + 1);
			(*(byte*)(answer.text + wpos)) = (byte)SM.ms;
			CON.write(0,"SHIPS_INFO:%i",SM.ms);
			// put data
			loop0i(SM.ms) {
				// can_id
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte)SM.ship[i].id;
				CON.write(0,"SHIPS_INFO:can_id:%i",SM.ship[i].id);
				// action
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte) (SM.ship[i].id>0?SM.ship[i].cfg->brain.action : SM.ship[i].action);
				CON.write(0,"SHIPS_INFO:action:%i",(SM.ship[i].id>0?SM.ship[i].cfg->brain.action : SM.ship[i].action));
				// state
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte) (SM.ship[i].state);
				CON.write(0,"SHIPS_INFO:state:%i",SM.ship[i].state);
				//follow_id
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte) (SM.ship[i].id>0?SM.ship[i].cfg->brain.follow_id : SM.ship[i].follow_id);
				CON.write(0,"SHIPS_INFO:follow_id:%i",(SM.ship[i].id>0?SM.ship[i].cfg->brain.follow_id : SM.ship[i].follow_id));
				// crv_id
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte) (SM.ship[i].crv_id);
				CON.write(0,"SHIPS_INFO:crv_id:%i",SM.ship[i].crv_id);
				// speed
				wpos = answer.size; answer.resize(answer.size + 4);
				(*(float*)(answer.text + wpos)) = (float) (SM.ship[i].id>0?SM.ship[i].cfg->brain.speed : SM.ship[i].speed);
				CON.write(0,"SHIPS_INFO:speed:%f",(SM.ship[i].id>0?SM.ship[i].cfg->brain.speed : SM.ship[i].speed));
				//target
				wpos = answer.size; answer.resize(answer.size + sizeof(vec2f));
				(*(vec2f*)(answer.text + wpos)) = vec2f(SM.ship[i].target.x, SM.ship[i].target.z);
				CON.write(0,"SHIPS_INFO:target:[%f %f]",SM.ship[i].target.x,SM.ship[i].target.z);
				// pos
				wpos = answer.size; answer.resize(answer.size + sizeof(vec2f));
				(*(vec2f*)(answer.text + wpos)) = vec2f(SM.ship[i].sp1.x, SM.ship[i].sp1.z);
				CON.write(0,"SHIPS_INFO:pos:[%f %f]",SM.ship[i].sp1.x, SM.ship[i].sp1.z);
			}
			break;
		}
		//case SET_SHIP_TO_POS_AND_STOP:break;
		case SET_SHIP_TRAJECTORY:{
			// read requested ship id
			byte *ship_id = (byte*)(buf + rpos); rpos++;

			// read trajectory id
			byte *traj_id = (byte*)(buf + rpos); rpos++;

			// set if ok
			if(SM.has(*ship_id)){
				SM.ship[SM.get_ship_idx_by_id(*ship_id)].set_traj_crv(*traj_id);

				wpos = answer.size; answer.resize(answer.size + 2);
				(*(byte*)(answer.text + wpos+0)) = 'O';
				(*(byte*)(answer.text + wpos+1)) = 'K';
			}else{
				wpos = answer.size; answer.resize(answer.size + 3);
				(*(byte*)(answer.text + wpos+0)) = 'E';
				(*(byte*)(answer.text + wpos+1)) = 'R';
				(*(byte*)(answer.text + wpos+2)) = 'R';
			}
			break; 
		}
		case SHIP_STOP:{
			// read requested ship id
			byte *ship_id = (byte*)(buf + rpos); rpos++;

			// set if ok
			if(SM.has(*ship_id)){
				SM.ship[SM.get_ship_idx_by_id(*ship_id)].set_action(SA_STOP);

				wpos = answer.size; answer.resize(answer.size + 2);
				(*(byte*)(answer.text + wpos+0)) = 'O';
				(*(byte*)(answer.text + wpos+1)) = 'K';
			}else{
				wpos = answer.size; answer.resize(answer.size + 3);
				(*(byte*)(answer.text + wpos+0)) = 'E';
				(*(byte*)(answer.text + wpos+1)) = 'R';
				(*(byte*)(answer.text + wpos+2)) = 'R';
			}
			break;
		}
		case SHIP_SWIM:{
			// read requested ship id
			byte *ship_id = (byte*)(buf + rpos); rpos++;

			// set if ok
			if(SM.has(*ship_id)){
				SM.ship[SM.get_ship_idx_by_id(*ship_id)].set_action(SA_AUTOPILOT);

				wpos = answer.size; answer.resize(answer.size + 2);
				(*(byte*)(answer.text + wpos+0)) = 'O';
				(*(byte*)(answer.text + wpos+1)) = 'K';
			}else{
				wpos = answer.size; answer.resize(answer.size + 3);
				(*(byte*)(answer.text + wpos+0)) = 'E';
				(*(byte*)(answer.text + wpos+1)) = 'R';
				(*(byte*)(answer.text + wpos+2)) = 'R';
			}
			break;
		}
		case SHIP_SWIM_TO:{
			// read requested ship id
			byte *ship_id = (byte*)(buf + rpos); rpos++;

			//read target pos
			vec2f *tpos = (vec2f*)(buf + rpos); rpos+=sizeof(vec2f);

			// set if ok
			if(SM.has(*ship_id)){
				SM.ship[SM.get_ship_idx_by_id(*ship_id)].set_action(SA_GOTO);
				SM.ship[SM.get_ship_idx_by_id(*ship_id)].target = vec3f((*tpos).x,0,(*tpos).y);

				wpos = answer.size; answer.resize(answer.size + 2);
				(*(byte*)(answer.text + wpos+0)) = 'O';
				(*(byte*)(answer.text + wpos+1)) = 'K';
			}else{
				wpos = answer.size; answer.resize(answer.size + 3);
				(*(byte*)(answer.text + wpos+0)) = 'E';
				(*(byte*)(answer.text + wpos+1)) = 'R';
				(*(byte*)(answer.text + wpos+2)) = 'R';
			}
			break;
		}
		// trajectory
		case GET_TRAJECTORY_LIST:{
			// traj_list_size
			wpos = answer.size; answer.resize(answer.size + 1);
			(*(byte*)(answer.text + wpos)) = (byte)TrajMan.list.size();

			loop0i(TrajMan.list.size()){
				//traj_i_name_len
				int slen = strlen(TrajMan.list[i]);
				wpos = answer.size; answer.resize(answer.size + 1);
				(*(byte*)(answer.text + wpos)) = (byte)slen;

				// traj_i_name
				wpos = answer.size; answer.resize(answer.size + slen);
				//(*(char*)(answer.text + wpos)) = TrajMan.list[i];
				memcpy(answer.text + wpos, TrajMan.list[i], slen);
			}
			break;
		}
		case LOAD_TRAJECTORY:{
			// read requested traj id
			byte *traj_id = (byte*)(buf + rpos); rpos++;

			// load preset
			if(TrajMan.load(*traj_id)==true){
				wpos = answer.size; answer.resize(answer.size + 2);
				(*(byte*)(answer.text + wpos+0)) = 'O';
				(*(byte*)(answer.text + wpos+1)) = 'K';
			}else{
				wpos = answer.size; answer.resize(answer.size + 3);
				(*(byte*)(answer.text + wpos+0)) = 'E';
				(*(byte*)(answer.text + wpos+1)) = 'R';
				(*(byte*)(answer.text + wpos+2)) = 'R';
			}
			break;
		}
		case GET_TRAJECTORY_INFO:{
			// traj_num_curves
			wpos = answer.size; answer.resize(answer.size + 1);
			(*(byte*)(answer.text + wpos)) = (byte)TrajMan.num;

			break;
		}
	}

	// do send answer
	int rez = send(NET.ttg, answer.text, answer.size, 0);

	wpos = answer.size; answer.resize(answer.size + 1);
	(*(byte*)(answer.text + wpos)) = '\0';// end of string for dump

	CON.write(0, "ANSWER SENT size = %i dump[%s]", rez, answer.text);

	return len - rpos;// if there merged packets
}
//======================================================================================
int main(int argc, char **argv){
	check_release();
	ProfilerThread.launch();
	LOGthread.launch();
	//LOG.write(0,"start");
	//THHPthread.launch();
	glClearColor(1, 1, 1, 0);
	MM.init();
	pl = CAMM.add(vec3f(200, 900, -150), vec3f(200, 0, -150));
	//img=CAMM.add(5.0f,0.0f);img->Load("img_cam.params");
	dxf_base.load("CAD\\base.dxf");
	dxf_cam_centers.load("CAD\\cam_centers.dxf");
	dxf_relief.load("CAD\\relief.dxf");
	dxf_shore.load("CAD\\shore.dxf");
	dxf_coils.load("CAD\\coils.dxf");
	dxf_cam_pos.load("CAD\\cam_pos.dxf");
	//----------------coil sort CAD\\coils.dxf 
	vec3f t;//swapped x<>z
	for(int i = 0;i < dxf_coils.mv - 2;i += 2){//sort in col
		for(int j = i + 2;j < dxf_coils.mv;j += 2){
			if(dxf_coils.v[i].x < dxf_coils.v[j].x){//swap
				t = dxf_coils.v[i];dxf_coils.v[i] = dxf_coils.v[j];dxf_coils.v[j] = t;
				t = dxf_coils.v[i + 1];dxf_coils.v[i + 1] = dxf_coils.v[j + 1];dxf_coils.v[j + 1] = t;
			};//swap
		};//j
	};//i    /**/
	for(int i = 0;i < dxf_coils.mv - 2;i += 2){//sort in row
		for(int j = i + 2;j < dxf_coils.mv;j += 2){
			if(dxf_coils.v[i].x == dxf_coils.v[j].x && dxf_coils.v[i].z > dxf_coils.v[j].z){//swap
				t = dxf_coils.v[i];dxf_coils.v[i] = dxf_coils.v[j];dxf_coils.v[j] = t;
				t = dxf_coils.v[i + 1];dxf_coils.v[i + 1] = dxf_coils.v[j + 1];dxf_coils.v[j + 1] = t;
			};//swap
		};//j
	};//i    /**/
	//----coil offset
	//for(int i=0;i<dxf_coils.ml;i++){if(dxf_coils.l[i].z!=1)continue;
	// dxf_coils.v[dxf_coils.l[i].x]+=vec3f(-4.5,0,6.5);
	// dxf_coils.v[dxf_coils.l[i].y]+=vec3f(-4.5,0,6.5);
	//};
	//----------------end coil sort    
	MAP.init();
	for(int i = 0;i < dxf_coils.ml;i++){
		vec3f pt = dxf_coils.center(i);
		MAP.add(vec2f(pt.x, pt.z));
	};
	MAP.triangulate();
	load_coff("data\\coil.offsets");
	//-------sort cams CAD\\cam_pos.dxf
	//rotate cam poses
	mat4 RM(1);RM.Rot(vec3f(0, -37 * D2R, 0));//53
	for(int i = 0;i < dxf_cam_pos.mv;i++){ dxf_cam_pos.v[i] = RM * dxf_cam_pos.v[i]; };
	//vec3f t;//swapped x<>z
	for(int i = 0;i < dxf_cam_pos.mv - 2;i += 2){//sort in col
		for(int j = i + 2;j < dxf_cam_pos.mv;j += 2){
			if(dxf_cam_pos.v[i].x < dxf_cam_pos.v[j].x){//swap
				t = dxf_cam_pos.v[i];dxf_cam_pos.v[i] = dxf_cam_pos.v[j];dxf_cam_pos.v[j] = t;
				t = dxf_cam_pos.v[i + 1];dxf_cam_pos.v[i + 1] = dxf_cam_pos.v[j + 1];dxf_cam_pos.v[j + 1] = t;
			};//swap
		};//j
	};//i    /**/
	for(int i = 0;i < dxf_cam_pos.mv - 2;i += 2){//sort in row
		for(int j = i + 2;j < dxf_cam_pos.mv;j += 2){
			if(fabs(dxf_cam_pos.v[i].x - dxf_cam_pos.v[j].x) < 40 && dxf_cam_pos.v[i].z > dxf_cam_pos.v[j].z){//swap
				t = dxf_cam_pos.v[i];dxf_cam_pos.v[i] = dxf_cam_pos.v[j];dxf_cam_pos.v[j] = t;
				t = dxf_cam_pos.v[i + 1];dxf_cam_pos.v[i + 1] = dxf_cam_pos.v[j + 1];dxf_cam_pos.v[j + 1] = t;
			};//swap
		};//j
	};//i    /**/
	//rotate back
	for(int i = 0;i < dxf_cam_pos.mv;i++){ dxf_cam_pos.v[i] = RM.IRotate(dxf_cam_pos.v[i]); };
	//------------------
   // LOG.write(0,"SND.Init();");
	SND.Init();
	snd_lost = SND.Load("sound\\lost.wma"),
		snd_found = SND.Load("sound\\found.wma"),
		snd_low = SND.Load("sound\\low_energy.wma"),//todo
		snd_yes_usb = SND.Load("sound\\usb_ok.wma"),
		snd_n2c_ok = SND.Load("sound\\n2c_ok.wma"),
		snd_anomaly = SND.Load("sound\\anomaly.wma"),//todo
		snd_no_usb = SND.Load("sound\\no_usb.wma"),
		snd_no_n2c = SND.Load("sound\\no_n2c.wma"),
		snd_no_cam = SND.Load("sound\\no_cam.wma"),//todo
		snd_at_base = SND.Load("sound\\at_base.wma");//todo?
	snd_no_move = SND.Load("sound\\no_move.wma");
	snd_collision = SND.Load("sound\\collision.wma");
	UI.Init(GUIResize);
	GUIResize(&UI);
	//--------------- end cam loader CAD\\cam_pos.dxf
	TM.init();
	TrajMan.init();
	//--------set
	//CPT.openPort("COM3");
	CPT.init("COM3\0");
	CPT.startCOMThread();
	CON.show = 0;
	//for(int i=0;i<CAPM.mc;i++)CAPM.cap[i]->filter=2;
	W2Cthread.launch();
	Ctrls.load("data\\controllers.bind");
	EX.load("data\\exclusion.box");
	//---------------marker com
	MCMDthread.launch();
	marker_send(MARKER_ALL, MARKER_FOUND);
	//marker_send(MARKER_ALL,MARKER_LIGHT_OFF);
	marker_send(MARKER_ALL, MARKER_FREQ);
	marker_send(MARKER_ALL, MARKER_SOUND);
	ship_id_thread.launch();
	load_anchors("data\\anchors.anc");
//	ship_brain_load();
	//char cmd=0xff;CPT.write_char(cmd);
	//cmd=0xd2;CPT.write_char(cmd);
	//cmd=0x5c;CPT.write_char(cmd);
	CON.write(0, "CoilsInit(example_callback)");
	coilAPIbad = CoilsInit(example_callback);
	if(coilAPIbad != 0){ CON.write(2, "Error init Coilapi, return code %i", coilAPIbad); };
	//------------test
	 //ship_pic[0]=MM.LoadTex(,);
	//ship_pic[0].loadJPG("data\\ship_unknown.jpg", GL_REPEAT, 0, 0);
	ship_configs.load_textures();
	//ship_pic[1].loadJPG("data\\ship_1.jpg", GL_REPEAT, 0, 0);
	//ship_pic[2].loadJPG("data\\ship_2.jpg", GL_REPEAT, 0, 0);
	//ship_pic[3].loadJPG("data\\ship_3.jpg", GL_REPEAT, 0, 0);
	//ship_pic[4].loadJPG("data\\ship_4.jpg", GL_REPEAT, 0, 0);
	//ship_pic[5].loadJPG("data\\ship_5.jpg", GL_REPEAT, 0, 0);
	//ship_pic[6].loadJPG("data\\ship_6.jpg", GL_REPEAT, 0, 0);
	//ship_pic[7].loadJPG("data\\ship_7.jpg", GL_REPEAT, 0, 0);
	CAMthread.launch();
	//  LOG.write(0,"CAMthread.launch();");
	return 0;
};
//======================================================================================
void main_end(){
	marker_send(MARKER_ALL, MARKER_SOUND);
	marker_send(MARKER_ALL, MARKER_FOUND);
	marker_send(MARKER_ALL, MARKERS_LIGHT_OFF);
	dxf_coils.OFF_ALL();
	coils_off_at_end = true;
	while(!CS.off()){ Sleep(100); };
	//Sleep(1000);
	CAM_LOOP = false;
	W2C_LOOP = false;
	SEND_HTTP_BOOL = false;
	MBUF_SEND_LOOP = false;
	while(MBUF_SEND_LOOP != -1)Sleep(100);
	//CON.write(0,"dxf_coils.OFF_ALL();");CON.flush();
	//CON.write(0,"CS.off");CON.flush();
	coil_close();
	SND.Unload();
	//  LOG.write(0,"main_end()");
	 //CON.write(0,"main_end");CON.flush();
};
//======================================================================================