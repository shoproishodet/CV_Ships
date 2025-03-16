//=======================================================================================
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#define NET_BUF_MAX 2048
//=======================================================================================
struct CSaddr{
	sockaddr_in A;
	char str[30];
	int m_ip[5];
	//-----------
	void Clear(){
		memset(&A, 0, sizeof(sockaddr_in));
		A.sin_family = AF_INET;
		A.sin_addr.s_addr = INADDR_ANY;
	};
	void set_ip(int new_ip){ *(int*) &A.sin_addr.s_addr = new_ip; };
	void set_port(word nport){ A.sin_port = htons(nport); };
	bool set(const char *fname){
			//in = fopen(fname, "wt");
			//fwrite("here",4,1,in);
			//fclose(in);
		FILE *in = fopen(fname, "rt"); if (in == NULL) { 
			return false; 
		}
		fgets(str, 20, in); A.sin_addr.s_addr = inet_addr(str); if (A.sin_addr.s_addr == INADDR_NONE) { fclose(in); return false; }
		fgets(str, 20, in);sscanf(str, "%i", &m_ip[4]);A.sin_port = htons(m_ip[4]);
		fclose(in);
		return true;
	};
	//-----------------
	void ip4(){
		m_ip[0] = A.sin_addr.s_addr & 0xff;
		m_ip[1] = (A.sin_addr.s_addr >> 8) & 0xff;
		m_ip[2] = (A.sin_addr.s_addr >> 16) & 0xff;
		m_ip[3] = (A.sin_addr.s_addr >> 24) & 0xff;
	};
	const char *IP5(){ ip4();vspf(str, "%i.%i.%i.%i:%i", m_ip[0], m_ip[1], m_ip[2], m_ip[3], htons(A.sin_port));return str; };
	const char *IP4(){ ip4();vspf(str, "%i.%i.%i.%i", m_ip[0], m_ip[1], m_ip[2], m_ip[3]);return str; };
};
//=======================================================================================
struct CSock{
	SOCKET S;
	CSaddr addr, from;
	//--------------------------------------------------------------------------------
	bool InitUDP(){
		S = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(S == INVALID_SOCKET){ CON.write(0, "SocketError:InitUPD()");return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	bool InitTCP(){
		S = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(S == INVALID_SOCKET){ CON.write(0, "SocketError:InitTCP()");return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Close(){
		if(S == INVALID_SOCKET)return false;
		if(closesocket(S) == SOCKET_ERROR){ CON.write(0, "SocketError:Close()");return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Bind(){
		if(::bind(S, (const sockaddr*) &addr.A, sizeof(sockaddr_in)) == SOCKET_ERROR){
			CON.write(0, "SocketError:Bind()");return false;
		};
		return true;
	};
	//--------------------------------------------------------------------------------
	int SendTo(sockaddr_in *TO, const char *data, int len){
		int rez = sendto(S, data, len, 0, (const sockaddr*) TO, sizeof(sockaddr_in));
		if(rez == SOCKET_ERROR){
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){ return rez; };
			CON.write(0, "SocketError:SendTo()");
			return rez;
		};
		return rez;
	};
	//--------------------------------------------------------------------------------
	int Send(const char *data, int len){
		int rez = send(S, data, len, 0);
		if(rez == SOCKET_ERROR){
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){ return rez; };
			CON.write(0, "SocketError:Send()");
			return rez;
		};
		return rez;
	};
	//--------------------------------------------------------------------------------
	bool Connect(CSaddr *TO){
		if(connect(S, (const sockaddr*) &TO->A, sizeof(sockaddr)) == SOCKET_ERROR){ CON.write(0, "SocketError:Connect() to '%s'", TO->IP5());return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Listen(int BL){
		if(listen(S, BL) == SOCKET_ERROR){ CON.write(0, "SocketError:Listen()");return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Accept(SOCKET *AS){
		int len = sizeof(sockaddr);
		CSaddr TA;TA.Clear();
		SOCKET _TS;
		_TS = accept(S, (sockaddr*) &TA.A, &len);
		if(_TS == INVALID_SOCKET){/*CON.write(0,"SocketError:Accept()");*/return false; };
		CON.write(0, "CSock::Accept():from '%s' sock %i", TA.IP5(), _TS);
		*AS = _TS;
		return true;
	};
	//--------------------------------------------------------------------------------
	bool SetNoBlock(bool noblock){
		unsigned long yi = noblock;
		if(ioctlsocket(S, FIONBIO, &yi) == SOCKET_ERROR){
			CON.write(0, "SocketError:SetNoBlock()");return false;
		};
		return true;
	};
	bool SetTimeout(int ms){
		if(setsockopt(S, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms,sizeof(ms)) == SOCKET_ERROR){
			CON.write(0, "SocketError:SetNoBlock()");return false;
		}
		return true;
	}
	//--------------------------------------------------------------------------------
	bool Receive(char *data, word &len){
		int fromlen = sizeof(sockaddr);
		memset(data, 0, NET_BUF_MAX);
		int error = recvfrom(S, data, NET_BUF_MAX, 0, (sockaddr*) &from.A, &fromlen);
		if(error == SOCKET_ERROR){
			int err = WSAGetLastError();
			if(err != WSAEWOULDBLOCK && err != WSAECONNRESET){ CON.write(0, "SocketError:Receive()"); };
			len = 0;
			return false;
		};
		len = error;
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Recv(char *data, word &len){
		int error = recv(S, data, NET_BUF_MAX, 0);
		if(error == SOCKET_ERROR){
			int err = WSAGetLastError();
			if(err != WSAEWOULDBLOCK && err != WSAECONNRESET){ CON.write(0, "SocketError:Recv()"); };
			len = 0;
			return false;
		};
		len = error;
		return true;
	};
	//--------------------------------------------------------------------------------
};
//=======================================================================================
#define NET_Connected 0
#define NET_Connecting 1
#define NET_Disconnected 2
//=======================================================================================
DWORD WINAPI ServerRunThread(LPVOID lpPtr);//FD
//--------------------------------------------------------------------------------
DWORD WINAPI CallServerThread(LPVOID lpPtr);//FD
//=======================================================================================
#define NET_NONE       0
#define NET_ACCEPT     1
#define NET_CONNECT    2
#define NET_DISCONNECT 3
#define NET_MSG        4
#define NET_STT        5
#define NET_PING       6
#define NET_PONG       7
//=======================================================================================
struct CNet{
	CSock PUT, GET, TMP, TMA;
	SOCKET ttg;
	byte state;
	char Sbuf[NET_BUF_MAX], Rbuf[NET_BUF_MAX];
	word Slen, Rlen, speed;
	Dword LST;
	CNet(){ Init();Create(); };
	~CNet(){ Close(); };
	//--------------------------------------------------------------------------------
	bool Init(){
		WORD winsock_ver = MAKEWORD(2, 2);
		WSADATA winsock_data;
		int error = WSAStartup(winsock_ver, &winsock_data);
		if(error != 0){ CON.write(0, "NETError:Init()");return false; };
		memset(Sbuf, 0, NET_BUF_MAX);Slen = 0;
		memset(Rbuf, 0, NET_BUF_MAX);Rlen = 0;
		LST = 0;
		speed = 200;
		ttg = INVALID_SOCKET;
		return true;
	};
	//--------------------------------------------------------------------------------
	bool Close(){
		PUT.Close();
		GET.Close();
		int error = WSACleanup();
		if(error == SOCKET_ERROR){ CON.write(0, "NETError:Close()");return false; };
		return true;
	};
	//--------------------------------------------------------------------------------
	void Create(){
		GET.addr.Clear();GET.InitUDP();GET.SetNoBlock(1);
		PUT.addr.Clear();
		TMP.addr.Clear();TMP.InitTCP();
		TMA.addr.Clear();TMA.InitTCP();
		//if(PUT.addr.set("wcmd_ip.txt")){
		//	CON.write(0, "NET:Create():SendTarget: '%s'", PUT.addr.IP5());
		//};
		char hostname[256];gethostname(hostname, 256);
		struct hostent *h = gethostbyname(hostname);
		int n = 0, ip;
		char *p;
		while((p = h->h_aliases[n++]) != NULL){ CON.write(0, "Alias: %s", p); };
		n = 0;
		while((p = h->h_addr_list[n]) != NULL && n < 16){ n++; };
		CON.write(0, "Host:'%s' have %i local addr", h->h_name, n);
		n = 0;
		while((p = h->h_addr_list[n]) != NULL && n < 16){
			ip = ntohl(*(int*) p);
			CON.write(0, "IP: %i.%i.%i.%i", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
			n++;
		};
		// GET.addr.set_ip(*(int*)h->h_addr_list[0]);
		//for(int i = 0;i < 10;i++){
		//	GET.addr.set_port(666 + i);
		//	if(GET.Bind()){ CON.write(0, "NET:Create():Binded on '%s'", GET.addr.IP5());break; };
		//	CON.write(0, "NET:Create():ERROR: '%s' port busy", GET.addr.IP5());
		//};
		if(TMP.addr.set("wcmd_ip.txt")){
			CON.write(0, "start server on '%s'", TMP.addr.IP5());
		};

		//int start_port = TMP.addr.ip4[4];
		//for(int i = 0;i < 10;i++){
		//	TMP.addr.set_port(start_port + i);
		//	if(TMP.Bind()){ CON.write(0, "NET:Create():Binded on '%s'", TMP.addr.IP5());break; };
		//	CON.write(0, "NET:Create():ERROR: '%s' port busy", TMP.addr.IP5());
		//};
		//if(TMP.Bind()){ CON.write(0, "NET:Create():Binded on '%s'", TMP.addr.IP5());};
		DWORD dwThreadID = 0;
		//HANDLE handle = CreateThread(NULL, 0, &CallServerThread, this, 0, &dwThreadID);
		DWORD dwThreadID2 = 0;
		HANDLE handle2 = CreateThread(NULL, 0, &ServerRunThread, this, 0, &dwThreadID2);

		//for(int i = 0;i < 10;i++){
		//	TMA.addr.set_port(669 + i);
		//	if(TMA.Bind()){ CON.write(0, "NET:Create():TMA:Binded on '%s'", TMA.addr.IP5());break; };
		//	CON.write(0, "NET:Create():ERROR: '%s' port busy", TMA.addr.IP5());
		//};
	};
	//--------------------------------------------------------------------------------
	int Process(){
		byte cmd = 0, ret = NET_NONE;
		word got = 0;
		if(GET.Receive(Rbuf, got)){
			Rlen = 0;Read(&cmd, 1);ret = cmd;
			switch(cmd){
				case NET_STT:{return ret;};
				case NET_MSG:{CON.write(0, "GOT MSG");break;};
			};
		};
		return ret;
	};
	//--------------------------------------------------------------------------------
	void Write(const void *sf, word len){ memcpy(&Sbuf[Slen], sf, len);Slen += len; };
	void Read(void *sf, word len){ memcpy(sf, &Rbuf[Rlen], len);Rlen += len; };
	//--------------------------------------------------------------------------------
	void Send2(byte cmd){
		Sbuf[0] = cmd;
		if(Slen < 1)Slen = 1;
		GET.SendTo(&PUT.addr.A, Sbuf, Slen);
	};
	//--------------------------------------------------------------------------------
	void Send3(byte cmd){
		if(TMA.Connect(&PUT.addr)){
			CON.write(0, "connected to '%s'", PUT.addr.IP5());
		};
	};
	//--------------------------------------------------------------------------------
	void Send4(char *buf){
		int nRet = send(TMA.S, buf, strlen(buf), 0);
		CON.write(0, "SEND MSG:'%s' %i", buf, nRet);
	};
	//--------------------------------------------------------------------------------
	bool ReadySend(){
		if(FPS.Milliseconds() - LST > speed){
			LST = FPS.Milliseconds();
			Slen = 1;
			return true;
		};
		return false;
	};
	//--------------------------------------------------------------------------------
};
//=======================================================================================
const DWORD32 WCMD_REQUEST = ('w') | ('c' << 8) | ('m' << 16) | ('R' << 24);
const DWORD32 WCMD_ANSWER = ('w') | ('c' << 8) | ('m' << 16) | ('A' << 24);
enum WCMD {
	PING=0,
	// ships
	SHIPS_INFO,// get id's num_ships
	SET_SHIP_TRAJECTORY,// ship_id, traj_id
	SHIP_STOP,// id
	SHIP_SWIM,// id
	SHIP_SWIM_TO,//id x,y
	// trajectory
	GET_TRAJECTORY_LIST,// num id's(0,1,2)
	GET_TRAJECTORY_INFO,
	LOAD_TRAJECTORY,
	NUM_WCMDS
};
const char *WCMD_S[WCMD::NUM_WCMDS]{
	"PING",
	// ships
	"SHIPS_INFO",// get id's num_ships
	"SET_SHIP_TRAJECTORY",// ship_id, traj_id
	"SHIP_STOP",// id
	"SHIP_SWIM",// id
	"SHIP_SWIM_TO",//id x,y
	// trajectory
	"GET_TRAJECTORY_LIST",// num id's(0,1,2)
	"GET_TRAJECTORY_INFO",
	"LOAD_TRAJECTORY",// id
};
int Process_WCMD(const char *buf, int len);//FD 
//=======================================================================================
DWORD WINAPI ServerRunThread(LPVOID lpPtr){
	CNet *pThis = reinterpret_cast<CNet*>(lpPtr);
	char szBuf[255];
	int len = 0;
	memcpy(szBuf, "test ping\0", 10);
	SOCKET TTA;
	float sock_conn_time = time();
	Sleep(3000);
	while(1){
		if(pThis->TMP.Bind()){ CON.write(0, "NET:Create():Binded on '%s'", pThis->TMP.addr.IP5());};
		while(pThis->TMP.S != INVALID_SOCKET){
			if(pThis->ttg != INVALID_SOCKET){
				//send(pThis->ttg, szBuf, 10, 0);
				len = recv(pThis->ttg, szBuf, sizeof(szBuf), 0);
				if(len > 0){
					szBuf[len] = 0;
					CON.write(0, "GOT MESSAGE:%s", szBuf);//memset(szBuf, 0, 255); 
					Process_WCMD(szBuf, len);
					pThis->ttg = INVALID_SOCKET;
					CON.write(0, "Connection closed, waiting connection");
					break;
				} else {
					if (len == -1) {
						CON.write(0, "Connection Lost");
						pThis->ttg = INVALID_SOCKET;
					}
				}
				//float ctime = time()-sock_conn_time;
				//if(ctime>2){
					//pThis->ttg = INVALID_SOCKET;
					//CON.write(0, "Connection timeout");
					//break;
				//}
				CON.write(0, "waiting for commands recv = %i",len);
			} else {
				// wait for connection
				CON.write(0, "waiting for connection");
				if (pThis->TMP.Listen(1)) {
					if (pThis->TMP.Accept(&TTA)) {
						pThis->ttg = TTA;
						sock_conn_time = time();
					} else CON.write(0, "Accept fails");
				} else CON.write(0, "Listen fails");
			}
			if(applooprun)	Sleep(100); else{CON.write(0, "SERVER BREAK1"); break;}
		};
		if(applooprun) Sleep(100); else{CON.write(0, "SERVER BREAK2"); break;}
	}
	//if(pThis->TMP.S != INVALID_SOCKET){
	//	if(TTA != INVALID_SOCKET) 		
	//}
	CON.write(0, "exit thread");
	return 0;
};
//--------------------------------------------------------------------------------
//DWORD WINAPI CallServerThread(LPVOID lpPtr){
//	CNet *pThis = reinterpret_cast<CNet*>(lpPtr);
//	while(TRUE){
//		Sleep(500);
//	};
//	return 0;
//};
//=======================================================================================
CNet NET;