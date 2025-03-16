//======================================================================================
void mainInput(){
 GetMouse();
 pl.Control(1);
 /*if(CAP.filter==2){
  if(mouse[0])CAP.col_min=CAP.getCol(mx,my);
  if(mouse[1])CAP.col_max=CAP.getCol(mx,my);
 };*/
 //CAP.Filtering();
 //if(!grappled)
 //if(UI.Grappled<0 && !(UI.Focused>0 && UI.P[UI.Focused]->type==UI_EDIT))PROCESS();// && !UI.Highest()
};
//======================================================================================
void DrawGrid(const mat4 &RM,const vec3f &pos){
 float zoom=0.1,alpha=fabs(dot(pl.ang.dir(),RM.F));if(alpha<0.01)return;
 vec4f CX=RM.Rotate(vec3f(1,0,0));CX.x=fabs(CX.x);CX.y=fabs(CX.y);CX.z=fabs(CX.z);CX.w=alpha*0.5;
 vec4f CY=RM.Rotate(vec3f(0,1,0));CY.x=fabs(CY.x);CY.y=fabs(CY.y);CY.z=fabs(CY.z);CY.w=alpha*0.5;
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glLineWidth(1);glDisable(GL_TEXTURE_2D);glDisable(GL_LIGHTING);
 for(byte i=0;i<5;i++){
  glBegin(GL_LINES);
  float px=(int)((RM.T.x-pos.x)/zoom)*zoom,py=(int)((RM.T.y-pos.y)/zoom)*zoom;
  for(int x=-10;x<11;x++){
   glColor4fv(CY);glVertex3fv(RM*vec3f(px+ x*zoom,py-10*zoom,0));glVertex3fv(RM*vec3f(px+ x*zoom,py+10*zoom,0));
   glColor4fv(CX);glVertex3fv(RM*vec3f(px-10*zoom,py+ x*zoom,0));glVertex3fv(RM*vec3f(px+10*zoom,py+ x*zoom,0));
  };
  glEnd();
  //PrTx.Add(RM*vec3f(px+5*zoom,py+5*zoom,-pos.z),"%.2f",zoom);
  zoom*=10;
 };
 glDisable(GL_BLEND);
};
//======================================================================================
void DrawScene3d(){
 glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);glLineWidth(2);
 glBegin(GL_LINES);
 glColor3f(1,0,0);glVertex3f(0,0,0);glVertex3f(1000,0,0);
 glColor3f(0,1,0);glVertex3f(0,0,0);glVertex3f(0,1000,0);
 glColor3f(0,0,1);glVertex3f(0,0,0);glVertex3f(0,0,1000);glEnd();
 glColor3f(1,1,1);
 //vec3f dir=norm(pl.Rpos-opz);
 //mat4 RM(1);RM.Plane(dir);RM.T=opz;//+dir;//r=norm(dir-opz*dot(dir,opz));
 //if(mouse[1])DrawGrid(RM,RM.T);else{
 /*DrawGrid(mat4(1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1),vec3f(-pl.Rpos.x,-pl.Rpos.z,0));
 DrawGrid(mat4(0,0,1,0, 0,1,0,0, 1,0,0,0, 0,0,0,1),vec3f(-pl.Rpos.z,-pl.Rpos.y,0));
 DrawGrid(mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1),vec3f(-pl.Rpos.x,-pl.Rpos.y,0));*/

 glPointSize(4);glBegin(GL_POINTS);
  glVertex3fv(pl.mouse_ip);
 glEnd();

 glEnable(GL_TEXTURE_2D);
 glColor3f(1,1,1);
 CAP.GPU();
 pl.glScreenQuad();
};
//======================================================================================
void mainDraw(){
 pl.GL(1,1);
 DrawScene3d();
  UF.TextOrthoMode();
  UI.Draw();
  UF.EndTextOrthoMode();
 font.TextOrthoMode();//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);//glDisable(GL_DEPTH_TEST);
  glBegin(GL_LINES);
   glColor3f(1,0,0);glVertex2f(mx,0);glVertex2f(mx,WndH);
   glColor3f(0,1,0);glVertex2f(0,my);glVertex2f(WndW,my);
  glEnd();
  glColor3f(1,1,1);
  glBegin(GL_LINE_LOOP);
   glVertex2f(SS.x,SS.y);glVertex2f(SS.x,SE.y);
   glVertex2f(SE.x,SE.y);glVertex2f(SE.x,SS.y);
  glEnd();
  glEnable(GL_TEXTURE_2D);
 PrTx.Draw();
 PrTx.mt=0;
 //-----------------------
 Dword mss=FPS.Milliseconds()/1000,msm=mss/60,msh=msm/60;
 msm-=msh*60;
 mss-=msm*60;       
 font.Print(font.m_height,0,0,"FPS:%i (%ih:%im:%is) TimeScale(%f) DeltaTime(%f)",FPS.fps,msh,msm,mss,FPS.TimeScale,FPS.dt);
 font.Print(font.m_height,font.m_height*2,0,"%f %f",SS.x,SS.y);
 font.posy=(float)WndH-((CON.msg+4)*font.m_height);
 for(word i=0;i<CON.msg;i++){
  if(font.posy+20<my || font.posy-15>my){
   switch(CON.el[i]){
    case 0:glColor3f(1,1,1);break;//BL
    case 1:glColor3f(1,1,0);break;//YL
    case 2:glColor3f(1,0,0);break;//RD
    case 3:glColor3f(1,0,1);break;//PP
   };
   font.Print(10,font.posy,0,"%s\0",CON.text[i]); 
  };font.posy+=font.m_height;  
 };                           //  */
 font.EndTextOrthoMode();// 
 MM.ltx=999;
};
//======================================================================================