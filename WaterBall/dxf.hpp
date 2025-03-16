struct DXF{
	Dword mv;
	vec3f *v;
	Dword ml, mc;
	vec3i *l;//
	float *power, *time;
	byte *auth;
	float radius;
	float scale;
	word poweredL, poweredR;
	~DXF(){       //this not work
	// CON.write(0,"TURN OFF ALL COILS");
	 //for(int i=0;i<ml;i++){if(l[i].z!=1)continue;
	 // SEND_com_command2(i,0);
	// };
	};
	void OFF_ALL(){
		CON.write(0, "TURN OFF ALL COILS ml=%i", ml);
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			//SEND_com_command2(i,0);
			set_power(i, 0);
		};
	};
	void init(){
		mv = ml = mc = 0;
		v = NULL;
		l = NULL;
		power = NULL; time = NULL;
		radius = 1;
		auth = NULL;
		poweredL = poweredR = 0;
	};
	int push_vertex(const vec3f &pv){
		for(int i = 0;i < mv;i++){
			if(VLEN(v[i] - pv) < EPSILON){ return i; };
		};
		mv++;
		RESIZE(v, vec3f, mv);
		v[mv - 1] = pv;
		return mv - 1;
	};
	int push_line(const vec3f &sv, const vec3f &ev){
		int p1 = push_vertex(sv);
		int p2 = push_vertex(ev);
		for(int i = 0;i < ml;i++){
			if(l[i].x == p1 && l[i].y == p2 && l[i].z == 0){ return i; };//dupe
		};
		ml++;
		RESIZE(l, vec3i, ml);
		l[ml - 1].set(p1, p2, 0);
		return ml - 1;
	};
	int push_circle(const vec3f &sv, const float &rad){
		int p1 = push_vertex(sv);
		int p2 = push_vertex(sv + vec3f(rad, 0, 0));
		for(int i = 0;i < ml;i++){
			if(l[i].x == p1 && l[i].y == p2 && l[i].z == 1){ return i; };//dupe
		};
		ml++;
		RESIZE(l, vec3i, ml);
		RESIZE(power, float, ml);
		RESIZE(time, float, ml);
		RESIZE(auth, byte, ml);
		l[ml - 1].set(p1, p2, 1);//1 - circle? rad=abs(p1.x-p2.x);
		radius = rad;//fabs(v[l[0].x].x-v[l[0].y].x)*0.1f;//CONST 
		//CON.write(0,"COIL RAD = %f",rad);
		power[ml - 1] = 0.0f;
		time[ml - 1] = 0.0f;
		auth[ml - 1] = 0;
		return ml - 1;
	};
	void load(const char *name){
		FILE *in = fopen(name, "rb");if(in == NULL){ return; };
		CON.write(0, "DXF::load [%s]", name);
		init();
		char sline[256];
		double td1 = 0, td2 = 0, td3 = 0, td4 = 0;
		int ml = 0, mc = 0;
		while(fgets(sline, 256, in) != NULL){
			if(strncmp(sline, "AcDbLine\r", 9) == 0){
				ml++;
				//CON.write(0,"DXF::line %i ",ml);
				//start point
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td1); };//X coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td2); };//Y coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){};//{sscanf(sline,"%d",&td);};//Z coord
				//end point
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td3); };//X coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td4); };//Y coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){};//{sscanf(sline,"%d",&td);};//Z coord
				push_line(vec3f(td1, 0, td2)*0.1, vec3f(td3, 0, td4)*0.1);
			};
			if(strncmp(sline, "AcDbCircle\r", 11) == 0){
				mc++;
				//CON.write(0,"DXF::circle %i ",mc);
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td1); };//X coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td2); };//Y coord
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){};//{sscanf(sline,"%d",&td);};//Z coord
				//radius
				if(fgets(sline, 256, in) != NULL){};//skip
				if(fgets(sline, 256, in) != NULL){ sscanf(sline, "%lf", &td3); };//rad
				push_circle(vec3f(td1, 0, td2)*0.1, td3*0.1);
			};
		};
		fclose(in);
	};
	/*
	void set_power(const int &num,const float &pwr,const byte &authory=0){
	 //4ship * 2magnet * 4ccoil = 32
	 if(powered>16-1 && pwr>0.0f){return;};//CON.write(1,"only 16 coils");
	 if(num>=0 && num<ml && authory>=auth[num]){// CON.write(2,"SET  (%i) to %.1f",num,pwr);
	  //if(power[num]<=0.0 && pwr==power[num])return; ??? not work
	  if(pwr==power[num] && (time[num]!=0.0f && ::time()-time[num]>=0.0))return;//no need to update coil state
	  power[num]=pwr;if(pwr>0.0f || authory)time[num]=::time();else time[num]=0.0f;
	  auth[num]=authory;
	  SEND_com_command2(num,pwr*100);
	 };
	};
	void check_time(){
	 float now=::time();
	 powered=0;
	 for(int i=0;i<ml;i++){if(l[i].z!=1 || time[i]==0.0f)continue;
	  if(now-time[i]>30 || (auth[i] && now-time[i]>10)){auth[i]=0;set_power(i,0.0f);};
	  if(power[i]>0.0f)powered++;
	 };
	};//*/
	bool ctrl_side_left(int num){
		int nnum = CTRL_get_adr(num);if(nnum < 0)return 0;
		int controller = 1 + nnum / 32;         //(k*32-1)+(i-1)
		//int coil=1+nnum%32;
		return (controller > 19);
	};
	void set_power(const int &num, const float &pwr, const byte &authory = 0){
		int ipwr = pwr * 100;
		//if(powered>16-1 && ipwr>0)return;//limit
		int powered = ctrl_side_left(num) ? poweredL : poweredR;
		if((!key['c'] && (powered > 30 - 1)) && ipwr > 0)return;//limit
		if(num < 0 || num >= ml || authory < auth[num])return;//outrange
		if(pwr == 0.0f && power[num] == 0.0f)return;//skip offed coils
		if(fabs(pwr - power[num]) < 0.009f)return;//small deviation
		power[num] = pwr;if(pwr > 0.0f || authory)time[num] = ::time();else time[num] = 0.0f;
		auth[num] = authory;
		SEND_com_command2(num, pwr * 100);
	};
	//void set_power(const int &num,const float &pwr,const byte &authory=0){
	// //4ship * 2magnet * 4ccoil = 32
	// if(powered>16-1 && pwr>0.0f){return;};//CON.write(1,"only 16 coils");
	// if(num>=0 && num<ml && authory>=auth[num]){// CON.write(2,"SET  (%i) to %.1f",num,pwr);
	//  //if(power[num]<=0.0 && pwr==power[num])return; ??? not work
	//  if(pwr==power[num] && (time[num]!=0.0f && ::time()-time[num]>=0.0))return;//no need to update coil state
	//  power[num]=pwr;if(pwr>0.0f || authory)time[num]=::time();else time[num]=0.0f;
	//  auth[num]=authory;
	//  SEND_com_command2(num,pwr*100);
	// };
	//};
	void check_time(){
		float now = ::time();
		poweredL = poweredR = 0;
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			if(auth[i] && now - time[i] > 30){ auth[i] = 0;set_power(i, 0.0f); };
			if(time[i] == 0.0f || (now - time[i] > 30))continue;
			if(power[i] > 0.009f)//powered++;
				if(ctrl_side_left(i))poweredL++;else poweredR++;
		};
	};//*/
	void draw(vec3f col, int inm = -1){
		//-----------lines
		glColor3fv(col);
		glBegin(GL_LINES);
		for(int i = 0;i < ml;i++){
			if(l[i].z != 0 || l[i].y >= mv || l[i].x >= mv)continue; //fix?
			glVertex3fv(v[l[i].x]);glVertex3fv(v[l[i].y]);
		};
		glEnd();
		//-----------circles
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			if(inm == i)glColor4f(1, 0, 1, 0.5);else glColor4f(1, 1 - power[i], 0.5, 0.5);
			if(inm == -1)glColor4fv(vec4f(col, 0.5));
			float cr = fabs(v[l[i].x].x - v[l[i].y].x);
			glBegin(GL_LINE_STRIP);
			for(byte j = 0;j <= 30;j++){ float ang = (float) j / 30 * TWOPI;glVertex3fv(v[l[i].x] + vec3f(0, 0, cr)*sin(ang) + vec3f(cr, 0, 0)*cos(ang)); };
			glEnd();
			if(time[i]){
				glBegin(GL_TRIANGLE_FAN);
				glVertex3fv(v[l[i].x]);
				for(byte j = (::time() - time[i]);j <= 30;j++){ 
					float ang = (float) j / 30 * TWOPI;
					glVertex3fv(v[l[i].x] + vec3f(0, 0, cr*0.7)*sin(ang) + vec3f(cr*0.7, 0, 0)*cos(ang)); 
				};
				glEnd();
			};

		};
	};
	int in_circle(const vec3f &mppos){
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			float cr = fabs(v[l[i].x].x - v[l[i].y].x);
			if(fVLEN2(v[l[i].x] - mppos) < cr){ return i; };
		};
		//else return closest
		return -1;
	};
	int closest_circle(const vec3f &mppos){
		int closest = -1;
		float mlen = 999;
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			//float cr=fabs(v[l[i].x].x-v[l[i].y].x);
			float len = fVLEN2(v[l[i].x] - mppos);
			if(len < mlen){ closest = i;mlen = len; };
		};
		return closest;
	};
	//----------------------------------------------------------------------------------
	int circles_in_rad(const vec3f &pos, const float &r, int *idx){
		int found = 0;
		for(int i = 0;i < ml;i++){
			if(l[i].z != 1)continue;
			float len = fVLEN2(v[l[i].x] - pos);
			if(len < r && found < 10){ idx[found] = i;found++; };//RESIZE(idx,int,found);
		};
		return found;
	};
	//----------------------------------------------------------------------------------
	vec3f center(int a){ return a >= 0 ? v[l[a].x] : vec3f(0.0); };
	int move(const vec3f &dir, const vec3f &ship){//sets power to near coils
	 //int scoil=in_circle(RM,ship);if(scoil<0)return -1;
		float cr = fabs(v[l[0].x].x - v[l[0].y].x);
		vec3f aim_pt(ship + dir * cr*1.5);
		return closest_circle(aim_pt);
	}; /**/
	int get_power(int num){
		if(num < 0 || num >= ml)return -1;
		return power[num] * 100;
	};
	vec3f move2(const vec3f &dir, const vec3f &ship){//sets power to near coils
	 //int scoil=in_circle(RM,ship);if(scoil<0)return -1;
		float cr = fabs(v[l[0].x].x - v[l[0].y].x);
		return (ship + dir * cr*1.5);
	}; /**/

};