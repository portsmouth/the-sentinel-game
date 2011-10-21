

#include "TextureLibrary.h"
#include <SDL/SDL.h>
#include <SDL_image/SDL_image.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

//#define TEXTURE_DEBUG

/**********************************************************/ 
/*                    Texture class                       */
/**********************************************************/ 
  
Texture::Texture() :
TextureName(0)
{};

Texture::~Texture()
{};


void Texture::Load(std::string inFilename)
{
	
#ifdef TEXTURE_DEBUG
	printf("loading texture: %s\n",inFilename.c_str());
#endif
	::glEnable(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	SDL_Surface *image = ::IMG_Load(inFilename.c_str());
	if(image == NULL) {
		printf("IMG_Load error loading image %s - %s\n", inFilename.c_str(), ::SDL_GetError());
		exit(1);
	}
	else
	{
		mWidth = image->w;
		mHeight = image->h;
		mDepth = image->format->BitsPerPixel;
#ifdef TEXTURE_DEBUG
		printf("image dims, height, width: %d %d\n",mHeight,mWidth);
#endif	
		// PROBLEM: I'm relying on an extension to get the textures loaded in native format... 
		//probably ought to do it the stupid-but-compatible way instead.
		
		glGenTextures(1, &TextureName);

		glBindTexture(GL_TEXTURE_2D, TextureName);
		
		switch (mipmapmode) {
			
			case NO_MIPMAPS:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
				
			case BUILD_MIPMAPS:
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);				
				break;
				
		}
														
		GLenum internal_format = GL_RGBA8;		
		// could be GL_RGB5_A1, if you don't need 8 bit alpha and want to save VRAM
		GLenum image_format, image_type;
		switch(mDepth)
		{
			case 32:
				image_format = GL_BGRA_EXT;
				image_type = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
				
			case 24:
				image_format = GL_RGB;
				image_type = GL_UNSIGNED_BYTE;
				break;
				
			case 16:
				image_format = GL_RGB5_A1;
				image_type = GL_UNSIGNED_SHORT_5_5_5_1;
				break;
				
			default:
				image_format = GL_LUMINANCE;
				image_type = GL_UNSIGNED_BYTE;
				break;
		}
		
		switch (mipmapmode) 
		{
			
			case NO_MIPMAPS:
				glTexImage2D(GL_TEXTURE_2D, 
							 0, 
							 internal_format, 
							 mWidth, mHeight, 
							 0, 
							 image_format, 
							 image_type, 
							 image->pixels);
				break;
				
			case BUILD_MIPMAPS:
				gluBuild2DMipmaps(GL_TEXTURE_2D,
								  internal_format,
								  image->w, image->h,
								  image_format,
								  image_type,
								  image->pixels);
				break;
				
		}
	
		GLenum gl_error = ::glGetError();
		if(gl_error != GL_NO_ERROR)
			fprintf(stderr, "OpenGL error: %d\n", (int)gl_error);
		
		::SDL_FreeSurface(image);
	}
	
}



void Texture::Apply()
{
	switch (mode)
	{
		case TEXTURE_MODULATE: glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); break;
		case TEXTURE_REPLACE:  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  break;
		case TEXTURE_ADD:      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD    );  break;
		case TEXTURE_COMBINE:  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);  break;
		case TEXTURE_BLEND:    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND  );  break;
		case TEXTURE_DECAL:    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL  );  break;
	}
	
	if(TextureName != 0)
	{
		glBindTexture(GL_TEXTURE_2D, TextureName);
	}
}


GLuint Texture::GetTextureName() const
{
	return TextureName;	
}



/**********************************************************/ 
/*                Texture library class                   */
/**********************************************************/ 

TextureLibrary* TextureLibrary::_instance = NULL;

TextureLibrary* TextureLibrary::Instance()
{
	if (_instance == NULL)
	{
		_instance = new TextureLibrary;
	}
	return _instance;
}

TextureLibrary::TextureLibrary()
{};

	
/* deletes the OpenGL texture object and then the Texture object */
void TextureLibrary::FreeTexture(std::string inName)
{
	Texture *texture = NULL;
	TextureContainer::const_iterator iter = Textures.find(inName);
	if(iter != Textures.end()) 
	{
		texture = iter->second;
		GLuint texname = texture->GetTextureName();
		glDeleteTextures(1,&texname);
		delete texture;
		Textures.erase(inName);
	}
}

/* return pointer to a texture, loading the texture if necessary */
Texture* TextureLibrary::GetTexture(std::string inName, mipmap_modes mmm, texture_apply_modes mode) const
{
	Texture *texture = NULL;
	TextureContainer::const_iterator iter = Textures.find(inName);
	
	if(iter != Textures.end())
		//texture already loaded, just return a pointer to it
		texture = iter->second;
	else
	{				
		//load texture in file inName (from texture folder)
		texture = new Texture;
		texture->mipmapmode = (mipmap_modes)mmm;
		texture->mode = (texture_apply_modes)mode;
		texture->Load(inName);
		Textures[inName] = texture;
	}
	
	return texture;
}

