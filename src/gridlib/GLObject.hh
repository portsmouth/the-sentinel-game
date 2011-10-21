/*----------------------------------------------------------------------
|
| $Id$
|
+---------------------------------------------------------------------*/

#ifndef GL_OBJECT_INCLUDED
#define GL_OBJECT_INCLUDED

//#include <glew.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#include "GbDefines.hh"

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/

#include "GbTypes.hh"

/*----------------------------------------------------------------------
|       declaration
+---------------------------------------------------------------------*/

class GLObject
{
public:
    GLObject() : valid_(false) {}
    virtual ~GLObject() { destroy(); }

    virtual GbBool create() = 0;
    virtual GbBool destroy() { return false; }

    // normal use (must be implemented)
    INLINE virtual GbBool bind() const = 0;
    INLINE virtual GbBool release() const = 0;

    // optimized use (can be implemented)
    INLINE virtual void enable() const {}
    INLINE virtual void disable() const {}

    // status
    INLINE virtual GbBool isValid() const { return valid_; }

protected:
    GbBool valid_;

    INLINE GbBool checkGLError(const char* str) const
	{
#ifdef DEBUG
	    GLenum error = glGetError();
	    if(error != GL_NO_ERROR) {
		errormsg(str<<" GL error: "<<error);
		return true;
	    }
#endif
	    return false;
	}

};

#endif // GL_OBJECT_INCLUDED
/*----------------------------------------------------------------------
|
| $Log$
|
+---------------------------------------------------------------------*/
