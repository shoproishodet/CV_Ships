//======================================================================================
struct TextPage{
	int pr, mem, pos, rr;
	cstring *row;
	int *stt;
	TextPage(){ pr = mem = pos = 0; row = NULL; init_ring(); };
	~TextPage() { DEL_(row); DEL_(stt); };//PVS
	void resize(int nmr){//thread safe??
		if (nmr < mem){ pr = nmr; return; };
		mem = nmr + 2;
		RESIZE(row, cstring, mem);
		RESIZE(stt, int, mem);
		for (int i = pr; i < mem; i++){ row[i].init(); stt[i] = 0; };
		pr = nmr;
	};
	void init_ring(int irr = 38){
		rr = irr;
		resize(rr);
		pr = pos = 0;
	};
	void ring_push(cstring *nr){//ring buffer
		if (pr < rr)resize(pr + 1);
		row[pos].print("%s", nr->text);
		pos = (pos + 1) % rr;
	};
};
TextPage LogPage;
//======================================================================================
//const char CLOG_MAX_MSG=100;
#define LOG_COMMON  0
#define LOG_WARNING 1
#define LOG_ERROR   2
#define LOG_FATAL   3
#define LOG_TEXT    4
#define LOG_ONLY    5
//======================================================================================
int vspf(char *buf, const char *fmt, ...){
	int cnt;
	va_list ap; va_start(ap, fmt); cnt = vsprintf(buf, fmt, ap); va_end(ap);
	return(cnt);
};
//======================================================================================
int CLOG_size = 0;
#define MLS_USED    1
#define MLS_UNUSED -1
#define MLS_REUSE  -2
#define MLS_BUSY   -3
struct msg_list{
	cstring *msg;
	int state, idx;
	msg_list *next, *prew;
	msg_list(msg_list *caller){ idx = CLOG_size++; msg = new cstring(); state = MLS_UNUSED; next = NULL; prew = caller; };
	~msg_list(){};
	//msg_list *find(int fi){
	// msg_list *sm=this;
	// if(idx>fi)while(sm && sm->idx!=fi)sm=sm->next;else if(idx<fi)while(sm && sm->idx!=fi)sm=sm->prew;
	// return sm;//(sm && sm->idx==fi)?sm:NULL;
	//};
	//msg_list *ins(){
	// msg_list *sn=next;
	// msg_list *im=add();//im->prew=this
	// im->next=sn;
	// sn->prew=im;
	// msg_list *nm=im;while(nm){nm->idx=nm->prew->idx+1;nm=nm->next;};
	// return im;
	//};
	//void del(){
	// prew->next=next;
	// next->prew=prew;
	// msg_list *nm=next;while(nm){next->idx--;nm=nm->next;};
	// delete [] this;//?
	//};
	msg_list *add(){ next = new msg_list(this); return next; };
	msg_list *get_free_slot(){
		if (state == MLS_UNUSED)return this;
		return (next == NULL) ? add() : next->get_free_slot();
	};
};
//======================================================================================
struct CLOG{
	msg_list *msgl;
	volatile int thread_run, towrite;//PVS
	bool show;
	//--------------------------------------------------------------------------------------
	CLOG(){
		show = false;
		msgl = new msg_list(NULL);
		towrite = 0;
		write(0, "CLOG->constructor");
	};
	//--------------------------------------------------------------------------------------
	~CLOG(){ write(0, "CLOG->destructor"); while (thread_run != 2){ Sleep(100); }; thread_run = false; };
	//--------------------------------------------------------------------------------------
	char *errlvlstr(int l){
		switch (l){
		case LOG_WARNING:return "WARNING";
		case LOG_ERROR:return "ERROR";
		case LOG_FATAL:return "FATAL";
			//case LOG_ONLY:?
		};
		return " ";
	};
	void flush(){};
	//--------------------------------------------------------------------------------------
	void write(char errorlevel, const char *fmt, ...){
		msg_list *cm = msgl->get_free_slot(); cm->state = MLS_BUSY;//CON.write(0,"---------CLOG:::use_slot:%i",cm->idx);
		//---------- va
		va_list ap; va_start(ap, fmt);
		int nlen = 1 + _vscprintf(fmt, ap);
		char *fmsg = new char[nlen];
		vsprintf(fmsg, fmt, ap); va_end(ap);
		//----------
		SYSTEMTIME obj;
		GetLocalTime(&obj);//   GetSystemTime
		cm->msg->print("(%id-%im-%iy %ih:%im:%is.%ims)::%s::%s\0", obj.wDay, obj.wMonth, obj.wYear, obj.wHour, obj.wMinute, obj.wSecond, obj.wMilliseconds, errlvlstr(errorlevel), fmsg);
		delete[] fmsg;
		cm->state = MLS_USED;
		towrite++;
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================
CLOG CON;
void LOG_WRITER_LOOP(){
	SYSTEMTIME obj; GetLocalTime(&obj);
	FILE *cn = fopen(cstring("log\\log_2_%im%ih_%id%im%iy.txt", obj.wMinute, obj.wHour, obj.wDay, obj.wMonth, obj.wYear), "wb");
	if (!cn){ return; };//no logging possible
	CON.thread_run = true;
	while (CON.thread_run){
		if (CON.towrite > 0){
			msg_list *cm = CON.msgl;
			while (CON.towrite && cm != NULL){
				if (cm->state == MLS_USED){ fprintf(cn, "%s\r\n", cm->msg->text); cm->state = MLS_UNUSED; CON.towrite--; LogPage.ring_push(cm->msg); };//CON.write(0,"---------CLOG:::release_slot:%i",cm->idx);};
				cm = cm->next;
			};
			//CON.thread_run=1;//has work to do
			fflush(cn);//all clear
		};//else{CON.thread_run=2;};//LOG.towrite
		Sleep(100);
	};
	fprintf(cn, "End Logger\r\n");
	fclose(cn);
};
//======================================================================================