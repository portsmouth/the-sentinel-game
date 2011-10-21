/*----------------------------------------------------------------------
|
| $Id: GLSLShader.hh,v 1.3 2006/01/16 12:23:21 DOMAIN-I15+prkipfer Exp $
|
+---------------------------------------------------------------------*/

#ifndef GLSL_SHADER_INCLUDED
#define GLSL_SHADER_INCLUDED

#include "GbDefines.hh"

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/

#include "GLObject.hh"
#include "GbTypes.hh"


/*----------------------------------------------------------------------
|       declaration
+---------------------------------------------------------------------*/

class GLSLShader 
    : public GLObject
{
public:
    GLSLShader();
    virtual ~GLSLShader();

    // inherited
    INLINE virtual GbBool bind() const;
    INLINE virtual GbBool release() const;
    INLINE virtual void enable() const;
    INLINE virtual void disable() const;
    INLINE virtual GbBool create();
    INLINE virtual GbBool destroy();

    // new methods
    GbBool loadFromFile(const char* vsfile, const char* fsfile);
    GbBool loadFromString(const char* vshader, const char* fshader);
    GLint getUniformLocation(GLcharARB *name) const;
    void uniform(GLcharARB *name, int a) const;
    void uniform(GLcharARB *name, float a) const;
    void uniform(GLcharARB *name, float a, float b) const;
    void uniform(GLcharARB *name, float a, float b, float c) const;
    void uniform(GLcharARB *name, float a, float b, float c, float d) const;

protected:
    void printInfoLog(GLhandleARB obj);

    GLhandleARB program_;
    GLhandleARB vs_,fs_;
    GLint vcompiled_,fcompiled_,linked_;

private:
    // no copying
    GLSLShader& operator= (const GLSLShader& rhs);
    GLSLShader(const GLSLShader& shader);
};

#ifndef OUTLINE
#include "GLSLShader.in"
#endif
		
#endif // GLSL_SHADER_INCLUDED
/*----------------------------------------------------------------------
|
| $Log: GLSLShader.hh,v $
| Revision 1.3  2006/01/16 12:23:21  DOMAIN-I15+prkipfer
| convenience uniform methods
|
| Revision 1.2  2005/10/21 09:46:26  DOMAIN-I15+prkipfer
| updated to OpenGL 2.0
|
| Revision 1.1  2004/11/08 11:05:46  DOMAIN-I15+prkipfer
| introduced GLSL shader and special texture handling classes
|
|
+---------------------------------------------------------------------*/
