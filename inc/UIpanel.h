//======================================================================================
vec3f Ecol(0.8f, 0.8f, 0.8f);
vec3f Pcol(0.92549f, 0.91372549f, 0.8470588f);
vec3f Dcol(0.2f, 0.2f, 0.2f);
vec3f Lcol(0.02f, 0.02f, 0.02f);
//======================================================================================
struct CPanel{
	float x, y, w, h, sx, sy, dx, dy, Bx, By;
	int type, z, lvl, id, rid;
	bool Down, inm, Vis, CanMove, AutoUp, AutoUpm, limit;
	vec3f color;
	cstring caption;
	CPanel *Parent, *param;
	float stp, pos, min, max, val;
	int mul, etype;
	//--------------------------------------------------------------------------------------
	typedef void(*CPCallBack)(CPanel*);
	CPCallBack use_OC, use_MO, use_Alt;
	void OnClick(){ if (use_OC){ (use_OC)(this); }; };
	void MouseOn(){ if (use_MO){ (use_MO)(this); }; };
	void UseAlt(){ if (use_Alt){ (use_Alt)(this); }; };
	void Assign_OnClick(CPCallBack newfunc){ use_OC = newfunc; };
	void Assign_MouseOn(CPCallBack newfunc){ use_MO = newfunc; };
	void Assign_UseAlt(CPCallBack newfunc){ use_Alt = newfunc; };
	//--------------------------------------------------------------------------------------
	CPanel(int nx, int ny, int nw, int nh, int nt){ Init(nx, ny, nw, nh, nt); };
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	void track_set_pos(float npos){
		pos = clamp(npos, 0.0, 1.0);
		val = min + (pos*(max - min));
		caption.print("%f", val);
	};
	void track_set_val(float nval){
		val = clamp(nval, min, max);
		pos = (val - min) / (max - min);
		caption.print("%f", val);
	};

	void edit_set_val(float sval){
		val = sval; 
		if (param != nullptr){
			float delta = (min + max);
			if(fabs(delta)<EPSILON) delta = 1;
			param->pos = (val - min) / delta; 
		}
		UseAlt();
		if (etype == UI_Ef)caption.print("%f", val); else caption.print("%i", (int)val);
	};
	void edit_set(byte edit_type, float step, bool limiters = 0, float emin = 0, float emax = 0){
		etype = edit_type; stp = step; limit = limiters; min = emin; max = emax;
	};
	//--------------------------------------------------------------------------------------
	void Init(int nx, int ny, int nw, int nh, int nt){
		x = nx; y = ny; w = nw; h = nh; type = nt;
		use_OC = use_MO = use_Alt = NULL;
		z = lvl = 0;
		sx = sy = dx = dy = 0;
		Down = inm = false;
		Vis = AutoUp = AutoUpm = true;
		color.set(0, 0, 0);
		caption.init();
		Parent = param = NULL;
		id = rid = 0;
		CanMove = false;
		Bx = By = 0;
		pos = 0.0f; stp = 1.0f;
		limit = false;
		etype = UI_Es; min = 0;
		max = 25; mul = 0; val = 0;
	};
	//--------------------------------------------------------------------------------------
	void Unload(){ caption.del(); };
	//--------------------------------------------------------------------------------------
	void SetScrolNULL(){ Bx = 0; By = 0; sx = 0; sy = 0; dx = 0; dy = 0; };
	//--------------------------------------------------------------------------------------
	void SetScrolLim(float vx, float vy){//dx,dy - ScrollLimX,ScrollLimY
		if (w < vx + sx){ Bx = 10; dx = vx - w + Bx; };//else{if(!sx){Bx=0;dx=0;}};//sx=0;
		if (h < vy + sy){ By = 10; dy = vy - h + By; };//else{if(!sy){By=0;dy=0;}};//sy=0;
	};
	//--------------------------------------------------------------------------------------
	void setParent(CPanel *p){
		if (p == NULL)return;
		Parent = p;
		z = p->z;
		lvl = p->lvl + 1;      //border
		p->SetScrolLim(x + w, y + h);//root?
	};
	//--------------------------------------------------------------------------------------
	void set_param(CPanel *p){
		param = p;
		if (p == NULL)return;
		//prid=p->id;
	};
	//--------------------------------------------------------------------------------------
	void Scroll(int dlx, int dly){
		sx -= dlx; if (sx<-dx)sx = -dx; if (sx>0)sx = 0;
		sy -= dly; if (sy<-dy)sy = -dy; if (sy>0)sy = 0;
		//if(w>vx+sx){};
	};
	//--------------------------------------------------------------------------------------
	bool Visible(){
		CPanel *TP = this;
		while (TP){
			if (!TP->Vis)return false;
			TP = TP->Parent;
		};
		return true;
	};
	//--------------------------------------------------------------------------------------
	void getpos(float &dlx, float &dly){
		dlx = dly = 0;
		CPanel *TP = Parent;
		while (TP){
			dlx += TP->x + TP->sx;
			dly += TP->y + TP->sy;
			TP = TP->Parent;
		};
	};
	//--------------------------------------------------------------------------------------
	bool in(float mx, float my){
		float dlx = 0, dly = 0;
		getpos(dlx, dly);
		int nh = h;
		if (type == UI_LIST && Down)nh = h*caption.ss;//list
		if (mx<x + dlx + 1 || mx>x + w + dlx - 1)return false;//Bx
		if (my<y + dly + 1 || my>y + nh + dly - 1)return false;//By
		CPanel *TP = Parent;
		while (TP){//mouse oqclusion
			if (TP->type == UI_MENU)return true;
			TP->getpos(dlx, dly);//!!
			if (mx > TP->x + dlx + TP->w - TP->Bx || mx<TP->x + dlx)return false;
			if (my>TP->y + dly + TP->h - TP->By || my < TP->y + dly)return false;
			TP = TP->Parent;
		};
		return true;
	};
	//--------------------------------------------------------------------------------------
	CPanel *Root(){
		CPanel *TP = this;
		while (TP->Parent){ TP = TP->Parent; };
		return TP;
	};
	//--------------------------------------------------------------------------------------
	void Move(float dlx, float dly){
		x -= dlx; if (x < 0)x = 0; 
		y -= dly; if (y<0)y = 0;
		if (type == UI_BUTTON && CanMove)setParent(Parent);//update scrollim
	};
	//--------------------------------------------------------------------------------------
	void DrawScrolls(){
		if (CanMove)return; //------draw scrolls if have
		if (dx>0){
			glBegin(GL_QUADS);
			float nw = (w - Bx * 2) - ((w - Bx * 2)*dx / (dx + (w - Bx * 2)));
			float sxk = Bx + x - sx* fabs((w - Bx * 2) - nw) / dx;
			glColor3fv(Pcol); glVertex2f(sxk, y + h - 10); glVertex2f(sxk + nw, y + h - 10);
			glColor3fv(Pcol - Dcol); glVertex2f(sxk + nw, y + h); glVertex2f(sxk, y + h);
			glEnd();
		};
		if (dy > 0){
			glBegin(GL_QUADS);
			float nh = (h - By * 2) - ((h - By * 2)*dy / (dy + (h - By * 2)));
			float syk = By + y - sy* fabs((h - By * 2) - nh) / dy;
			glColor3fv(Pcol - Dcol); glVertex2f(x + w - 10, syk); glVertex2f(x + w, syk);
			glColor3fv(Pcol); glVertex2f(x + w, syk + nh); glVertex2f(x + w - 10, syk + nh);
			glEnd();
		};
	};
	//--------------------------------------------------------------------------------------
	void DrawPB(){//Panel,Button
		vec3f uc(inm ? Pcol + Lcol : Pcol), lc(uc - Dcol), tc;
		if (Down){ tc = lc; lc = uc; uc = tc; };
		glBegin(GL_QUADS);
		glColor3fv(uc); glVertex2f(x, y); glVertex2f(x + w, y);
		glColor3fv(type == UI_PANEL ? uc : lc); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
		glEnd();
		DrawScrolls();
		glColor3fv(vec3f(Down == 0)); glBegin(GL_LINE_STRIP); glVertex2f(x, y + h); glVertex2f(x, y); glVertex2f(x + w, y); glEnd(); //border
		glColor3fv(vec3f(Down == 1)); glBegin(GL_LINE_STRIP); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h); glEnd();
		if (caption.size > 0){//    CON.write(1,"%s",caption.text);
			glColor3f(0, 0, 0);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			UF.Print(x + w*0.5f, y + h*0.5f, 1, "%s", caption.text);
			//glBegin(GL_LINES);glVertex2f(x,y);glVertex2f(x+w*0.5f,y+h*0.5f);glEnd();
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
		};
	};
	//--------------------------------------------------------------------------------------
	void DrawG(){//Group
		int hfh = UF.m_height >> 1, hfw = (int)(UF.width("%s", caption.text)) >> 1, hw = w*0.5;
		glColor3f(0, 0, 0);//0.5f,0.5f,0.5f);
		glBegin(GL_LINE_STRIP);
		glVertex2f(x - 1 + hfw + hw, y - 1 + hfh); glVertex2f(x + w - 1, y - 1 + hfh);
		glVertex2f(x + w - 1, y + h - 1); glVertex2f(x - 1, y + h - 1);
		glVertex2f(x - 1, y - 1 + hfh); glVertex2f(x - 1 - hfw + hw, y - 1 + hfh);
		glEnd();
		glColor3f(1, 1, 1);
		glBegin(GL_LINE_STRIP);
		glVertex2f(x + hfw + hw, y + hfh); glVertex2f(x + w, y + hfh);
		glVertex2f(x + w, y + h); glVertex2f(x, y + h);
		glVertex2f(x, y + hfh); glVertex2f(x - hfw + hw, y + hfh);
		glEnd();
		DrawScrolls();
		if (caption.size > 0){
			glColor3f(0, 0, 0);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			UF.Print(x + hw, y + (float)hfh, 1, "%s", caption.text);
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
		};
	};
	//--------------------------------------------------------------------------------------
	void DrawCB(){//CheckBox
		vec3f uc(inm ? Pcol + Lcol : Pcol);
		glColor3fv(uc);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(x + 5, y + 2); glVertex2f(x + 15, y + 2);
		glVertex2f(x + 5, y + 14); glVertex2f(x + 15, y + 14);
		glEnd();
		glColor3f(1, 1, 1); glBegin(GL_LINE_STRIP); glVertex2f(x + 15, y + 2); glVertex2f(x + 15, y + 14); glVertex2f(x + 5, y + 14); glEnd();
		glColor3f(0, 0, 0); glBegin(GL_LINE_STRIP); glVertex2f(x + 5, y + 14); glVertex2f(x + 5, y + 2); glVertex2f(x + 15, y + 2); glEnd();
		if (Down){
			glLineWidth(2);
			glBegin(GL_LINE_STRIP); glVertex2f(x + 7, y + 8); glVertex2f(x + 10, y + 13); glVertex2f(x + 15, y + 5); glEnd();
			glLineWidth(1);
		};
		if (caption.size > 0){
			glColor3f(0, 0, 0);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			UF.Print(x + UF.width(caption)*0.5 + 20, y + h*0.5f, 1, "%s", caption.text);
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
		};
	};
	//--------------------------------------------------------------------------------------
	void DrawE(){//editbox
		glColor3fv(vec3f(inm ? Ecol + Lcol : Ecol));
		glBegin(GL_TRIANGLE_STRIP); glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x, y + h); glVertex2f(x + w, y + h); glEnd();
		if (caption.size){
			glColor3fv(color);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			//int dlx,dly;getpos(dlx,dly);glScissor(x+dlx,(int)WndH-y-dy-h,w,h);//for now, - future may do in XX.Print(limiter)
			UF.Print(x + 5.0f, y + 2, 0, "%s", caption.text);
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
			//glDisable(GL_SCISSOR_TEST);//!!
		};
		glColor3f(1, 1, 1); glBegin(GL_LINE_STRIP); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h); glEnd();
		glColor3f(0, 0, 0); glBegin(GL_LINE_STRIP); glVertex2f(x, y + h); glVertex2f(x, y); glVertex2f(x + w, y); glEnd();
		if (inm){//textpos
			float wdt = 2.0f + UF.width(caption.pos, caption) + (5 - 1);
			glBegin(GL_LINES); glVertex2f(x + wdt, y + 2); glVertex2f(x + wdt, y + UF.m_height + 2); glEnd();
		};//else{caption.pos=caption.size-1;};?
	};
	//--------------------------------------------------------------------------------------
	void DrawTB(){//trackbar
		vec3f uc(inm ? Pcol + Lcol : Pcol), lc(uc - Dcol), tc;
		if (Down){ tc = lc; lc = uc; uc = tc; };
		glBegin(GL_QUADS);
		glColor3fv(uc); glVertex2f(x, y); glVertex2f(x + w, y);
		glColor3fv(lc); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
		glEnd();
		glColor3f(1, 1, 1); glBegin(GL_LINE_STRIP); glVertex2f(x, y + h); glVertex2f(x, y); glVertex2f(x + w, y); glEnd(); //border
		glColor3f(0, 0, 0); glBegin(GL_LINE_STRIP); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h); glEnd();
		glBegin(GL_LINES);
		//---------------------------------------------------------------------------
		stp = (w - 10) / max;
		mul = 1;
		while (stp<5 && stp>0.000001f){ mul *= 2; stp *= 2; };//simplify it
		int sp = x + 5;
		int cnt = 0;
		glBegin(GL_LINES);
		for (float j = 0; j <= (float)(w - 10); j += stp){
			glVertex2i(sp + j, 20);
			if (cnt > 9){ glVertex2f(sp + j, h - 5); cnt = 0; }
			else{ glVertex2f(sp + j, 30); cnt++; };
		};
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex2i(sp + pos + 2.5f, 20); glVertex2f(sp + pos + 2.5f, h - 5);
		glVertex2f(sp + pos - 2.5f, h - 5); glVertex2i(sp + pos - 2.5f, 20);
		glEnd();
		//--------------------------------------------------------------------------
		glColor3f(0, 0, 0);
		glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
		for (int j = 0; j < max; j += 10 * mul){ UF.Print(sp + j*stp / mul, y + h - UF.m_height / 2, 1, "%i", j); };
		glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
	};
	//--------------------------------------------------------------------------------------
	void DrawTBar_s(){
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		float wdt = pos;
		glColor3fv(Pcol - Dcol);
		glBegin(GL_QUADS);
		glVertex2f(x, y); glVertex2f(x + w*wdt, y);
		glVertex2f(x + w*wdt, y + h); glVertex2f(x, y + h);
		glEnd();
		glColor3f(0, 0, 0);
		glBegin(GL_LINE_STRIP);
		glVertex2i(x, y + h*0.5f); glVertex2i(x + w, y + h*0.5f);
		glEnd();
		glColor3f(1, 1, 1);
		glBegin(GL_LINE_STRIP); glVertex2i(x, y + h); glVertex2i(x, y); glVertex2i(x + w, y); glEnd();
		glColor3f(0, 0, 0);
		glBegin(GL_LINE_STRIP); glVertex2i(x + w, y); glVertex2i(x + w, y + h); glVertex2i(x, y + h); glEnd();
		glColor3f(0, 0, 0);
		glBegin(GL_LINES); glVertex2i(x + w*wdt, y + 2); glVertex2i(x + w*wdt, y + h - 3); glEnd();
		if (caption.size){
			glColor3fv(color);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			//int dlx,dly;getpos(dlx,dly);glScissor(x+dlx,(int)WndH-y-dy-h,w,h);//for now, - future may do in XX.Print(limiter)
			UF.Print(x + 5.0f, y + 2, 0, "%s", caption.text);
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
			//glDisable(GL_SCISSOR_TEST);//!!
		};

	};
	//--------------------------------------------------------------------------------------
	void calc_list_pos(int mx, int my){//caption.se
		float dlx = 0, dly = 0;
		getpos(dlx, dly);//crush?
		int nh = h;
		if (type == UI_LIST && Down)nh = h*caption.ss;//list
		if (mx<x + dlx + 1 || mx>x + w + dlx - 1){ return; };
		if (my<y + dly + 1 || my>y + nh + dly - 1){ return; };//outside
		for (int i = 0; i < caption.ss; i++){
			if (my<y + dly + h*(i + 1)){ caption.se = i; return; };//CON.write(2,"list got pos = %i",i);
		};
	};
	//--------------------------------------------------------------------------------------
	void UpdateList(const char *list){
		caption.print("%s", list);
		caption.ss = 1 + caption.num_chr('\n');
		if (caption.pos>caption.ss)caption.pos = caption.ss;
		// CON.write(1,"UpdateList::num_symbol in [%s] = [%i]",caption.text,caption.ss);
	};
	//--------------------------------------------------------------------------------------
	void DrawList(){
		vec3f uc(inm ? Pcol + Lcol : Pcol), lc(uc - Dcol), tc;
		int nh = h;
		if (Down){ tc = lc; lc = uc; uc = tc; nh *= caption.ss; };
		glBegin(GL_QUADS);
		glColor3fv(uc); glVertex2f(x, y); glVertex2f(x + w, y);
		glColor3fv(lc); glVertex2f(x + w, y + nh); glVertex2f(x, y + nh);
		glEnd();
		if (Down){
			glColor3fv(lc); glBegin(GL_QUADS); glVertex2f(x, y + h*caption.se); glVertex2f(x + w, y + h*caption.se); glVertex2f(x + w, y + h*(caption.se + 1)); glVertex2f(x, y + h*(caption.se + 1)); glEnd();
		};
		//DrawScrolls();
		glColor3f(1, 1, 1); glBegin(GL_LINE_STRIP); glVertex2f(x, y + nh); glVertex2f(x, y); glVertex2f(x + w, y); glEnd(); //border
		glColor3f(0, 0, 0); glBegin(GL_LINE_STRIP); glVertex2f(x + w, y); glVertex2f(x + w, y + nh); glVertex2f(x, y + nh); glEnd();
		if (caption.size > 0){//    CON.write(1,"%s",caption.text);
			glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D);
			UF.Print(x + w*0.5f, y + h*0.5f, 1, h, "%s", &caption.text[Down ? 0 : caption.get_line_at_pos(caption.pos, '\n')]);
			glDisable(GL_BLEND); glDisable(GL_TEXTURE_2D);
		};
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================