//======================================================================================
//#include "Matherial.h"
//#include "Model.h"
#include "Texture.h"
struct vboVertex{ float v[14]; };
//======================================================================================
struct Manager{
	word mmd;
	//Model **model;
	word mmh;
	//Mesh *mesh;
	bool *dmh;
	cstring *mhName;
	word mt;
	Texture *t;
	bool *dt;
	Dword ltx;
	word ma;
	//CHAnim *a;
	bool *da;
	cstring *aName;
	//--------------------------------------------------------------------------------------
	void init(){
		ltx = 0;
		mmd = mmh = mt = ma = 0;
		//model = NULL;
		mt++;
		t = NULL;RESIZE(t, Texture, mt);t[0].Init();t[0].CheckBoard("CheckerBoard.gen");
		dt = NULL;RESIZE(dt, bool, mt);dt[0] = false;
		//while(mt<t[0].id){mt++;CON.write(0,"add mised tex");}; ?
		mmh++;
		//mesh = NULL;RESIZE(mesh, Mesh, mmh);mesh[0].clear();
		//mesh[0].Box(0, 0, 0);
		//mesh[0].ScaleVerts(10, 10, 10);
		//mesh[0].ReNormal(0);
		mhName = NULL;RESIZE(mhName, cstring, mmh);mhName[0].init();mhName[0].print("ERROR\0");
		dmh = NULL;RESIZE(dmh, bool, mmh);dmh[0] = false;
		ma++;
		//a = NULL;RESIZE(a, CHAnim, ma);a[0].Clear();
		aName = NULL;RESIZE(aName, cstring, ma);aName[0].init();
		da = NULL;RESIZE(da, bool, ma);da[0] = false;
		CON.write(0, "MM::initialized");
	};
	//--------------------------------------------------------------------------------------
	void del(){
		word i;
		CON.write(0, "MM::Unload Begin()");
		//for(i = 0;i < mmh;i++){
		//	if(dmh[i])continue;
		//	CON.write(0, "MM::Mesh%i:[%s]", i, mhName[i].text);//mesh[i].del();mhName[i].del();
		//};
		////DEL_(mesh);
		DEL_(mhName);
		DEL_(dmh);
		for(i = 0;i < mt;i++){ CON.write(0, "MM::Tex%i:[%s]", i, t[i].name.text);glDeleteTextures(1, &t[i].id);t[i].del(); };
		DEL_(t);
		DEL_(dt);
		//for(i = 0;i < mmd;i++){ CON.write(0, "MM::Model%i", i);model[i]->del(); };
		//DEL_(model);
		//for(i = 0;i < ma;i++){ CON.write(0, "MM::Anim%i:[%s]", i, aName[i].text);a[i].Unload();aName[i].del(); };
		//DEL_(a);
		DEL_(da);
		DEL_(aName);
		CON.write(0, "MM::Unload End()");
	};
	//--------------------------------------------------------------------------------------
	word GetMeshMMi(const char *Sname){
		for(word i = 1;i < mmh;i++){ if(mhName[i] == Sname){ return i; }; };//mmi???
		return 0;
	};
	//--------------------------------------------------------------------------------------
	word GetAnimMMi(const char *Sname){
		for(word i = 1;i < ma;i++){ if(aName[i] == Sname){ return i; }; };
		return 0;//do no anim
	};
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	void RegisterTex(const char *name, word gl_id){//seems only 4 font gentex
		CON.write(0, "MM::register texture [%s] [t%i]", name, gl_id);
		mt++;
		RESIZE(t, Texture, mt);t[mt - 1].Init();
		t[mt - 1].id = gl_id;
		t[mt - 1].name.print("%s\0", name);
		RESIZE(dt, bool, mt);dt[mt - 1] = false;
	};
	//--------------------------------------------------------------------------------------
	Dword GetTexMMi(const char *Sname){
		for(word i = 1;i < mt;i++){ if(t[i].name == Sname){ return t[i].id; }; };
		return t[0].id;
	};
	//--------------------------------------------------------------------------------------
	Dword LoadTex(const char *nm, int param, int mode, bool alpha){//CON.write(0,"LoadTex");
		Dword unum = GetTexMMi(nm);
		if(unum != t[0].id){ CON.write(0, "MM::used texture [%s] [t%i]", nm, unum);return unum; };
		mt++;
		RESIZE(t, Texture, mt);t[mt - 1].Init();
		long size = -1;
		if(strstr(nm, ".jpg")){ size = t[mt - 1].loadJPG(nm, param, mode, alpha);unum = t[mt - 1].id; } else{
			if(strstr(nm, ".tga")){ size = t[mt - 1].loadTGA(nm, param, mode, alpha);unum = t[mt - 1].id; } else{
				CON.write(LOG_ERROR, "MM::can't load [%s]", nm);
			};
		};
		if(size == -1 || unum == -1){ mt--;return t[0].id; };
		CON.write(0, "MM::loaded texture [%s](%ix%i) [t%i(%i)]", nm, t[mt - 1].w, t[mt - 1].h, unum, t[mt - 1].id);
		RESIZE(dt, bool, mt);dt[mt - 1] = false;
		return unum;
	};
	//--------------------------------------------------------------------------------------
	void SetTexture(word n){
		//CON.write(1,"Try Set Tex [t%i] %i>%i",n,n,mt+1);
		if(ltx != n){   //coz 1st a font tex
			if(!n || n > mt + 1){/*CON.write(1,"can't found tex [%i]",n);*/n = t[0].id; };
			glBindTexture(GL_TEXTURE_2D, n);ltx = n;//tex_switch++;
			//CON.write(0,"Set Tex [t%i]",n);
		};
	};
};
//======================================================================================  