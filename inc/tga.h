#define TGA_FILE_NOT_FOUND 13 /* file was not found */
#define TGA_BAD_IMAGE_TYPE 14 /* color mapped image or image is not uncompressed */
#define TGA_BAD_DIMENSION  15 /* dimension is not a power of 2 */
#define TGA_BAD_BITS       16 /* image bits is not 8, 24 or 32 */
#define TGA_BAD_DATA       17 /* image data could not be loaded */

struct Ctga{
public:
GLenum texFormat;
int width;  // width image
int height; // height image
unsigned char *data;   // image

int checkSize (int x){
    if (x == 2	 || x == 4 || 
        x == 8	 || x == 16 || 
        x == 32  || x == 64 ||
        x == 128 || x == 256 ||
        x == 512 || x == 1024 || x == 2048)
        return 1;
    else return 0;
}

unsigned char *getRGBA (FILE *s, int size){
    unsigned char *rgba;
    unsigned char temp;
    size_t bread;
    int i;

    rgba =(unsigned char *) malloc (size * 4);
    if (rgba == NULL)return 0;
    bread = fread (rgba, sizeof (unsigned char), size * 4, s); 
    /* TGA is stored in BGRA, make it RGBA */
    if (bread != size * 4){free(rgba);return 0;}
    for (i = 0; i < size * 4; i += 4 )    {
        temp = rgba[i];
        rgba[i] = rgba[i + 2]; //swap 
        rgba[i + 2] = temp;
    }
    texFormat = GL_RGBA;
    return rgba;
}

unsigned char *getRGB (FILE *s, int size){
    unsigned char *rgb;
    unsigned char temp;
    size_t bread;
    int i;

    rgb =(unsigned char *) malloc (size * 3);
    if (rgb == NULL)return 0;
    bread = fread (rgb, sizeof (unsigned char), size * 3, s);
    if (bread != size * 3)  {
        free (rgb);
        return 0;
    }
    /* TGA is stored in BGR, make it RGB */
    for (i = 0; i < size * 3; i += 3)   {
        temp = rgb[i];
        rgb[i] = rgb[i + 2];
        rgb[i + 2] = temp;
    }
    texFormat = GL_RGB;
    return rgb;
}

unsigned char *getGray (FILE *s, int size){
    unsigned char *grayData;
    size_t bread;
    grayData =(unsigned char *) malloc (size);
    if (grayData == NULL)   return 0;
    bread = fread (grayData, sizeof (unsigned char), size, s);
    if (bread != size)   {
        free (grayData);
        return 0;
    }
    texFormat = GL_ALPHA;
   //texFormat = GL_LUMINANCE;
    return grayData;
}

unsigned char *getData (FILE *s, int sz, int iBits){
    if (iBits == 32)return getRGBA (s, sz);
    else if (iBits == 24)return getRGB (s, sz);	
    else if (iBits == 8)return getGray (s, sz);
    return NULL;
}

int returnError (FILE *s, int error){
    fclose (s);
    return error;
}

bool Load(const char *name){
 unsigned char type[4];
 unsigned char info[7];
 int imageBits, size;
 FILE *s;
 if(NULL==(s=fopen(name,"r+bt"))){CON.write(LOG_ERROR,"TGA_FILE_NOT_FOUND '%s'",name);return 0;};// TGA_FILE_NOT_FOUND;
 fread(&type,sizeof(char),3,s); // read in colormap info and image type, byte 0 ignored
 fseek(s,12,SEEK_SET);			// seek past the header and useless info
 fread(&info,sizeof(char),6,s);
 if(type[1]!=0 || (type[2]!=2 && type[2]!=3)){CON.write(LOG_ERROR,"TGA_BAD_IMAGE_TYPE '%s'",name);return 0;};//Error (s, TGA_BAD_IMAGE_TYPE);
 width=info[0]+info[1]*256;
 height=info[2]+info[3]*256;
 imageBits=info[4];
 size=width*height;
 /* make sure dimension is a power of 2 */
 if(!checkSize(width) || !checkSize(height)){CON.write(LOG_ERROR,"TGA_BAD_DIMENSION '%s'",name);return 0;};//Error (s, TGA_BAD_DIMENSION);
 /* make sure we are loading a supported type */
 if(imageBits!=32 && imageBits!=24 && imageBits!=8){CON.write(LOG_ERROR,"TGA_BAD_BITS '%s'",name);return 0;};//Error (s, TGA_BAD_BITS);
 data=getData(s,size,imageBits);
 /* no image data */
 if(!data){CON.write(LOG_ERROR,"TGA_BAD_DATA '%s'",name);return 0;};//Error (s, TGA_BAD_DATA);
 fclose(s);
 return true;
};
};//end class tga