//======================================================================================
HTTP_ping n2c("192.168.1.254");

float no_input_time = 0;

void process(){
	if(applooprun == 255){
		if(key[K_ESC])applooprun = 1;
		if(check_if_all_ships_are_stopped() || (time() - force_exit_time) > 30)
			applooprun = 0;
	};

	GetMouse();
	mousemove = (lmx != mx || lmy != my);
	dxf_coils.check_time();// ! coil timer update
	UI.Process((int) mx, (int) my, (int) lmx, (int) lmy);
	if(UI.Grappled >= 0)return;
	bool free = true;
	pl->set(0, 0, WndW, WndH);
	if(!free)pl->Update();else pl->Control();

	if(UI.Grappled < 0 && UI.Focused < 0 && EditBRD_btn && EditBRD_btn->Down){
		COLCRV.edit(pl->msdir, calcPlane(vec3f(0, 1, 0), 0.0));
	};
	if(UI.Grappled < 0 && UI.Focused < 0 && EditMap_btn && EditMap_btn->Down){
		if(AddNewMapBtn->Down && mouse[0] && !lmouse[0]){
			TrajMan.num++;
			TrajMan.traj[TrajMan.num-1].init();
			AddNewMapBtn->Down = false;
		}
		for(int i = 1;i < TrajMan.num;i++){//if(CRV[i].mv<1 && AddNewMapBtn->Down==false)continue;
			bool edited = TrajMan.get(i)->edit(pl->msdir, calcPlane(vec3f(0, 1, 0), 0.0));
			if(edited)break;//only one curve at time
		};
	};
	if(mousemove || keypressed || mouseclick[0] || mouseclick[1] || mouseclick[2])no_input_time = ::time();
	if(::time() - no_input_time > 60 * 5){//60sec*5
		no_input_time = ::time();
		fit_all_btn(NULL);
	};
	//------------- setup ship base
	vec3f IP = intplane(pl->msdir.p, pl->msdir.n, vec3f(0, 1, 0), vec3f(0, 0, 0));
	if(SetShipBase_btn->Down && selShipIDX() >= 0){
		if(mouse[0] && !lmouse[0]){ ship_anchors[selShipIDX()].set(IP.x, IP.z, ship_anchors[selShipIDX()].z); };//set pos
		if(mouse[0] && lmouse[0]){ ship_anchors[selShipIDX()].z = atan2(IP.z - ship_anchors[selShipIDX()].y, IP.x - ship_anchors[selShipIDX()].x); };//set ang
	   //};
	};
	pl->Anchor(0.0);
	//------------axis snap
	if(!pl->mouselook && !mousemove && !keypressed){
		pl->Allign(vec3f(1, 0, 0));pl->Allign(vec3f(-1, 0, 0));
		pl->Allign(vec3f(0, 1, 0));pl->Allign(vec3f(0, -1, 0));
		pl->Allign(vec3f(0, 0, 1));pl->Allign(vec3f(0, 0, -1));
	};//key
	if(ExBLB_btn->Down)EX.edit(IP.x, IP.z);
	//----------------------------------- cam image control
	if(key['u'] && !lkey['u']){ //save images - lightmaps
		for(int i = 0;i < CAPM.mc;i++){
			cstring name("lightmaps\\lm_%s.jpg", CAPM.cap[i]->name);
			imwrite(name.text, CAPM.cap[i]->lightmap);
		};
	};
	if(key['y'] && !lkey['y']){ //load images - lightmaps
		for(int i = 0;i < CAPM.mc;i++){
			cstring name("lightmaps\\lm_%s.jpg", CAPM.cap[i]->name);
			CAPM.cap[i]->lightmap = imread(name.text, CV_LOAD_IMAGE_GRAYSCALE);
		};
	};
	//---------------------
	static float stmp = 37 * D2R;
	if(CAPM.closest >= 0 && CAPM.closest < CAPM.mc){//h-get angle, j&k-rotate   l-default
		SCamera *img = CAPM.cap[CAPM.closest]->cam;
		if(key['h']){//get cam angle
			mat4 RM(1);RM = img->ang;
			vec3f a = RM.Ang();
			stmp = a[2];
		};
		if(key['j']){// && !lkey['j']){     rotate CW
			mat4 RM(1);RM = img->ang;
			vec3f a = RM.Ang();
			stmp += FPS.dt*0.25;
			a[0] = -PI * 1.5;
			a[1] = stmp;
			a[2] = 0;
			RM.Rot(a);
			img->ang = RM;
		};
		if(key['k']){// && !lkey['j']){   //rotate CCW
			mat4 RM(1);RM = img->ang;
			vec3f a = RM.Ang();
			stmp -= FPS.dt*0.25;
			a[0] = -PI * 1.5;
			a[1] = stmp;
			a[2] = 0;
			RM.Rot(a);
			img->ang = RM;
		};
		if(key['l']){// && !lkey['j']){   //load cad 
			mat4 RM(1);RM = img->ang;
			vec3f a = RM.Ang();
			stmp -= FPS.dt*0.25;
			a[0] = -PI * 1.5;
			a[1] = 37 * D2R;
			a[2] = 0;
			RM.Rot(a);
			img->ang = RM;
			//reset pos
			float cam_pos_h = 255;//226;
			int start_ip = 10, off = 0, i = CAPM.cap[CAPM.closest]->m_id;
			int col = dxf_cam_pos.v[dxf_cam_pos.l[i + off].x].x, row = dxf_cam_pos.v[dxf_cam_pos.l[i + off].x].z;
			img->pos = vec3f(col, cam_pos_h, row);
		};
		if(key['i']){           // muve up
			//float hval = CAPM.cap[0]->cam->pos.y + FPS.dt * 5;
			//for(int i = 0;i < CAPM.mc;i++){
			//	CAPM.cap[i]->cam->pos.y = hval;
			//	CAPM.cap[i]->cam->Rlim();
			//	CAPM.cap[i]->cam->Update();
			//};
			//CamHeightVal->caption.print("h=%.0f", hval);
			float hval = img->pos.y + FPS.dt * 5;
				img->pos.y = hval;
				img->Rlim();
				img->Update();
			CamHeightVal->caption.print("h=%.0f", hval);
		};
		if(key['m']){                 //move down
			//float hval = CAPM.cap[0]->cam->pos.y - FPS.dt * 5;
			//for(int i = 0;i < CAPM.mc;i++){
			//	CAPM.cap[i]->cam->pos.y = hval;
			//	CAPM.cap[i]->cam->Rlim();
			//	CAPM.cap[i]->cam->Update();
			//};
			//CamHeightVal->caption.print("h=%.0f", hval);
			float hval = img->pos.y - FPS.dt * 5;
				img->pos.y = hval;
				img->Rlim();
				img->Update();
			CamHeightVal->caption.print("h=%.0f", hval);
		};
	};//end closest
	//-------------------------------------- net2can
	n2c.startThread();// flag init
	if(n2c.state != n2c.lstate){// n2c.lstate>=0 && n2c.lstate<0 && 
		if(n2c.state < 0){ W2C_p->caption.print("NET2CAN-ERR");SND.Play(snd_no_n2c); };//lost after found
		if(n2c.state >= 0){ W2C_p->caption.print("NET2CAN-OK");if(n2c.lstate != -2)SND.Play(snd_n2c_ok); };//found after lost 
		n2c.lstate = n2c.state;
	};
};
//======================================================================================