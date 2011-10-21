

#ifndef TEXTURELIBRARY_H
#define TEXTURELIBRARY_H

#include <string>
#include <map>
#include <OpenGL/gl.h>

/* Texture class */

enum mipmap_modes 
{
	NO_MIPMAPS, 
	BUILD_MIPMAPS
};

enum texture_apply_modes
{
	TEXTURE_MODULATE,
	TEXTURE_REPLACE,
	TEXTURE_ADD,
	TEXTURE_COMBINE,
	TEXTURE_BLEND,
	TEXTURE_DECAL
};

class Texture 
{
		
	/* Note that the texture coords of the texture on the screen are oriented as follows:
	
	  (0,0) +--------+ (1,0)
	        |        |
	        |        | 
	        |        |
	  (0,1) +--------+ (1,1)
		
	 */
	
public:
	Texture();
	~Texture();
	
	void Load(std::string inFilename);
	void Apply();	
	GLuint GetTextureName() const;
	enum mipmap_modes mipmapmode;
	enum texture_apply_modes mode;
	
private:
	GLuint TextureName;
	unsigned int mWidth;
	unsigned int mHeight;	
	unsigned int mDepth;
	
};

/* Texture library class */

class TextureLibrary {

protected:
	
	// SINGLETON
	TextureLibrary();
	
private:
	
	static TextureLibrary* _instance;
	
public:	
	
	static TextureLibrary* Instance();
	
	~TextureLibrary(); //destructor
	
	Texture* GetTexture(std::string inName, 
						mipmap_modes mmm = BUILD_MIPMAPS, 
						texture_apply_modes mode = TEXTURE_MODULATE) const;
	
	void FreeTexture(std::string inName);
		
private:

protected:
	typedef std::map<std::string, Texture*> TextureContainer;
	mutable TextureContainer Textures;
	
};

#endif //TEXTURELIBRARY_H
