//======================================================================================
/*#define t_eng_big(c)(c>64 && c<91)
#define t_eng_small(c)(c>96 && c<123)
#define t_eng(c)((c>64 && c<91) || (c>96 && c<123))

#define t_rus_Ee_ASCII(c)(c==240 && c==241)
#define t_rus_Ee_ANSI(c)(c==168 && c==184)
#define t_rus_big_ASCII(c)(c>127 && c<160 || c==240)
#define t_rus_big_ANSI(c)((c>191 && c<224) || c==168)
#define t_rus_small_ASCII(c)((c>159 && c<176) || (c>223 && c<240) || c==241)
#define t_rus_small_ANSI(c)(c>223 || c==184)
#define t_rus_ASCII(c)((c>127 && c<176) || (c>223 && c<242))
#define t_rus_ANSI(c)(c>191 || c==184 || c==168)

#define t_adv_char(c)((c>31 && c<48) || (c>57 && c<65) || (c>90 && c<97) || (c>122 && c<127))
#define t_nums(c)(c>47 && c<58)
#define t_other(c)((c<32 || (c>126 && c<192)) && !t_rus_Ee_ANSI(c))

//#define t_printable(c)(t_eng(c) || t_rus(c) || t_nums(c) || t_adv_char(c))
#define t_adv_printable(c)(c==32 || c==39 || (c>43 && c<48) || c==59 || c==61 || (c>90 && c<94))
#define t_printable(c)(!t_other(c))
#define t_shift_printable(c)((c>32 && c<39) || (c>39 && c<44) || c==58 || c==60 || c==62 || c==63 || c==64 || c==94 || c==95 || (c>122 && c<127))
#define t_adv2(c)(c==0 || c==27 || c==8)

#define t_printable_ANSI(c)(c>191 || (c>32 && c<126) || c==184 || c==168);/**/
//======================================================================================
struct fontchar_s{
	int width;
	float u, v;
	float u2, v2;
};
//======================================================================================
struct CFontGL{
	//public:
	int m_init, m_height, m_width, m_memsize, TW, TH;
	float posy;
	unsigned int m_textureId;
	fontchar_s m_chars[256];
	char m_text[2048];
	//--------------------------------------------------------------------------------------
	void Init(const char *pFontName, int pointSize){
		int i;
		HDC hDC = CreateCompatibleDC(NULL);
		SetMapMode(hDC, MM_TEXT);
		int height = MulDiv(pointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		HFONT hFont = CreateFont(
			height,                   // logical height of font
			0,                        // logical average character width
			0,                        // angle of escapement
			0,                        // base-line orientation angle
			FW_DONTCARE,              // font weight
			0,                        // italic attribute flag
			0,                        // underline attribute flag
			0,                        // strikeout attribute flag
			DEFAULT_CHARSET,          // character set identifier
			OUT_TT_PRECIS,	        // Output Precision
			CLIP_DEFAULT_PRECIS,	    // Clipping Precision
			ANTIALIASED_QUALITY,	    // Output Quality
			FF_DONTCARE | DEFAULT_PITCH,// Family And Pitch
			pFontName                 // pointer to typeface name string
		);
		if(hFont){
			HGDIOBJ oldFont = SelectObject(hDC, hFont);
			TEXTMETRIC textMetric;
			GetTextMetrics(hDC, &textMetric);
			height = textMetric.tmHeight;
			m_height = height;//-1;//--------seems that a bug are here
			int widths[256];
			GetCharWidth(hDC, 0, 255, widths);// read the widths from the truetype font
			int textArea = 0;
			m_width = 0;
			int chr = 0;
			for(i = 0;i < 256;i++){
				//if(i>31 && widths[i]>0 && (i<126 || i>191)){//isprint(i) &&
				if(m_width < widths[i]){ m_width = widths[i];chr = i; };
				if(widths[i] > 0){// && t_printable(i)){
					textArea += height * (widths[i] + 1);
				} else{
					widths[i] = 0;
				};
			};
			int bitmapSize = 32;
			int bitmapArea = 32 * 32;
			int BMXSize = 32, BMYSize = 32;
			while(bitmapArea < textArea){
				BMXSize *= 2;
				bitmapArea = BMXSize * BMYSize;
				if(bitmapArea < textArea){
					BMYSize *= 2;
					bitmapArea = BMXSize * BMYSize;
				};
			};
			CON.write(0, "Font::create texture (%ix%i)%s", BMXSize, BMYSize, MKB(BMXSize*BMYSize * 2));
			BITMAPINFO bmi;// create a DIB
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = BMXSize;
			bmi.bmiHeader.biHeight = BMYSize;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			char *pBits = NULL;
			char *IMG = NULL;//=new char[BMXSize*BMYSize*2];
			RESIZE(IMG, char, (BMXSize*BMYSize * 2));
			HBITMAP hDIB = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**) &pBits, 0, 0);
			HGDIOBJ oldDIB = SelectObject(hDC, hDIB);
			// Render the text to the DIB
			SetTextColor(hDC, (COLORREF) 0x00FFFFFF);	// white text 0x00bbggrr
			SetBkColor(hDC, (COLORREF) 0x00000000);	// black background
			// fill background with black
			PatBlt(hDC, 0, 0, BMXSize, BMYSize, BLACKNESS);
			int x = 0, y = 0;
			float oobitmapSize = 1.0f / (float) BMXSize;
			float oobitmapSizey = 1.0f / (float) BMYSize;
			char tmp[4] = {0,0,0,0};
			tmp[1] = 0;
			// render each printable character to the DIB (wrap letters)
			for(i = 0;i < 256;i++){
				fontchar_s *pLetter = m_chars + i;
				if(widths[i]){
					tmp[0] = (char) i;
					if((x + widths[i]) >= BMXSize){
						x = 0;
						y += height;
					};
					TextOut(hDC, x, y, tmp, 1);
					// generate texture coordinates for this letter
					pLetter->u = x * oobitmapSize;
					pLetter->u2 = (x + widths[i])*oobitmapSize;
					// flip v's, the DIB is upside down
					pLetter->v = 1.0f - (y*oobitmapSizey);
					pLetter->v2 = 1.0f - ((y + height)*oobitmapSizey);//-------bug was here h-1
					x += widths[i] + 1;
					if((x + widths[i]) >= BMXSize){
						x = 0;
						y += height;
					};
					pLetter->width = widths[i];
				} else{
					pLetter->width = 0;
				};
			};
			// add the alpha channel
			unsigned char *pOut = (unsigned char*) pBits;
			unsigned char *pOut2 = (unsigned char*) IMG;
			for(i = 0;i < BMXSize*BMYSize;i++){
				if(pOut[0] || pOut[1] || pOut[2]){
					pOut[3] = 255U;// filled pixel
				} else{
					pOut[3] = 0;// empty pixel
				};
				pOut2[0] = pOut[0];
				pOut2[1] = pOut[3];
				pOut2 += 2;
				pOut += 4;
			};
			// upload the font texture to GL
			glGenTextures(1, &m_textureId);glBindTexture(GL_TEXTURE_2D, m_textureId);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MMF[0][0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MMF[0][1]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexImage2D(GL_TEXTURE_2D, 0, 2, BMXSize, BMYSize, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, IMG);
			TW = BMXSize;
			TH = BMYSize;
			//delete [] IMG;
			DEL_(IMG);
			// clean up the DC
			SelectObject(hDC, oldDIB);
			SelectObject(hDC, oldFont);
			DeleteObject(hFont);
			DeleteObject(hDIB);
			m_init = true;
		};
		DeleteDC(hDC);
		glBindTexture(GL_TEXTURE_2D, 0);
		MM.RegisterTex(pFontName, m_textureId);
	};
	//--------------------------------------------------------------------------------------
	void set_tex(){
		glBindTexture(GL_TEXTURE_2D, m_textureId);
	};
	//--------------------------------------------------------------------------------------
	void TextOrthoMode(){
		glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0, WndW, WndH, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);glLoadIdentity();
		glColor3f(1, 1, 1);
		//glPushAttrib(GL_ALL_ATTRIB_BITS);//!
		glEnable(GL_BLEND);glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glDisable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_TEXTURE_2D);
	};
	//--------------------------------------------------------------------------------------
	void EndTextOrthoMode(){
		//glPopAttrib();//!
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);/**/
	};
	//--------------------------------------------------------------------------------------
	float width(const char *fmt, ...){//---change>?
		if(!m_init)return 0;
		va_list ap;
		va_start(ap, fmt);vsprintf(m_text, fmt, ap);va_end(ap);
		fontchar_s *pLetter;
		unsigned char letter;
		const char *Ttext = m_text;
		float x = 0;
		do{
			letter = *Ttext++;
			if(letter == '\n')break;//todo
			pLetter = m_chars + letter;
			if(letter && pLetter->width){ x += pLetter->width; };
		} while(letter);
		return x;
	};
	//--------------------------------------------------------------------------------------
	float width(int pos, const char *Ttext){//*fmt, ...){//used in UI (mousepos at text) //speed up
		if(!m_init)return 0;
		fontchar_s *pLetter;
		unsigned char letter;
		float x = 0;
		int cnt = 0;
		do{
			if(cnt >= pos)break;
			cnt++;
			letter = *Ttext++;
			pLetter = m_chars + letter;
			if(letter && pLetter->width){ x += pLetter->width; };
		} while(letter);
		return x;
	};
	//--------------------------------------------------------------------------------------
	void Print(float x, float y, bool center, const char *fmt, ...){
		if(!m_init)return;
		va_list ap;va_start(ap, fmt);vsprintf(m_text, fmt, ap);va_end(ap);//crush 27.10.2014 test ring buffer
		fontchar_s *pLetter;
		unsigned char letter;
		const char *pText = m_text;
		if(center){
			x -= width(m_text)*0.5f;
			y -= m_height * 0.5f;
		};
		float startx = x;//  CON.write(1,"%s",m_text);
		do{
			letter = *pText++;
			if(letter == '\n'){ x = startx;y += m_height;posy += m_height;continue; };
			pLetter = m_chars + letter;
			if(letter && pLetter->width){                                    //change int->float ??
				glBegin(GL_QUADS);//change to triangle strip?
				glTexCoord2f(pLetter->u, pLetter->v);glVertex2i((int) x, (int) y);  //crush 04.27.2014 on load found ships  +1 at run 05.07.2014 +1 27.08.2014 at load
				glTexCoord2f(pLetter->u, pLetter->v2);glVertex2i((int) x, (int) y + m_height);
				glTexCoord2f(pLetter->u2, pLetter->v2);glVertex2i((int) x + pLetter->width, (int) y + m_height);
				glTexCoord2f(pLetter->u2, pLetter->v);glVertex2i((int) x + pLetter->width, (int) y);
				glEnd();//crush on load 17:49,12.05.2014   +++ 11.09.2014
				x += pLetter->width;
			};
		} while(letter);
	};
	//--------------------------------------------------------------------------------------
	void Print(float x, float y, bool center, int height, const char *fmt, ...){
		if(!m_init)return;
		va_list ap;va_start(ap, fmt);vsprintf(m_text, fmt, ap);va_end(ap);
		fontchar_s *pLetter;
		unsigned char letter;
		const char *pText = m_text;
		if(center){
			x -= width(m_text)*0.5f;
			y -= m_height * 0.5f;
		};
		float startx = x;//  CON.write(1,"%s",m_text);
		do{
			letter = *pText++;
			if(letter == '\n'){ x = startx;y += height;posy += height;continue; };
			pLetter = m_chars + letter;
			if(letter && pLetter->width){                                    //change int->float ??
				glBegin(GL_QUADS);//change to triangle strip?
				glTexCoord2f(pLetter->u, pLetter->v);glVertex2i((int) x, (int) y);
				glTexCoord2f(pLetter->u, pLetter->v2);glVertex2i((int) x, (int) y + m_height);
				glTexCoord2f(pLetter->u2, pLetter->v2);glVertex2i((int) x + pLetter->width, (int) y + m_height);
				glTexCoord2f(pLetter->u2, pLetter->v);glVertex2i((int) x + pLetter->width, (int) y);
				glEnd();
				x += pLetter->width;
			};
		} while(letter);
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================