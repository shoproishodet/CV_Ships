//======================================================================================
struct Blobber{
	int Bnum, mem, FBnum;//=0;,FBnum=0;
	vec3f *BLB;//=NULL;//=new vec3f[8*4];
	//CAPM.get_blobs(BLB,Bnum);//if(Bnum>8*4){CON.write(2,"Bnum>8*4");Bnum=8*4;};
	bool *BLB_fused;//=new bool[Bnum];memset(BLB_fused,0,sizeof(bool)*Bnum);
	vec3f *FBLB;//=new vec3f[Bnum];
	Blobber(){ init(); };
	~Blobber(){ DEL_(BLB);DEL_(FBLB);DEL_(BLB_fused); };//Destructor
	void init(){ Bnum = mem = 0;BLB = NULL;BLB_fused = NULL;FBLB = NULL; };
	void resize(int nsz){
		if(mem > nsz){ Bnum = nsz;return; };
		mem = nsz * 2;
		RESIZE(BLB, vec3f, mem);// crach on load cam 12:01,14.05.2014 +1 BLB==NULL  Fixed?
		RESIZE(BLB_fused, bool, mem);
		RESIZE(FBLB, vec3f, mem);
		Bnum = nsz;
	};
	void prepare(int size){
		resize(size);
		//memset(BLB_fused,0,sizeof(bool)*Bnum);
		for(int i = 0;i < Bnum;i++)BLB_fused[i] = 0;
	};
	void fuse(float rad){
		FBnum = 0;
		//if(Bnum<2)return;
		for(int i = 0;i < Bnum;i++){
			if(BLB_fused[i])continue;//group points that closer than min ship len
			FBnum++;
			FBLB[FBnum - 1] = BLB[i];//set
			int fnum = 1;
			//if(i==Bnum)break;??
			for(int j = i + 1;j < Bnum;j++){
				if(BLB_fused[j])continue;
				if(fVLEN2(BLB[i] - BLB[j]) < rad){ BLB_fused[j] = true;FBLB[FBnum - 1] += BLB[j];fnum++; };//fuse
			};
			FBLB[FBnum - 1] /= fnum;//mean
		};
	};
	void Draw(){
		glPointSize(4);
		glBegin(GL_POINTS);
		glColor3f(1, 0, 0);for(int i = 0;i < Bnum;i++){ glVertex3fv(BLB[i]); };//all points  RED
		glEnd();
		glPointSize(8);
		glBegin(GL_POINTS);
		glColor3f(0, 1, 0);for(int i = 0;i < FBnum;i++){ glVertex3fv(FBLB[i]); };//fused points GREEN
		glEnd();
	};
};
Blobber BLB;
//======================================================================================
void glBox(float x, float y, float z){
	glBegin(GL_LINE_LOOP);
	glVertex3fv(vec3f(0, z, 0));
	glVertex3fv(vec3f(x, z, 0));
	glVertex3fv(vec3f(x, z, y));
	glVertex3fv(vec3f(0, z, y));
	glEnd();
};
//====================================================================================== coil border off buffer
word mctoff = 0;
struct cto{//coil to off
	int coil;
	float time;
	void set(int nc){ coil = nc, time = ::time(); };
	float timeout(){ return (::time() - time); };
};
cto *coils_to_off = NULL;//x=coil, y=exec,
void add_coil_to_off(int inc){
	for(int i = 0;i < mctoff;i++){
		if(coils_to_off[i].coil < 0 || coils_to_off[i].coil == inc){ coils_to_off[i].set(inc);return; };
	};
	mctoff++;
	RESIZE(coils_to_off, cto, mctoff);//CON.write(2,"coils_to_off RESIZED %i",mctoff);
	coils_to_off[mctoff - 1].set(inc);
};
void update_border_coil_off(){
	for(int i = 0;i < mctoff;i++){
		if(coils_to_off[i].coil < 0)continue;
		if(coils_to_off[i].timeout() > 0.5){ dxf_coils.set_power(coils_to_off[i].coil, 0.0f);coils_to_off[i].coil = -1; };
	};                          //0.5 sec to off 
};
void keep_in_coil_field(const vec3f &p, const vec3f &d, bool use = true){
	//check if p near border
	bool inside = COLCRV.mc ? COLCRV.C[0].inside(p) : false;//BRD.inside(p);
	vec3f CP = COLCRV.C[0].closest_point(p);//BRD.closest_point(p);   warning COLCRV.C[0]
	if(fVLEN2(CP - p) < 20 || !inside){
		int inc = dxf_coils.closest_circle(p + norm(CP - p) * 20 * (inside ? -1 : 1));
		if(use){
			dxf_coils.set_power(inc, 1.0f);
			add_coil_to_off(inc);//save inc to off it .. when, after 0.5 sec 
		};
		glColor3f(1, 0, 0);glLineWidth(4);
		glBegin(GL_LINES);
		glVertex3fv(p);glVertex3fv(dxf_coils.center(inc));    //crush 15.07.2014 on add box  [+1 15.08.2014 1 ship on detect] 
		glVertex3fv(p);glVertex3fv(CP);
		glVertex3fv(p);glVertex3fv(p + norm(CP - p) * 20 * (inside ? -1 : 1));
		glEnd();
	};
	//check if d turned outside border
	//if it is - push p outside border (reflect?)
};
//======================================================================================
void DrawScene3d(){
	glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);glLineWidth(2);
	//glBegin(GL_LINES);
	//glColor3f(1,0,0);glVertex3f(0,0,0);glVertex3f(1000,0,0);
	//glColor3f(0,1,0);glVertex3f(0,0,0);glVertex3f(0,1000,0);
	//glColor3f(0,0,1);glVertex3f(0,0,0);glVertex3f(0,0,1000);glEnd();
	glColor3f(1, 1, 1);
	vec3f dir = norm(pl->Rpos - pl->target);
	mat4 RM(1);RM.Plane(dir);RM.T = pl->target;//+dir;//r=norm(dir-opz*dot(dir,opz));
	glPointSize(4);glBegin(GL_POINTS);glVertex3fv(pl->mouse_ip);glEnd();
	//--------- autopilot
	vec3f IP = intplane(pl->msdir.p, pl->msdir.n, vec3f(0, 1, 0), vec3f(0, 0, 0));
	if(DEV_MODE && EndbleBRD_cb->Down){ keep_in_coil_field(IP, vec3f(1, 0, 1), false); };
	// CAPM.reset_force_state();
	 //for(int i=0;i<SM.ms;i++){
	 // if(SM.ship[i].id==selShipID()){UIRead_ship_state2(i);};
	 //};
	SM.Update(IP);
	//-----------------
	glColor3f(1, 1, 1);
	CAPM.draw(pl->msdir);//DRAW CAM
	if(ExBLB_btn->Down)EX.Draw();
	pl->Draw(true);
	//--------trajectories
	for(int i = 1;i < TrajMan.num;i++)TrajMan.get(i)->Draw();
	if(EndbleBRD_cb->Down){
		COLCRV.Draw();
		update_border_coil_off();
	};
	//------------------------------------map+coils
	glColor3f(1, 0, 0);
	if(DEV_MODE && DrawCoils->Down)MAP.draw(mat4(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1), 0, 0);//triangulation
	glColor3f(1, 1, 0);TM.draw();//all trails
	glLineWidth(1);if(DrawBase && DrawBase->Down)dxf_base.draw(vec3f(0.8, 0.8, 0.8));
	if(DEV_MODE && DrawCams->Down)dxf_cam_centers.draw(vec3f(0.0, 0.8, 0.8));//Cyan
	glLineWidth(2);if(DrawConour && DrawConour->Down)dxf_relief.draw(vec3f(0.8, 0.0, 0.8));//Magenta
	glLineWidth(4);if(DrawShore && DrawShore->Down)dxf_shore.draw(vec3f(0.8, 0.8, 0.0));//Yellow
	//dxf_contour.draw(dxfRM,vec3f(1,0,0));
	//----------base points
	//if(SetShipBase_btn->Down || ){  GOTO base->Down
	glColor3f(1, 0, 1);glLineWidth(4);
	for(int i = 1;i <= MAX_SHIPS;i++){
		//vec3f dir(cos(ship_anchors[i].z)*50,sin(ship_anchors[i].z)*50,0);
		DrawShip(ship_anchor_pos(i), ship_anchor_pos(i, 1), GL_LINE_LOOP);
		//glBegin(GL_LINES);
		// glVertex3f(ship_anchors[i].x,0,ship_anchors[i].y);
		// glVertex3f(ship_anchors[i].x+dir.x,0,ship_anchors[i].y+dir.y);
		//glEnd();
	};
	//};
	//----------------------------------------------draw cam pos at dxf pos
	if(DEV_MODE){//DrawCams->Down){
		glPointSize(10);
		glBegin(GL_POINTS);
		for(int i = 0;i < dxf_cam_pos.ml;i++){
			if(dxf_cam_pos.l[i].z != 1 || CAPM.realoc)continue;//circle centers
			if(i < CAPM.mc && CAPM.cap[i] && CAPM.cap[i]->state != -1){//Opened){    //crush onload 03.07.2014 +1 05.07  +1 15.07
				if(CAPM.cap[i]->state == 1)glColor3f(0, 1, 0);//ready
				if(CAPM.cap[i]->_FORCED || CAPM.cap[i]->lnew_frame)glColor3f(1, 0, 1);//forced
				if(CAPM.cap[i]->state == 0)glColor3f(1, 1, 0);//connection
				if(CAPM.cap[i]->foundBLOB)glColor3f(0, 1, 1);//found blobs
			} else glColor3f(1, 0, 0);
			int idx = dxf_cam_pos.l[i].x;
			glVertex3fv(dxf_cam_pos.v[idx]);
		};
		glEnd();
		if(CAPM.closest >= 0){
			glDrawCircle(vec3f(0, 0, 5), vec3f(5, 0, 0), dxf_cam_pos.v[dxf_cam_pos.l[CAPM.closest].x], 20);
		};
	};
	//--------------------------coils
	static int inc = 0;
	if(!(mouse[0] && lmouse[0]))inc = dxf_coils.closest_circle(IP);
	glEnable(GL_BLEND);
	if(DEV_MODE && DrawCoils->Down){ dxf_coils.draw(vec3f(1, 1, 0.5), inc); };
	//---------------------------------------- controllers modify
	if(UI.Grappled < 0 && ReBindCoils && ReBindCoils->Down){
		if(mouse[0] && !lmouse[0])Ctrls.add_coil(Ctrls.selc, inc);
		if(key[K_BACK] && !lkey[K_BACK])Ctrls.backspace();
	};
	Ctrls.draw();
	//move coil centers
	if(DEV_MODE && inc >= 0 && move_coil_center_btn->Down){
		if(mouse[0]){
			dxf_coils.v[dxf_coils.l[inc].x].x = IP.x;dxf_coils.v[dxf_coils.l[inc].y].x = IP.x + dxf_coils.radius;MAP.v[inc].x = IP.x;
			dxf_coils.v[dxf_coils.l[inc].x].z = IP.z;dxf_coils.v[dxf_coils.l[inc].y].z = IP.z;                 MAP.v[inc].y = IP.z;
		};
	};
	if(DEV_MODE && !move_coil_center_btn->Down && !SetShipBase_btn->Down && SM.sel == -1 && EditMap_btn->Down == false && UI.Grappled < 0 && inc >= 0 && CamGizmo->Down == false){
		if(mouse[0] && !lmouse[0]){ dxf_coils.set_power(inc, 1.0f, 200); };//ON      
		if(mouse[1] && !lmouse[1]){ dxf_coils.set_power(inc, 0.0f, 200); };//OFF
	};
	//---------------- cam image edges test
	/*int lw=53,lh=-70,lz=2;
	glBegin(GL_LINES);glColor3f(0,0,1);
	 for(int i=0;i<8;i++){glVertex3f(lw*i,lz,0);glVertex3f(lw*i,lz,lh*4);};
	 glVertex3f(0,lz,lh);glVertex3f(lw*8,lz,lh);
	 glVertex3f(0,lz,lh*2);glVertex3f(lw*8,lz,lh*2);
	 glVertex3f(0,lz,lh*3);glVertex3f(lw*8,lz,lh*3);
	glEnd(); */
	//------------coil nums
	//if(inc>=0 && inc<dxf_coils.ml){PrTx.Add(pl->Project(dxf_coils.v[dxf_coils.l[inc].x]+vec3f(0,0,5)),"[%i]",inc+1);}; count number
	if(DEV_MODE)Ctrls.DrawCCname(inc);
	// if(DrawCoils->Down){
	 // for(int i=0;i<dxf_coils.mv;i+=2){//if(dxf_coils.)
	   //PrTx.Add(pl->Project(dxf_coils.v[i]),"[%i]",i/2+1);
	  //};/*coil nums*/
	// };
	 //glColor3f(1,0,0);
	 //for(int i=0;i<dxf_cam_pos.mv;i+=2){
	  //PrTx.Add(pl->Project(dxf_cam_pos.v[i]),"[%i]",i/2+1);
	 //};/*cam nums*/
	 //---------------- kalman test
	 /*static float ltime=::time();
	 if(::time()-ltime>0.1){//ShipTrail.once()){
	  ltime=::time();
	  vec3f MPT(pl->pos+pl->VRay(mx,WndH-my));
	  if(mouse[0]){
	   mtrl.add2(MPT);//0.1
	  }//else{mtrl.add(mtrl.KF.kpred);};
	  //mtrl.KF.predict();
	  //mtrl2.add(mtrl.KF.kpred);//,0.1
	 };
	 //mtrl.KF.predict();
	 //mtrl2.KF.predict();
	  glColor3f(1,0,0);mtrl.draw2();//red
	 // glColor3f(0,1,0);mtrl2.draw();//green    */
	 //-------------------------------------------------------------------------- ship points
	if(ShipDetector->Down){
		BLB.prepare(CAPM.get_blob_max()); //CRUSH: onload 23.07.2014 , 16.09.2014 +1, mutexed
		CAPM.get_blobs(BLB.BLB, BLB.Bnum);
		BLB.fuse(SM.flen);//flen=30
		if(shp_trk)shp_trk->caption.print("Blobs:%i", BLB.FBnum);
		for(int i = 0;i < BLB.FBnum;i++){
			//find closest trail > lerp to it 
			int cl = TM.closest(BLB.FBLB[i]);//frad=30
			//if(cl<0)continue;//
			//vec3f pt= (cl>=0 && TM.T[cl].mv>5)? lerp(TM.T[cl].v[0],BLB.FBLB[i],FPS.dt*2.5) :BLB.FBLB[i];// FBLB[FBnum-1];// 
			TM.update(cl, BLB.FBLB[i]);//pt);//FBLB[FBnum-1]);//v[0]=lerp(v[0],pt,0.4);   
		};
		BLB.Draw();
		//--------------- ship searcher
		for(int i = 0;i < TM.mt;i++){
			if(TM.T[i].lost())continue;
			////   TM.T[i].identify();  || TM.T[i].mode<1
			if(TM.T[i].num_rate < 100)continue;
			if(EndbleBRD_cb->Down){ keep_in_coil_field(TM.pos(i), TM.dir(i)); };
			glLineWidth(2);
			if(TM.T[i].lff){ TM.T[i].lff = 0;glColor3f(0, 1, 0);glDrawCircle(vec3f(0, 0, SM.flen), vec3f(SM.flen, 0, 0), TM.pos(i)); };//found a point (stable)
			//if(SM.find_pt(i)==-1){//found new point trail no afk
			if(!SM.used_pt(i)){//
				glDrawCircle(vec3f(0, 0, SM.flen*0.9), vec3f(SM.flen*0.9, 0, 0), TM.pos(i));//trying to find second
				float closest = 110;//max search radius of second point
				int cnum = -1;                                                         //SM.find_pt(j)!=-1     TM.T[j].mode<1 || TM.T[i].mode==TM.T[j].mode
				for(int j = 0;j < TM.mt;j++){
					if(i == j || TM.T[j].num_rate < 100 || TM.T[j].lost() || SM.used_pt(j) || (TM.T[j].blinking > 0.9f && TM.T[i].blinking > 0.9f) || (TM.T[j].blinking < 0.9f && TM.T[i].blinking < 0.9f))continue;
					float len = VLEN(TM.pos(i) - TM.pos(j));//TM.len(i,j);
					if(len > SM.flen && len < closest){ closest = len;cnum = j; };
				};//j
				if(cnum != -1){ SM.add_get(i, cnum); };//CON.write(0,"add ship [%i %i]",i,cnum);
			};//unused i
			//vec3f pt(TM.T[i].v[TM.T[i].mv-1]);
			//PrTx.Add(pl->Project(TM.T[i].v[0]),"  [%i] fs=%i fl[%i] dif(%i) a(%.3f) LT(%.3f) FT(%.3f) dT(%.2f)",i,TM.T[i].lfs,TM.T[i].lfl,TM.T[i].lfs-TM.T[i].lfl,(float)TM.T[i].lfs/(float)TM.T[i].lfl, TM.T[i].Ltime,TM.T[i].Ftime,::time()-TM.T[i].Ttime);
		};// */
	   // new searcher == find pair (1=off 2=on || 1=on 2=off)
	  /*  glLineWidth(6);
		for(int i=0;i<TM.mt;i++){if(TM.T[i].lost())continue;
		 if( TM.T[i].ff){TM.T[i].fs++;if(TM.T[i].fl){TM.T[i].Ltime=::time()-TM.T[i].Ttime; TM.T[i].Ttime=::time(); TM.T[i].lfl=TM.T[i].fl;TM.T[i].fl=0;};};//found
		 if(!TM.T[i].ff){TM.T[i].fl++;if(TM.T[i].fs){TM.T[i].Ftime=::time()-TM.T[i].Ttime; TM.T[i].Ttime=::time(); TM.T[i].lfs=TM.T[i].fs;TM.T[i].fs=0;};};//lost
		 TM.T[i].lff=TM.T[i].ff;
		 TM.T[i].ff=0;
		 TM.T[i].ffnd--;
		 if(TM.T[i].mv<10)continue;
		 glColor3f(0,1,0);glDrawCircle(vec3f(0,0,SM.flen),vec3f(SM.flen,0,0),TM.pos(i));
		 //current must blink
		 if(fabs(TM.T[i].Ltime-TM.T[i].Ftime)<0.3 && (::time()-TM.T[i].Ttime)<1){//blink test
		  if(TM.T[i].lff)glDrawCircle(vec3f(0,0,SM.flen*0.9),vec3f(SM.flen*0.9,0,0),TM.pos(i));else glDrawCircle(vec3f(0,0,SM.flen*0.8),vec3f(SM.flen*0.8,0,0),TM.pos(i));
		  //if not used
		  //find opposite blinker
		  for(int j=0;j<TM.mt;j++){if(i==j || TM.T[j].mv<10 || TM.T[j].lost() || SM.find_pt(j)!=-1)continue;
		   if(fabs(TM.T[j].Ltime-TM.T[j].Ftime)<0.3 && (::time()-TM.T[j].Ttime)<1){//blink test
			if(TM.T[i].lff==TM.T[j].lff && fabs(TM.T[i].Ttime-TM.T[j].Ttime)>0.01)continue;// not asyncronous blinking
			glColor3f(1,0,1);glBegin(GL_LINES);glVertex3fv(TM.pos(i));glVertex3fv(TM.pos(j));glEnd();
		   };
		  };
		 };
		 PrTx.Add(pl->Project(TM.T[i].v[0]),"  [%i] fs=%i fl[%i] dif(%i) a(%.3f) LT(%.3f) FT(%.3f) dT(%.2f)",i,TM.T[i].lfs,TM.T[i].lfl,TM.T[i].lfs-TM.T[i].lfl,(float)TM.T[i].lfs/(float)TM.T[i].lfl, TM.T[i].Ltime,TM.T[i].Ftime,::time()-TM.T[i].Ttime);
		};// */
		SM.Draw();
	};//ShipDetector->Down
};
//======================================================================================
void render(){
	SND.Update();
	if(key['t'] && !lkey['t'])SND.Play(snd_lost);
	if(!applooprun)return;//CON.write(1,"render()");
	//NOTE tan(90)=INF
	float hzoom = pl->rad*tan(90 * 0.5*D2R);// CON.write(0,"rad=%f hzoom=%f",pl->rad,hzoom);
	float asp = (float) pl->VP[2] / (float) pl->VP[3];
	pl->OW = hzoom * 2 * asp;
	pl->OH = hzoom * 2;
	pl->PPM = Orthographic_gl(-hzoom * asp, hzoom*asp, -hzoom, hzoom, NearPlane, FarPlane);
	pl->Update();
	pl->GL(1, 1);
	DrawScene3d();
	glLineWidth(1);
	//------------------------
	if(ship_detection){
		int det = 0, und = 0;
		for(int i = 0;i < SM.ms;i++){
			if(SM.ship[i].state == SS_RECYCLE)continue;
			if(SM.ship[i].id > 0)det++;else und++;
		};
		PrTx.Add(vec3f(1, 0, 0), true, vec3f(300, WndH / 3, 0), "DETECT SHIP [%i] (%idet/%iundet)", ship_to_detect, det, und);
	};
	//------------------------                    pl->Project(vec3f(40,0,40))
	if(applooprun == 255)PrTx.Add(vec3f(1, 0, 0), 1, vec3f(WndW / 2 - 35, WndH - (float) WndH / 5.0, 0), "Остановка кораблей перед выходом %is", 30 - int(time() - force_exit_time));
	glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0, WndW, WndH, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);glLoadIdentity();
	SM.DrawInfo2d();
	font.TextOrthoMode();
	//glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);
   //glColor3f(1,1,1);
	glColor3f(0, 1, 0);glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	PrTx.Draw();
	PrTx.mt = 0;
	//CON.write(1,"render ui()");
	UF.TextOrthoMode();
	UI.Draw();
	//if(GRAPH && GRAPH->Down)Draw_grahs();
	UF.EndTextOrthoMode();
	//CON.write(1,"render font()");
	font.TextOrthoMode();//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	 //glColor3f(1,1,1);pl->glScreenQuad2D();
	 //glDisable(GL_BLEND);CAPM.Draw_2D_Ribbon();font.TextOrthoMode();
	glDisable(GL_TEXTURE_2D);glDisable(GL_BLEND);//glDisable(GL_DEPTH_TEST);
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(0, 1, 0);glVertex2f(mx, 0);glVertex2f(mx, WndH);
	glColor3f(1, 0, 0);glVertex2f(0, my);glVertex2f(WndW, my);
	glEnd();
	glColor3f(1, 1, 1);
	//glBegin(GL_LINE_LOOP);
	 //glVertex2f(SS.x,SS.y);glVertex2f(SS.x,SE.y);
	 //glVertex2f(SE.x,SE.y);glVertex2f(SE.x,SS.y);
	//glEnd();
	glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);
	//glColor3f(1,1,1);
	glColor3f(1, 1, 0);glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//CON.write(1,"PrTx.Draw();");
	//PrTx.Draw();
	//PrTx.mt=0;
	//-----------------------
	//glColor3f(0,0,0);
	//font.posy=150;
	//for(int i=0;i<PF.mm;i++){
	// font.Print(WndW-350,font.posy,0,"%.4f:%s\0",PF.dt[i],PF.M[i].text); 
	// font.posy+=font.m_height;
	//};
	Dword mss = FPS.Milliseconds() / 1000, msm = mss / 60, msh = msm / 60;
	mss -= msm * 60;
	msm -= msh * 60;
	glColor3f(1, 0, 0);
	//CON.write(1,"FPS");
	//glColor3f(0,0,0);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	font.Print(5, 0, 0, "FPS:%.4i runtime(%ih:%im:%is) DeltaTime(%f)", FPS.fps, msh, msm, mss, FPS.dt);
	SYSTEMTIME obj;
	GetLocalTime(&obj);//   GetSystemTime
	//..obj.wDay << "." << obj.wMonth << "." << obj.wYear     WORD wYear;    WORD wMonth;
	font.Print(5, 20, 0, "TIME:(%id.%im.%iy %ih:%im:%is)", obj.wDay, obj.wMonth, obj.wYear, obj.wHour, obj.wMinute, obj.wSecond);
	if(DEV_MODE)font.Print(5, 40, 0, "trackers=%i(FBLB=%i) powered_coils=%i(L=%i,R=%i)", TM.mt, BLB.FBnum, dxf_coils.poweredL + dxf_coils.poweredR, dxf_coils.poweredL, dxf_coils.poweredR);
	if(key['`'] && !lkey['`'])CON.show = !CON.show;
	//CON.write(1,"CON");
	if(CON.show){
		font.posy = 150;
		glColor3f(0, 0, 0);
		for(int i = 0;i < LogPage.pr;i++){
			font.Print(10, font.posy, 0, "%s\0", LogPage.row[(LogPage.pos + i) % (LogPage.pr)].text);
			font.posy += font.m_height;
		};
		//(float)WndH-((CON.msg+1)*font.m_height);
		//for(word i=10;i<CON.msg;i++){//CON.write(1,"CON %i",i);
		// switch(CON.el[i]){
		//  case 0:glColor3f(0,0,0);break;//BL
		//  //case 0:glColor3f(1,1,1);break;//BL
		//  case 1:glColor3f(0.7,0.7,0.1);break;//YL
		//  case 2:glColor3f(1,0,0);break;//RD
		//  case 3:glColor3f(1,0,1);break;//PP
		// };
		// font.Print(10,font.posy,0,"%s\0",CON.text[i]);   //here crushed
		// font.posy+=font.m_height;
		// //if(font.posy>WndH-10)break;
		//};                           //  */
	};//CON.show
	//CON.write(1,"CON end");
	font.EndTextOrthoMode();// 
	MM.ltx = 999;
	//CON.flush();
	CAP_IP_FRAMES = 0;
	//PF.push("CON.Draw()");
	//CON.write(1,"render end()");
};
//======================================================================================