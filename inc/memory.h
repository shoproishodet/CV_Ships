//====================================================================================== 
//CON.write(0,"RESIZE [%i]x[%i] at [%s:%i][%s]",s,sizeof(t),__FILE__,__LINE__,__FUNCTION__);\

#define RESIZE(p,t,s)\
p=(t*)realloc(p,sizeof(t)*s);\
//if(!p){CON.write(LOG_ERROR,"can't resize [%i]x[%i] at [%s:%i][%s]",s,sizeof(t),__FILE__,__LINE__,__FUNCTION__);};\

//======================================================================================  
//CON.write(0,"RESIZE CLEAR [%i]->[%i]x[%i] at [%s:%i][%s]",os,s,sizeof(t),__FILE__,__LINE__,__FUNCTION__);\
//RESIZEC(SF,byte,s - nmf=mf+1,os - start=mf);resizeF(mf+1,mf);
//#define RESIZEC(p,t,s,os)\
//p=(t*)realloc(p,sizeof(t)*s);\
//if(!p){CON.write(LOG_ERROR,"can't resizec %i",s);};\
//if(os<s){for(int _i=os;_i<s;_i++)memset(p[_i],0,sizeof(t));};\ 
//memset(p+(sizeof(t)*os),0,sizeof(t)*s);};\
//======================================================================================  
//CON.write(0,"DEL_ at [%s:%i][%s]",__FILE__,__LINE__,__FUNCTION__);\

#define DEL_(p)\
if(p){free(p);p=NULL;};\
//======================================================================================
