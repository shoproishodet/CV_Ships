//======================================================================================
int WndW, WndH, WndL, WndT;
bool WndF = 1;
HDC hDC;
HWND hWnd;
HGLRC glContext;//,glContext2;
//======================================================================================
void SetMouse(int x, int y){
	if(!Active)return;
	mpos.x = x;
	mpos.y = y;//WndH/2;
	ClientToScreen(hWnd, &mpos);SetCursorPos(mpos.x, mpos.y);
};
//======================================================================================
void GetMouse(){//filter
	GetCursorPos(&mpos);ScreenToClient(hWnd, &mpos);
	lmx = mx;lmy = my;
	mx = (float) mpos.x;
	my = (float) mpos.y;
};
//======================================================================================
#include "FontGL.h"
CFontGL font, BIGfont;
//======================================================================================
void get_display(){
	DEVMODE mode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
	WndW = mode.dmPelsWidth;//GetSystemMetrics(SM_CXSCREEN)
	WndH = mode.dmPelsHeight;
	if(WndW <= 0 || WndW > 9000)WndW = 1024;
	if(WndH <= 0 || WndH > 9000)WndH = 768;
};
//======================================================================================
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//====================================================================================== //LoadIcon(NULL,IDI_APPLICATION)
bool GL_Window(char *title, int width, int height, bool fs){
	//CON.write(0, "CPU:(%s)%iMHz ", GetProcessorVendorId(), GetCpuSpeed());
	//---win ver
	OSVERSIONINFOEX osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(GetVersionEx((LPOSVERSIONINFOA) (&osv)) != NULL){
		char osname[10];
		if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 2)memcpy(osname, "8\0", 3);
		else if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 1)memcpy(osname, "7\0", 3);
		else if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 0)memcpy(osname, "Vista\0", 7);
		else if(osv.dwMajorVersion == 5 && osv.dwMinorVersion == 2)memcpy(osname, "2003\0", 6);
		else if(osv.dwMajorVersion == 5 && osv.dwMinorVersion == 1)memcpy(osname, "XP\0", 4);
		else if(osv.dwMajorVersion == 5 && osv.dwMinorVersion == 0)memcpy(osname, "2000\0", 6);
		else if(osv.dwMajorVersion == 4 && osv.dwMinorVersion == 0)memcpy(osname, "NT4\0", 5);
		else if(osv.dwMajorVersion < 4)memcpy(osname, "Old\0", 5);
		else if(osv.dwMajorVersion > 6)memcpy(osname, "New\0", 5);
		CON.write(0, "OS:Windows %s build[%i] Ver[%i/%i] SP[%i/%i]", osname, osv.dwBuildNumber, osv.dwMajorVersion, osv.dwMinorVersion, osv.wServicePackMajor, osv.wServicePackMinor);
	} else CON.write(1, "can't get os version");
	//---mem
	//MEMORYSTATUS stat; // not WORK?
	//GlobalMemoryStatus(&stat);
	//CON.write(0,"RAM: %s total / %s free - [%ld] free",MKB(stat.dwTotalPhys),MKB(stat.dwAvailPhys),100-stat.dwMemoryLoad);
	//---names
	LPTSTR lpszSystemInfo;
	DWORD cchBuff = 256;
	TCHAR tchBuffer[1024];
	lpszSystemInfo = tchBuffer;
	if(GetComputerName(lpszSystemInfo, &cchBuff))CON.write(0, "Computer name: %s", lpszSystemInfo);
	if(GetUserName(lpszSystemInfo, &cchBuff))CON.write(0, "User name: %s", lpszSystemInfo);
	//---sys info
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	CON.write(0, "OEM ID: %u, %u processors, Page size: %u, Processor type: %u", siSysInfo.dwOemId, siSysInfo.dwNumberOfProcessors, siSysInfo.dwPageSize, siSysInfo.dwProcessorType);
	//---done
	get_display();
	CON.write(0, "Create OpenGL Window [%ix%i] %s", WndW, WndH, WndF ? "fullscreen" : "windowed");                 //stencil
	PIXELFORMATDESCRIPTOR pfd = {0,1,PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,32,0,0,0,0,0,0,0,0,0,0,0,0,0,32,1,1,PFD_MAIN_PLANE,0,0,0,0};
	DEVMODE dmScreenSettings = {"",0,0,sizeof(dmScreenSettings),0,DM_PELSWIDTH | DM_PELSHEIGHT,0,0,0,0,0,0,0,0,0,0,0,0,0,"",0,0,WndW,WndH,0,0,0,0,0,0,0,0,0,0};
	WNDCLASSEX wc = {sizeof(WNDCLASSEX),0,WndProc,0,0,NULL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(101)),LoadCursor(NULL,IDC_ARROW),(HBRUSH) (COLOR_WINDOW + 1),NULL,"GL",LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(101))};
	RegisterClassEx(&wc);
	if(fs){
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);//hWnd=CreateWindow(title,0,WS_POPUP|WS_VISIBLE|WS_MAXIMIZE,0,0,0,0,0,0,0,0);//bug
		hWnd = CreateWindowEx(WS_EX_APPWINDOW, "GL", title, WS_POPUP | WS_VISIBLE, 0, 0, WndW, WndH, NULL, NULL, NULL, NULL);//|WS_MAXIMIZE
		hDC = GetDC(hWnd);
	} else{
		hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, "GL", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, WndW, WndH, NULL, NULL, NULL, NULL);
		hDC = GetDC(hWnd);
		//ShowWindow(hWnd,SW_SHOW);
		ShowWindow(hWnd, SW_MAXIMIZE);
	};
	UpdateWindow(hWnd);SetForegroundWindow(hWnd);SetFocus(hWnd);
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	glContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, glContext);
	//GL
	GLenum err = glewInit();
	if(GLEW_OK != err){ CON.write(2, "GLEW Error: "); };//%s", glewGetErrorString(err));}
	if(wglSwapIntervalEXT != NULL){
		wglSwapIntervalEXT(Vsync);
	} else{
		CON.write(LOG_WARNING, "WGL:we got a problem with wgl (wglSwapIntervalEXT)");Vsync = true;
	};
	CON.write(0, "OpenGl %s", glGetString(GL_VERSION));
	CON.write(0, "Vendor %s", glGetString(GL_VENDOR));
	CON.write(0, "Render %s", glGetString(GL_RENDERER));
	CON.write(0, "Vsync is %s", Vsync ? "on" : "off");
	MM.init();
	font.Init("Comic Sans MS", 15);
	BIGfont.Init("Consolas", 25);
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glEnable(0x864F);
	glEnable(GL_CULL_FACE);glCullFace(GL_BACK);//GL_FRONT);//
	//glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
	glEnable(GL_TEXTURE_2D);glEnable(GL_COLOR_MATERIAL);glBlendFunc(GL_ONE, GL_ONE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	return 0;
};
//======================================================================================
int applooprun=255;
//int keys[512];
//int num_keys = 0;
//int keys_buffer[64];
//======================================================================================
void process_key(int i, bool state){
	if(!Active)return;
	int j = i;
	//if(isascii(j)){
	 //if(key[K_SHIFT])j=toupper(j);else j=tolower(j);
	 //if(t_eng_big(j)){ if(!key[K_SHIFT])j=tolower(i);//}else{
	//}else{
	j = trans_sys_key(i);
	if(j == i)j = sys_spec(i);
	if(j == i && key[K_SHIFT])j = sys_shift_num(i);
	if(j == i && isascii(j))if(!key[K_SHIFT])j = tolower(i);
	//};
	lkey[j] = key[j];//lkb_key=kb_key;
	key[j] = state;
	kb_key = j;
	keypressed = state;
	//if(key[K_SHIFT])CON.write(0,"SHIFT %i %i %i",j,trans_sys_key(j),isascii(j));
	//if(key[K_ALT])CON.write(0,"ALT");
	//if(key[K_CTRL])CON.write(0,"CTRL");
	//if(key[K_DEL])CON.write(0,"DEL");
};
//======================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_ACTIVATE:if(!HIWORD(wParam) && LOWORD(wParam) != WA_INACTIVE){ Active = true; } else{ Active = false; };return 1;
		case WM_SIZE:if(wParam == SIZE_MINIMIZED){ Active = true; } else{ Active = false; };WndW = LOWORD(lParam);WndH = HIWORD(lParam);
		case WM_MOVE:RECT wRC;GetWindowRect(hWnd, &wRC);WndL = wRC.left;WndT = wRC.top;return 1;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:process_key((int) wParam, true);return 1;
		case WM_SYSKEYUP:
		case WM_KEYUP:process_key((int) wParam, false);return 1;
		case WM_CLOSE:applooprun = 0;CON.write(0, "applooprun=0");return 0;
		case 0x020A:wheel = ((short) HIWORD(wParam)) > 0 ? 1 : -1;return 1;/*#define WM_MOUSEWHEEL 0x020A*/
		case WM_SYSCOMMAND:if(wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)return 1;
			//case WM_TIMER:read_direct_input();return 1;
	};
	return DefWindowProc(hWnd, msg, wParam, lParam);
};
//======================================================================================
extern void process();
extern void render();
extern int main(int argc, char **argv);
extern void main_end();
//======================================================================================
void apploop(){
	MSG msg;
	CON.write(0, "--------- start main loop ---------");
	while(applooprun){ //CON.write(0,"while(applooprun)");
		if(Active){
			lmouse[0] = mouse[0];mouse[0] = GetAsyncKeyState(VK_LBUTTON);
			lmouse[1] = mouse[1];mouse[1] = GetAsyncKeyState(VK_RBUTTON);
			lmouse[2] = mouse[2];mouse[2] = GetAsyncKeyState(VK_MBUTTON);
		};
		wheel = 0;
		for(int i = 0;i < 3;i++){
			mouseclick[i] = lmouseclick[i] = false;
			if(mouse[i] && !lmouse[i])mouseclick[i] = true;
			if(!mouse[i] && lmouse[i])lmouseclick[i] = true;
		};
		//memcpy(lkey,key,255);//keypressed=false;//memset(key,0,255);lkb_key=kb_key;kb_key=0;
		for(int i = 0;i < 255;i++)lkey[i] = key[i];
		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)){
			if(!GetMessage(&msg, NULL, 0, 0))break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		};
		FPS.Update();
		//if(Active){//Sleep(100);continue;};
		process();//CON.write(0,"process");
		render();//CON.write(0,"render");
	   //};
		SwapBuffers(hDC);//SwapBuffers(wglGetCurrentDC());//hDC?    EXEPTION::16.09.2014
		//CON.write(0,"SwapBuffers");//Sleep(1);?
	};
	//CON.write(0,"end");
	//CON.flush();
};
//======================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	applooprun = true;
	int argc = 1;
	char *argv[256];argv[0] = "none";
	while(*lpCmdLine && argc < 256){
		while(*lpCmdLine && (*lpCmdLine <= ' ' || *lpCmdLine == '"'))lpCmdLine++;
		if(*lpCmdLine){
			if(*lpCmdLine == '"')lpCmdLine++;
			argv[argc++] = lpCmdLine;
			while(*lpCmdLine && *lpCmdLine > ' ' && *lpCmdLine != '"')lpCmdLine++;
			if(*lpCmdLine)*(lpCmdLine++) = '\0';
		};
	};
	CON.write(0, "--------- start init loop ---------");
	GL_Window("GL", WndW, WndH, 1);
	ShowCursor(false);
	main(argc, argv);
	apploop(); /// MAIN LOOP
	CON.write(0, "--------- end main loop ---------");CON.flush();
	main_end();
	//CON.write(0,"MM.del(");CON.flush();
	MM.del();
	//GLWnd_del();
	CON.write(0, "_exit(1)");
	CON.flush();
	Sleep(1000);
	_exit(1);// TODO: find why it hangs
	return 0;
};
//======================================================================================
/*vec3f GetMouseDir(){//move from here TODO
 vec3f S;
 mat4 MVM,PM;
 int VP[4];
 glGetFloatv(GL_MODELVIEW_MATRIX,MVM.m);
 glGetFloatv(GL_PROJECTION_MATRIX,PM.m);
 glGetIntegerv(GL_VIEWPORT,VP);
 UnProject(vec3f(mx,WndH-my,1),MVM,PM,VP,S);//
 //vec3f mDir(S-pl.orig);mDir.norm();
 return S;
}; */
//======================================================================================