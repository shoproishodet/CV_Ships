// #include <stdarg.h>    Use the <stdarg.h> header (or, if you must, the older <varargs.h>).
//======================================================================================
//eng
#define t_eng_big(c)(c>64 && c<91)
#define t_eng_small(c)(c>96 && c<123)
#define t_eng(c)((c>64 && c<91) || (c>96 && c<123))
//rus ASCII
#define t_rus_Ee(c)(c==240 || c==241)
#define t_rus_big(c)(c>127 && c<160 || c==240)
#define t_rus_small(c)((c>159 && c<176) || (c>223 && c<240) || c==241)
#define t_rus(c)((c>127 && c<176) || (c>223 && c<242))
//rus ANSI
#define t_rus_Ee_ANSI(c)(c==168 || c==184)
#define t_rus_big_ANSI(c)((c>191 && c<224) || c==168)
#define t_rus_small_ANSI(c)(c>223 || c==184)
#define t_rus_ANSI(c)(c>191 || c==184 || c==168)
//special
#define t_nums(c)(c>47 && c<58)//0123456789
#define t_special(c)((c>57 && c<65) || (c>90 && c<97) || (c>122 && c<128) || (c>31 && c<42) || (c>41 && c<48) || c==9 || c==10 || c==13)
//system                 :;<=>?@            [\]^_`           {|}~           (space)!"#$%&'()      *+,-./        tab      /r      /n
#define t_system(c)(c<32)
#define t_ASCIIGraph(c)(c>175 && c<224)
#define t_dop_ASCII(c)(c>241)//242 - 255 DOPx     
//globals                                                     tab      /r      /n
#define t_printable(c)((c>31 && c<176) || (c>223 && c<242))// || c==9 || c==10 || c==13)
#define t_numeric(c)((c>47 && c<58) || (c>42 && c<47))//0123456789 +,-.
//======================================================================================
struct cstring{//constructor used in CPanel
	char *text;
	int size, msize;
	int pos, ss, se;
	//--------------------------------------------------------------------------------------
	cstring(){/*CON.write(0,"cstring constr");*/init(); };
	~cstring(){ del(); };
	void init(){ text = NULL; size = 0; msize = 0; pos = ss = se = 0; };
	void del(){ DEL_(text); };
	void resize(int nsize){//,const char *fname){memcpy(P_FUNC,fname,64);
		size = nsize;
		if (msize > size)return;
		while (nsize >= msize){ msize += 32; };
		RESIZE(text, char, msize);
	};
	cstring(int i){ init(); resize(i); for (int j = 0; j < i; j++)text[j] = '\0'; };
	cstring(const char *c){ init(); print("%s\0", c); };
	cstring(const char *fmt, ...){
		init();
		va_list ap; va_start(ap, fmt);
		int nlen = _vscprintf(fmt, ap);
		resize(nlen + 1);
		vsprintf((char*)text, fmt, ap); va_end(ap);
		//text[nlen]=0;
	};// */
	//char *read_next(){//PVS
	//	return 
	//};
	//--------------------------------------------------------------------------------------
	int num_chr(const char &chr){
		int num = 0;
		for (int i = 0; i < size; i++)num += (bool)(chr == text[i]);
		return num;
	};
	int get_line_at_pos(const int &cpz, const char &chr){
		int num = cpz, pz = 0;
		for (int i = 0; i < size; i++){
			if (text[i] == chr){ if (!num)return pz; num--; pz = i + 1; };
		};
		return pz;
	};
	//char &operator[](int i){return ((char*)text)[i];};
	//const char &operator[](int i)const{return text[i];};
	operator char*(){ return (char*)text; };
	operator const char*()const{ return (char*)text; };
	//--------------------------------------------------------------------------------------
	void print(const char *fmt, ...){//if(fmt==NULL)return;//clear();
		va_list ap; va_start(ap, fmt);
		//FILE *nD=fopen("nul:","w");
		int nlen = _vscprintf(fmt, ap);//vfprintf(nD,fmt,ap);// 
		//fclose(nD);
		resize(nlen + 1);
		vsprintf((char*)text, fmt, ap); va_end(ap);//crush 15.09.2014 test CLOG
		//CON.write(1,"%s",text);
	};
	//--------------------------------------------------------------------------------------
	bool have(const char *str){//CON.write(0,"compare [%s]==[%s]",text,str);
		return strstr(text, str) != NULL;
	};
	bool have(const cstring *str){//CON.write(0,"compare [%s]==[%s]",text,str->text);
		return (size == str->size && strstr(text, str->text) != NULL);
	};
	//--------------------------------------------------------------------------------------
	bool operator==(const char *strp){
		const char *S = strp, *T = &text[0];
		//CON.write(0,"compare [%s]==[%s]",S,T);
		do{
			if (*S != *T){ return false; };
		} while (*S++ && *T++);
		return true;
	};
	//--------------------------------------------------------------------------------------
	bool is(const cstring *str){
		//CON.write(0,"compare [%s]==[%s]",text,str->text);
		if (size == str->size){
			for (int i = 0; i < size; i++){
				if (text[i] != str->text[i]){ return false; };
			};
			return true;
		};//else{CON.write(0,"%i %i",text.size,ssz);};
		return false;
	};
	//--------------------------------------------------------------------------------------
	bool operator==(const cstring *str){
		//CON.write(0,"compare [%s]==[%s]",text,str->text);
		if (size == str->size){
			for (int i = 0; i < size; i++){
				if (text[i] != str->text[i]){ return false; };
			};
			return true;
		};//else{CON.write(0,"%i %i",text.size,ssz);};
		return false;
	};
	//--------------------------------------------------------------------------------------
	/*void getNameExt(char *where,char *name,char *ext){   //move to cstring
	 bool flag=true;
	 word cnt=0,pos=0;
	 while(*where){if(*where=='.'){pos=cnt;};*where++;cnt++;};
	 while(cnt){*where--;cnt--;};
	 cnt=0;
	 while(*where){
	 if(*where=='.' && pos==cnt){flag=false;};
	 if(flag){if(name){*name++=*where;};}else{if(ext){*ext++=*where;};};
	 *where++;
	 if(pos>0){cnt++;};
	 };
	 if(name)*name='\0';
	 if(ext)*ext='\0';
	 };*/
	//--------------------------------------------------------------------------------------
	cstring *GetExt(){
		cstring *ext = new cstring(size);//fixed "tompor. return local var."
		const char *Tx = &text[0];
		int cnt = 0, posd = 0;
		while (*Tx){ if (*Tx == '.'){ posd = cnt; }; Tx++; cnt++; };//PVS
		Tx = &text[posd];
		char *R = &ext->text[0];
		ext->size = 0;
		while (*Tx){ *R++ = *Tx++; ext->size++; };
		*R = '\0';
		return ext;
	};
	//--------------------------------------------------------------------------------------
	cstring *GetPathName2(){
		cstring *PN = new cstring(size);//fixed "tompor. return local var."
		const char *Tx = &text[0];
		int cnt = 0, posd = 0;
		while (*Tx){ if (*Tx == '.'){ posd = cnt; }; Tx++; cnt++; };//PVS
		Tx = &text[0];
		char *R = &PN->text[0];
		PN->size = 0;
		while (posd--){ *R++ = *Tx++; PN->size++; };
		*R = '\0';
		return PN;
	};
	//--------------------------------------------------------------------------------------
	char *GetName(){
		char *ret = new char[size], *R = &ret[0];
		const char *Tx = &text[0];
		int cnt = 0, posn = 0, posd = 0;
		while (*Tx){ if (*Tx == '\\'){ posn = cnt; }; Tx++; cnt++; };//PVS
		Tx = &text[posn]; cnt = -1;//?
		while (*Tx){ if (*Tx == '.'){ posd = cnt; }; Tx++; cnt++; };//PVS
		Tx = &text[posn + 1];
		while (posd--){ *R++ = *Tx++; };   //warning if have no name - deadloop
		*R = '\0';
		return ret;
	};
	//--------------------------------------------------------------------------------------
	char *GetPathName(){
		char *ret = new char[size], *R = &ret[0];
		const char *Tx = &text[0];
		int cnt = 0, pos = 0;
		while (*Tx){ if (*Tx == '.'){ pos = cnt; }; Tx++; cnt++; };//PVS
		Tx = &text[0];
		while (pos--){ *R++ = *Tx++; };
		*R = '\0';
		return ret;
	};
	//--------------------------------------------------------------------------------------
	bool CheckExt(const char *ext){
		if (!ext)return false;
		int cnt = 0;
		const char *Ext = ext, *Tx = &text[size - 1];
		while (*Ext++){ Tx--; };//PVS
		Ext = ext;
		int c1, c2;
		while (*Ext){
			c1 = *Tx++; c2 = *Ext++;
			if (c1 != c2){
				if (c1 >= 'a' && c1 <= 'z'){ c1 -= ('a' - 'A'); };
				if (c2 >= 'a' && c2 <= 'z'){ c2 -= ('a' - 'A'); };
				if (c1 != c2)return false;
			};
		};
		return true;
	};
	//--------------------------------------------------------------------------------------delete
	void DelAt(bool back){//backspace & delete
		if ((pos<size - 1 && !back) || (back && pos>0)){
			size--;
			for (int i = pos - back; i < size; i++){ text[i] = text[i + 1]; };//shift
			text[size - 1] = '\0';
			if (back)pos--;
		};
	};
	//--------------------------------------------------------------------------------------insert
	void InsAt(char chr){//insert at pos                        CON.write(1,"String::pos[%i] size(%i)",pos,size-1);
		if (pos<0 || pos>size - 1){ pos = size - 1; return; };
		resize(size + 1);
		text[size - 1] = '\0';
		if (pos<size - 2){ for (int i = size - 2; i>pos; i--){ text[i] = text[i - 1]; }; };//shift right and stop at [pos]
		text[pos] = chr;
		pos++;
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================
//======================================================================================
