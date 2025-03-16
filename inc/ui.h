//======================================================================================
#define UI_BUTTON 1
#define UI_PANEL  2
#define UI_GROUP  3
#define UI_EDIT   4
#define UI_MENU   5
#define UI_TBAR   6
#define UI_CBOX   7
#define UI_TBAR_s 8
#define UI_LIST   9
#define UI_Ef 2
#define UI_Ei 1
#define UI_Es 0
CFontGL UF;
//======================================================================================
#include "UIpanel.h"
//======================================================================================
void Check_EDIT(CPanel *p){//p->inm=true;  
	if(mouse[0]){//---------------------mouse
		if(mx != lmx && p->etype > 0){
			double val = atof(p->caption) + (mx - lmx)*p->stp;
			if(p->limit)val = clamp(val, p->min, p->max);
			if(p->etype == UI_Ef)p->caption.print("%f", val);else p->caption.print("%i", (int) val);
			p->val = val;//miss it?
			p->UseAlt();//realtime
			//CON.write(0,"EDIT:%i",)
		};
		float dlx, dly;p->getpos(dlx, dly);
		//p->caption.pos=mx-p->x-dlx;
		if(mx < p->x + dlx + UF.width(p->caption) + 1){ while(mx > p->x + dlx + UF.width(p->caption.pos, p->caption)){ p->caption.pos++; }; } else{ p->caption.pos = p->caption.size - 1; };//move right
		if(mx > p->x + dlx){ while(mx < p->x + dlx + UF.width(p->caption.pos, p->caption)){ p->caption.pos--; }; } else{ p->caption.pos = 0; };//move left
	};//--------------------------------end mouse
	p->inm = true;//draw cursor pos
	if(!keypressed)return;
	if(key[K_LEFT]){ if(p->caption.pos > 0)p->caption.pos--;return; };
	if(key[K_RIGHT]){ if(p->caption.pos < p->caption.size - 1)p->caption.pos++;return; };
	if(key[K_UP] || key[K_DOWN])return;
	if(key[K_HOME]){ p->caption.pos = 0;return; };
	if(key[K_END]){ p->caption.pos = p->caption.size - 1;return; };
	if(kb_key == K_BACK){ p->caption.DelAt(1);return; };
	if(kb_key == K_DEL){ p->caption.DelAt(0);return; };
	if(key[K_ENTER]){ p->UseAlt();return; };
	//if(key[K_SHIFT] || key[K_CTRL] || key[K_ALT])return;
	if(t_printable(kb_key))p->caption.InsAt(kb_key);
};
//======================================================================================
void CTBAR_sMPOS(CPanel *p){
	float dx, dy;
	p->getpos(dx, dy);
	float npos = clamp((mx - dx - p->x) / p->w, 0, 1);
	if(fabs(p->pos - npos) < EPSILON)return;
	p->pos = npos;
	//if(p->pos<0.0f)p->pos=0.0f;
	//if(p->pos>1.0f)p->pos=1.0f;
	p->UseAlt();
	if(p->param){
		//p->param->caption.print("%f",p->pos);
		p->param->UseAlt();
	};
};
//======================================================================================
struct CUI{
	CPanel **P, *L;
	int lb, lp, lg, le, lm, maxz;
	int LW, LH, LT, LL;
	word mp;
	int Grappled, Focused;
	//--------------------------------------------------------------------------------------
	typedef void(*UIresizeCB)(CUI*);
	UIresizeCB USE;
	void Resize(){ if(USE){ (USE) (this); }; };
	//--------------------------------------------------------------------------------------
	void Init(UIresizeCB newfunc){
		CON.write(0, "UI::Init()");
		P = NULL;mp = 0;L = NULL;
		USE = newfunc;
		Grappled = Focused = -1;
		UF.Init("MS Sans Serif", 10);
		maxz = 0;
	};
	//--------------------------------------------------------------------------------------
	void Unload(){
		CON.write(0, "UI::Unload::Begin()");
		for(int i = 0;i < mp;i++){ P[i]->Unload(); };DEL_(P);
		CON.write(0, "UI::Unload::End()");
	};
	//--------------------------------------------------------------------------------------
	CPanel *Add(int tp, float nx, float ny, float nw, float nh, CPanel *PT, char *cpt, CPanel::CPCallBack func, bool AutoUpb, bool Visb, bool Floatb){
		mp++;
		RESIZE(P, CPanel*, mp);
		P[mp - 1] = new CPanel(nx, ny, nw, nh, tp);
		L = P[mp - 1];L->Vis = Visb;L->CanMove = Floatb;L->AutoUp = AutoUpb;
		if(cpt)L->caption.print("%s\0", cpt);
		switch(tp){
			case UI_BUTTON:lb = mp - 1;break;
			case UI_PANEL:lp = mp - 1;break;
			case UI_GROUP:lg = mp - 1;break;
			case UI_EDIT:le = mp - 1;break;
			case UI_MENU:lm = mp - 1;break;
			case UI_CBOX:{L->w = 15 + 7 + UF.width(L->caption.text);break;};
		};                                            //15==group header height
		if(ny == -1 && PT){ L->y = getH(PT->id);if(L->y < 15)L->y = 15; };//autoheight 
		if(L->type == UI_CBOX)L->AutoUp = false;
		L->id = mp - 1;//else crush line:92 //set new id. when [1111_2222_ SetFG(3)333 _222 33 22 444]
		if(L->type == UI_TBAR_s){
			L->Assign_MouseOn(CTBAR_sMPOS);
			//L->Assign_OnClick(CTBAR_sMPOS);redundant?
		};
		if(PT){ L->setParent(PT); } else{ L->z = maxz + 1;SetFG(L->id); };
		if(func){ L->Assign_OnClick(func); } else{};
		return L;
	};
	//--------------------------------------------------------------------------------------
	float getH(int n){
		if(n < 0 || n >= mp)return 17;
		float gh = P[n]->h;
		for(int i = 0;i < mp;i++){
			if(!P[i]->Parent || P[i]->Parent->id != n)continue;
			float th = P[i]->y + P[i]->h;
			if(th > gh)gh = th;
		};
		P[n]->h = gh;P[n]->SetScrolNULL();
		return gh;
	};
	//--------------------------------------------------------------------------------------
	void swap(int a, int b){ CPanel *TPN = P[a];P[a] = P[b];P[b] = TPN; };
	//--------------------------------------------------------------------------------------
	void SetFG2(int num){
		Focused = num;//!
		int i, j, oldz = P[num]->z, N = 0;
		if(maxz == oldz)return;
		for(i = 0;i < mp;i++){ if(P[i]->z == oldz)N++; };//number of childs
		CPanel **T = new CPanel*[N + 1];
		num = P[num]->Root()->id;
		for(j = 0, i = num;i < mp;i++){ if(P[i]->z == oldz)T[j++] = P[i]; };//fill T[] root.1.2.3
		for(i = num, j = num;j < mp;j++){ if(P[j]->z != oldz){ P[i] = P[j];P[i]->id = i;P[i]->z--;i++; }; };// all T[] to P[] exept numz
		for(i = 0;i < N;i++){ T[i]->id = mp - N + i;T[i]->z = maxz;P[mp - N + i] = T[i]; };//set new id
		delete[] T;
		Grappled = -1;
	};
	//--------------------------------------------------------------------------------------
	void SetFG(int num){
		SetFG2(num);return;Focused = num;//!
		int i, j, oldz = P[num]->z, N = 0;
		if(maxz == oldz)return;
		for(i = 0;i < mp;i++){ if(P[i]->z == oldz)N++; };//number of childs
		CPanel **T = new CPanel*[N + 1];
		num = P[num]->Root()->id;
		for(j = 0, i = num;i < mp;i++){ if(P[i]->z == oldz)T[j++] = P[i]; };//fill T[] root.1.2.3
		for(i = num, j = num;j < mp;j++){ if(P[j]->z != oldz){ P[i] = P[j];P[i]->id = i;P[i]->z--;i++; }; };// all T[] to P[] exept numz
		for(i = 0;i < N;i++){ T[i]->id = mp - N + i;T[i]->z = maxz;P[mp - N + i] = T[i]; };//set new id
		delete[] T;
		Grappled = -1;
	};
	//--------------------------------------------------------------------------------------
	bool MenuFocused(const int num){
		if(Focused < 0)return false;
		CPanel *TP = P[Focused];
		while(TP){
			if(TP->id == num)return true;
			TP = TP->Parent;
		};
		return false;
	};
	//--------------------------------------------------------------------------------------
	void Process(float mx, float my, float lmx, float lmy){
		int one = -1, oneZ = -1, oneL = -1;
		if(LW != WndW || LH != WndH || LT != WndT || LL != WndL){ LW = WndW;LH = WndH;LT = WndT;LL = WndL; };// Resize();
		if(!mouse[0] && !mouse[1])Grappled = -1;
		/*if(Grappled>=0 && P[Grappled]->type==UI_TBAR){one=Grappled;
		  P[one]->MouseOn();//track bar
		}else
		if(Grappled>=0 && P[Grappled]->type==UI_BUTTON){one=Grappled;
		 if(P[one]->type==UI_BUTTON && P[one]->CanMove){P[one]->Move(lmx-mx,lmy-my);};
		}else
		if(Grappled>=0 && (P[Grappled]->type==UI_PANEL || P[Grappled]->type==UI_GROUP)){one=Grappled;
		 CPanel *Root=P[one]->Root();
		 if(Root->CanMove)Root->Move(lmx-mx,lmy-my);
		 if(Root==P[one])Root->Scroll(lmx-mx,lmy-my);
		 if(P[one]->type==UI_GROUP)P[one]->Scroll(lmx-mx,lmy-my);*/
		if(Grappled >= 0){
			one = Grappled;
			//if(P[one]->type==UI_LIST && P[one]->Down){P[one]->Down=false;CON.write(2,"UP");return;};//P[one]->OnClick();P[one]->calc_list_pos(mx,my);};
			//if(P[one]->type==UI_LIST){P[one]->MouseOn();return;};
			if(P[one]->type == UI_TBAR_s){ P[one]->MouseOn();return; };
			if(P[one]->Parent && P[one]->Parent->type == UI_TBAR){ P[one]->MouseOn();return; };
			if(P[one]->type == UI_BUTTON && P[one]->CanMove){ P[one]->Move(lmx - mx, lmy - my);return; };
			if(P[one]->type == UI_PANEL || P[Grappled]->type == UI_GROUP){
				CPanel *Root = P[one]->Root();
				if(Root->CanMove)Root->Move(lmx - mx, lmy - my);
				if(Root == P[one])Root->Scroll(lmx - mx, lmy - my);
				if(P[one]->type == UI_GROUP)P[one]->Scroll(lmx - mx, lmy - my);return;
			};
		};
		for(int i = 0;i < mp;i++){
			if(!P[i]->Visible())continue;
			P[i]->inm = false;
			if(P[i]->type == UI_EDIT && ((Focused == i && !mouse[0]) || (Grappled == i && mouse[0])))Check_EDIT(P[i]);
			if(!P[i]->in(mx, my)){
				if(P[i]->type == UI_MENU && !MenuFocused(i))P[i]->param->Vis = false;
				if(P[i]->type == UI_BUTTON && !P[i]->AutoUp && Focused == i){ P[i]->Down = !P[i]->AutoUpm; };//like in windows     
				//if(P[i]->type==UI_LIST && P[i]->Down){CON.write(2,"UP");};//P[one]->OnClick();P[one]->calc_list_pos(mx,my);};
				//if(P[i]->type==UI_LIST && P[i]->Down){P[one]->calc_list_pos(mx,my);};
				if(P[i]->type == UI_BUTTON && P[i]->Down && P[i]->AutoUp){ P[i]->Down = false; };continue;
			};
			if(oneZ < P[i]->z || (oneZ == P[i]->z && oneL < P[i]->lvl)){ one = i;oneZ = P[i]->z;oneL = P[i]->lvl; };//setone
		};
		//};//end search the one
		if(one != -1)P[one]->inm = true;
		if(Grappled != -3 && one != -1){
			if(mouse[0] && Grappled == -1){ Grappled = one;SetFG(one); };
			if(P[one]->type == UI_CBOX){ if(lmouse[0] && !mouse[0]){ P[one]->Down = !P[one]->Down;P[one]->OnClick(); }; };
			if(P[one]->type == UI_MENU){//menu show rule
				if(mouse[0] && P[one]->param && !P[one]->param->Vis)P[one]->param->Vis = true;
			};
			if(P[one]->type == UI_LIST){
				if(P[one]->Down){ P[one]->calc_list_pos(mx, my); };
				if(!lmouse[0] && mouse[0]){
					if(P[one]->Down){ P[one]->caption.pos = P[one]->caption.se;P[one]->Down = false;CON.write(2, "SEL %i", P[one]->caption.pos); } else{ P[one]->Down = true; };//CON.write(2,"press");
				};
			};
			if(P[one]->type == UI_BUTTON){
				if(P[one]->AutoUp){
					if(P[one]->Down && lmouse[0] && !mouse[0] && Focused == one){ P[one]->OnClick();if(MenuFocused(one))Focused = -1; };//1!
					if(Focused == one)P[one]->Down = mouse[0];//2!
				} else{
					if(!lmouse[0] && mouse[0]){ P[one]->Down = !P[one]->Down;P[one]->AutoUpm = P[one]->Down; };//press
					if(mouse[0] && Focused == one)P[one]->Down = P[one]->AutoUpm;//hold
					if(lmouse[0] && !mouse[0] && Focused == one){ P[one]->OnClick();P[one]->AutoUpm = !P[one]->AutoUpm;if(MenuFocused(one))Focused = -1; };//release
				};
			};
		} else{ if(Grappled == -1 && mouse[0]){ Grappled = -3;Focused = -1; }; };//grap empty
	};
	//--------------------------------------------------------------------------------------
	//void UpdateList(CPanel *EM_list,const char *list){
	// EM_list->caption.print("%s",list);
	//};
	//--------------------------------------------------------------------------------------
	void Draw(){
		float dlx, dly;
		int cnt = 0;
		glDisable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_TEXTURE_2D);
		for(int i = 0;i < mp;i++){
			if(!P[i]->Visible())continue;//P && P[i] &&  
			if(P[i]->Parent){
				//scissor
				CPanel *TP = P[i]->Parent, *TP2 = NULL;
				int SX = 0, SY = 0, SW = WndW, SH = WndH, pbx, pby;
				while(TP){
					if(TP->type == UI_MENU)break;//?? what if more complex
					TP->getpos(dlx, dly);           float dn = 0;
					if(SX < TP->x + dlx)SX = TP->x + dlx;if(TP2){ dn = TP2->x + TP->sx - TP2->Bx;if(dn < 0)dn = 0; };if(SW > TP->w - dn)SW = TP->w - dn;
					if(SY < TP->y + dly)SY = TP->y + dly;if(TP2){ dn = TP2->y + TP->sy - TP2->By;if(dn < 0)dn = 0; };if(SH > TP->h - dn)SH = TP->h - dn;
					TP2 = TP;//??
					TP = TP->Parent;
				};//glColor3f(1,0,0);glBegin(GL_LINE_LOOP);glVertex2i(SX,SY);glVertex2i(SX,SY+SH);glVertex2i(SX+SW,SY+SH);glVertex2i(SX+SW,SY);glEnd();
				glScissor(SX, WndH - SY - SH, SW, SH);
				glEnable(GL_SCISSOR_TEST);
				P[i]->getpos(dlx, dly);glPushMatrix();glTranslatef(dlx, dly, 0);
			};
			switch(P[i]->type){
				case UI_BUTTON:
				case UI_MENU:
				case UI_PANEL:P[i]->DrawPB();break;
				case UI_GROUP:P[i]->DrawG();break;
				case UI_EDIT:P[i]->DrawE();break;
				case UI_CBOX:P[i]->DrawCB();break;
				case UI_TBAR:P[i]->DrawTB();break;
				case UI_TBAR_s:P[i]->DrawTBar_s();break;
				case UI_LIST:P[i]->DrawList();break;
			};
			if(Focused == i){
				int nh = P[i]->h;
				if(P[i]->type == UI_LIST && P[i]->Down)nh = P[i]->h*P[i]->caption.ss;//list
				glColor3f(1, 0, 0);glBegin(GL_LINE_LOOP);glVertex2f(P[i]->x, P[i]->y);glVertex2f(P[i]->x + P[i]->w, P[i]->y);glVertex2f(P[i]->x + P[i]->w, P[i]->y + nh);glVertex2f(P[i]->x, P[i]->y + nh);glEnd();
			};
			if(P[i]->Parent){ glDisable(GL_SCISSOR_TEST);glPopMatrix(); };
		};
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================