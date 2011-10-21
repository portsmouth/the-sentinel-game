/*----------------------------------------------------------------------
|
| $Id: GLTexture.hh,v 1.4 2006/01/16 12:22:57 DOMAIN-I15+prkipfer Exp $
|
+---------------------------------------------------------------------*/

#ifndef _GLTEXTURE_HH_
#define _GLTEXTURE_HH_

#include "GbDefines.hh"

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/

#include "GbTypes.hh"
#include "GLObject.hh"
#include <map>

/*----------------------------------------------------------------------
|       declaration
+---------------------------------------------------------------------*/

class GLTexture
    : public GLObject
{
public:
    GLTexture();
    GLTexture(GLenum tgt);
    virtual ~GLTexture();

    // inherited
    INLINE virtual GbBool bind() const;
    INLINE virtual GbBool release() const;
    INLINE virtual void enable() const;
    INLINE virtual void disable() const;
    INLINE virtual GbBool create();
    INLINE virtual GbBool destroy();

    // new methods
    INLINE GLenum target() const;
    INLINE GLuint texture() const;
    INLINE GLenum unit() const;
    INLINE void parameter(GLenum pname, GLint i) const;
    INLINE void parameter(GLenum pname, GLfloat f) const;
    INLINE void parameter(GLenum pname, GLint * ip) const;
    INLINE void parameter(GLenum pname, GLfloat * fp) const;
    INLINE void priority(const GLclampf *prio) const;
    INLINE virtual GbBool bind(GLenum unit) const;
    INLINE virtual GbBool release(GLenum unit) const;
    INLINE virtual GbBool create(GLenum unit);

    INLINE GbBool registerParameter(GLenum pname, GLint i);
    INLINE GbBool registerParameter(GLenum pname, GLfloat f);
    INLINE GbBool registerParameter(GLenum pname, GLint* ip);
    INLINE GbBool registerParameter(GLenum pname, GLfloat* fp);
    INLINE void clearParameter();

    virtual GbBool loadBMP(const GbString& name);
    virtual GbBool writeBMP(const GbString& name);
    virtual GbBool makeReflectionMap( int w, int h, float ka=0.1f, float kd=0.3f, float kr=0.6f, float shininess=0.5f );

    INLINE virtual int getWidth() const;
    INLINE virtual int getHeight() const;
    INLINE virtual void setWidthHeight(int w, int h=1);
    INLINE virtual int getNumComponents() const;
    INLINE virtual unsigned char* getData() const;
    INLINE virtual unsigned char getR(int x, int y) const;
    INLINE virtual unsigned char getG(int x, int y) const;
    INLINE virtual unsigned char getB(int x, int y) const;
    INLINE virtual unsigned char getA(int x, int y) const;

protected:
    virtual GbBool create(GLenum unit, GLenum target);
    INLINE void setParameter() const;

    GLenum unit_;
    GLenum target_;
    GLuint texture_;

    int width_,height_,components_;
    unsigned char* data_;

    typedef std::map<GLenum,GLint>    Paramsi;
    typedef std::map<GLenum,GLfloat>  Paramsf;
    typedef std::map<GLenum,GLint*>   Paramsip;
    typedef std::map<GLenum,GLfloat*> Paramsfp;
    Paramsi  paramsI_;
    Paramsf  paramsF_;
    Paramsip paramsIP_;
    Paramsfp paramsFP_;

private:
    // no copying
    GLTexture& operator= (const GLTexture& rhs);
    GLTexture(const GLTexture& tex);
};

class GLTexture1D 
    : public GLTexture
{ 
public: 
    GLTexture1D();
    virtual ~GLTexture1D() {}
    virtual GbBool create(GLenum unit) 
	{ return GLTexture::create(unit, GL_TEXTURE_1D); }
};
	
class GLTexture2D 
    : public GLTexture
{ 
public: 
    GLTexture2D();
    virtual ~GLTexture2D() { }
    virtual GbBool create(GLenum unit) 
	{ return GLTexture::create(unit, GL_TEXTURE_2D); }
};

class GLTexture3D 
    : public GLTexture
{ 
public: 
    GLTexture3D();
    virtual ~GLTexture3D() { }
    INLINE virtual int getDepth() { return depth_; }
    INLINE virtual void setDepth(int d) { depth_=d; }
    virtual GbBool create(GLenum unit) 
	{ return GLTexture::create(unit, GL_TEXTURE_3D); }
protected:
    int depth_;
};

class GLTextureCubeMap 
    : public GLTexture
{ 
public: 
    GLTextureCubeMap();
    virtual ~GLTextureCubeMap() { }
    virtual GbBool create(GLenum unit);
};

class GLTextureRectangle 
    : public GLTexture
{ 
public: 
    GLTextureRectangle();
    virtual ~GLTextureRectangle() { }
    virtual GbBool create(GLenum unit);
};

#ifndef OUTLINE
#include "GLTexture.in"
#endif

#endif  // GLTEXTURE_HH
/*----------------------------------------------------------------------
|
| $Log: GLTexture.hh,v $
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
+---------------------------------------------------------------------*/
