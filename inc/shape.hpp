//int det(vec2f u,vec2f v){return u.x*v.y-u.y*v.x;};
//bool isLeft(vec2f a,vec2f b,vec2f c){return det(b-a,c-a)<0;}
struct shape{
	word mv, memv;
	vec2f *v;
	int *vl;//vertex link? a.vl[i,a.mv]=(j,b.mv);
	int selv;
	word mf, memf;
	vec3w *f;
	vec3i *fnb;
	byte *fstt;
	word me, meme;
	int *e[2];//user convex border line  [-1=loop -2=curve(open,no loop)]
	int *ef[2];//edge face
	byte *eid;//shape index?
   //-------------
	void init(){
		mv = memv = 0;v = NULL;selv = -1;vl = NULL;
		mf = memf = 0;f = NULL;fnb = NULL;fstt = NULL;
		me = meme = 0;e[0] = e[1] = NULL;ef[0] = ef[1] = NULL;eid = NULL;
	};
	void resizeV(const word nmv){
		if(nmv < memv){ mv = nmv;return; };
		while(memv < nmv)memv += 12;
		RESIZE(v, vec2f, memv);
		RESIZE(vl, int, memv);for(int i = mv;i < memv;i++){ vl[i] = -1; };
		mv = nmv;
	};
	void resizeE(const word nme){
		if(nme < meme){ me = nme;return; };
		while(meme < nme)meme += 12;
		RESIZE(e[0], int, meme);RESIZE(e[1], int, meme);
		RESIZE(ef[0], int, meme);RESIZE(ef[1], int, meme);
		RESIZE(eid, byte, meme);
		for(int i = me;i < meme;i++){ e[0][i] = e[1][i] = -1;ef[0][i] = ef[1][i] = -1;eid[i] = 0; };
		me = nme;
	};
	void resizeF(const word nmf){
		if(nmf < memf){ mf = nmf;return; };
		while(memf < nmf)memf += 12;
		RESIZE(f, vec3w, memf);
		RESIZE(fnb, vec3i, memf);
		RESIZE(fstt, byte, memf);for(int i = mf;i < memf;i++){ fnb[i].set(-1, -1, -1);fstt[i] = 0; };
		mf = nmf;
	};
	//add vertex link
	void Save(const char *name){
		FILE *in = fopen(name, "wb");if(!in){ CON.write(2, "can't save [%s]", name);return; };
		fwrite(&mv, sizeof(word), 1, in);fwrite(v, sizeof(vec2f), mv, in);
		fwrite(&me, sizeof(word), 1, in);fwrite(e[0], sizeof(int), me, in);fwrite(e[1], sizeof(int), me, in);
		fclose(in);
	};
	void Load(const char *name){
		FILE *in = fopen(name, "rb");if(!in){ CON.write(2, "can't load [%s]", name);return; };
		word nmv;fread(&nmv, sizeof(word), 1, in);resizeV(nmv);fread(v, sizeof(vec2f), mv, in);
		fread(&nmv, sizeof(word), 1, in);resizeE(nmv);fread(e[0], sizeof(int), me, in);fread(e[1], sizeof(int), me, in);
		fclose(in);
		//self_intersect();
		triangulate();
	};
	void add(const vec2f &point){
		//if(me && selv!=-1 && e[1][me-1]==-1){e[0][me-1]=selv;};
		resizeV(mv + 1);selv = mv - 1;v[selv] = point;
		//if(me && e[1][me-1]==-1){e[1][me-1]=selv;};
		//resizeE(me+1);e[0][me-1]=selv;
	};
	//--------------------------------------------triangulation
	bool PlaneSide(int c, int a, int b){//return isLeft(v[a],v[b],v[c]);
		vec2f N(v[b].y - v[a].y, v[a].x - v[b].x);// vec3f N;N.normal(v[a],v[b],viewer);
		return dot(N, v[c]) > dot(N, v[a]);
	};
	bool isEdge(int a, int b){
		for(int i = 0;i < me;i++)if((e[0][i] == a || e[1][i] == a) && (e[0][i] == b || e[1][i] == b))return true;
		return false;
	};
	bool Pinside(const vec2f &p)const{//http://alienryderflex.com/polyspline/
		bool inside = false;
		for(int i = 0;i < me;i++){
			if(e[1][i] < 0)continue;
			if(((v[e[0][i]].y < p.y && v[e[1][i]].y >= p.y) || (v[e[1][i]].y < p.y && v[e[0][i]].y >= p.y)) && (v[e[0][i]].x <= p.x || v[e[1][i]].x <= p.x)){
				inside ^= (v[e[0][i]].x + (p.y - v[e[0][i]].y) / (v[e[1][i]].y - v[e[0][i]].y)*(v[e[1][i]].x - v[e[0][i]].x) < p.x);
			};
		};
		return inside;
	};
	bool int_contour(const vec2f &a, int b, int ev0 = -1, int ev1 = -1){//do not int 2 egdes at i       //do not int a,b edge
		for(int i = 0;i < me;i++){
			if(e[1][i] < 0 || e[0][i] == b || e[1][i] == b || (e[0][i] == ev0 && e[1][i] == ev1) || (e[0][i] == ev1 && e[1][i] == ev0))continue;
			if(SegIntersects(a, v[b], v[e[0][i]], v[e[1][i]]))return true;
		};
		return false;
	};
	int findTriC(int a, int b, bool ue){//CON.write(0,"triangulation: search C for(%i %i)",a,b);     ue?a:-1,ue?b:-1
		for(int i = 0;i < mv;i++){
			if(i == a || i == b || PlaneSide(i, a, b))continue;// || int_contour((v[a]+v[b])*0.5,i,a,b))continue;
			vec3f cc;
			if(!Circle2(v[b] - v[a], v[i] - v[a], cc))continue;//parallel
			float r = VLEN(cc);
			//if(r>30)continue;//hack
			cc += v[a];
			bool inside = false;
			for(int j = 0;j < mv;j++){ if(j != a && j != b && j != i && VLEN(v[j] - cc) <= r){ inside = true;break; }; };// && !int_contour((v[a]+v[b]+v[i])/3.0,j)){inside=true;break;};};// i+1   
			if(inside)continue;
			return i;
		};
		return -1;
	};
	int findEdge(int a, int b){
		for(int i = 0;i < me;i++){
			if((e[0][i] == a || e[0][i] == b) && (e[1][i] == b || e[1][i] == a))return i;
		};
		return -1;
	};
	void addEdge(int a, int b){
		resizeE(me + 1);
		e[0][me - 1] = a;
		e[1][me - 1] = b;
	};
	void recAddTri(int a, int b, int pfnum, byte backi){
		//if(VLEN(v[a]-v[b])>60)return;//hack 
		int c = findTriC(a, b, !backi);if(c < 0)return;
		bool dupe = false; //if a,b,c already exist? 1,2,3 && 2,3,1
		for(int i = 0;i < mf;i++){
			if(f[i][0] == c || f[i][1] == c || f[i][2] == c){
				if(f[i][0] == a || f[i][1] == a || f[i][2] == a){
					if(f[i][0] == b || f[i][1] == b || f[i][2] == b){ dupe = true;break; };
				};
			};
		};
		if(dupe)return;
		int pfhas_ac = -1, pfhas_cb = -1;
		for(int i = 0;i < mf;i++){
			if(f[i][0] == c || f[i][1] == c || f[i][2] == c){
				if(f[i][0] == a || f[i][1] == a || f[i][2] == a){ pfhas_ac = i;if(pfhas_cb != -1)break; };
				if(f[i][0] == b || f[i][1] == b || f[i][2] == b){ pfhas_cb = i;if(pfhas_ac != -1)break; };
			};
		};//end has edge  */
		if(VLEN(v[a] - v[b]) > 60)return;//hack
		if(VLEN(v[b] - v[c]) > 60)return;//hack
		if(VLEN(v[c] - v[a]) > 60)return;//hack
		resizeF(mf + 1);int i = mf - 1;
		f[i].set(a, c, b);
		if(findEdge(a, b) == -1){ addEdge(a, b); };//ef[0][me-1]=i;}
		if(findEdge(b, c) == -1){ addEdge(b, c); };//ef[0][me-1]=i;}
		if(findEdge(c, a) == -1){ addEdge(c, a); };//ef[0][me-1]=i;}
		fnb[i].set(pfnum, pfhas_ac, pfhas_cb);
		if(pfnum != -1)fnb[pfnum][backi] = i;//set back
		if(pfhas_cb == -1)recAddTri(c, b, i, 1);
		if(pfhas_ac == -1)recAddTri(a, c, i, 2);
		//if(pfhas_cb==-1 && !isEdge(c,b))recAddTri(c,b,i,1);
		//if(pfhas_ac==-1 && !isEdge(a,c))recAddTri(a,c,i,2);
	};
	void triangulate(){
		mf = 0;
		//int last=-1,first=-1;
		recAddTri(0, 1, -1, 0);
		return;
		for(int i = 0;i < me;i++){
			if(e[1][i] < 0)continue;
			//if(first==-1)first=e[0][i];
			//last=e[1][i];
			recAddTri(e[0][i], e[1][i], -1, 0);
		};
		//if(last!=-1 && first!=-1)recAddTriE(last,first,-1,0);
	};

	void DrawCircle(const mat4 &RM, vec3f v, float r)const{
		glBegin(GL_LINE_LOOP);
		for(byte i = 0;i < 60;i++){ float ang = (float) i / 30 * PI;glVertex3fv(RM*v + RM.U*(r*sin(ang)) + RM.R*(cos(ang)*r)); };
		glEnd();
	};
	void draw_links(const mat4 &RMa, const mat4 &RMb, const shape &b){//warning vl[i] must be exactly for b.
		glBegin(GL_LINES);
		for(word i = 0;i < mv;i++){
			if(vl[i] < 0)continue;
			glVertex3fv(RMa*v[i]);glVertex3fv(RMb*b.v[vl[i]]);
		};
		glEnd();
	};
	void draw(const mat4 &RM, bool tri = 1, bool circ = 0)const{
		//------------------------triangles
		if(tri){
			for(word i = 0;i < mf;i++){//if(!fstt[i])continue;
			 //glColor3f(fstt[i]==1,!fstt[i],fstt[i]==2);//green triangles
				glBegin(GL_TRIANGLES);//
				glVertex3fv(RM*v[f[i][0]]);
				glVertex3fv(RM*v[f[i][2]]);
				glVertex3fv(RM*v[f[i][1]]);
				glEnd();
			};//i
		};
		//-----------triangle lines
		glLineWidth(1);
		for(word i = 0;i < mf;i++){//if(!fstt[i])continue;
			if(circ){
				vec3f cc = Circle(v[f[i][1]] - v[f[i][0]], v[f[i][2]] - v[f[i][0]]);
				float r = VLEN(cc);cc += v[f[i][0]];
				//glColor3f(1,0,1);
				DrawCircle(RM, cc, r);
				//glBegin(GL_LINES);glVertex3fv(RM*cc);glVertex3fv(RM*((v[f[i][1]]+v[f[i][0]]+v[f[i][2]])/3));glEnd();
			   //glColor3f(1,1,1);
			};
			glBegin(GL_LINE_LOOP);
			glVertex3fv(RM*v[f[i][0]]);
			glVertex3fv(RM*v[f[i][1]]);
			glVertex3fv(RM*v[f[i][2]]);
			glEnd();
		};//i
		//first vertex circle    
		//for(int i=0;i<me;i++){if(!i || e[1][i-1]<0)DrawCircle(RM,v[e[0][i]],0.05);};
		//for(int i=0;i<mv;i++){DrawCircle(RM,v[i],0.0125);};
		//glColor3f(1,0,0);//-------------edges
		/*glLineWidth(2);
		glBegin(GL_LINES);
		 for(int i=0;i<me;i++){if(e[1][i]<0)continue;
		  glVertex3fv(RM*v[e[0][i]]);glVertex3fv(RM*v[e[1][i]]);
		 };
		glEnd(); //*/
		// draw skelet glColor3f(0,0,1);glBegin(GL_LINES);for(int i=0;i<mf;i++){DrawSkelet(RM,i);};glEnd();
		if(selv != -1){//----------selected point
			glPointSize(4);
			glColor3f(1, 1, 0);
			glBegin(GL_POINTS);glVertex3fv(RM*v[selv]);glEnd();
		};
	};
	//------------ext
	int closest_vertex(const mat4 &RM, const vec3f &pt){
		int rez = 0;
		float clen = VLEN(RM*v[rez] - pt);
		for(word i = 1;i < mv;i++){
			float len = VLEN(RM*v[i] - pt);
			if(len < clen){ clen = len;rez = i; };
		};
		return rez;
	};
};//end shapes