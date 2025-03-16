//======================================================================================
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>//file
#include <math.h>//sqrt...
#include <stdlib.h>//free
#include <mmsystem.h>//timer
#pragma comment(lib,"winmm.lib")//timer
//======================================================================================
#define GLEW_STATIC 1
#include "glew/glew.c"
#include "GL/gl.h"
#pragma comment(lib,"opengl32.lib")
//======================================================================================
#define loop0i(end_l) for(int i=0;i<end_l;++i)
#define loop0j(end_l) for(int j=0;j<end_l;++j)
#define loop0k(end_l) for(int k=0;k<end_l;++k)
#define loopi(start_l,end_l) for(int i=start_l;i<end_l;++i)
#define loopj(start_l,end_l) for(int j=start_l;j<end_l;++j)
#define loopk(start_l,end_l) for(int k=start_l;k<end_l;++k)
#define loopl(start_l,end_l) for(int l=start_l;l<end_l;++l)
#define loopm(start_l,end_l) for(int m=start_l;m<end_l;++m)
#define loopn(start_l,end_l) for(int n=start_l;n<end_l;++n)
//======================================================================================
#include "keyboard.h"
#include "memory.h"
#include "string.h"
#include "log2.h"
//======================================================================================
#define EPSILON 1e-6f
#ifndef byte
typedef unsigned char byte;
#endif
#ifndef word
typedef unsigned short word;
#endif
#ifndef Dword      
#define Dword unsigned int
#endif
//qword [0 to 4.294.967.295]
#ifndef Qword      
#define Qword unsigned long
#endif
#define PI 3.14159265358979323846f
#define HALFPI 1.57079632679489661923f
#define TWOPI 6.283185307f
#define PI_SQR 9.86960440108935861883449099987615114f;
//#define HPI PI/2.0f
#define D2R (PI/180.0f)
#define R2D (180.0f/PI)
//======================================================================================
#define	F_NAN(x)			(((*(const unsigned long *)&x) & 0x7f800000) == 0x7f800000)
#define F_INF(x)			(((*(const unsigned long *)&x) & 0x7fffffff) == 0x7f800000)
#define F_IND(x)			((*(const unsigned long *)&x) == 0xffc00000)
#define	FLOAT_IS_DENORMAL(x)	(((*(const unsigned long *)&x) & 0x7f800000) == 0x00000000 && ((*(const unsigned long *)&x) & 0x007fffff) != 0x00000000 )
#ifndef RAD2DEG
#define RAD2DEG(x)((float)(x)*(float)(180.f/M_PI))
#endif
#ifndef DEG2RAD
#define DEG2RAD(x)((float)(x)*(float)(M_PI/180.f))
#endif
#ifndef mmin
#define mmin(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef mmax
#define mmax(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef sign_b
#define sign_b(a) ((a)>0?1:-1)
#endif
float NormRad(float a){
	while(a >= PI)a -= TWOPI;
	while(a < -PI)a += TWOPI;
	return a;
};
float NormDeg(float a){
	while(a >= 180)a -= 360;
	while(a < -180)a += 360;
	return a;
};
inline float clamp(float x, float a, float b){ return x<a ? a : x>b ? b : x; };
//======================================================================================
char *MKBstr = new char[512];
const char *MKB(const Dword &i){
	//char *str=new char[256];//freemem ???
	memset(MKBstr, 0, 256);
	Dword gb, mb, kb, b, gb2b, mb2b;//Dword lim == 1024*1024*1024*4 ~eq 4Gb -> no Tb
	gb = i >= 1073741824 ? i / 1073741824 : 0;gb2b = gb ? gb * 1073741824 : 0;
	mb = i >= 1048576 ? (i - gb2b) / 1048576 : 0;mb2b = mb ? mb * 1048576 : 0;
	kb = i >= 1024 ? (i - gb2b - mb2b) / 1024 : 0;
	b = i - gb2b - mb2b - (kb ? kb * 1024 : 0);
	if(gb)vspf(MKBstr, "%iGb", gb);
	if(mb)vspf(MKBstr, "%s%s%iMb", MKBstr, gb ? " " : "", mb);
	if(kb)vspf(MKBstr, "%s%s%iKb", MKBstr, gb || mb ? " " : "", kb);
	if(b || (!b && !gb && !mb && !kb))vspf(MKBstr, "%s%s%ib", MKBstr, gb || mb || kb ? " " : "", b);//fix draw zero
	return MKBstr;
};
//======================================================================================
#include "fps.h"
CTimer FPS;
//======================================================================================
//Y UP - vec3f(dir-roll,up-yaw,right-pitch)
#define YAW   1
#define PITCH 2
#define ROLL  0
#define V_DIR   0
#define V_UP    1
#define V_RIGHT 2
#include "vector.h"
#include "matrix.h"
#include "quat.h"

//typedef bool vec3bl[3];//???
typedef bool vec4bl[3];//??? handy
#define COPY(dest,v1)dest[0]=v1[0];dest[1]=v1[1];dest[2]=v1[2];
//#define COPY4(dest,v1)dest[0]=v1[0];dest[1]=v1[1];dest[2]=v1[2];dest[3]=v1[3];
#define COPY2(dest,v1)dest[0]=v1[0];dest[1]=v1[1];
#define SET(v,x,y,z){v[0]=x;v[1]=y;v[2]=z;};
#define MAX(a,b) a>b?a:b;
#define MIN(a,b) a<b?a:b;
//selFrom3d frustum clip
#define vSCALE(v,val)v[0]*=val;v[1]*=val;v[2]*=val;
#define DOT(v1,v2)(v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define CROSS(dest,v1,v2)\
 dest[0]=v1[1]*v2[2]-v1[2]*v2[1];\
 dest[1]=v1[2]*v2[0]-v1[0]*v2[2];\
 dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
//======================================================================================
Dword MMF[11][2] = {
 {GL_NEAREST,GL_NEAREST},
 {GL_LINEAR,GL_NEAREST},
 {GL_NEAREST,GL_LINEAR},
 {GL_LINEAR,GL_LINEAR},
 {GL_NEAREST,GL_NEAREST_MIPMAP_NEAREST},
 {GL_NEAREST,GL_LINEAR_MIPMAP_NEAREST},
 {GL_NEAREST,GL_NEAREST_MIPMAP_LINEAR},
 {GL_LINEAR,GL_NEAREST_MIPMAP_NEAREST},
 {GL_LINEAR,GL_LINEAR_MIPMAP_NEAREST},
 {GL_LINEAR,GL_NEAREST_MIPMAP_LINEAR},
 {GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR}
};
//======================================================================================
bool Circle2(const vec3f &b, const vec3f &c, vec3f &center){
	if(fabs(b.x*c.y - b.y*c.x) < EPSILON)return false;
	float BB = dot(b, b), BC = dot(b, c), CC = dot(c, c), BBCC = BB * CC;
	float d = (BBCC - BC * BC) * 2;
	center = b * ((BBCC - CC * BC) / d) + c * ((BBCC - BB * BC) / d);
	return true;
};
vec3f Circle(const vec3f &b, const vec3f &c){
	float BB = dot(b, b), BC = dot(b, c), CC = dot(c, c), BBCC = BB * CC;
	float d = (BBCC - BC * BC) * 2;
	return vec3f(b*((BBCC - CC * BC) / d) + c * ((BBCC - BB * BC) / d));
};
float SphereInt(vec3f ro, vec3f rd, vec4f sph){
	vec3f L = sph - ro;//?
	float tca = dot(L, rd);if(tca <= 0)return 0;
	float d = dot(L, L) - tca * tca;
	if(d > sph.w*sph.w)return 0;
	float thc = sqrt(sph.w*sph.w - d);
	return tca - thc;
};
inline vec3f intplane(const vec3f &S, const vec3f &D, const vec3f &N, const vec3f &A){ return S + D * (dot(N, S) - dot(N, A)) / -dot(N, D); };
inline float distplane(const vec3f &S, const vec3f &D, const vec3f &N, const vec3f &A){
	float dist = dot(N, S) - dot(N, A);//if(dist<EPSILON)return 0;
	float len = -dot(N, D);//if(len<EPSILON)return 0;
	return dist / len;
};
//======================================================================================
vec3f i2cubev(byte i){ return vec3f(i == 0 ? 1 : i == 3 ? -1 : 0, i == 1 ? 1 : i == 4 ? -1 : 0, i == 2 ? 1 : i == 5 ? -1 : 0); };//i[0..6]

float angle(const vec3f &a, const vec3f &b, const vec3f &N){ return atan2(dot(N, cross(b, a)), dot(b, a)); };//atan2//a=norm(proj_on_plane(N,a))
//float angle2(const vec3f &a,const vec3f &b,const vec3f &N){return acos(dot(b,a)) * sign( dot(N,cross(b,a)) );};//acos
vec3f proj_on_vec(const vec3f &N, const vec3f &delta){ return N * dot(N, delta); };
vec3f proj_on_plane(const vec3f &N, const vec3f &delta){ return delta - N * dot(N, delta); };//return delta-proj_on_vec(N,delta);};
float angle_on_plane(const vec3f &trg, const vec3f &dir, const vec3f &N){ vec3f a(norm(proj_on_plane(N, trg)));return atan2(dot(N, cross(dir, a)), dot(dir, a)); };//on plane
//====================================================================================== WIN_GL
float mx, my, lmx, lmy;
POINT mpos;
float NearPlane = 0.1;
float FarPlane = 5000;
float GFOV = 90;
//======================================================================================
bool key[512], lkey[512], keypressed;
word kb_key, lkb_key;
int wheel;
bool Active = true, Vsync = false;
bool mouse[3], lmouse[3], mouseclick[3], lmouseclick[3], mousemove;
//======================================================================================
#include "manager.h"
Manager MM;
#include "shape.hpp"
#include "WIN_GL.H"
#include "Thread.h"
#include "ui.h"
CUI UI;
//======================================================================================
struct ProjText{
	word mt, memt;
	vec3f *P, *PP, color;
	vec4f *c;
	bool *usecolor, *big;
	cstring *T;
	ProjText(){ Init(); };
	void Init(){ mt = memt = 0;P = NULL;PP = NULL;T = NULL;c = NULL;color.set(0.0);usecolor = NULL;big = NULL; };
	void resize(word nt){
		if(nt < memt){ mt = nt;return; };
		while(memt < nt)memt += 12;
		RESIZE(P, vec3f, memt);
		RESIZE(PP, vec3f, memt);
		RESIZE(T, cstring, memt);
		RESIZE(c, vec4f, memt);
		RESIZE(usecolor, bool, memt);RESIZE(big, bool, memt);
		for(word i = mt;i < memt;i++){ T[i].init();c[i].set(0.0);usecolor[i] = 0;big[i] = 0; };
		mt = nt;
	};
	void set_color(vec3f clr){ color = clr; };
	void Add(vec3f clr, bool bigf, vec3f V, const char *fmt, ...){
		if(V.x == 0 && V.y == 0 && V.z == 0)return;
		resize(mt + 1);
		P[mt - 1] = V;
		usecolor[mt - 1] = 1;
		c[mt - 1] = clr;
		big[mt - 1] = bigf;
		va_list ap;va_start(ap, fmt);
		int nlen = _vscprintf(fmt, ap) + 1;if(nlen < 1)return;  //+1 - fix?- no
		T[mt - 1].resize(nlen + 1);
		vsprintf(T[mt - 1].text, fmt, ap);    //CRUSH: at detect ship 19.08.2014 +7 at run, 16.08.2014
		va_end(ap);
	};
	void Draw(){
		BIGfont.set_tex();
		for(word i = 0;i < mt;i++){
			if(!big[i])continue;
			if(usecolor[i])glColor3f(c[i].x, c[i].y, c[i].z);else glColor3fv(color);
			BIGfont.Print(P[i].x, P[i].y, true, T[i].text);
		};
		font.set_tex();
		for(word i = 0;i < mt;i++){
			if(big[i])continue;
			if(usecolor[i])glColor3f(c[i].x, c[i].y, c[i].z);else glColor3fv(color);
			font.Print(P[i].x, P[i].y, true, T[i].text);
		};
	};
};
//======================================================================================
ProjText PrTx;//constructor
//======================================================================================
#include "Gizmo.h"
#include "camera.h"
//======================================================================================
