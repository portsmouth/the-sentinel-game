/*
 *  DisplayHandler.h
 *  The Sentinel
 *
 *  Created by Jamie Portsmouth on 24/04/2007.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include <set>
using std::set;

// Handles display of all dynamic game objects 
// i.e. boulders, trees, synthoids and The Sentinel

class DisplayObject;
class Model;
class Texture;
class Vec4;

class DisplayHandler
{
	
protected:
	
	// SINGLETON
	DisplayHandler();

private:
	
	static DisplayHandler* _instance;
	
public:	
	
	static DisplayHandler* Instance();

	~DisplayHandler();
	
	// add or remove display objects
	DisplayObject* createDisplayObject( Model* model, Texture* texture, Vec4& translation,  float rotation = 0.f );
	void removeDisplayObject( DisplayObject* dObj );
	
	// render all display objects
	void renderAll();
		
	// register a particular DisplayObject as transparent
	void makeTransparent(DisplayObject* dObj);
	
	// tell handler that opaque objects need to be redrawn
	void setOpaqueDirty();
	
	// tell handler that transparent objects need to be redrawn
	void setTransparentDirty();

	// delete all display objects
	void reset();
		
private:
	
	set<DisplayObject*> m_opaqueDisplayObjects;
	set<DisplayObject*> m_transparentDisplayObjects;
		
	GLuint m_opaqueDisplayList;
	GLuint m_transparentDisplayList;

	bool m_opaqueDirty;	
	bool m_transparentDirty;	
	
	void recompileTransparent();
	void recompileOpaque();

};


#endif // DISPLAYHANDLER_H
