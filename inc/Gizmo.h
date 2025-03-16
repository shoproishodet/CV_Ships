#define G_T_AROT 0
#define G_T_AMOV 1
#define G_T_PMOV 2
#define G_T_PSCL 3
#define G_T_CTER 4 //center
struct Gizmo {
	mat4 SM, RM, NM;//save,edit,norm
	int sel_p, tool, QiR, QiU, QiF;
	bool grappled;
	ray lmray;
	void init() { SM.identity(); RM.identity(); NM.identity(); grappled = false; sel_p = -1; tool = -1; };
	void edit(mat4 &M, const ray &mray) {
		//change cam mouselook on center tool==4??
		float r = 0.0625*VLEN(mray.p - M.T);//sqrt(2.0)*0.5;//1;
		RM = M; ray axis;
		NM.Comp(norm(M.R), norm(M.U), norm(M.F), M.T);
		//NM=M;
		QiU = dot(vec3f(NM.U), mray.p) < dot(vec3f(NM.U), NM.T) ? -1 : 1;//camera axis sign
		QiR = dot(vec3f(NM.R), mray.p) < dot(vec3f(NM.R), NM.T) ? -1 : 1;
		QiF = dot(vec3f(NM.F), mray.p) < dot(vec3f(NM.F), NM.T) ? -1 : 1;
		grappled = lmouse[0] && mouse[0];//!!! TODO
		if (!grappled) {//-----------------------------select SM
			sel_p = -1;
			tool = -1;                                        //R0-[F2,U1],U1-[R0,F2],F2-[U1,R0]
			float dist = mray.PlaneDist(calcPlane(NM.R, NM.T)); if (VLEN(NM.inv()*mray.at(dist)) < r) { sel_p = 0; tool = G_T_AROT; };
			float dist2 = mray.PlaneDist(calcPlane(NM.U, NM.T)); if ((sel_p == -1 || (sel_p != -1 && dist2 < dist)) && VLEN(NM.inv()*mray.at(dist2)) < r) { sel_p = 1; tool = G_T_AROT; dist = dist2; };
			dist2 = mray.PlaneDist(calcPlane(NM.F, NM.T)); if ((sel_p == -1 || (sel_p != -1 && dist2 < dist)) && VLEN(NM.inv()*mray.at(dist2)) < r) { sel_p = 2; tool = G_T_AROT; dist = dist2; };
			axis.p = NM.T;
			if (sel_p != -1 && VLEN(NM.inv()*mray.at(dist)) < r*0.9) {
				tool = G_T_PMOV;
				axis.n = NM.F; if (VLEN(axis.ip(mray) - mray.at(dist)) < r*0.1) { sel_p = 2; tool = G_T_AMOV; };
				axis.n = NM.R; if (VLEN(axis.ip(mray) - mray.at(dist)) < r*0.1) { sel_p = 0; tool = G_T_AMOV; };
				axis.n = NM.U; if (VLEN(axis.ip(mray) - mray.at(dist)) < r*0.1) { sel_p = 1; tool = G_T_AMOV; };
			};
			lmray = mray; lmray.len = dist;
			SM = M;//save before edit
		} else {//-------------------------------------deform SM
			vec3f RF = !sel_p ? RM.R : sel_p == 1 ? RM.U : RM.F;
			if (tool == G_T_AMOV) { axis.set(RM.T, RF); RM.T = SM.T + (axis.ip(mray) - axis.ip(lmray)); };//axis move
			if (tool == G_T_PMOV) { RM.T = SM.T + (mray.at(mray.PlaneDist(calcPlane(RF, RM.T))) - lmray.at(lmray.len)); };//plane move
			M = RM;
		};
	};
	void Draw(const ray &mdir) {
		glDisable(GL_CULL_FACE);
		float r = 0.0625*VLEN(mdir.p - NM.T);//sqrt(2.0)*0.5;  //R(1,0,0)-C(0,1,1),G(0,1,0)-M(1,0,1),B(0,0,1)-Y(1,1,0)
		for (byte i = 0; i < 3; i++) {
			vec3f v(0.0); v[i] = r;
			vec3f icl(0.0); icl[(i + 1) % 3] = icl[(i + 2) % 3] = 1;
			vec3f cl(0.0); cl[i] = 1;
			glLineWidth((sel_p == i && tool < 2) || (tool == 2 && sel_p != i) ? 4 : 1);
			glBegin(GL_LINES); glColor3fv(cl); glVertex3fv(NM.T); glVertex3fv(NM*v); glColor3fv(icl); glVertex3fv(NM.T); glVertex3fv(NM*-v); glEnd();
			glColor3fv(cl);
			glLineWidth(sel_p == i && tool == 0 ? 4 : 1);
		};//i,dim
		r *= sqrt(2.0)*0.5;  //R(1,0,0)-C(0,1,1),G(0,1,0)-M(1,0,1),B(0,0,1)-Y(1,1,0)
		vec3f iV(QiR, QiU, QiF);
		glLineWidth(sel_p == 2 && tool == 2 ? 4 : 1); glBegin(GL_LINES); glColor3f(QiR > 0 ? 1 : 0, QiR < 0 ? 1 : 0, QiR < 0 ? 1 : 0); glVertex3fv(NM*(vec3f(r, r, 0)*iV)); glVertex3fv(NM*(vec3f(0, r, 0)*iV));
		glColor3f(QiU < 0 ? 1 : 0, QiU>0 ? 1 : 0, QiU < 0 ? 1 : 0); glVertex3fv(NM*(vec3f(r, r, 0)*iV)); glVertex3fv(NM*(vec3f(r, 0, 0)*iV)); glEnd();//plane move
		if (sel_p == 2 && tool == 2) { glColor3f(1, 1, 0); glBegin(GL_QUADS); glVertex3fv(NM*(vec3f(r, r, 0)*iV)); glVertex3fv(NM*(vec3f(0, r, 0)*iV)); glVertex3fv(NM*vec3f(0, 0, 0)); glVertex3fv(NM*(vec3f(r, 0, 0)*iV)); glEnd(); };
		glLineWidth(sel_p == 0 && tool == 2 ? 4 : 1); glBegin(GL_LINES); glColor3f(QiU < 0 ? 1 : 0, QiU>0 ? 1 : 0, QiU < 0 ? 1 : 0); glVertex3fv(NM*(vec3f(0, r, r)*iV)); glVertex3fv(NM*(vec3f(0, 0, r)*iV));
		glColor3f(QiF < 0 ? 1 : 0, QiF < 0 ? 1 : 0, QiF>0 ? 1 : 0); glVertex3fv(NM*(vec3f(0, r, r)*iV)); glVertex3fv(NM*(vec3f(0, r, 0)*iV)); glEnd();//plane move
		if (sel_p == 0 && tool == 2) { glColor3f(1, 1, 0); glBegin(GL_QUADS); glVertex3fv(NM*(vec3f(0, r, r)*iV)); glVertex3fv(NM*(vec3f(0, 0, r)*iV)); glVertex3fv(NM*vec3f(0, 0, 0)); glVertex3fv(NM*(vec3f(0, r, 0)*iV)); glEnd(); };
		glLineWidth(sel_p == 1 && tool == 2 ? 4 : 1); glBegin(GL_LINES); glColor3f(QiF < 0 ? 1 : 0, QiF < 0 ? 1 : 0, QiF>0 ? 1 : 0); glVertex3fv(NM*(vec3f(r, 0, r)*iV)); glVertex3fv(NM*(vec3f(r, 0, 0)*iV));
		glColor3f(QiR > 0 ? 1 : 0, QiR < 0 ? 1 : 0, QiR < 0 ? 1 : 0); glVertex3fv(NM*(vec3f(r, 0, r)*iV)); glVertex3fv(NM*(vec3f(0, 0, r)*iV)); glEnd();//plane move
		if (sel_p == 1 && tool == 2) { glColor3f(1, 1, 0); glBegin(GL_QUADS); glVertex3fv(NM*(vec3f(r, 0, r)*iV)); glVertex3fv(NM*(vec3f(r, 0, 0)*iV)); glVertex3fv(NM*vec3f(0, 0, 0)); glVertex3fv(NM*(vec3f(0, 0, r)*iV)); glEnd(); };
		glLineWidth(1);
		glDisable(GL_BLEND); glColor3f(1, 1, 1);
		//glEnable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);
	};
};