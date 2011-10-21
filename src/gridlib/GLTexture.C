#ifdef USE_RCSID
static const char RCSid_GLTexture[] = "$Id: GLTexture.C,v 1.4 2006/01/16 12:22:57 DOMAIN-I15+prkipfer Exp $";
#endif

/*----------------------------------------------------------------------
|
|
| $Log: GLTexture.C,v $
| Revision 1.4  2006/01/16 12:22:57  DOMAIN-I15+prkipfer
| added parameter maps
|
| Revision 1.2  2005/07/13 11:51:05  DOMAIN-I15+prkipfer
| updated little OpenGL helper classes
|
| Revision 1.1  2004/11/08 11:05:46  DOMAIN-I15+prkipfer
| introduced GLSL shader and special texture handling classes
|
|
|
+---------------------------------------------------------------------*/

#include "GLTexture.hh"

#ifdef OUTLINE
#include "GLTexture.in"
#endif

#include "GbMath.hh"

#include <sys/stat.h>
#include <fstream>

GLTexture::GLTexture()
    : unit_(0)
    , target_(0)
    , texture_(0) 
    , width_(0)
    , height_(0)
    , components_(0)
    , data_(NULL)
    , paramsI_()
    , paramsF_()
    , paramsIP_()
    , paramsFP_()
{ 
}

GLTexture::GLTexture(GLenum tgt) 
    : unit_(0)
    , target_(tgt)
    , texture_(0)
    , width_(0)
    , height_(0)
    , components_(0)
    , data_(NULL)
    , paramsI_()
    , paramsF_()
    , paramsIP_()
    , paramsFP_()
{
}
		
GLTexture::~GLTexture()
{ 
    destroy();
}

GbBool
GLTexture::create(GLenum unit, GLenum target)
{
    destroy();

    unit_ = unit;
    target_ = target;

    // set up some reasonable defaults
    glGenTextures(1, &texture_);
    GbBool err=checkGLError("gen tex id");
    glActiveTextureARB(unit_);
    err=err||checkGLError("active tex unit");
    glBindTexture(target_, texture_);
    err=err||checkGLError("bind tex");
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    err=err||checkGLError("modif pixel store alignment");
    glTexParameterf(target_, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(target_, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(target_, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameterf(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    err=err||checkGLError("set tex filter");

    valid_ = !err;

    return valid_;
}

GbBool
GLTexture::makeReflectionMap( int w, int h, float ka, float kd, float kr, float shininess )
{
    if (!valid_) return false;

    assert( 0.0f <= ka && ka <= 1.0f );
    assert( 0.0f <= kd && kd <= 1.0f );
    assert( 0.0f <= kr && kr <= 1.0f );
    float k = ka + kd + kr;
    assert( 0.0f <= k  &&  k <= 1.0f );

    components_ = 4;
    width_ = w;
    height_ = h;

    delete[] data_;
    data_ = (unsigned char*) new char[width_*height_*components_];

    for( int i = 0; i < height_; ++i ) {
	for( int j = 0; j < width_; ++j ) {

	    float x = ( float(j) + 1.0f ) / (float(width_) + 1.0f);
	    float y = ( float(i) + 1.0f ) / (float(height_) + 1.0f);

	    float LT = 2.0f * x - 1.0f;
	    float VT = 2.0f * y - 1.0f;

	    float intensity = 0.0f;
	    intensity += ka;
	    intensity += kd * GbMath<float>::Sqrt( 1.0f - LT*LT );
	    intensity += kr * GbMath<float>::Pow( GbMath<float>::Abs( LT*VT - 
								      GbMath<float>::Sqrt( 1.0f - LT*LT ) * 
								      GbMath<float>::Sqrt( 1.0f - VT*VT ) ), 
						  shininess * 255.0f );

	    if( intensity < 0.0f || 1.0f < intensity ) {
		warningmsg("intensity: " << intensity);
		if( intensity < 0.0f ) intensity = 0.0f;
		if( 1.0f < intensity ) intensity = 1.0f;
	    }
	    assert( 0.0f <= intensity && intensity <= 1.0f );

	    int c = int(intensity * 255.0f);
	    assert( 0 <= c && c <= 255 );

	    data_[(j+i*width_)*components_]   = (char) c;
	    data_[(j+i*width_)*components_+1] = (char) c;
	    data_[(j+i*width_)*components_+2] = (char) c;
	    data_[(j+i*width_)*components_+3] = 127;

	}
    }
    
    return true;
}

GLTexture1D::GLTexture1D() 
    : GLTexture(GL_TEXTURE_1D) 
{
} 

GLTexture2D::GLTexture2D() 
    : GLTexture(GL_TEXTURE_2D) 
{ 
}

GLTexture3D::GLTexture3D() 
    : GLTexture(GL_TEXTURE_3D) 
{
} 

GLTextureCubeMap::GLTextureCubeMap() 
    : GLTexture(GL_TEXTURE_2D)
{
    if (GLEW_ARB_texture_cube_map)
	target_ = GL_TEXTURE_CUBE_MAP_ARB;
     else if (GLEW_EXT_texture_cube_map)
 	target_ = GL_TEXTURE_CUBE_MAP_EXT;
    else
	errormsg("Cube map texture not supported");
}

GbBool
GLTextureCubeMap::create(GLenum unit)
{
    if (GLEW_ARB_texture_cube_map)
	return GLTexture::create(unit, GL_TEXTURE_CUBE_MAP_ARB);
     else if (GLEW_EXT_texture_cube_map)
 	return GLTexture::create(unit, GL_TEXTURE_CUBE_MAP_EXT);

    errormsg("Cube map texture not supported");
    return false;
}

GLTextureRectangle::GLTextureRectangle() 
    : GLTexture(GL_TEXTURE_2D)
{
    if (GLEW_EXT_texture_rectangle)
	target_ = GL_TEXTURE_RECTANGLE_EXT;
    else if (GL_NV_texture_rectangle)
	target_ = GL_TEXTURE_RECTANGLE_NV;
    else
	errormsg("Rectangular texture not supported");
}

GbBool
GLTextureRectangle::create(GLenum unit)
{
    if (GLEW_EXT_texture_rectangle)
	return GLTexture::create(unit, GL_TEXTURE_RECTANGLE_EXT);
    else if (GL_NV_texture_rectangle)
	return GLTexture::create(unit, GL_TEXTURE_RECTANGLE_NV);

    errormsg("Rectangular texture not supported");
    return false;
}


// simple BMP-loader. Refer to lesson06 for Linux at http://nehe.gamedev.net
GbBool
GLTexture::loadBMP(const GbString& name) 
{
    unsigned short bfType;
    long bfOffBits;
    short biPlanes;
    short biBitCount;
    long biSizeImage;

    if (name == GbString()) {
	errormsg("no filename given");
	return false;
    }
    
    debugmsg("trying to load bitmap from "<<name.c_str());

    // check existance
    struct stat s;
    if (stat(name.c_str(),&s)==0) {
	// load 
	std::ifstream file(name.c_str(),std::ios::binary | std::ios::in);

	// make sure file can be read
	file.read((char*)&bfType,sizeof(short));
	// check if file is a bitmap
	if (bfType != 19778) {
	    errormsg("Not a bitmap");
	    return false;
	}

	// get the file size
	// skip file size and reserved fields of bitmap file header
	file.seekg(8, std::ios::cur);
	// get the position of the actual bitmap data
	file.read((char*)&bfOffBits, sizeof(long));

	file.seekg(4, std::ios::cur);                     // skip size of bitmap info header    
	file.read((char*)&width_, sizeof(int));   // get the width of the bitmap    
	file.read((char*)&height_, sizeof(int));  // get the height of the bitmap  
	file.read((char*)&biPlanes, sizeof(short));       // get the number of planes
	if (biPlanes != 1) {
	    errormsg("Number of bitplanes was not 1"); 	
	    return false;
	}

	// get the number of bits per pixel
	file.read((char*)&biBitCount, sizeof(short));
	if (biBitCount != 24) {
	    errormsg("File not 24Bpp");
	    return false;
	}

	// calculate the size of the image in bytes
	biSizeImage = width_ * height_ * 3;
	components_ = 3;
	delete[] data_;
	data_ = (unsigned char*) new char[biSizeImage];
	// seek to the actual data
	file.seekg(bfOffBits, std::ios::beg);

	file.read((char*)data_, biSizeImage);

	// swap red and blue (bgr -> rgb)
	for (int i = 0; i < biSizeImage; i += 3) {
	    unsigned char temp = data_[i];
	    data_[i] = data_[i + 2];
	    data_[i + 2] = temp;
	}

	return true;
    }
    errormsg("bitmap "<<name<<" not found");
    return false;
}

GbBool
GLTexture::writeBMP(const GbString& name) 
{
#ifdef WIN32
#pragma pack(1)
#endif

    struct tagBITMAPINFOHEADER{
	unsigned long  biSize; 
	long   biWidth; 
	long   biHeight; 
	unsigned short   biPlanes; 
	unsigned short   biBitCount; 
	unsigned long  biCompression; 
	unsigned long  biSizeImage; 
	long   biXPelsPerMeter; 
	long   biYPelsPerMeter; 
	unsigned long  biClrUsed; 
	unsigned long  biClrImportant; 
    } 
#ifdef LINUX
    __attribute__ ((aligned(1))) 
#endif
	bmpheader;

    struct tagBITMAPFILEHEADER { 
	unsigned short    bfType; 
	unsigned long   bfSize; 
	unsigned short    bfReserved1; 
	unsigned short    bfReserved2; 
	unsigned long   bfOffBits; 
    }
#ifdef LINUX
    __attribute__ ((aligned(1))) 
#endif
	fileheader; 

    if (name == GbString()) {
	errormsg("no filename given");
	return false;
    }
    
    debugmsg("trying to write bitmap to "<<name.c_str());
	
    // check existance
    struct stat s;
    if (stat(name.c_str(),&s)==0) {
	warningmsg("overwriting texture file "<<name);
    }
    // save
    std::ofstream file(name.c_str(),std::ios::binary | std::ios::out);

    memset(&bmpheader,0,sizeof(struct tagBITMAPINFOHEADER));
    memset(&fileheader,0,sizeof(struct tagBITMAPFILEHEADER));

    // There is no RGBQUAD array for the 24-bit-per-pixel format. 

    bmpheader.biSize = sizeof(struct tagBITMAPINFOHEADER);
    bmpheader.biWidth = long(width_);   // get the width of the bitmap    
    bmpheader.biHeight = long(height_);  // get the height of the bitmap  
    bmpheader.biPlanes = 1;   // get the number of planes

    // get the number of bits per pixel
    bmpheader.biBitCount = 24;
    bmpheader.biCompression = 0UL; // BI_RGB

    // calculate the size of the image in bytes (and align)
    // For Windows NT, the width must be DWORD aligned unless 
    // the bitmap is RLE compressed.
    // For Windows 95/98/Me, the width must be WORD aligned unless the 
    // bitmap is RLE compressed.
    bmpheader.biSizeImage = ((bmpheader.biWidth * 24UL +31UL) & ~31UL) /8UL * bmpheader.biHeight; 
    char *outdata = new char[bmpheader.biSizeImage];

    assert(bmpheader.biSizeImage >= (unsigned int)(width_*height_*3));

    if (components_ == 4) {
	// swap red and blue (bgr -> rgb) and drop alpha
	int i=0;
	int j=0;
	while (i < (width_*height_*4)) {
	    outdata[j++] = data_[i+2];
	    outdata[j++] = data_[i+1];
	    outdata[j++] = data_[i];
	    i+=4;
	}
    }
    else {
	// swap red and blue (bgr -> rgb)
	for (int i = 0; i < width_ * height_ * 3; i += 3) {
	    outdata[i]   = data_[i+2];
	    outdata[i+1] = data_[i+1];
	    outdata[i+2] = data_[i];
	}
    }

    fileheader.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // size of entire file
    fileheader.bfSize = sizeof(struct tagBITMAPFILEHEADER) + 
	bmpheader.biSize + 
	// bmpheader.pbih->biClrUsed * sizeof(RGBQUAD) + // color index not used here
	bmpheader.biSizeImage; 
    fileheader.bfOffBits =	sizeof(struct tagBITMAPFILEHEADER) + 
	bmpheader.biSize;  
    // + bmpheader.biClrUsed * sizeof (RGBQUAD) // color index not used here

    file.write((char*)&fileheader,sizeof(struct tagBITMAPFILEHEADER));
    file.write((char*)&bmpheader,sizeof(struct tagBITMAPINFOHEADER));
    file.write((char*)outdata, bmpheader.biSizeImage);

    delete[] outdata;

    return true;
}

