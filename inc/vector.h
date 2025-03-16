//======================================================================================
struct vec2f;
struct vec3f;
struct vec4f;
//======================================================================================
struct vec3i{
	union{
		struct{ int x, y, z; };
		int v[3];
	};
	inline vec3i() :x(0), y(0), z(0){};
	inline vec3i(int a, int b, int c) :x(a), y(b), z(c){};
	inline vec3i(int a) :x(a), y(a), z(a){};
	inline vec3i(const int *v) :x(v[0]), y(v[1]), z(v[2]){};
	inline vec3i(const vec3i &v) :x(v.x), y(v.y), z(v.z){};
//	inline vec3i(const float *f) :x(FtoI(f[0])), y(FtoI(f[1])), z(FtoI(f[2])){};
	inline vec3i(const vec3f &v);//FD :x(FtoI(v.x)),y(FtoI(v.y)),z(FtoI(v.z)){};
	//inline int &operator[](int i){return((int*)&x)[i];};
	//inline const int operator[](int i)const{return((int*)&x)[i];};
	inline void set(const int &a, const int &b, const int &c){ x = a;y = b;z = c; };

	inline operator int*(){ return (int*) &x; };
	inline operator const int*()const{ return (int*) &x; };

	inline const vec3i operator*(int f)const{ return vec3i(x*f, y*f, z*f); };
	inline const vec3i operator/(int f)const{ return vec3i(x / f, y / f, z / f); };
	inline const vec3i operator+(int f)const{ return vec3i(x + f, y + f, z + f); };
	inline const vec3i operator-(int f)const{ return vec3i(x - f, y - f, z - f); };
	inline const vec3i operator+(const vec3i &v)const{ return vec3i(x + v.x, y + v.y, z + v.z); };
	inline const vec3i operator-(const vec3i &v)const{ return vec3i(x - v.x, y - v.y, z - v.z); };
	inline const vec3i operator-()const{ return vec3i(-x, -y, -z); };
	inline const vec3i operator*(const vec3i &v)const{ return vec3i(x*v.x, y*v.y, z*v.z); };

	inline vec3i &operator*=(int f){ return *this = *this*f; };
	inline vec3i &operator/=(int f){ return *this = *this / f; };
	inline vec3i &operator+=(const vec3i &v){ return *this = *this + v; };
	inline vec3i &operator-=(const vec3i &v){ return *this = *this - v; };
	inline vec3i &operator*=(const vec3i &v){ return *this = *this*v; };

};
//======================================================================================
struct vec3w{
	union{
		struct{ word x, y, z; };
		word v[3];
	};
	inline vec3w() :x(0), y(0), z(0){};
	inline vec3w(word a, word b, word c) :x(a), y(b), z(c){};
	inline vec3w(const word *v) :x(v[0]), y(v[1]), z(v[2]){};
	inline vec3w(const vec3w &v) :x(v.x), y(v.y), z(v.z){};
	inline operator word*(){ return (word*) &x; };
	inline operator const word*()const{ return (word*) &x; };
	inline void set(const word &a, const word &b, const word &c){ x = a;y = b;z = c; };
};
//======================================================================================
//struct vec4l;//fd
struct vec3l{
	union{
		struct{ Dword x, y, z; };
		Dword v[3];
	};
	inline vec3l() :x(0), y(0), z(0){};
	inline vec3l(Dword a, Dword b, Dword c) :x(a), y(b), z(c){};
	inline vec3l(const Dword *v) :x(v[0]), y(v[1]), z(v[2]){};
	inline vec3l(const vec3l &v) :x(v.x), y(v.y), z(v.z){};
	//inline Dword &operator[](int i){return((Dword*)&x)[i];};
	//inline const Dword operator[](int i)const{return((Dword*)&x)[i];};
	inline operator Dword*(){ return (Dword*) &x; };
	inline operator const Dword*()const{ return (Dword*) &x; };
	inline void set(const Dword &a, const Dword &b, const Dword &c){ x = a;y = b;z = c; };
	//inline vec3l operator=(const vec4l f){x=f.x;y=f.y;z=f.z;};
};
//======================================================================================
struct vec4l{
	union{
		struct{ Dword x, y, z, w; };
		Dword v[4];
	};
	inline vec4l() :x(0), y(0), z(0), w(0){};
	inline vec4l(Dword a, Dword b, Dword c, Dword d) :x(a), y(b), z(c), w(d){};
	inline vec4l(const Dword *v) :x(v[0]), y(v[1]), z(v[2]), w(v[3]){};
	inline vec4l(const vec4l &v) :x(v.x), y(v.y), z(v.z), w(v.w){};
	// inline Dword &operator[](int i){if(i>3 || i<0){CON.write(0,"ERR");}; return((Dword*)&x)[i];};
	 //inline const Dword operator[](int i)const{if(i>3 || i<0){CON.write(0,"ERR");};return((Dword*)&x)[i];};
	inline operator Dword*(){ return (Dword*) &x; };
	inline operator const Dword*()const{ return (Dword*) &x; };
	// inline operator=(const vec3l f){x=f.x;y=f.y;z=f.z;w=0;};

	inline void set(const Dword &a){ x = a;y = a;z = a;w = a; };
	inline void set(const Dword &a, const Dword &b, const Dword &c){ x = a;y = b;z = c;w = 0; };
	inline void set(const Dword &a, const Dword &b, const Dword &c, const Dword &d){ x = a;y = b;z = c;w = d; };
};
//======================================================================================
struct vec2l{
	union{
		struct{ Dword x, y; };
		Dword v[2];
	};
	inline vec2l() :x(0), y(0){};
	inline vec2l(Dword a, Dword b) :x(a), y(b){};
	inline vec2l(const Dword *v) :x(v[0]), y(v[1]){};
	// inline vec2l(const vec3l &v):x(v.x),y(v.y){};
	 //inline Dword &operator[](int i){return((Dword*)&x)[i];};
	 //inline const Dword operator[](int i)const{return((Dword*)&x)[i];};
	inline operator Dword*(){ return (Dword*) &x; };
	inline operator const Dword*()const{ return (Dword*) &x; };
	inline void set(const Dword &a, const Dword &b){ x = a;y = b; };
};
//======================================================================================
/*struct vec3w{
 union{
  struct{word x,y,z;};
  word v[3];
 };
 inline vec3w():x(0),y(0),z(0){};
 inline vec3w(word x,word y,word z):x(x),y(y),z(z){};
 inline vec3w(const word *v):x(v[0]),y(v[1]),z(v[2]){};
 inline vec3w(const vec3w &v):x(v.x),y(v.y),z(v.z){};
 inline word &operator[](int i){return((word*)&x)[i];};
 inline const word operator[](int i)const{return((word*)&x)[i];};
 inline void set(const word &a,const word &b,const word &c){x=a;y=b;z=c;};
};
//======================================================================================
struct vec2w{
 union{
  struct{word x,y;};
  word v[2];
 };
 inline vec2w():x(0),y(0){};
 inline vec2w(word x,word y):x(x),y(y){};
 inline vec2w(const word *v):x(v[0]),y(v[1]){};
 inline vec2w(const vec3w &v):x(v.x),y(v.y){};
 inline word &operator[](int i){return((word*)&x)[i];};
 inline const word operator[](int i)const{return((word*)&x)[i];};
 inline void set(const word &a,const word &b){x=a;y=b;};
};            */
//======================================================================================
struct vec2f{
	union{
		struct{ float x, y; };
		float v[2];
	};
	inline vec2f() :x(0), y(0){};
	inline vec2f(float a, float b) :x(a), y(b){};
	inline vec2f(const float *v) :x(v[0]), y(v[1]){};
	inline vec2f(const float f) :x(f), y(f){};
	inline vec2f(const vec2f &v) :x(v.x), y(v.y){};
	inline vec2f(const vec3f &v);

	inline const vec2f operator*(float f)const{ return vec2f(x*f, y*f); };
	inline const vec2f operator/(float f)const{ return vec2f(x / f, y / f); };
	//inline const vec2f operator=(const float f)const{return vec2f(f,f);};
	inline const vec2f operator+(const vec2f &v)const{ return vec2f(x + v.x, y + v.y); };
	inline const vec2f operator-(const vec2f &v)const{ return vec2f(x - v.x, y - v.y); };
	inline const vec2f operator-()const{ return vec2f(-x, -y); };
	inline const vec2f operator*(const vec2f &v)const{ return vec2f(x*v.x, y*v.y); };

	inline vec2f &operator*=(float f){ return *this = *this*f; };
	inline vec2f &operator/=(float f){ return *this = *this / f; };
	inline vec2f &operator+=(const vec2f &v){ return *this = *this + v; };
	inline vec2f &operator-=(const vec2f &v){ return *this = *this - v; };
	inline vec2f &operator*=(const vec2f &v){ return *this = *this*v; };

	operator float*(){ return (float*) &x; };
	operator const float*()const{ return (float*) &x; };

	// float &operator[](int i){return ((float*)&x)[i];};
	// const float operator[](int i)const{return ((float*)&x)[i];};

	inline void norm(){
		float len = x * x + y * y;
		if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
		len = 1.0f / sqrt(len);
		x *= len;
		y *= len;
	};
	inline void set(const float &a, const float &b){ x = a;y = b; };
};
//======================================================================================
vec3f cross(const vec3f &a, const vec3f &b);
struct vec3f{
	union{
		struct{ float x, y, z; };
		float v[3];
	};
	inline vec3f() :x(0), y(0), z(0){};
	inline vec3f(const float a, const float b, const float c) :x(a), y(b), z(c){};
	inline vec3f(const float *v) :x(v[0]), y(v[1]), z(v[2]){};
	inline vec3f(const vec3f &v) :x(v.x), y(v.y), z(v.z){};
	inline vec3f(const vec2f &v) :x(v.x), y(v.y), z(0){};
	inline vec3f(const vec3i &v) :x(v.x), y(v.y), z(v.z){};
	inline vec3f(const float f) :x(f), y(f), z(f){};
	inline vec3f(const vec4f &v);

	// inline float &operator[](int i){return ((float*)&x)[i];};
	// inline const float operator[](int i)const{return ((float*)&x)[i];};

	inline operator float*(){ return (float*) &x; };
	inline operator const float*()const{ return (float*) &x; };

	inline const vec3f operator*(float f)const{ return vec3f(x*f, y*f, z*f); };
	inline const vec3f operator/(float f)const{ return vec3f(x / f, y / f, z / f); };
	inline const vec3f operator+(float f)const{ return vec3f(x + f, y + f, z + f); };
	inline const vec3f operator+(int f)const{ return vec3f(x + f, y + f, z + f); };//!
	inline const vec3f operator-(float f)const{ return vec3f(x - f, y - f, z - f); };
	inline const vec3f operator-(int f)const{ return vec3f(x - f, y - f, z - f); };
	//inline const vec3f operator=(const float f)const{return vec3f(f,f,f);};
	inline vec3f &operator =(const float &f){ return vec3f(f, f, f); };
	inline const vec3f operator+(const vec3f &v)const{ return vec3f(x + v.x, y + v.y, z + v.z); };
	inline const vec3f operator-(const vec3f &v)const{ return vec3f(x - v.x, y - v.y, z - v.z); };
	inline const vec3f operator-(const vec3i &v)const{ return vec3f(x - v.x, y - v.y, z - v.z); };
	inline const vec3f operator+(const vec3i &v)const{ return vec3f(x + v.x, y + v.y, z + v.z); };
	inline const vec3f operator-()const{ return vec3f(-x, -y, -z); };//not work?
	inline const vec3f operator*(const vec3f &v)const{ return vec3f(x*v.x, y*v.y, z*v.z); };

	inline vec3f &operator*=(float f){ return *this = *this*f; };
	inline vec3f &operator/=(float f){ return *this = *this / f; };
	inline vec3f &operator+=(const vec3f &v){ return *this = *this + v; };
	inline vec3f &operator-=(const vec3f &v){ return *this = *this - v; };
	inline vec3f &operator*=(const vec3f &v){ return *this = *this*v; };
	inline vec3f abs(){ return vec3f(fabs(x), fabs(y), fabs(z)); };
	inline void norm(){
		float len = x * x + y * y + z * z;//if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
		len = fabs(len)>EPSILON ? 1.0f / sqrt(len) : 1;
		x *= len;
		y *= len;
		z *= len;
	};
	inline float normalize(){
		float len = x * x + y * y + z * z;
		if(fabs(len)>EPSILON){
			len = sqrt(len);
			x /= len;
			y /= len;
			z /= len;
		};
		return len;
	};
	inline vec3f norm2()const{
		float len = x * x + y * y + z * z;//if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
		len = fabs(len)>EPSILON ? 1.0f / sqrt(len) : 1;
		return vec3f(x*len, y*len, z*len);
	};
	inline void normal(const vec3f &a, const vec3f &b, const vec3f &c){
		*this = cross(c - a, b - a);//crush! see renormal
		this->norm();
	};
	inline void normal(const vec3f &a, const vec3f &b, const vec3f &c, const vec3f &d){//quad
		*this = cross(c - a, b - a) + cross(d - a, c - a); //why? here; renormal tri quad
		this->norm();
	};
	inline void set(const float &a, const float &b, const float &c){ x = a;y = b;z = c; };
	inline void set(const float &a){ x = a;y = a;z = a; };
	//http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
	inline float yaw_angle()const{ return atan2(z, x); };//YAW angle at [pos1] to [pos2]; use:norm(p1-p2).angle();  bug: if p1.(z,x)~~p2.(z,x); y-UP
	inline vec3f N2UV(){
		//return vec3f((atan2(y,x)/PI+1.0)*0.5,(z+1.0)*0.5,0); dual half sphere XY
		/*float phi=atan2(sqrt(x*x+z*z),y);
		float theta=atan2(z,x);
		float r = phi / HALFPI;
		float u = 0.5 * (r * cos(theta) + 1);
		float v = 0.5 * (r * sin(theta) + 1);
		 return vec3f(u,v,0);*/
		 //            //-pi-pi   0-pi/2
		 //return vec3f(atan2(y,x),atan2(-z,sqrt(x*x+y*y)),0);
		float div = 1.0 / (1.0 - y);
		//if(div<0)div=1.0/(1.0-div);
		return vec3f(2 * x*div, 0, 2 * z*div);
	};
	inline vec3f UV2N(){
		vec3f sph = (*this)*2.0 - vec3f(1.0);
		float sx = sqrt(1.0 - y * y);
		return vec3f(cos(y*PI)*sx, sin(x*PI)*sx, y);
	};
	inline vec3f ang2right(){
		float //cx = cos(x*D2R),sx = sin(x*D2R),
			cy = cos(y*D2R), sy = sin(y*D2R),
			cz = cos(z*D2R), sz = sin(z*D2R);//, cc = cx*cz,cs = cx*sz,sc = sx*cz,ss = sx*sz;
		return vec3f(cy*cz, cy*sz, -sy);
	};
	inline vec3f    ang2up(){
		float cx = cos(x*D2R), sx = sin(x*D2R),
			cy = cos(y*D2R), sy = sin(y*D2R),
			cz = cos(z*D2R), sz = sin(z*D2R), cc = cx * cz, cs = cx * sz, sc = sx * cz, ss = sx * sz;
		return vec3f(sy*sc - cs, sy*ss + cc, cy*sx);
	};
	inline vec3f   ang2dir(){
		float cx = cos(x*D2R), sx = sin(x*D2R),
			cy = cos(y*D2R), sy = sin(y*D2R),
			cz = cos(z*D2R), sz = sin(z*D2R), cc = cx * cz, cs = cx * sz, sc = sx * cz, ss = sx * sz;
		return vec3f(sy*cc + ss, sy*cs - sc, cy*cx);
	};
	//inline vec3f intPlane(const vec3f &S,const vec3f &D);//FD dot {return S+D*(dot(v,S)/-dot(D,vec3f(v)));};
	vec3f rotZ(float rad){ return vec3f(x*cos(rad) - y * sin(rad), x*sin(rad) + y * cos(rad), 0); };//normal=tangent.rotZ(-HPI);
	vec3f rotY(float rad)const{ return vec3f(x*cos(rad) - z * sin(rad), 0, x*sin(rad) + z * cos(rad)); };//normal=tangent.rotZ(-HPI);
};
inline vec3f norm(const vec3f &v){
	float len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);//if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
	if(len == 0.0)return vec3f(1, 0, 0);
	if(len == 1.0)return v;
	//len=len?1.0f/sqrt(len):1;
	return vec3f(v.x / len, v.y / len, v.z / len);
};
inline vec3f normal(const vec3f &a, const vec3f &b, const vec3f &c){ return norm(cross(c - a, b - a)); };
inline vec3f normal(const vec3f &a, const vec3f &b, const vec3f &c, const vec3f &d){ return norm(cross(c - a, b - a) + cross(d - a, c - a)); };
//inline vec3i::vec3i(const vec3f &v) :x(FtoI(v.x)), y(FtoI(v.y)), z(FtoI(v.z)){};
//======================================================================================
struct vec4f{
	union{
		struct{ float x, y, z, w; };
		float v[4];
	};
	inline vec4f() :x(0), y(0), z(0), w(1){};
	inline vec4f(float a, float b, float c, float d) :x(a), y(b), z(c), w(d){};
	inline vec4f(const float *v) :x(v[0]), y(v[1]), z(v[2]), w(v[3]){};
	inline vec4f(const vec4f &v) :x(v.x), y(v.y), z(v.z), w(v.w){};
	inline vec4f(const vec3f &v){ x = v.x;y = v.y;z = v.z;w = 1; };
	inline vec4f(const vec3f &v, const float &vw){ x = v.x;y = v.y;z = v.z;w = vw; };
	// inline float &operator[](int i){return ((float*)&x)[i];};
	// inline const float operator[](int i)const{return ((float*)&x)[i];};
	inline operator float*(){ return (float*) &x; };
	inline operator const float*()const{ return (float*) &x; };

	inline const vec4f operator*(float f)const{ return vec4f(x*f, y*f, z*f, w*f); };
	inline const vec4f operator/(float f)const{ return vec4f(x / f, y / f, z / f, w / f); };
	inline const vec4f operator+(const vec4f &v)const{ return vec4f(x + v.x, y + v.y, z + v.z, w + v.w); };
	inline const vec4f operator-(const vec4f &v)const{ return vec4f(x - v.x, y - v.y, z - v.z, w - v.w); };
	inline const vec4f operator-()const{ return vec4f(-x, -y, -z, -w); };
	inline const vec4f operator*(const vec4f &v)const{ return vec4f(x*v.x, y*v.y, z*v.z, w*v.w); };

	inline vec4f &operator*=(float f){ return *this = *this*f; };
	inline vec4f &operator/=(float f){ return *this = *this / f; };
	inline vec4f &operator+=(const vec4f &v){ return *this = *this + v; };
	inline vec4f &operator-=(const vec4f &v){ return *this = *this - v; };
	inline vec4f &operator*=(const vec4f &v){ return *this = *this*v; };

	//inline vec4f calcPlane(const vec3f &N,const vec3f &A);//FD dot {return vec4f(N,dot(N,A));};
	inline vec3f &intPlane(const vec3f &S, const vec3f &D)const;//FD dot {return S+D*dot(this,S)/-dot(N,D);};
	inline float distPlane(const vec3f &S, const vec3f &D);//FD dot {return dot(this,S)/-dot(N,D);};

	inline void norm(){
		float len = x * x + y * y + z * z;
		if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
		len = 1.0f / sqrt(len);
		x *= len;
		y *= len;
		z *= len;
	};
	inline void set(const float &a, const float &b, const float &c, const float &d){ x = a;y = b;z = c;w = d; };
	inline void set(const float &a){ x = a;y = a;z = a;w = a; };
	inline void set(const vec3f &a, const float &b){ x = a.x;y = a.y;z = a.z;w = b; };
};
inline vec4f norm4(const vec4f &v){
	float len = v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;//if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return;
	len = fabs(len)>EPSILON ? 1.0f / sqrt(len) : 1;
	return vec4f(v.x*len, v.y*len, v.z*len, v.w*len);
};
//======================================================================================
inline vec3f::vec3f(const vec4f &v) :x(v.x), y(v.y), z(v.z){};
inline vec2f::vec2f(const vec3f &v) :x(v.x), y(v.y){};
inline vec3f cross(const vec3f &a, const vec3f &b){ return vec3f(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); };//6mul:3sub:3set
inline float dot(const vec3f &a, const vec3f &b){ return a.x*b.x + a.y*b.y + a.z*b.z; };//3mul:2add:1set
inline vec3f reflect(const vec3f &N, const vec3f &V){ return V - N * (dot(V, N) * 2); };
inline vec3f project(const vec3f &N, const vec3f &A, const vec3f &V){ return A - N * dot(A - V, N); };
float dot(const vec2f &a, const vec2f &b){ return a.x*b.x + a.y*b.y; };

float dot4(const vec4f &a, const vec3f &b){ return a.x*b.x + a.y*b.y + a.z*b.z + a.w; };
float dot(const vec4f &a, const vec4f &b){ return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; };//!!!
float dot(const vec3f &a, const vec4f &b){ return a.x*b.x + a.y*b.y + a.z*b.z; };
float dot(const vec3f &a, const vec2f &b){ return a.x*b.x + a.y*b.y; };//*shape dot plineside
//======================================================================================
float VSQR(const vec3f &a){
	return a.x*a.x + a.y*a.y + a.z*a.z;
};
//======================================================================================
float fVLEN2(const vec3f &a){
	float len = a.x*a.x + a.z*a.z;
	if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return 0.0f;
	return sqrt(len);
};
//======================================================================================
float VLEN(const vec3f &a){
	float len = a.x*a.x + a.y*a.y + a.z*a.z;
	//if(fabs(len)<EPSILON)return len<0?-EPSILON:EPSILON;
	if(!len || F_NAN(len) || F_INF(len) || F_IND(len))return 0.0f;
	return sqrt(len);
	//if(len==1)return 1.0f;
	//if(len>0.0f){return (float)sqrt(len);}else{return 0.0f;};
};
//======================================================================================
float VLEN(const vec3f &v1, const vec3f &v2){// SQRT????????????????????????????????????
	return(((v1.x - v2.x)*(v1.x - v2.x)) + ((v1.y - v2.y)*(v1.y - v2.y)) + ((v1.z - v2.z)*(v1.z - v2.z)));
};
//======================================================================================
bool SegIntersects(const vec2f &a, const vec2f &b, const vec2f &c, const vec2f &d){
	vec2f u = b - a;
	vec2f v = d - c;
	float D = u.x*v.y - u.y*v.x;if(fabs(D) < EPSILON)return false; //parallel test
	vec2f w = a - c;
	float s = v.x*w.y - v.y*w.x;if(s > 0 && s > D || s < 0 && s < D)return false;//if(s<0 && s>-D || s>0 && s<-D)return false;
	float t = u.x*w.y - u.y*w.x;if(t > 0 && t > D || t < 0 && t < D)return false;// s<0 || s>D
	return true;
};
bool SegIntersects(const vec2f &a, const vec2f &b, const vec2f &c, const vec2f &d, vec2f &ip){//tested twice!
	vec2f u = b - a;
	vec2f v = d - c;
	float D = u.x*v.y - u.y*v.x;if(fabs(D) < EPSILON)return false; //parallel test
	vec2f w = a - c;
	float s = v.x*w.y - v.y*w.x;if(s > EPSILON && s >= D || s < EPSILON && s <= D)return false;//if(s<0 && s>-D || s>0 && s<-D)return false;   if(s<0 || s>D)return false;//
	float t = u.x*w.y - u.y*w.x;if(t > EPSILON && t >= D || t < EPSILON && t <= D)return false;// s<0 || s>D    if(t<0 || t>D)return false;//
	ip = a + u * (s / D); //or c+v*(t/D);// 
	return true;//*/
};
bool SegIntersects_st(const vec2f &a, const vec2f &b, const vec2f &c, const vec2f &d, float &s, float &t){
	vec2f u = b - a;
	vec2f v = d - c;
	float D = u.x*v.y - u.y*v.x;if(fabs(D) < EPSILON)return 0; //parallel test
	vec2f w = a - c;
	s = v.x*w.y - v.y*w.x;if(s > 0 && s > D || s < 0 && s < D)return 0;//if(s<0 && s>-D || s>0 && s<-D)return false;
	t = u.x*w.y - u.y*w.x;if(t > 0 && t > D || t < 0 && t < D)return 0;// s<0 || s>D
	s = s / D;
	t = t / D;
	return 1;//s/D;//vec3f ip=a+(b-a)*SegIntersectsf();
};
//======================================================================================
struct ray{//plane are inv ray
	vec3f p, n;
	float len;
	ray(){ p = 0.0;n = 0.0;len = 1.0; };
	ray(const vec3f &POS, const vec3f &NORMAL){ p = POS;n = NORMAL;len = 1.0; };
	void set(const vec3f &pos, const vec3f &dir, const float &length = 1.0f){ p = pos;n = dir;len = length; };//normalize?
	void normalize(){ len = n.normalize(); };
	inline vec3f at(const float &t)const{ return p + n * t; };
	inline vec3f ip(const ray &b)const{//2cross 1vlen 1dot 1div 1mul
		vec3f Ncross = cross(n, b.n);
		float NClen = VLEN(Ncross);
		return at(dot(cross(b.p - p, b.n), Ncross) / (NClen*NClen));
	};
	inline float ip_dist(const ray &b)const{//2cross 1vlen 1dot 1div 1mul
		vec3f Ncross = cross(n, b.n);
		float NClen = VLEN(Ncross);
		return dot(cross(b.p - p, b.n), Ncross) / (NClen*NClen);
	};
	inline vec3f closest(const vec3f &v, bool normlz = true){//vec3f V=n/len;return p+V*clamp(dot(V,v-p),0.0,len);
		if(normlz)normalize();
		return at(clamp(dot(n, v - p), 0.0, len));
	};
	inline float closest_dist(const vec3f &v){ return clamp(dot(n, v - p), 0.0, VLEN(n)); };
	inline float closest_dist_len(const vec3f &v){ return clamp(dot(n, v - p), 0.0, len); };
	inline float angle(const vec3f &v)const{ return dot(norm(v - p), n); };//-1..1
	inline float PlaneDist(const vec4f &P)const{ return dot4(P, p) / -dot(vec3f(P), n); };
	inline vec4f Plane(){ return vec4f(n, -dot(n, p)); };//construct plane
	//inline int PlaneSide(const vec3f &v){return (dot(n,v)>dot(n,p))?1:-1;};//?? plane dist
};
//http://en.wikibooks.org/wiki/GLSL_Programming/Vector_and_Matrix_Operations
//====================================================================================== PLANE vec4f
inline vec4f calcPlane(const vec3f &N, const vec3f &A){ return vec4f(N, -dot(N, A)); };
//inline vec3f &vec4f::intPlane(const ray &R)const{return R.at((dot4(*this,R.p)/-dot(vec3f(*this),R.n)));};
inline vec3f intPlane(const ray &R, const vec4f &P){ return R.at(dot4(P, R.p) / -dot(vec3f(P), R.n)); };
inline vec3f &vec4f::intPlane(const vec3f &S, const vec3f &D)const{ return vec3f(S + D * (dot4(*this, S) / -dot(vec3f(*this), D))); };
inline float vec4f::distPlane(const vec3f &S, const vec3f &D){ return dot4(*this, S) / -dot(vec3f(*this), D); };
//inline vec3f intplane(const vec3f &S,const vec3f &D,const vec3f &N,const vec3f &A){return S+D*(dot(N,S)-dot(N,A))/-dot(N,D);};
//======================================================================================
//======================================================================================
vec3f lerp(const vec3f &a, const vec3f &b, const float &t){ return a * (1 - t) + b * t; };
//======================================================================================