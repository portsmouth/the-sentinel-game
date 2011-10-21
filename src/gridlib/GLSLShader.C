#ifdef USE_RCSID
static const char RCSid_GLSLShader[] = "$Id: GLSLShader.C,v 1.4 2006/01/16 12:23:21 DOMAIN-I15+prkipfer Exp $";
#endif

/*----------------------------------------------------------------------
|
|
| $Log: GLSLShader.C,v $
| Revision 1.4  2006/01/16 12:23:21  DOMAIN-I15+prkipfer
| convenience uniform methods
|
| Revision 1.3  2005/10/21 09:46:26  DOMAIN-I15+prkipfer
| updated to OpenGL 2.0
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

#include "GLSLShader.hh"

#include "SDL/SDL.h"

#ifdef OUTLINE
#include "GLSLShader.in"
#endif

#include <fstream>
#include <vector>



GLSLShader::GLSLShader() 
    : program_(0)
    , vs_(0)
    , fs_(0)
    , vcompiled_(0)
    , fcompiled_(0)
    , linked_(0) 
{
}

GLSLShader::~GLSLShader()
{
    destroy();
}

GbBool 
GLSLShader::loadFromFile(const char* vsfile, const char* fsfile)
{
    std::ifstream vfin(vsfile);
    std::ifstream ffin(fsfile);

    if(!vfin) {
        errormsg("Unable to open vertex shader "<<vsfile);
        return false;
    }

    if(!ffin) {
        errormsg("Unable to open fragment shader "<<fsfile);
        return false;
    }

    GbString v_shader_str;
    GbString f_shader_str;
    GbString line;

    // Probably not efficient, but who cares...
    while(std::getline(vfin, line)) {
        v_shader_str += line + "\n";
    }

    // Probably not efficient, but who cares...
    while(std::getline(ffin, line)) {
        f_shader_str += line + "\n";
    }

    GbBool retVal = loadFromString(v_shader_str.c_str(), f_shader_str.c_str());

    if (retVal) {
        debugmsg("Loaded program ["<<vsfile<<","<<fsfile<<"]");
    }

    return retVal;
}

GbBool 
GLSLShader::loadFromString(const char* vshader, const char* fshader) 
{
	checkGLError("OpenGL error before loading shader");

    if (program_) destroy();

    assert(!program_);
	
    vs_ = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	checkGLError("cannot create vertex shader object");
	
    fs_ = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    checkGLError("cannot create fragment shader object");

    glShaderSourceARB(vs_,1,&vshader,NULL);
    checkGLError("sourcing vertex shader");
    glShaderSourceARB(fs_,1,&fshader,NULL);
    checkGLError("sourcing fragment shader");

    glCompileShaderARB(vs_);
    checkGLError("compiling vertex shader");
    glGetObjectParameterivARB(vs_,GL_OBJECT_COMPILE_STATUS_ARB,&vcompiled_);
    printInfoLog(vs_);

    if (!vcompiled_) {
	errormsg("error compiling vertex shader");
	return false;
    }

    glCompileShaderARB(fs_);
    checkGLError("compiling fragment shader");
    glGetObjectParameterivARB(fs_,GL_OBJECT_COMPILE_STATUS_ARB,&fcompiled_);
    printInfoLog(fs_);

    if (!fcompiled_) {
	errormsg("error compiling fragment shader");
	return false;
    }

    program_ = glCreateProgramObjectARB();
    checkGLError("creating program object");

    glAttachObjectARB(program_,vs_);
    checkGLError("attaching vertex shader to program");
    glAttachObjectARB(program_,fs_);
    checkGLError("attaching fragment shader to program");

    glLinkProgramARB(program_);
    checkGLError("linking program");
    glGetObjectParameterivARB(program_,GL_OBJECT_LINK_STATUS_ARB,&linked_);
    printInfoLog(program_);

    if (!linked_) {
	errormsg("cannot link shaders to program object");
	return false;
    }

    return true;
}

void
GLSLShader::printInfoLog(GLhandleARB obj)
{
    GLint infologLength = 0;
    GLint charsWritten = 0;

    checkGLError("before reading info log");

    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
    checkGLError("reading info log length");

    if (infologLength > 1) {
	std::vector<GLcharARB> infoLog;
	infoLog.resize(infologLength);
	glGetInfoLogARB(obj, infologLength, &charsWritten, &(infoLog[0]));
	infomsg("info log("<<infologLength<<"): "<<&(infoLog[0]));
    }

    checkGLError("after reading info log");
}

GLint
GLSLShader::getUniformLocation(GLcharARB *name) const
{
    GLint loc = glGetUniformLocationARB(program_, name);
    if (loc == -1) errormsg("No such uniform name \""<<name<<"\"");
#ifdef DEBUG
    char s[512];
    sprintf(s,"get uniform location \"%s\"",name);
    checkGLError(s);
#endif
    return loc;
}

void 
GLSLShader::uniform(GLcharARB *name, int a) const
{
    glUniform1iARB(getUniformLocation(name),a);
}

void 
GLSLShader::uniform(GLcharARB *name, float a) const
{
    glUniform1fARB(getUniformLocation(name),a);
}

void 
GLSLShader::uniform(GLcharARB *name, float a, float b) const
{
    glUniform2fARB(getUniformLocation(name),a,b);
}

void 
GLSLShader::uniform(GLcharARB *name, float a, float b, float c) const
{
    glUniform3fARB(getUniformLocation(name),a,b,c);
}

void 
GLSLShader::uniform(GLcharARB *name, float a, float b, float c, float d) const
{
    glUniform4fARB(getUniformLocation(name),a,b,c,d);
}

