//======================================================================================
//======================================================================================
struct SCamera {//unic pos,ang,target,mouse_ip,VP
	vec3f pos, Rpos, mdir, vel, va, vb, vc, vd, mouse_ip;
	ray dir, msdir;
	quat ang, Rang;
	mat4 PPM, MVM, MVP, iMVP, RM;
	int VP[4];
	float fov, Near, Far, Rmin, Rmax, rad, ox, oy, cx, cy, dis[5], OW, OH, lastTime;
	//Frustum F;
	vec3f target, objpos;
	bool mouselook;
	Dword id;
	Gizmo G;
	char *name;
	char con_adr[64];
	float crop_border;
	//----edit callback
	typedef void(*SCAMCallBack)(ray*);
	SCAMCallBack edit_callback;//NULL
	void edit() { if (edit_callback) { (edit_callback)(&this->msdir); }; };
	void Assign_edit(SCAMCallBack newfunc) { edit_callback = newfunc; };
	//----------------
	SCamera() { init(GFOV, NearPlane, FarPlane); };
	void init(float fv, float nr, float fr) { 
		crop_border=1;
		RM.identity(); lastTime = 0.0f; G.init(); name = NULL; fov = fv; Near = nr; Far = fr; pos = 0; ang = 0; rad = 1; target = 0; mouselook = false; id = 0; Rlim(); edit_callback = NULL; for (byte i = 0; i < 5; i++)dis[i] = 0; Rmin = Near; Rmax = Far; Rlim(); 
	};
	void Rlim() { Rpos = pos; Rang = ang; dir.set(pos, ang.dir()); };
	void set(int x, int y, int w, int h) { VP[0] = x; VP[1] = y; VP[2] = w; VP[3] = h; PPM = Perspective(fov, (float)VP[2] / (float)VP[3], Near, Far); Update(); };
	void Save(char *nm = NULL) {
		//if(!nm)nm=name;else name=nm;
		cstring str("CAM\\%s.params", nm ? nm : name);
		CON.write(0, "CAM::save %s", str.text);
		FILE *in = fopen(str.text, "wt"); if (!in) { CON.write(2, "can't open [%s]", str.text); return; };
		fprintf(in, "frame: %i %i\n", VP[2], VP[3]);
		fprintf(in, "distortion5: %f %f %f %f %f\n", dis[0], dis[1], dis[2], dis[3], dis[4]);
		fprintf(in, "projection: %f %f %f %f %f %f %f %f\n", ox, oy, cx, cy, Near, Far, Rmin, Rmax);
		fprintf(in, "pos: %f %f %f\n", pos[0], pos[1], pos[2]);
		fprintf(in, "ang: %f %f %f %f\n", ang[0], ang[1], ang[2], ang[3]);//*/
		fprintf(in, "con_str: %s\n", con_adr);//*/
		fprintf(in, "crop_border: %f\n", crop_border);//*/
		fclose(in);
	};
	void Load(char *nm = NULL) {
		//if(!nm)nm=name;else name=nm;
		cstring str("CAM\\%s.params", nm ? nm : name);
		FILE *in = fopen(str.text, "rt"); if (!in) { return; };//CON.write(2,"can't open [%s]",str.text);return;};
		char sline[256];
		fgets(sline, 256, in); sscanf(sline, "frame: %i %i\n", &VP[2], &VP[3]);//                                         CON.write(0,"frame: %i %i",VP[2],VP[3]);
		fgets(sline, 256, in); sscanf(sline, "distortion5: %f %f %f %f %f\n", &dis[0], &dis[1], &dis[2], &dis[3], &dis[4]);//CON.write(0,"distortion5: %f %f %f %f %f",dis[0],dis[1],dis[2],dis[3],dis[4]);
		fgets(sline, 256, in); sscanf(sline, "projection: %f %f %f %f %f %f %f %f\n", &ox, &oy, &cx, &cy, &Near, &Far, &Rmin, &Rmax);//           CON.write(0,"projection: %f %f %f %f %f %f %f %f",ox,oy,cx,cy,Near,Far,Rmin,Rmax);
		fgets(sline, 256, in); sscanf(sline, "pos: %f %f %f\n", &pos[0], &pos[1], &pos[2]);          ///                    CON.write(0,"pos: %f %f %f",pos[0],pos[1],pos[2]);
		fgets(sline, 256, in); sscanf(sline, "ang: %f %f %f %f\n", &ang[0], &ang[1], &ang[2], &ang[3]);//                   CON.write(0,"ang: %f %f %f %f",ang[0],ang[1],ang[2],ang[3]);
		con_adr[0]=0;
		if(fgets(sline, 256, in) != NULL){
			sscanf(sline, "con_str: %s\n", con_adr);//                  
			CON.write(0,"--- debug ------ load adr [%s]",con_adr);
		}
		if(fgets(sline, 256, in) != NULL){
			sscanf(sline, "crop_border: %f\n", &crop_border);//                  
			CON.write(0,"--- debug ------ crop_border [%f]",crop_border);
		}
		// */
	   // LookAt(intplane(pos,ang.dir(),vec3f(0,1,0),vec3f(0,0,0)),true);
		LookAt(vec3f(pos.x, 0, pos.z), true); Rlim();
		//LookAt(pos+ang.dir()*rad,true);Rlim();
		setCV(VP[2], VP[3], ox, oy, cx, cy);
		fclose(in);
		Update();
		//CON.write(0,"CAM[%ix%i] fovX=%.2f fovY=%.2f fovD=%.2f", VP[2],VP[3], acos(dot(va.norm2(),vb.norm2()))*R2D, acos(dot(va.norm2(),vc.norm2()))*R2D, acos(dot(va.norm2(),vd.norm2()))*R2D);
	};
	void setCV(int w, int h, float nox, float noy, float ncx, float ncy) {
		ox = nox; oy = noy; cx = ncx; cy = ncy;
		VP[0] = 0; VP[1] = 0; VP[2] = w; VP[3] = h;
		PPM = Perspective(fov, (float)VP[2] / (float)VP[3], Near, Far);
		PPM[0] = (2 * ox / w);//+(edit1->val-128)*0.05;     intrinsic.at<double>(0, 0)
		PPM[5] = (2 * oy / h);//+(edit2->val-128)*0.05;     intrinsic.at<double>(1, 1)
		PPM[8] = -(-1 + (2 * cx / w));//+(edit3->val-128)*0.01;   intrinsic.at<double>(0, 2)
		PPM[9] = -(-1 + (2 * cy / h));//+(edit4->val-128)*0.01; intrinsic.at<double>(1, 2)
		PPM[10] = (-(Far + Near) / (Far - Near));//*(b[16]->Down?-1:1);
		PPM[11] = -1;
		PPM[14] = -(2 * Far*Near) / (Far - Near);
		Update();
	};
	void LookAt(const vec3f &new_target, bool set_target_only = false) {
		target = new_target;//mouse_ip=target;
		if (!set_target_only)ang.LookAt(pos, target, vec3f(0, 1, 0));
		float dist = VLEN(target - pos);
		rad = dist;//clamp(dist,Near*2,Far*0.25);
	};
	void Control() {//bool mouselook=mouse[1]){ only for this user
	 //------------------------------------------------------keyboard
		vec3f stp(0.0f), stp2d(0.0f);
		if (key['w'])stp += ang.dir();
		if (key['s'])stp -= ang.dir();
		if (key['d']) { stp += ang.right(); stp2d += ang.right(); };
		if (key['a']) { stp -= ang.right(); stp2d -= ang.right(); };
		if (key['e']) { stp += mouselook ? vec3f(0, 1, 0) : ang.up(); stp2d += ang.up(); };
		if (key['q']) { stp -= mouselook ? vec3f(0, 1, 0) : ang.up(); stp2d -= ang.up(); };
		if (key[K_SPACE])stp *= 10.0f;
		if (key[K_CTRL])stp *= 0.1f;
		//if(!pl_mouse_mode)stp*=rad*0.1;//autospeed
		//---------- move
		if (!mouselook) {
			pos += stp * FPS.dt*rad;
			target += stp2d * FPS.dt*rad;
		}
		else {
			pos += stp * FPS.dt * 10;
		};
		// if(wheel)pos-=dir.n*wheel*rad*0.1;
		//--------------------------------------------------------mouse //dir.at(rad)
		///~if(mouseclick[1])if(mouselook){mouselook=false;LookAt(target);}else{mouselook=true;};
		if (mouselook) {// || mouse[2]){//normal mouselook
			SetMouse(VP[2] / 2, VP[3] / 2);
			if (lmouseclick[1]) { mx = VP[2] / 2; my = VP[3] / 2; };
			quat dx, dy;
			dx.unit_from_axis_angle(vec3f(0, -1, 0), ((VP[2] / 2) - mx)*0.1*D2R*sign_b(dot(vec3f(0, 1, 0), ang.up())));
			dy.unit_from_axis_angle(vec3f(-1, 0, 0), ((VP[3] / 2) - my)*0.1*D2R);//dy.unit_from_axis_angle(ang.right(),-((VP[3]/2)-my)*0.1*D2R);//ang=dy*dx*ang;
			ang = dx * ang*dy; ang.norm();
		};//else LookAt(target);
		//set(0,0,WndW,WndH);
		if (wheel)rad = clamp(rad - wheel * rad*0.1, Near * 2, Far*0.5);
		if (mouse[2]) { vec3f mov = ang.right()*((lmx - mx) / VP[2] * OW) + ang.up()*((my - lmy) / VP[3] * OH); pos += mov; target += mov; lmx = mx; lmy = my; };
		Update();//2
		//if(mouselook && !mouse[2]){mouse_ip=target=dir.at(rad);}; new commented
		//if(!mouse[2]){
		mouse_ip = msdir.at(rad);
		//};
		//edit callback??? quat ang;ray msdir;vec3f plane_point;
		//edit();
	};
	void Update() {
		//Nvel=vel+acc*dt;
		//pos=(Nvel+vel)*0.5*dt;
		//vel=Nvel;
		Rpos = pos;//Rpos+=(pos-Rpos)*FPS.dt*20;//
		Rang = ang;//Rang.slerp(Rang,ang,FPS.dt*20);// 
		MVM = ModelView(Rang, Rpos);
		//PPM=Perspective(fov,(float)VP[2]/(float)VP[3],Near,Far);
		//MVP=(PPM*MVM).inv();
		MVP = PPM.mul4(MVM);
		//F.set(MVP);//UPDATE FRUSTUM
		//FR = F;//COPY4MM::DRAW
		iMVP = MVP.inv();
		//iMVP.inverse4();
		vec4f vz;//4VRay
		vz = iMVP.Rotate(vec4f(-1, -1, 1, 1)); va = vz / vz.w;//va.norm();
		vz = iMVP.Rotate(vec4f(1, -1, 1, 1)); vb = vz / vz.w;//vb.norm();
		vz = iMVP.Rotate(vec4f(1, 1, 1, 1)); vd = vz / vz.w;//vd.norm();
		vz = iMVP.Rotate(vec4f(-1, 1, 1, 1)); vc = vz / vz.w;//vc.norm();
		mdir = ang.dir();//VRay(mx,VP[3]-my);//vector to near plane
		//mdir=NearPlanePos(mx,VP[3]-my);
		//mdir=UnProj(mx,VP[3]-my);//vector to near plane
		//msdir.set(Rpos,norm(mdir));//norm?
		msdir.set(Rpos + ang.right()*(mx / VP[2] - 0.5)*OW + ang.up()*((1.0 - my / VP[3]) - 0.5)*OH, norm(mdir));
		dir.set(Rpos, Rang.dir());//ray
	};
	void glScreenQuad2D() {
		glDisable(GL_DEPTH_TEST);//glEnable(GL_DEPTH_WRITE);//
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2f(0, 0);
		glTexCoord2i(1, 0); glVertex2f(VP[2], 0);
		glTexCoord2i(1, 1); glVertex2f(VP[2], VP[3]);
		glTexCoord2i(0, 1); glVertex2f(0, VP[3]);
		glEnd(); glEnable(GL_DEPTH_TEST);
	};
	void glScreenQuad(float dist = -1) {
		glDisable(GL_CULL_FACE);//glDisable(GL_DEPTH_TEST);//glEnable(GL_DEPTH_WRITE);//
		if (dist == -1)dist = Near;
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3fv(intplane(Rpos, va, dir.n, dir.at(dist)));//Rpos+va*dist);
		glTexCoord2f(1, 0); glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(dist)));//target));//Rpos+vb*dist);
		glTexCoord2f(1, 1); glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(dist)));//target));//Rpos+vd*dist);
		glTexCoord2f(0, 1); glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(dist)));//target));//Rpos+vc*dist);
		glEnd();//glEnable(GL_DEPTH_TEST);
	};
	void glScreenQuadRay(vec3f PN, vec3f PA) {
		glDisable(GL_CULL_FACE);//glDisable(GL_DEPTH_TEST);//glEnable(GL_DEPTH_WRITE);//
		float vec[4];
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);
		float vec1[4] = { 1.0, 0.0, 0.0, 0.0 };
		float vec2[4] = { 0.0, 1.0, 0.0, 0.0 };
		float vec3[4] = { 0.0, 0.0, 1.0, 0.0 };
		float vec4[4] = { 0.0, 0.0, 0.0, 1.0 };
		glTexGenfv(GL_S, GL_EYE_PLANE, vec1); //vec не меняется 
		glTexGenfv(GL_T, GL_EYE_PLANE, vec2);
		glTexGenfv(GL_R, GL_EYE_PLANE, vec3);
		glTexGenfv(GL_Q, GL_EYE_PLANE, vec4);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glMatrixMode(GL_TEXTURE); glLoadIdentity();
		glTranslatef(0.5f, 0.5f, 0.0f);
		glScalef(0.5f, 0.5f, 1.0f);
		glMultMatrixf(PPM);
		glMultMatrixf(MVM);
		vec3f vv;
		glBegin(GL_QUADS);
		vv = intplane(Rpos, va, PN, PA); glTexCoord2fv(vec2f(vv.v)); glVertex3fv(vv);//Rpos+va*dist);
		vv = intplane(Rpos, vb, PN, PA); glTexCoord2fv(vec2f(vv.v)); glVertex3fv(vv);//target));//Rpos+vb*dist);
		vv = intplane(Rpos, vd, PN, PA); glTexCoord2fv(vec2f(vv.v)); glVertex3fv(vv);//target));//Rpos+vd*dist);
		vv = intplane(Rpos, vc, PN, PA); glTexCoord2fv(vec2f(vv.v)); glVertex3fv(vv);//target));//Rpos+vc*dist);
		glEnd();//glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_TEXTURE); glLoadIdentity();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	};
	void GL(bool clearColor = 0, bool clearDepth = 0) {
		glViewport(VP[0], VP[1], VP[2], VP[3]);
		if (clearColor)glClear(GL_COLOR_BUFFER_BIT);
		if (clearDepth)glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION); glLoadMatrixf(PPM);
		glMatrixMode(GL_MODELVIEW); glLoadMatrixf(MVM);
	};
	float DistToCursor(const vec3f &P) { return VLEN(Rpos + msdir.n*dot(msdir.n, P - Rpos) - P); };
	bool Anchor(const vec3f &obj_pos) {
		if (mouselook && !mouse[2] && !mouse[1] && !mouse[0] &&
			VLEN(obj_pos - target) > EPSILON //not same position
			&& dot(obj_pos - Rpos, dir.n) > 0 //infront camera
			&& VLEN(vec2f(Project(obj_pos)) - vec2f(VP[2] / 2, VP[3] / 2)) < VLEN(vec2f(Project(target)) - vec2f(VP[2] / 2, VP[3] / 2))) {//less screen dist to cursor 2d //DistToCursor(obj_pos)<rad){
			LookAt(obj_pos, !mouse[2]); return true;
		};
		return false;
	};
	void Allign(const vec3f &idir) { if (!mouse[2] && dot(dir.n, idir) > 0.99)pos = target - idir * VLEN(pos - target); };
	vec3f Project(const vec3f &obj) {//if(!F.PointIn(obj))return 0.0;//3D->screen
		vec4f in(obj, 1.0f);
		in = MVP * in; if (in[3] == 0.0f)in[3] = 1.0;      // CON.write(2,"Project z== %f",(1.0f+in[2]/in[3])*0.5);
		return vec3f(VP[0] + (1.0f + in[0] / in[3])*VP[2] * 0.5, VP[1] + (1.0f - in[1] / in[3])*VP[3] * 0.5, (1.0f + in[2] / in[3])*0.5);
	};
	vec3f NearPlanePos(const float &x, const float &y) {//[-1,1] -> [W,H] ->
		return ang.dir()*NearPlane + ang.right()*(x - VP[2] * 0.5f) + ang.up()*(y - VP[3] * 0.5f);
	};
	vec3f VRay(const float &x, const float &y) {//return UnProj(x,y);//screen->3D
		float sx = (x - VP[0]) / VP[2], sy = (y - VP[1]) / VP[3];//crush from CapManager::draw()  25.12*3 27.12(on load)  09.01.14(onload)
		vec3f a(va + (vb - va)*sx); // interpolate by x
		vec3f b(vc + (vd - vc)*sx);
		return (a + (b - a)*sy);// interpolate by y
	};
	vec3f at(const float &x, const float &y, float r = 0) { return Rpos + VRay(x, y)*(r == 0 ? rad : r); };

	void DrawPlaneBox() {
		glBegin(GL_LINE_LOOP);//border?
		glVertex3fv(intplane(Rpos, va, dir.n, dir.at(rad)));//Rpos+va*rad);
		glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(rad)));//Rpos+vb*rad);
		glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(rad)));//Rpos+vd*rad);
		glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(rad)));//Rpos+vc*rad);
		glEnd();
	};
	void Draw(bool inside = true, bool volume = false) {
		//glColor3f(0.5,0.5,0.5);
		glLineWidth(1);
		if (!inside) {
			glBegin(GL_LINES);
			glColor3f(1, 0, 0); glVertex3fv(Rpos); glVertex3fv(Rpos + Rang.dir()*rad);//optical axis RED
			glColor3f(0, 1, 0);// Green
			glVertex3fv(Rpos); glVertex3fv(Rpos + Rang.up());//up
			vec4f p1 = calcPlane(cross(va, vb), Rpos); glVertex3fv(target); glVertex3fv(p1.intPlane(target, -Rang.up()));//target
			vec4f p2 = calcPlane(cross(vc, vd), Rpos); glVertex3fv(target); glVertex3fv(p2.intPlane(target, Rang.up()));
			glColor3f(0, 0, 1);// Blue
			glVertex3fv(Rpos); glVertex3fv(Rpos + Rang.right());//right
			p1 = calcPlane(cross(va, vc), Rpos); glVertex3fv(target); glVertex3fv(p1.intPlane(target, -Rang.right()));
			p2 = calcPlane(cross(vb, vd), Rpos); glVertex3fv(target); glVertex3fv(p2.intPlane(target, Rang.right()));
			glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3fv(Rpos); glVertex3fv(intplane(Rpos, va, dir.n, dir.at(rad)));//Rpos+va*rad);
			glVertex3fv(Rpos); glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(rad)));//Rpos+vb*rad);
			glVertex3fv(Rpos); glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(rad)));//Rpos+vc*rad);
			glVertex3fv(Rpos); glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(rad)));//Rpos+vd*rad);
			glEnd();

			DrawPlaneBox();
			glLineWidth(1);
			if (volume) {
				glColor3f(1, 0, 1);//VOLUME
				glBegin(GL_LINE_LOOP);
				glVertex3fv(intplane(Rpos, va, dir.n, dir.at(Rmin)));
				glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(Rmin)));
				glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(Rmin)));
				glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(Rmin)));
				glEnd();
				glBegin(GL_LINE_LOOP);
				glVertex3fv(intplane(Rpos, va, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(Rmax)));
				glEnd();
				glBegin(GL_LINES);
				glVertex3fv(intplane(Rpos, va, dir.n, dir.at(Rmin))); glVertex3fv(intplane(Rpos, va, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(Rmin))); glVertex3fv(intplane(Rpos, vb, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(Rmin))); glVertex3fv(intplane(Rpos, vd, dir.n, dir.at(Rmax)));
				glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(Rmin))); glVertex3fv(intplane(Rpos, vc, dir.n, dir.at(Rmax)));
				glEnd();
			};
			//len to ground plane
			glBegin(GL_LINES);
			float hh = VLEN(intplane(Rpos, va, dir.n, dir.at(rad)) - intplane(Rpos, vc, dir.n, dir.at(rad)))*0.5f;
			glVertex3fv(Rpos); glVertex3fv(Rpos - Rang.up()*hh);//side
			glVertex3fv(Rpos - Rang.up()*hh); glVertex3fv(target - Rang.up()*hh);//len
			glVertex3fv(target); glVertex3fv(target - Rang.up()*hh);//side
			glEnd();

		}
		else {//end other view cam outside
		//inside camera
		//scale circles
			glColor3f(0, 1, 0);
			mat4 RM(1); RM.Plane(Rpos - target);
			float z = 0.1;
			for (int j = 0; j < 6; j++) {//if(F.PointIn(target+RM.U*z))
				PrTx.Add(vec3f(1), 0, Project(target + RM.U*z), z < 1 ? "%.1f" : "%.0f", z);
				glBegin(GL_LINE_LOOP); for (byte i = 0; i < 60; i++) { float ang = (float)i / 30 * PI; glVertex3fv(target + RM.R*sin(ang)*z + RM.U*cos(ang)*z); }; glEnd();
				z *= 10;
			};
		};
		glColor3f(1, 0, 0);
		glPointSize(4); glBegin(GL_POINTS);
		glVertex3fv(mouse_ip);
		glVertex3fv(target);
		//glColor3f(1,0,1);glVertex3fv(opz+TP.get(dir));
		//glColor3f(0,0,1);for(byte i=0;i<TP.mp;i++)glVertex3fv(opz+TP.V[i]);
		glEnd();
		glLineWidth(1);
	};
};
//======================================================================================
struct CamManager {
	Dword mc, realoc;
	SCamera **cam;
	CamManager() { init(); };
	void init() { mc = 0; cam = NULL; realoc = false; };
	SCamera *add(const vec3f &npos, const vec3f &ntarget) {
		realoc = true;
		//resize(mc+1);
		int id = mc;
		int nmc = mc + 1;
		RESIZE(cam, SCamera*, nmc);
		cam[id] = NULL;
		cam[id] = new SCamera();
		cam[id]->pos = npos;
		cam[id]->LookAt(ntarget); cam[id]->Rlim();
		cam[id]->set(0, 0, WndW, WndH);
		cam[id]->id = id;
		//for(int i=0;i<mc;i++){
		 //CON.write(2,"cam[%i](%i)->pos(%f %f %f) npos(%f %f %f)",i,cam[i]->id,cam[i]->pos.x,cam[i]->pos.y,cam[i]->pos.z,npos.x,npos.y,npos.z);
		//};
		mc = nmc; realoc = false;
		return cam[id];
	};
};
//======================================================================================