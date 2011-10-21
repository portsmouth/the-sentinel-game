

#include "DisplayHandler.h"
#include "DisplayObject.h"

#include "Model.h"
#include "GameObject.h"
#include "terrain.h"
#include "FileUtils.h"

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>


// Toggle object shader

//#define USE_SHADERS

#ifdef USE_SHADERS
#include "GbDefines.hh"
#include "GLSLShader.hh"
static GLSLShader shader;
#endif



DisplayHandler* DisplayHandler::_instance = NULL;

DisplayHandler* DisplayHandler::Instance()
{
	if (_instance == NULL)
	{
		_instance = new DisplayHandler;
	}
	return _instance;
}



DisplayHandler::DisplayHandler() :

m_opaqueDirty(true),
m_transparentDirty(true)	

{
	m_opaqueDisplayList = glGenLists(1);
	m_transparentDisplayList = glGenLists(1);
	
	glVertexArrayParameteriAPPLE(GL_VERTEX_ARRAY_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
	
#ifdef USE_SHADERS
	printf("loading objects shader\n");
	
	string shader1_vert = FileUtils::GetApplicationResourcesFolderPath() + string("/Shaders/object.vs");
	string shader1_frag = FileUtils::GetApplicationResourcesFolderPath() + string("/Shaders/object.fs");
	
	shader.loadFromFile( shader1_vert.c_str(), shader1_frag.c_str() );
 	shader.bind();
 	shader.uniform("BlackBorder",0.03f);
 	shader.uniform("InteriorRange",0.05f,0.3f);
 	shader.release();
#endif
	
}

DisplayHandler::~DisplayHandler()
{
	m_opaqueDisplayObjects.clear();
	m_transparentDisplayObjects.clear();
	
	glDeleteLists(m_opaqueDisplayList, 1);
	glDeleteLists(m_transparentDisplayList, 1);
}

void DisplayHandler::reset()
{
	set<DisplayObject*>::iterator iter;
			
	while (m_opaqueDisplayObjects.size()>0)
	{
		iter = m_opaqueDisplayObjects.begin();
		DisplayObject* dObj = *iter;
		delete dObj; // also erases dObj from the set
	}
	
	while (m_transparentDisplayObjects.size()>0)
	{
		iter = m_transparentDisplayObjects.begin();
		DisplayObject* dObj = *iter;
		delete dObj; // also erases dObj from the set
	}
}

void DisplayHandler::setOpaqueDirty()
{
	m_opaqueDirty = true;
}

void DisplayHandler::setTransparentDirty()
{
	m_transparentDirty = true;
}

void DisplayHandler::makeTransparent(DisplayObject* dObj)
{
	if (m_opaqueDisplayObjects.count(dObj) == 1)
	{
		m_opaqueDisplayObjects.erase(dObj);
		m_transparentDisplayObjects.insert(dObj);
	}
}

DisplayObject* DisplayHandler::createDisplayObject( Model* model, Texture* texture, Vec4& translation, float rotation )
{
	if (!model) return NULL; // but texture can be NULL, if no texture to be applied
	
	DisplayObject* dObj = new DisplayObject( model, texture, translation, rotation );
	
	// Add dObj to opaque display object list (new ones are always opaque)
	m_opaqueDisplayObjects.insert(dObj);
	m_opaqueDirty = true;
	
	return dObj;
}

void DisplayHandler::removeDisplayObject( DisplayObject* dObj )
{
	if (dObj->isOpaque())
	{
		m_opaqueDisplayObjects.erase(dObj);
	}
	
	if (!dObj->isOpaque())
	{
		m_transparentDisplayObjects.erase(dObj);
	}
}

void DisplayHandler::recompileOpaque()
{
	// recompile lists of dirty opaque objects
	set<DisplayObject*>::iterator iter;
	iter = m_opaqueDisplayObjects.begin();
	
	while ( iter != m_opaqueDisplayObjects.end() )
	{
		DisplayObject* dObj = *iter;
		if (dObj->isDirty())
		{
			dObj->compileList();
		}
		iter++;
	}
	
	// make the new opaque object display list
	glNewList( m_opaqueDisplayList, GL_COMPILE );
	
	iter = m_opaqueDisplayObjects.begin();
	while ( iter != m_opaqueDisplayObjects.end() )
	{
		DisplayObject* dObj = *iter;
		dObj->render();
		iter++;
	}
	glEndList();
	
	m_opaqueDirty = false;
}


void DisplayHandler::recompileTransparent()
{
	set<DisplayObject*>::iterator iter;
	iter = m_transparentDisplayObjects.begin();
	
	while ( iter != m_transparentDisplayObjects.end() )
	{
		DisplayObject* dObj = *iter;
		if (dObj->isDirty())
		{
			dObj->compileList();
		}
		iter++;
	}	
	
	// make the new transparent object display list
	glNewList( m_transparentDisplayList, GL_COMPILE );
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	
	iter = m_transparentDisplayObjects.begin();	
	while ( iter != m_transparentDisplayObjects.end() )
	{
		DisplayObject* dObj = *iter;
		glBlendColor(0.f, 0.f, 0.f, 1.0f - dObj->getTransparency());
		dObj->render();
		iter++;
	}	
	
	glDisable(GL_BLEND);
	glEndList();
	
	m_transparentDirty = false;
}


const float specular[4] = {0.6f, 0.6f, 0.6f, 1.f};
const float shine[1] = { 10.0f };

void DisplayHandler::renderAll()
{
	if (m_opaqueDirty)      recompileOpaque();
	if (m_transparentDirty) recompileTransparent();

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
		
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
	
	// Render opaque display objects first (so opaque ones visible through the transparent ones are not culled)
	glCallList(m_opaqueDisplayList);
	glCallList(m_transparentDisplayList);
}





