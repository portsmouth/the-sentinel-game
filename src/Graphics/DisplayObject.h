

#ifndef DISPLAYOBJECT_H
#define DISPLAYOBJECT_H

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "Maths.h"

class Model;
class Texture;

// data needed to render a particular object.

class DisplayObject
{
	
public:
	
	DisplayObject( Model* model, Texture* texture, Vec4& translation, float rotation );
	~DisplayObject();
	
	friend class GameObject;
	
	void render();
	void compileList();
	
	void activate();
	void deactivate();
	
	void setTranslation( class Vec4& translation );
	void setRotation( float rotation );
	
	void setTransparency( float transparency );
	float getTransparency();
	
	Model* GetModel();
	
	AABB& getWorldAABB();
	
	inline bool isOpaque()
	{
		return m_isOpaque;
	}
	
	void setDirty();
		
	inline bool isDirty()
	{
		return m_isDirty;
	}
	
private:
	
	// activation status
	bool m_active;	
	
	// dirty status
	bool m_isDirty;
	
	void informHandlerOfChange();	
	
	// model
	Model* m_model;
	Texture* m_texture;
	
	// model transform
	Vec4 m_translation;
	float m_rotation; //in degrees
	
	// world AABB on creation
	AABB m_worldAABB;
	
	// transparency status
	float m_transparency;
	bool m_isOpaque;
	
	// display lists
	GLuint m_displayList;
	
};



#endif //DISPLAYOBJECT_H
