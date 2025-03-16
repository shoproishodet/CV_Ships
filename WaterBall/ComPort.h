/*bool WriteComPort(char *PortSpecifier,char *data){
 HANDLE hPort = CreateFile(
	PortSpecifier,
	GENERIC_WRITE,
	0,
	NULL,
	OPEN_EXISTING,
	0,
	NULL
 );
 DCB dcb;
 if(!GetCommState(hPort,&dcb))return false;
 dcb.BaudRate = CBR_9600; //9600 Baud
 dcb.ByteSize = 8; //8 data bits
 dcb.Parity = NOPARITY; //no parity
 dcb.StopBits = ONESTOPBIT; //1 stop
 if(!SetCommState(hPort,&dcb))return false;
 DWORD byteswritten;
 bool retVal = WriteFile(hPort,data,1,&byteswritten,NULL);
 CloseHandle(hPort); //close the handle
 return retVal;
};

int ReadByte(char *PortSpecifier){
 int retVal;
 BYTE Byte;
 DWORD dwBytesTransferred;
 DWORD dwCommModemStatus;
 HANDLE hPort = CreateFile(
	PortSpecifier,
	GENERIC_READ,
	0,
	NULL,
	OPEN_EXISTING,
	0,
	NULL
 );
 DCB dcb;
 if(!GetCommState(hPort,&dcb))return 0x100;
 dcb.BaudRate = CBR_9600; //9600 Baud
 dcb.ByteSize = 8; //8 data bits
 dcb.Parity = NOPARITY; //no parity
 dcb.StopBits = ONESTOPBIT; //1 stop
 if(!SetCommState(hPort,&dcb))return 0x100;
 SetCommMask(hPort, EV_RXCHAR | EV_ERR); //receive character event
 WaitCommEvent(hPort, &dwCommModemStatus, 0); //wait for character
 if(dwCommModemStatus & EV_RXCHAR) ReadFile(hPort, &Byte, 1, &dwBytesTransferred, 0); //read 1
 else if(dwCommModemStatus & EV_ERR)retVal = 0x101;
 retVal = Byte;
 CloseHandle(hPort);
 return retVal;
};// */
bool USB2COM3 = false;

struct COMPort{
	//word ndts,ndts_pos;
	HANDLE hPort;
	DCB dcb;
	DWORD byteswritten;
	HANDLE COMPortThread;
	//bool DataToWrite;
	char lcode;
	char *port;
	volatile int run_thread;//PVS

	//COMPort(int n){//char *port){
	// init();
	// //openPort(port);
	//};
	~COMPort(){ closePort(); };
	void init(const char *prt){
		//DataToWrite=false;
		//ndts=0;
		//ndts_pos=0;
		lcode = 0;
		port = new char[5];strcpy(port, prt);
	};
	/*
	void addLine(){
	 ndts++;
	 RESIZE(toSend,cstring,ndts);
	 toSend[ndts-1].init();
	};
	void Write(char *data){
	 if(!ndts){addLine();};
	 if(ndts_pos>0){//

	 }else{
	  addLine();
	 };
	 RESIZE(toSend[0],char,ndts);
	};

	static DWORD WINAPI StaticThreadStart(void *Param){
	 COMPort *This = (COMPort*)Param;
	 return This->ThreadStart();
	};
	DWORD ThreadStart(void){
	 while(1){
	  if(DataToWrite)writePort();else Sleep(1000);
	 };
	 return 1;
	};
	void startCOMThread(){
	 DWORD ThreadID;
	 COMPortThread=CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
	};	 */
	void openPort(char *oport){
		CON.write(0, "TRY OPEN COMPORT %s", oport);
		hPort = CreateFile(oport, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(!GetCommState(hPort, &dcb)){ hPort = NULL;CON.write(2, "GET ERROR %s", oport);USB2COM3_p->caption.print("USB2COM3-ERR");return; }
		dcb.BaudRate = CBR_9600; //9600 Baud    CBR_115200;//
		dcb.ByteSize = 8; //8 data bits 
		dcb.Parity = NOPARITY; //no parity 
		dcb.StopBits = ONESTOPBIT; //1 stop 
		if(!SetCommState(hPort, &dcb)){ hPort = NULL;CON.write(2, "SET ERROR %s", oport);USB2COM3_p->caption.print("USB2COM3-ERR");return; };
		CON.write(0, "OPENNED COMPORT %s", oport);
		USB2COM3_p->caption.print("USB2COM3-OK");
		USB2COM3 = true;
		//startCOMThread();
	};
	//------------------ port thread check connection
	static DWORD WINAPI StaticThreadStart(void *Param){
		COMPort *This = (COMPort*) Param;
		return This->ThreadStart();
	};
	DWORD ThreadStart(void){
		CON.write(0, "COM:: start thread at [%s]", port);
		run_thread = true;
		openPort(port);
		while(run_thread){
			if(USB2COM3){
				if(!GetCommState(hPort, &dcb)){ hPort = NULL;CON.write(2, "GET ERROR %s", port);USB2COM3_p->caption.print("USB2COM3-ERR");USB2COM3 = false; };
				Sleep(10000);continue;
			};
			openPort(port);
			if (run_thread == 0)break;
			if(USB2COM3 == false)SND.Play(snd_no_usb);else SND.Play(snd_yes_usb); //crush at exit +2
			Sleep(10000);
		};
		run_thread = 2;//done
		CON.write(0, "COM:: close [%s]", port);
		return 1;
	};
	void startCOMThread(){
		DWORD ThreadID;
		COMPortThread = CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
	};
	//----------------- connection thread
	void closePort(){ run_thread = false;while(run_thread != 2)Sleep(100);CloseHandle(hPort); };//CloseHandle(COMPortThread);};// char c=0;writePort(&c);
	//bool writePort(char *data){if(lcode==data[0])return 0;lcode=data[0];
	// //CON.write(2,"SEND %i",data[0]);
	// bool rez=WriteFile(hPort,data,1,&byteswritten,NULL);
	// //CON.write(0,"COM::Send(%i)(%c) written=%i",data[0],data[0],byteswritten);  PurgeComm(hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	// return rez;
	//};
	void send_maker_cmd(const byte &boat, const byte &cmd){
		if(debug_marker_cb->Down)CON.write(2, "MARKER::SEND boat=%i cmd=%i", boat, cmd);
		if(!WriteFile(hPort, &boat, 1, &byteswritten, NULL))USB2COM3 = false;
		if(!WriteFile(hPort, &cmd, 1, &byteswritten, NULL))USB2COM3 = false;
		byte eo = 0x5c;if(!WriteFile(hPort, &eo, 1, &byteswritten, NULL))USB2COM3 = false;
	};
	//void write_char(const char &data){WriteFile(hPort,&data,1,&byteswritten,NULL);};
	//bool writePort_b(const char *data){//if(lcode==data[0])return 0;lcode=data[0];
	// bool rez=WriteFile(hPort,data,strlen(data)+1,&byteswritten,NULL);
	// //CON.write(1,"SEND [%s][%i][w%i]",data,strlen(data),byteswritten);
	// //CON.write(0,"COM::Send(%i)(%c) written=%i",data[0],data[0],byteswritten);  
	// //PurgeComm(hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	// //Sleep(5);
	// return rez;
	//};
	/*bool writePort(){
	 bool rez=WriteFile(hPort,toSend[ndts_pos],1,&byteswritten,NULL);
	 DataToWrite=ndts>0;??
	 return rez;
	};*/
};