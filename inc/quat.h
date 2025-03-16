//======================================================================================
struct quat{
 union{
  struct{float x,y,z,w;};
  float v[4];
 };
//--------------------------------------------------------------------------------------
 inline quat():x(0),y(0),z(0),w(1){};
 inline quat(float x,float y,float z,float w):x(x),y(y),z(z),w(w){};
 inline quat(const float x):x(x),y(x),z(x),w(x){};
 //inline quat(const int x):x(x),y(x),z(x),w(x){};
 inline quat(const float *v):x(v[0]),y(v[1]),z(v[2]),w(v[3]){};
 inline quat(const float *v,const float t):x(v[0]),y(v[1]),z(v[2]),w(t){};
 inline quat(const quat &v):x(v.x),y(v.y),z(v.z),w(v.w){};

 inline quat(const vec3f &a){Rot(a);};
 inline quat(const quat &q1,const quat &q2,const float time){slerp(q1,q2,time);};
 
 inline const quat operator+(const quat &q)const{return quat(x+q.x,y+q.y,z+q.z,w+q.w);};
 inline const quat operator-(const quat &q)const{return quat(x-q.x,y-q.y,z-q.z,w-q.w);};
 inline const quat operator*(const float &q)const{return quat(x*q,y*q,z*q,w*q);};
 inline const quat operator*(const quat &q)const{
  return quat(w*q.x + x*q.w + y*q.z - z*q.y,
	             w*q.y + y*q.w + z*q.x - x*q.z,
	             w*q.z + z*q.w + x*q.y - y*q.x,
	             w*q.w - x*q.x - y*q.y - z*q.z);};
 inline quat &operator+=(const quat &q){return *this=*this+q;};
 inline quat &operator-=(const quat &q){return *this=*this-q;};
 inline quat &operator*=(const float &q){return *this=*this*q;};
 inline quat &operator*=(const quat &q){return *this=*this*q;};

 inline quat &operator =(const float &f){return quat(f,f,f,0);};
 inline quat &operator =(const int &f){return quat(f,f,f,0);};

// inline float &operator[](int i){return((float*)&x)[i];};
// inline const float operator[](int i)const{return((float*)&x)[i];};
 inline operator float*(){return (float*)&x;};
 inline operator const float*()const{return (float*)&x;};
//--------------------------------------------------------------------------------------
 inline void set(const float &a,const float &b,const float &c,const float &d){x=a;y=b;z=c;w=d;};
 inline void set(const float &a){x=a;y=a;z=a;w=a;};
 vec3f operator*(const vec3f &vec)const{
  vec3f xyz(x,y,z);
  return vec3f(vec + cross(xyz,cross(xyz,vec)+vec*w)*2.0f);
 };
//--------------------------------------------------------------------------------------
 vec3f Rotate(const vec3f &vec)const{
  vec3f yzw(x,y,z);
  return vec3f(vec + cross(yzw,cross(yzw,vec)+vec*w)*2.0f);//14mul's
 };
//--------------------------------------------------------------------------------------
vec3f Ang(){//NEW
 float m[12]={1-2*(y*y+z*z),  2*(x*y-w*z),  2*(x*z+w*y),0,//glm
                2*(x*y+w*z),1-2*(x*x+z*z),  2*(y*z-w*x),0,
                2*(x*z-w*y),  2*(y*z+w*x),1-2*(x*x+y*y),0};
 vec3f a(0.0);
 float xyDist=sqrt(m[10]*m[10]+m[9]*m[9]);
 if(xyDist>0.00001f){
   bool inv=m[0]>0;
   if(inv)a.x=atan2( m[9], m[10]);else a.x=-atan2(m[9], -m[10]);
   if(inv)a.y=atan2(-m[8],xyDist);else a.y=-atan2(m[8],-xyDist);
   if(inv)a.z=atan2( m[4],  m[0]);else a.z=-atan2(m[4],  -m[0]);
  }else{ // CON.write(2,"Q::xyDist!!");
   a.x=atan2(-m[6],  m[5]);
   a.y=atan2(-m[8],xyDist);
  };
  return a;
};
//--------------------------------------------------------------------------------------
inline void Rot(const vec3f &a){//NEW
	float cx = cos(a.x*0.5), cy = cos(a.y*0.5), cz = cos(a.z*0.5);     //blender
	float sx = sin(a.x*0.5), sy = sin(a.y*0.5), sz = sin(a.z*0.5);
	float cc = cx*cz, cs = cx*sz, sc = sx*cz, ss = sx*sz;
	w = cy*cc + sy*ss;
	x = cy*sc - sy*cs;
	y = cy*ss + sy*cc;
	z = cy*cs - sy*sc;
};
//--------------------------------------------------------------------------------------
float mag(){return x*x+y*y+z*z+w*w;};
//--------------------------------------------------------------------------------------
void norm(){
 float L=x*x+y*y+z*z+w*w;
 if(fabs(L)<0.000001f){set(0,0,0,1);return;};
 L=1.0f/sqrt(L);
 for(byte i=0;i<4;i++)v[i]*=L;
};
//--------------------------------------------------------------------------------------
/*glm inverse conjugate(q) / dot(q, q);*/
inline quat inv(){return quat(-x,-y,-z,-w);};
//--------------------------------------------------------------------------------------
inline quat conj(){return quat(-x,-y,-z,w);};
//--------------------------------------------------------------------------------------
inline void unit_from_axis_angle(const vec3f &axis,const float &angle){
 float half_angle = angle*0.5f;
 float sin_a = sin(half_angle);
 set(axis.x*sin_a,axis.y*sin_a,axis.z*sin_a,cos(half_angle));
};
//--------------------------------------------------------------------------------------
void slerp(const quat &q1,const quat &q,const float &interp){  //http://www.gamedev.ru/code/articles/?id=4215&page=3?
 byte i;
 float a=0,b=0;
 quat q2(q);
 for(i=0;i<4;i++){
  a+=(q1[i]-q2[i])*(q1[i]-q2[i]);
  b+=(q1[i]+q2[i])*(q1[i]+q2[i]);
 };
 if(a>b){q2=q2.inv();};
 float cosom=q1[0]*q2[0]+q1[1]*q2[1]+q1[2]*q2[2]+q1[3]*q2[3];
 double sclq1,sclq2;
 if((1.0+cosom)>EPSILON){
  if((1.0-cosom)>EPSILON){//0.00000001
   double omega=acos(cosom);
   double sinom=sin(omega);
   sclq1=sin((1.0-interp)*omega)/sinom;
   sclq2=sin(interp*omega)/sinom;
  }else{
   sclq1=1.0-interp;
   sclq2=interp;
  };
  for(i=0;i<4;i++){v[i]=(float)(sclq1*q1[i]+sclq2*q2[i]);};
 }else{
  x=-q1[1];//??but works
  y= q1[0];
  z=-q1[3];
  w= q1[2];
  sclq1=sin((1.0-interp)*HALFPI);
  sclq2=sin(interp*HALFPI);
  for(i=0;i<3;i++){v[i]=(float)(sclq1*q1[i]+sclq2*v[i]);};
 };
};
//--------------------------------------------------------------------------------------M2Q
void operator=(const mat4 &m){//NEW
 float fourX = m[0*4+0] - m[1*4+1] - m[2*4+2]; //glm
 float fourY = m[1*4+1] - m[0*4+0] - m[2*4+2];
 float fourZ = m[2*4+2] - m[0*4+0] - m[1*4+1];
 float fourW = m[0*4+0] + m[1*4+1] + m[2*4+2];
 byte idx = 0;
 float fourBiggest = fourW;
 if(fourX > fourBiggest){fourBiggest = fourX;idx = 1;};
 if(fourY > fourBiggest){fourBiggest = fourY;idx = 2;};
 if(fourZ > fourBiggest){fourBiggest = fourZ;idx = 3;};
 float biggestVal = sqrt(fourBiggest + 1)*0.5f;
 float mult = 0.25f/biggestVal;
 switch(idx){
  case 0:w = biggestVal; 
         x = (m[1*4+2] - m[2*4+1])*mult;
         y = (m[2*4+0] - m[0*4+2])*mult;
         z = (m[0*4+1] - m[1*4+0])*mult;break;
  case 1:w = (m[1*4+2] - m[2*4+1])*mult;
         x = biggestVal;
         y = (m[0*4+1] + m[1*4+0])*mult;
         z = (m[2*4+0] + m[0*4+2])*mult;break;
  case 2:w = (m[2*4+0] - m[0*4+2])*mult;
         x = (m[0*4+1] + m[1*4+0])*mult;
         y = biggestVal;
         z = (m[1*4+2] + m[2*4+1])*mult;break;
  case 3:w = (m[0*4+1] - m[1*4+0])*mult;
         x = (m[2*4+0] + m[0*4+2])*mult;
         y = (m[1*4+2] + m[2*4+1])*mult;
         z = biggestVal;                break;
 };
};
//--------------------------------------------------------------------------------------
inline vec3f right()const{return vec3f(1-2*(y*y+z*z),  2*(x*y+w*z),  2*(x*z-w*y));};//NEW
inline vec3f    up()const{return vec3f(  2*(x*y-w*z),1-2*(x*x+z*z),  2*(y*z+w*x));};
inline vec3f   dir()const{return vec3f(  2*(x*z+w*y),  2*(y*z-w*x),1-2*(x*x+y*y));};
//--------------------------------------------------------------------------------------
void LookAt2(const vec3f &eye,const vec3f &center,const vec3f &up){//almost work http://stackoverflow.com/questions/12435671/quaternion-lookat-function
    vec3f F(center-eye);F.norm();
    float dotp = dot(vec3f(0,0,1), F);
    //if(fabs(dotp - (-1.0f)) < 0.000001f){set(0, 1, 0, PI);return;};
    //if(fabs(dotp - (1.0f)) < 0.000001f){set(0,0,0,1);return;};
    float rotAngle = (float)acos(dotp);
    vec3f rotAxis = cross(vec3f(0,0,1), F);rotAxis.norm();
    unit_from_axis_angle(rotAxis,rotAngle);
    //return CreateFromAxisAngle(rotAxis, rotAngle);
 //vec3f tv;tv=cross(vec3f(0.0f, 0.0f, 1.0f), F)*sqrt(0.5 - dotp/2);
 //set(tv.x,tv.y,tv.z,sqrt(0.5 + dotp/2));
};
void LookAt(const vec3f &eye,const vec3f &center,const vec3f &up){
 mat4 LAM(1);
 LAM.LookAt(eye,center,up);
 *this=LAM;
};
//--------------------------------------------------------------------------------------
void print(char *s){CON.write(0,"%s\n%f %f %f %f\n",s,v[0],v[1],v[2],v[3]);};
//--------------------------------------------------------------------------------------
};
float dot(const quat &a,const quat &b){return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;};//!!!
/*gml cross
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
	        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
	        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
	        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x);
*/
//======================================================================================end quat
void mat4::operator=(const quat &q){//FD   Q2M 
 R=q.right();
 U=q.up();
 F=q.dir();
};
/*
 m[0*4+0]= 1.0-2*(q.y*q.y-q.z*q.z);  blender
	m[0*4+1]=     2*(q.w*q.z+q.x*q.y);
	m[0*4+2]=    -2*(q.w*q.y+q.x*q.z);

	m[1*4+0]=    -2*(q.w*q.z+q.x*q.y);
	m[1*4+1]= 1.0-2*(q.x*q.x-q.z*q.z);
	m[1*4+2]=     2*(q.w*q.x+q.y*q.z);

	m[2*4+0]=     2*(q.w*q.y+q.x*q.z);
	m[2*4+1]=    -2*(q.w*q.x+q.y*q.z);
	m[2*4+2]= 1.0-2*(q.x*q.x-q.y*q.y);// */
//======================================================================================

//======================================================================================
mat4 ModelView(const quat &q,const vec3f &t){
 vec3f r(q.right()),u(q.up()),f(q.dir());
 return mat4(      r.x,      u.x,    -f.x,0,
		                 r.y,      u.y,    -f.y,0,
		                 r.z,      u.z,    -f.z,0,
		           -dot(t,r),-dot(t,u),-dot(t,-f),1);// 
};
//======================================================================================