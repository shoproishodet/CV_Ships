//======================================================================================
#include "jpeg.h"
#include "tga.h"
//======================================================================================
bool MipMap(byte *in, word &width, word &height, byte p) {   //move to texture manager glGenerateMipmapEXT(GL_TEXTURE_2D);
	if (width <= 1 && height <= 1)return 0;
	int i, j, row = width * p;
	byte *out = in;
	if (width > 1)width >>= 1;
	if (height > 1)height >>= 1;
	for (i = 0; i < height; i++, in += row) {
		for (j = 0; j < width; j++, out += p, in += p * 2) {
			if (p > 0)out[0] = (in[0] + in[p] + in[row] + in[row + p]) >> 2;
			if (p > 1)out[1] = (in[1] + in[p + 1] + in[row + 1] + in[row + p + 1]) >> 2;
			if (p > 2)out[2] = (in[2] + in[p + 2] + in[row + 2] + in[row + p + 2]) >> 2;
			if (p > 3)out[3] = (in[3] + in[p + 3] + in[row + 3] + in[row + p + 3]) >> 2;
		};
	};
	return 1;
};
//======================================================================================
//======================================================================================
struct Texture {
	Dword id;
	word w, h;
	int param, mode;
	bool alpha, turb;
	cstring name;
	byte *T, p;
	vec3f L;
	float zoom, amp, seed, lac, gain, oct;
	//Noise N;
	//--------------------------------------------------------------------------------------
	void Init() { id = 0; w = h = 0; name.init(); param = mode = 0; alpha = turb = 0; T = NULL; /*N.Init();*/ oct = 1; };
	void del() { name.del(); DEL_(T);/* N.Del();*/ };//
	void delGPU() { if (id)glDeleteTextures(1, &id); id = 0; };
	//--------------------------------------------------------------------------------------
	void Set(word ww, word hh, byte pp, float Z, float A, float S, int O = 1, bool TU = false) {
		w = ww; h = hh; p = pp; zoom = Z; amp = A; seed = S; DEL_(T); T = new byte[w*h*p]; lac = 2; gain = 1 / lac; oct = O; turb = TU;
	};
	//--------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------
	void SetColor() {
		byte *TT = new byte[w*h * 3];
		Dword cnt = 0, cnt2 = 0;
		for (word i = 0; i < w; i++) {
			for (word j = 0; j < h; j++) {
				TT[cnt++] = (float)T[cnt2] * 0.2;
				TT[cnt++] = (float)T[cnt2] * 0.7;
				TT[cnt++] = (float)T[cnt2] * 0.1;
				cnt2++;
			};
		};
		p = 3;
		DEL_(T);
		T = TT; TT = NULL;
	};
	//--------------------------------------------------------------------------------------
	void Copy(Texture *P, word x, word y) {
		Dword cnt = 0;
		for (word i = x; i < x + P->w; i++) {
			for (word j = y; j < y + P->h; j++) {
				T[i*w*p + j * p] = P->T[cnt]; if (P->p > 1)cnt++;
				T[i*w*p + j * p + 1] = P->T[cnt]; if (P->p > 1)cnt++;
				T[i*w*p + j * p + 2] = P->T[cnt++];
			};
		};
	};
	//--------------------------------------------------------------------------------------
	Dword UnloadToGPU(int tmode) {
		if (!T) { CON.write(1, "Texture::ToGPU:: no texture"); return 0; }; mode = tmode; param = GL_REPEAT;
		delGPU();
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MMF[mode][0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MMF[mode][1]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		word mw = w, mh = h;
		GLenum FMT = GL_RGBA;
		byte *TT = new byte[w*h*p]; memcpy(TT, T, w*h*p);
		switch (p) {
		case 1:FMT = GL_LUMINANCE; break;
		case 3:FMT = GL_RGB; break;
		};
		if (mode > 3) {
			int miplvl = 0;
			do {
				glTexImage2D(GL_TEXTURE_2D, miplvl, p, mw, mh, 0, FMT, GL_UNSIGNED_BYTE, TT);
				miplvl++;
			} while (MipMap(TT, mw, mh, p));//lose texture
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, p, mw, mh, 0, FMT, GL_UNSIGNED_BYTE, TT);
		};
		delete[] TT;
		Dword tid = id;
		//del();?????
		//CON.write(0,"TEX::toGPU::got %i",tid);
		return tid;
	};
	//--------------------------------------------------------------------------------------
	Dword CheckBoard(const char *nm) {
		w = 8; h = 8; name.print(nm);
		unsigned char *data = new unsigned char[8 * 8 * 3];
		int cnt = 0;
		byte color;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if ((i % 2 && j % 2) || (!(i % 2) && !(j % 2)))color = 255; else color = 0;
				data[cnt] = color; cnt++;
				data[cnt] = color; cnt++;
				data[cnt] = color; cnt++;
			};
		};
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MMF[0][0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MMF[0][1]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		delete[] data;
		return id;
	};
	//--------------------------------------------------------------------------------------
	Dword loadJPG(const char *nm, int tparam, int tmode, bool talpha) { //CON.write(0,"loadJPG %s",nm);//init!!!
		name.print(nm); param = tparam; mode = tmode; alpha = talpha;
		long size = -1;
		Cjpeg jpg;
		if (jpg.load_jpeg(nm)) {
			w = jpg.width; h = jpg.height;//CON.write(0,"load_jpeg %s",nm);
			glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MMF[mode][0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MMF[mode][1]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			alpha = true;
			word mw = w, mh = h;
			if (mode > 3) {
				int miplvl = 0;
				do {
					glTexImage2D(GL_TEXTURE_2D, miplvl, 4, mw, mh, 0, GL_RGBA, GL_UNSIGNED_BYTE, jpg.data);
					miplvl++;
				} while (MipMap(jpg.data, mw, mh, 4));/**/
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, 4, mw, mh, 0, GL_RGBA, GL_UNSIGNED_BYTE, jpg.data);
			};
			if (jpg.data) { delete[] jpg.data; };
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else { CON.write(LOG_ERROR, "Jpg::file not found [%s]", name.text); return size; };
		return id;
	};
	//--------------------------------------------------------------------------------------
	Dword loadTGA(const char *nm, int tparam, int tmode, bool talpha) {
		name.print(nm); param = tparam; mode = tmode; alpha = talpha;
		long size = -1;
		Ctga tga;
		if (tga.Load(nm)) {
			w = tga.width; h = tga.height; //warning 32 bit - mean alpha
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MMF[mode][0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MMF[mode][1]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			alpha = true;
			word mw = w, mh = h;
			if (mode > 3) {
				int miplvl = 0;
				do {
					//CON.write(0,"%i %i %i",miplvl,mw,mh);
					glTexImage2D(GL_TEXTURE_2D, miplvl, 4, mw, mh, 0, GL_RGBA, GL_UNSIGNED_BYTE, tga.data);
					miplvl++;
				} while (MipMap(tga.data, mw, mh, 4));/**/
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, 4, mw, mh, 0, GL_RGBA, GL_UNSIGNED_BYTE, tga.data);
			};
			if (tga.data) { delete[] tga.data; };
		}
		else { CON.write(LOG_ERROR, "Tga::file not found [%s]", name.text); return size; };
		return id;
	};
	//--------------------------------------------------------------------------------------
};
//======================================================================================