//======================================================================================
#include "bass.h"

#include <io.h>// 4 find and load
//======================================================================================
//#include <math.h>//pow
#define BASS_fade_up 0
#define BASS_fade_dn 1
#define BASS_played  2
#define BASS_stopped 3
struct CBASS{
	word ms;
	QWORD *len;
	byte *state, *loop;
	float *fade;
	HSTREAM *M;
	BASS_3DVECTOR pos, vel;
	//--------------------------------------------------------------------------------------
	void Init(){
		ms = 0; M = NULL; len = NULL; state = NULL; fade = NULL; loop = NULL;
		if(!BASS_Init(-1, 44100, 0, hWnd, NULL)){ CON.write(2, "Can't initialize bass.dll."); };
		//BASS_SetConfig(BASS_CONFIG_BUFFER, 15000);
	   /* BASS_Set3DFactors(1,1,1);
		BASS_SetEAXParameters(-1,0,-1,-1);
		float pw=pow((float)2.0,(float)(1.0f-10)/5.0f);
		BASS_Set3DFactors(-1,-1,pw);
		pos.x=0;
		pos.y=0;
		pos.z=0;
		vel.x=0;
		vel.y=0;
		vel.z=30;*/
	};
	//--------------------------------------------------------------------------------------
	void Unload(){
		BASS_Stop();
		for(int i = 0;i < ms;i++){ BASS_StreamFree(M[i]); };
		DEL_(M);DEL_(len);DEL_(state);DEL_(fade);DEL_(loop);
		BASS_Free();
	};
	//--------------------------------------------------------------------------------------
	void resize(word nms){
		if(nms <= ms){ ms = nms;return; };
		RESIZE(M, HSTREAM, nms);
		RESIZE(len, QWORD, nms);
		RESIZE(state, byte, nms);
		RESIZE(fade, float, nms);
		RESIZE(loop, byte, nms);
		for(Dword i = ms;i < nms;i++){ state[i] = BASS_stopped;fade[i] = 0;loop[i] = 0; };
		ms = nms;
	};
	//--------------------------------------------------------------------------------------
	void Set(int num, QWORD spos){ BASS_ChannelSetPosition(M[num], spos, BASS_POS_BYTE); };
	//--------------------------------------------------------------------------------------
	void SetVol(int num, float val){ BASS_ChannelSetAttribute(M[num], BASS_ATTRIB_VOL, val); };
	//--------------------------------------------------------------------------------------
	void Update(){
		for(Dword num = 0;num < ms;num++){
			if(state[num] == BASS_stopped)continue;
			if(state[num] == BASS_fade_up){
				state[num] = BASS_played;
				if(!BASS_ChannelPlay(M[num], 0))CON.write(2, "Can't play stream");
				BASS_ChannelSetAttribute(M[num], BASS_ATTRIB_VOL, 1);
				//CON.write(2,"PLAY %i",num);
			};
			if(state[num] == BASS_played && BASS_ChannelIsActive(M[num]) != BASS_ACTIVE_PLAYING){ state[num] = BASS_stopped; } else break;
			//if(state[num]==BASS_fade_up){fade[num]+=FPS.dt*0.5;if(fade[num]>=1.0){fade[num]=1.0;state[num]=BASS_played;};BASS_ChannelSetAttribute(M[num],BASS_ATTRIB_VOL,fade[num]);};
			//if(state[num]==BASS_fade_dn){fade[num]-=FPS.dt*0.5;if(fade[num]<=0.0){fade[num]=0.0;state[num]=BASS_stopped;BASS_ChannelStop(M[num]);}else BASS_ChannelSetAttribute(M[num],BASS_ATTRIB_VOL,fade[num]);};
			//if(state[num]!=BASS_stopped)break;
		};
	};
	//--------------------------------------------------------------------------------------
	void emit(char *name){
		//Load(name);
		//Play(ms-1);
	};
	//--------------------------------------------------------------------------------------
	void Play(int num, bool restart = 0, bool fading = 0){
		if(num < 0 || state==NULL)return;
		//if(state[num]==BASS_played){return;};
		state[num] = BASS_fade_up;
		//CON.write(1,"song %i fade %f",num,fade[num]);
		//if(fading && state[num]==BASS_stopped){state[num]=BASS_fade_up;};//CON.write(1,"song %i fade up %f",num,fade[num]);};
		//if(state[num]==BASS_fade_up){fade[num]+=FPS.dt;if(fade[num]>=1.0){fade[num]=1.0;state[num]=BASS_played;};BASS_ChannelSetAttribute(M[num],BASS_ATTRIB_VOL,fade[num]);};
		//if(state[num]==BASS_fade_dn){fade[num]-=FPS.dt;if(fade[num]<=0.0){fade[num]=0.0;state[num]=BASS_stopped;BASS_ChannelStop(M[num]);return;};BASS_ChannelSetAttribute(M[num],BASS_ATTRIB_VOL,fade[num]);};
		//if(state[num]==BASS_fade_up){
		 //CON.write(1,"play song %i fade %f",num,fade[num]);
		//////if(loop[num]){if(!BASS_ChannelPlay(BASS_SampleGetChannel(M[num],0),0))CON.write(2,"Can't play chanel");}else
		/////// if(!BASS_ChannelPlay(M[num],0))CON.write(2,"Can't play stream");// play the stream (continue from current position)
		//};
	};
	//--------------------------------------------------------------------------------------
	QWORD sec(int num){ return BASS_ChannelGetPosition(M[num], BASS_POS_BYTE); };// TODO
	void Pause(int num, bool fading = 0){ if(fading){ state[num] = BASS_fade_dn; } else{ BASS_ChannelStop(M[num]);state[num] = BASS_stopped; }; };
	//--------------------------------------------------------------------------------------
	int FindAndLoad(char *dir){
		_finddata_t fl;
		intptr_t hFile;
		char path[255], ext[100];
		vspf(path, "%s*.*\0", dir);
		if((hFile = _findfirst(path, &fl)) != -1){
			// do{
			if(fl.name != "." && fl.name != ".." && (strcmp(fl.name, "*.wav") == 4 || strcmp(fl.name, "*.mp3") == 4 || strcmp(fl.name, "*.ogg") == 4)){
				//SongName=(char**)realloc(SongName,sizeof(char*)*Scnt);
				//SongName[Scnt-1]=new char[(strlen(fl.name)+1)];
				//getNameExt(fl.name,SongName[Scnt-1],ext);
				vspf(path, "%s%s\0", dir, fl.name);
				_findclose(hFile);
				return Load(path);
			};
			// }while(_findnext(hFile,&fl)==0);
			_findclose(hFile);
		};
		//lend=ms-1;
		return -1;
	};
	//--------------------------------------------------------------------------------------
	int Load(char *name, bool looping = 0, bool sample = 0){
		FILE *in = fopen(name, "rb");if(!in)return -1;//CON.write(0,"load %s %i",name,CWsize(name,in));//if(in){
		fseek(in, 0, SEEK_END);
		int fsz = ftell(in);
		fseek(in, 0, SEEK_SET);
		//int fsz = //fsize(name, "rb");
		char *buf = new char[fsz + 1];fread(buf, fsz, 1, in);fclose(in);
		resize(ms + 1);
		Dword smpl = 0;
		if(sample){ M[ms - 1] = BASS_SampleLoad(true, buf, 0, fsz, 10, 0);loop[ms - 1] = 1; } else M[ms - 1] = BASS_StreamCreateFile(true, buf, 0, fsz, 0);//BASS_SAMPLE_LOOP
		//if(sample)=BASS_SampleGetChannel(smpl,FALSE);
		//delete [] buf;
		//}else{
		// M[ms-1]=BASS_StreamCreateFile(false,name,0,0,0);//BASS_SampleLoad(FALSE,name,0,0,1,BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_MONO);
		//};//BASS_SampleGetChannel(M[ms-1],FALSE);//
		if(!M[ms - 1]){ CON.write(2, "BASS::Couldn't load [%s]", name);ms--;return -1; };
		len[ms - 1] = BASS_ChannelGetLength(M[ms - 1], BASS_POS_BYTE);
		return ms - 1;
		//BASS_ChannelStop(M[ms-1]);loop[ms-1]=looping;
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================