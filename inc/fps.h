//======================================================================================
struct CTimer {
	// public:
	float dt, gdt, Delta, TimeScale, Resolution, ms, dt2, dt3;
	int FPSmin, FPSmax, framesPerSecond, fps;
	LARGE_INTEGER Frequency, LastTickCount, TframeTime, TickCount;
	LARGE_INTEGER now, last, last2;
	float cur, prew;
	Dword timeBase, timeBase2;
	bool ToLog;
	//--------------------------------------------------------------------------------------
	CTimer() { Init(); };
	//--------------------------------------------------------------------------------------
	void Init() {
		QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&LastTickCount);
		TframeTime = LastTickCount;
		Resolution = 1.0f / (float)Frequency.QuadPart;
		framesPerSecond = fps = 0;
		dt = gdt = dt2 = dt3 = ms = 0.0f;
		Delta = 0.0f;
		FPSmin = 1000;
		FPSmax = 0;
		TimeScale = 1.0;
		now.QuadPart = last.QuadPart = 0;
		//--------
		timeBase = timeGetTime();
		ToLog = 1;
		QueryPerformanceCounter((LARGE_INTEGER*)&last);
		timeBase2 = (Dword)last.QuadPart;
	};
	//--------------------------------------------------------------------------------------
	  //warp 49.71 days
	Dword Milliseconds(bool reset = false) { if (reset)timeBase = timeGetTime(); return (timeGetTime() - timeBase); };
	//--------------------------------------------------------------------------------------
	float Milliseconds2(bool reset = false) {
		LARGE_INTEGER ct;
		QueryPerformanceCounter(&ct);
		if (reset)timeBase2 = ct.QuadPart;
		return ((float)ct.QuadPart - timeBase2) / (float)Frequency.QuadPart;
	};
	//--------------------------------------------------------------------------------------
	 /* float ddt(){
	   QueryPerformanceCounter((LARGE_INTEGER*)&now);
	   float d = (float)((double)(now - last2) / (double)(Frequency)) * TimeScale;
	   //dt+=d;
	   last2=now;
	   //dt+=dtt;
	   return dt-d;
	  };//        */
	  //--------------------------------------------------------------------------------------
	void Update() {
		//float avgPeriod=10; // #frames involved in average calc (suggested values 5-100)
		float smoothFactor = 0.3f; // adjusting ratio (suggested values 0.01-0.5)
		QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);// 4 notebooks
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
		//cur=((float)now.QuadPart)/((float)Frequency.QuadPart);
		//SetProcessAffinityMask(GetCurrentProcess(), 2);
		//fps=(float)((double)Frequency.QuadPart/(double)(now.QuadPart-last.QuadPart));
		framesPerSecond++;
		//if (ToLog) {
		//	if (FPSmin > fps) { FPSmin = fps; CON.write(0, "Timer::FPS::min %i", fps); };
		//	if (FPSmax < fps) { FPSmax = fps; CON.write(0, "Timer::FPS::max %i", fps); };
		//};
		//dt3=dt2;
		//dt2=dt;
		dt3 = (float)((double)(now.QuadPart - last.QuadPart) / (double)(Frequency.QuadPart)) * TimeScale;
		ms += dt3; if (ms > 1.0f) { fps = framesPerSecond; framesPerSecond = 0; ms = 0; };
		if (fabs(dt)<EPSILON) { dt = dt3; dt2 = dt; };//PVS
		//dt2=(dt3+dt2*(avgPeriod-1))/avgPeriod;
		dt += (dt3 - dt)*smoothFactor;
		//gdt=dt;
		//dt = (dt3*0.6+(dt2*0.6 + (float)((double)(now - last) / (double)(Frequency)) * TimeScale*0.4)*0.4);
		//dt = (dt2 + (float)((double)(now - last) / (double)(Frequency)) * TimeScale)*0.5;
		//dt=(cur-prew);
		//prew=cur;
		last = now;
		//cur=((double)timeGetTime())/1000.0;
		//dt=(cur-prew);
		//prew=cur;
		// while(dt>0.15)dt-=0.050f;
		// while(dt>0.10)dt-=0.010f;
		return;//    */
	  /*  QueryPerformanceCounter(&frmEnd);
		dt  = (float)(frmEnd.QuadPart-frmBegin.QuadPart)/(float)frmFrq.QuadPart*100;
		fps  = 100.0f/dt;

		Delta += dt;
		count ++;
		if (time > 100.0) {
		  aFps = count*100.0f/Delta;
		  Delta = 0;
		  count = 0;
		}
		 return;*/
		 /*QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);// 4 notebooks
		 Resolution=1.0f/(float)Frequency;
		 QueryPerformanceCounter((LARGE_INTEGER*)&TickCount);
		 framesPerSecond++;
		 dt=(float)(TickCount-TframeTime)*Resolution;
		 TframeTime=TickCount;
		 Delta=(float)(TickCount-LastTickCount)*Resolution;
		 //ms+=Delta*1000.0;
		 if(Delta>=1.0f){
		  LastTickCount=TickCount;
		  fps=framesPerSecond;
		  if(ToLog){
		   if(FPSmin>fps){FPSmin=fps;CON.write(0,"Timer::FPS::min %i",fps);};
		   if(FPSmax<fps){FPSmax=fps;CON.write(0,"Timer::FPS::max %i",fps);};
		  };
		  framesPerSecond=0;
		 };
		 dt*=TimeScale;
		 //while(dt>0.15)dt-=0.050f;
		 //while(dt>0.10)dt-=0.010f;*/
	};
	//--------------------------------------------------------------------------------------
};
float time() {//last_time - time()>1.0 === 1sec?
	static __int64 start = 0;
	static __int64 frequency = 0;
	if (start == 0) {
		QueryPerformanceCounter((LARGE_INTEGER*)&start);
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
		return 0.0f;
	}
	__int64 counter = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	return (float)((counter - start) / double(frequency));
}
//  const float newTime = time();
//  float deltaTime = newTime - currentTime;
//  currentTime = newTime;
//  accumulator += deltaTime;


//======================================================================================
