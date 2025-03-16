//For commn
#include <iostream>
#include <string>
//#include <stdlib.h>
//#include <assert.h>

#include <Winsock2.h>
#pragma comment (lib,"wsock32.lib")

#define SEND_RQ(MSG) \
                /*cout<<send_str;*/ \
  send(sock,MSG,strlen(MSG),0);

using namespace std;
//<exe> hostname api parameters
int request(char *hostname, char *api, char *parameters, string &message){
	CON.write(1, "marker_request:[%s%s%s]", hostname, api, parameters);
	WSADATA	WsaData;
	WSAStartup(0x0101, &WsaData);
	sockaddr_in       sin;
	int sock = socket(AF_INET, SOCK_STREAM, 0); if(sock == -1) { return -100; }
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short) 80);
	struct hostent * host_addr = gethostbyname(hostname);
	if(host_addr == NULL) { CON.write(2, "Unable to locate host");return -103; }
	sin.sin_addr.s_addr = *((int*) *host_addr->h_addr_list);
	//CON.write(0,"Port : %i, Address : %i",sin.sin_port,sin.sin_addr.s_addr);
	if(connect(sock, (const struct sockaddr *)&sin, sizeof(sockaddr_in)) == -1) { CON.write(2, "connect failed");return -101; }
	string send_str;
	SEND_RQ("GET ");
	SEND_RQ(api);
	SEND_RQ(" HTTP/1.1\r\n");
	SEND_RQ("Host: ");
	SEND_RQ(hostname);
	SEND_RQ("\r\n");
	SEND_RQ("Connection: keep-alive\r\n");
	SEND_RQ("Cache-Control: max-age=0\r\n");
	SEND_RQ("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n");
	SEND_RQ("User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.116 Safari/537.36\r\n");
	SEND_RQ("Accept-Encoding: gzip,deflate,sdch\r\n");
	SEND_RQ("Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4\r\n");

	//SEND_RQ("GET ");
	//SEND_RQ(api);
	//SEND_RQ(" HTTP/1.1\r\n");
	//SEND_RQ("Accept: */*\r\n");
	//SEND_RQ("User-Agent: Mozilla/4.0\r\n");

	//char content_header[100];
	//sprintf(content_header,"Content-Length: %d\r\n",strlen(parameters));
	//SEND_RQ(content_header);
	//SEND_RQ("Accept-Language: en-us\r\n");
	//SEND_RQ("Accept-Encoding: gzip, deflate\r\n");
	//SEND_RQ("Host: ");
	//SEND_RQ("hostname");
	//SEND_RQ("\r\n");
	//SEND_RQ("Content-Type: application/x-www-form-urlencoded\r\n");

	//If you need to send a basic authorization
	//string Auth        = "username:password";
	//Figureout a way to encode test into base64 !
	//string AuthInfo    = base64_encode(reinterpret_cast<const unsigned char*>(Auth.c_str()),Auth.length());
	//string sPassReq    = "Authorization: Basic " + AuthInfo;
	//SEND_RQ(sPassReq.c_str());

	SEND_RQ("\r\n");
	SEND_RQ("\r\n");
	//SEND_RQ(parameters);
	//SEND_RQ("\r\n");
	//CON.write(0,"%s",send_str.c_str());
	//CON.write(0,"####HEADER####");  CON.flush();
	char c1[1];
	int l, line_length=0;//PVS
	bool loop = true;
	bool bHeader = false;

	while(loop) {
		l = recv(sock, c1, 1, 0);
		if(l < 0) loop = false;
		if(c1[0] == '\n') {
			if(line_length == 0) loop = false;
			line_length = 0;
			if(message.find("200") != string::npos)bHeader = true;
		} else if(c1[0] != '\r') line_length++;
		message += c1[0];
	}
	//CON.write(0,"%s",message.c_str()); CON.flush();

	message = "";
	if(1) {
		// CON.write(0,"####BODY####");
		char p[1024];
		while((l = recv(sock, p, 1023, 0)) > 0)  {
			cout.write(p, l);
			p[l] = '\0';
			message += p;
		}
		// CON.write(0,"%s",message.c_str());
	} else {
		return -102;
	}
	WSACleanup();
	return 0;
};

cstring HTTP_cmd;
bool SEND_HTTP_BOOL = true;
void SEND_HTTP(){
	string message;
	while(SEND_HTTP_BOOL){
		if(HTTP_cmd.size){
			request("192.168.1.5", HTTP_cmd.text, "", message);
			HTTP_cmd.size = 0;
			CON.write(0, "HTTP_MARKER_SEND_DONE");
		};
		Sleep(100);
		//CON.write(1,"SEND_HTTP::exit loop %i",SEND_HTTP_BOOL);
	};
	CON.write(1, "SEND_HTTP::exit loop");
};
Thread THHPthread(&SEND_HTTP);

//----------------------------------
//boat - 00,11(17)
//cmd  - 00 - all off
//       FF(255) - all on
//------------------- lights
//       10(16),20(32),30(48),40(64) - off
//       11(17),21(33),31(49),41(65) - on
//------------------- IR
//       50(80) - IR1 - on, IR2 - blink
//       51(81) - switched blink
//       5[2..F]((82)..(95)) - blink freq  13
//----------------------------------
 //char buffer[33];
 // for(int i=0;i<500;i++){
 //  itoa(i,buffer,16);CON.write(2,"i[%i] to hex h[%s]",i,buffer);
 // };
//int marker_light(boat,lnum,stt)
const char MARKERS_LIGHT_ON = 255;//FF
const char MARKERS_LIGHT_OFF = 0;//0
const char MARKER_FOUND = 80;//50
const char MARKER_DETECT = 81;//51
const char MARKER_FREQ = 82;//52..5F => +[0..13]
const char MARKER_ALL = 255;
const char MARKER_SOUND = 210;//d2
const char MARKER_BATTERY = 173;//ad
//======================================================================================
struct MarkerCMDBuffer{
	int ms;
	vec3i *stt;//0-used,1-boat,2-cmd
	MarkerCMDBuffer(){ ms = 0;stt = NULL; };
	bool off_all(){
		for(int i = 0;i < ms;i++){
			if(stt[i][0] == 0)continue;
			return false;
		};
		return true;
	};
	void push(const byte &boat, const byte &cmd){
		//for(int i=0;i<ms;i++){if(stt[i][0]!=0 && stt[i][1]==boat){stt[i][2]=cmd;return;};};//renew state
		int idx = -1;
		for(int i = 0;i < ms;i++){ if(stt[i][0] == 0){ idx = i;break; }; };//get unused idx
		if(idx == -1){ idx = ms;ms++;RESIZE(stt, vec3i, ms); };//add new to stack, get new idx
		stt[idx][0] = 1;//set on
		stt[idx][1] = boat;
		stt[idx][2] = cmd;
	};
	void sync(){//if(noCOMconnection)return;
		for(int i = 0;i < ms;i++){
			if(stt[i][0]){
				CPT.send_maker_cmd(stt[i][1], stt[i][2]);
				stt[i][0] = 0;//set off
				Sleep(200);
			};
		};
	};
};
MarkerCMDBuffer MBUF;
volatile int MBUF_SEND_LOOP = true;//PVS

void SEND_MCMD_LOOP(){
	CON.write(1, "MCMD::enter loop");
	while(MBUF_SEND_LOOP){
		//CON.write(0,"loop %i",COM_SEND_CMD);
		Sleep(1000);
		MBUF.sync();
	};
	MBUF_SEND_LOOP = -1;
	CON.write(1, "MCMD::exit loop");
};
Thread MCMDthread(&SEND_MCMD_LOOP);

int marker_send(int boat, int cmd){
	//marker_send(AllShip_cb->Down?MARKER_ALL:selShipID(),);};
	byte lcmd = LightONbtn->Down ? MARKERS_LIGHT_ON : MARKERS_LIGHT_OFF;
	MBUF.push(255, lcmd);
	if(cmd == lcmd)return 0;
	MBUF.push(boat, cmd);
	//MBUF.sync();
	return 0;

	//cstring command;command.init();
	//CON.write(2,"%c=%i %c=%i %c=%i %c=%i %c=%i %c=%i",bbuf[0],bbuf[0],bbuf[1],bbuf[1],bbuf[2],bbuf[2],bbuf[3],bbuf[3],bbuf[4],bbuf[4],bbuf[5],bbuf[5]);
	CPT.send_maker_cmd(boat, cmd);return 0;
	//char scmd=boat;CPT.write_char(scmd);
	//scmd=cmd;CPT.write_char(scmd);
	//scmd=0x5c;CPT.write_char(scmd);
	//string message;
	//char bbuf[33],cbuf[33];itoa(boat,bbuf,16);itoa(cmd,cbuf,16);
	//if(cmd<16){cbuf[1]=cbuf[0];cbuf[0]='0';cbuf[2]='\0';};
	//if(boat<16){bbuf[1]=bbuf[0];bbuf[0]='0';bbuf[2]='\0';};
	//HTTP_cmd.print("/index.py?boat=%s&cmd=%s",bbuf,cbuf);
   // return request("192.168.1.5", command.text, "", message);
};
//-----------------test
int http_main(bool c){
	string message;   //192.168.1.1/redirect.cgi?arip=192.168.1.1
	//CON.write(0,"HTTP_REQ:www.ya.ru, /yandsearch, ?text=test&lr=2");
   //request("192.168.7.2", "index.py", "?boat=ff&cmd=2f", message);
   //http://192.168.7.2/index.py?boat=ff&cmd=f1
	if(c)
		return request("192.168.7.2", "/index.py?boat=11&cmd=00", "", message);
	else
		return request("192.168.7.2", "/index.py?boat=00&cmd=11", "", message);
	//return request("192.168.1.1", "/redirect.cgi", "?arip=192.168.1.1", message);
	// message contains response!
};
//------------------------------------------- ping web2can
int get_http_state(const char *adr){
	//CON.write(1,"HTTP::request:[%s]",adr);  
	WSADATA WsaData;WSAStartup(0x0101, &WsaData);
	//CON.write(0,"HTTP::-1");
	sockaddr_in sin;
	int sock = socket(AF_INET, SOCK_STREAM, 0);if(sock == -1){ return -100; };
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short) 80);
	struct hostent *host_addr = gethostbyname(adr);if(host_addr == NULL){ CON.write(2, "Unable to locate host [%s]", adr);return -103; }
	sin.sin_addr.s_addr = *((int*) *host_addr->h_addr_list);//CON.write(0,"Port : %i, Address : %i",sin.sin_port,sin.sin_addr.s_addr);
	//CON.write(0,"HTTP::0");
	if(connect(sock, (const struct sockaddr *)&sin, sizeof(sockaddr_in)) == -1){ CON.write(2, "connect failed [%s]", adr);return -101; };
	string send_str;
	SEND_RQ("GET ");SEND_RQ("ping");
	SEND_RQ(" HTTP/1.1\r\n");
	SEND_RQ("Host: ");SEND_RQ(adr);SEND_RQ("\r\n");
	SEND_RQ("Connection: keep-alive\r\n");
	SEND_RQ("Cache-Control: max-age=0\r\n");
	SEND_RQ("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n");
	SEND_RQ("User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.116 Safari/537.36\r\n");
	SEND_RQ("Accept-Encoding: gzip,deflate,sdch\r\n");
	SEND_RQ("Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4\r\n");
	SEND_RQ("\r\n");SEND_RQ("\r\n");
	char c1[1];
	int l, line_length=0;//PVS
	bool loop = true;
	bool bHeader = false;
	string message;
	//CON.write(0,"HTTP::1");
	while(loop){
		l = recv(sock, c1, 1, 0);
		if(l < 0) loop = false;
		if(c1[0] == '\n') {
			if(line_length == 0) loop = false;
			line_length = 0;
			if(message.find("200") != string::npos)bHeader = true;
		} else if(c1[0] != '\r') line_length++;
		message += c1[0];
	};
	//if(!bHeader)CON.write(2,"NO HEADER");
	//CON.write(0,"%s",message.c_str()); CON.flush();
	message = "";
	//CON.write(0,"HTTP::2");
	if(1){
		// CON.write(0,"####BODY####");
		char p[1024];
		while((l = recv(sock, p, 1023, 0)) > 0){
			cout.write(p, l);
			p[l] = '\0';
			message += p;
		};
		// CON.write(0,"%s",message.c_str());
	} else{
		return -102;
	}
	WSACleanup();
	return 0;
};
//----------
struct HTTP_ping{
	int loopthread, state, lstate;
	cstring adress;
	HTTP_ping(const char *adr){
		adress.print("%s\0", adr);
		state = -2;lstate = -2;
		loopthread = 0;
		//startThread();
	};
	//------------------ thead code
	static DWORD WINAPI StaticThreadStart(void *Param){
		HTTP_ping *This = (HTTP_ping*) Param;
		return This->ThreadStart();
	};
	DWORD ThreadStart(void){
		loopthread = 1;
		CON.write(0, "HTTP_ping start loop thread for [%s]", adress.text);
		while(loopthread){
			//lstate=state;
			state = get_http_state(adress.text);//CON.write(0,"HTTP::answer %i",state);
			Sleep(10000);
		};//while
		CON.write(0, "HTTP_ping exit loop thread for [%s]", adress.text);
		loopthread = -1;
		return 1;
	};
	void startThread(){
		if(loopthread != 0)return;loopthread = 1;
		DWORD ThreadID;
		CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
	};
	//-------------------------- thread code
};
//-------------------------------------------

//====================================================================================== listenger
//-----------------------------
#include "..\\inc\\net.h"
//-----------------------------
struct LAN_MSG{//:15123
	int loopthread, state, lstate;
	cstring adress;
	CSock GET;
	SOCKET ttg;
	//byte state;
	char Sbuf[NET_BUF_MAX], Rbuf[NET_BUF_MAX];
	word Slen, Rlen, speed;
	Dword LST;
	//------------------ thead code
	static DWORD WINAPI StaticThreadStart(void *Param){
		LAN_MSG *This = (LAN_MSG*) Param;
		return This->ThreadStart();
	};
	DWORD ThreadStart(void){
		loopthread = 1;
		CON.write(0, "LAN_MSG start loop thread for [%s]", adress.text);
		WORD winsock_ver = MAKEWORD(2, 2);
		WSADATA winsock_data;
		int error = WSAStartup(winsock_ver, &winsock_data);
		if(error != 0){ CON.write(0, "NETError:Init()"); };
		memset(Sbuf, 0, NET_BUF_MAX);Slen = 0;
		memset(Rbuf, 0, NET_BUF_MAX);Rlen = 0;
		LST = 0;
		speed = 200;
		ttg = INVALID_SOCKET;

		GET.addr.Clear();GET.InitUDP();GET.SetNoBlock(1);

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

		for(int i = 0;i < 10;i++){
			GET.addr.set_port(15123);
			if(GET.Bind()){ CON.write(0, "NET:Create():Binded on '%s'", GET.addr.IP5());break; };
			CON.write(0, "NET:Create():ERROR: '%s' port busy", GET.addr.IP5());
		};

		char szBuf[255];
		int len = 0;

		while(loopthread){
			if(ttg != INVALID_SOCKET){
				len = recv(ttg, szBuf, sizeof(szBuf), 0);
				if(len > 0){ CON.write(0, "GOT MESSAGE3:%s", szBuf);memset(szBuf, 0, 255); };
			};
			Sleep(100);
		};//while
		CON.write(0, "LAN_MSG exit loop thread for [%s]", adress.text);
		GET.Close();
		loopthread = -1;
		return 1;
	};
	void startThread(){
		if(loopthread != 0)return;loopthread = 1;
		DWORD ThreadID;
		CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
	};
	//-------------------------- thread code
};// */
//======================================================================================

//======================================================================================