/*
 *  DisplayObject.cpp
 *  The Sentinel
 *
 *  Created by Jamie Portsmouth on 24/04/2007.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Game.h"

#include "DisplayObject.h"
#include "DisplayHandler.h"
#include "ModelLibrary.h"
#include "TextureLibrary.h"

#include "Model.h"


DisplayObject::DisplayObject( Model* model, Texture* texture, Vec4& translation, float rotation ) :

m_active(true),
m_isDirty(true),     //dirty on creation, so display list compiled immediately
m_model(model),
m_texture(texture),  //can be NULL, if no texture applied,
m_translation(translation),
m_rotation(rotation),
m_transparency(0.f), //new DisplayObject is opaque
m_isOpaque(true)     //new DisplayObject is opaque

{
	m_displayList = glGenLists(1);
	
	// Compute world AABB
	m_worldAABB = model->m_AABB.transform(translation, rotation);
}

DisplayObject::~DisplayObject()
{
	glDeleteLists(m_displayList, 1);
	
	DisplayHandler::Instance()->removeDisplayObject(this);
}

AABB& DisplayObject::getWorldAABB()
{
	return m_worldAABB;
}

void DisplayObject::setDirty()
{
	m_isDirty = true;
}

void DisplayObject::activate()
{
	if (!m_active) 
	{
		m_active = true;
		informHandlerOfChange();
	}
}

void DisplayObject::deactivate()
{
	if (m_active) 
	{
		m_active = false;
		informHandlerOfChange();
	}
}


void DisplayObject::render()
{
	if (!m_active) return;
	
	glPushMatrix();
	
	glTranslatef(m_translation[0], m_translation[1], m_translation[2]);
	glRotatef(m_rotation, 0.f, 0.f, 1.f);
	
	glCallList(m_displayList);
	
	glPopMatrix();
}

void _drawDebugNormal( const Vec4& V, const Vec4& N )
{
	//centroid
	glBegin(GL_LINES);
	
	Vec4 normTip = V + 0.05f*N;
	glVertex3f( V[0], V[1], V[2] );
	glVertex3f( normTip[0], normTip[1], normTip[2] );
	
	glEnd();
}


void DisplayObject::compileList()
{
	glNewList( m_displayList, GL_COMPILE );
	
	const float alpha = 1.f;
	
	if (m_texture) 
	{
		glEnable(GL_TEXTURE_2D);
		m_texture->Apply();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
		
	const vector<Vec4>& verts = m_model->vList;
	const vector<Vec4>& norms = m_model->nList;
	const vector<Vec4>& texcs = m_model->tList;
	const vector<Model::FACE>& faces = m_model->faceList;
	
	// NORMAL PER TRIANGLE (FACETED)
	if (m_model->isFlatShaded())
	{
		glBegin(GL_TRIANGLES);
		for (int i=0;i<m_model->fTotal;i++)
		{
			const Model::FACE& face = faces[i];
			const Vec4& N = norms[face[0].n];
			glNormal3fv( &N[0] );

			for (int j=0;j<3;j++)
			{ 
				const Model::Vertex& vertex = face[j];
				const Vec4& V = verts[vertex.v];
				const Vec4& T = texcs[vertex.t];
				Material& M = m_model->mat[vertex.m];
				
				glColor4f( M.Kd[0], M.Kd[1], M.Kd[2], alpha );
				glTexCoord2fv( &T[0] );
				glVertex3fv( &V[0] );
			}
		}
		glEnd();
	}
	
	// NORMAL PER VERTEX
	else
	{
		glBegin(GL_TRIANGLES);
		for (int i=0;i<m_model->fTotal;i++)
		{
			const Model::FACE& face = faces[i];
			for (int j=0;j<3;j++)
			{ 
				const Model::Vertex& vertex = face[j];
				const Vec4& V = verts[vertex.v];
				const Vec4& N = norms[vertex.n];
				const Vec4& T = texcs[vertex.t];
				Material& M = m_model->mat[vertex.m];
				
				glColor4f( M.Kd[0], M.Kd[1], M.Kd[2], alpha );
				glTexCoord2fv( &T[0] );
				glNormal3fv( &N[0] );
				glVertex3fv( &V[0] );
			}
		}
		glEnd();		
	}
		
	//m_model->m_AABB.draw();
	
	glEndList();
	
	m_isDirty = false;
}

/*
	for (int i=0;i<m_model->fTotal;i++)
	{
		for (int j=0;j<3;j++)
		{ 
			const Model::Vertex& vertex = faces[i][j];
			const Vec4& N = norms[vertex.n];
			const Vec4& V = verts[vertex.v];
			
			_drawDebugNormal(V,N);
		}
	}
 */



Model* DisplayObject::GetModel()
{
	return m_model;
}


void DisplayObject::informHandlerOfChange()
{
	if (m_isOpaque)
		DisplayHandler::Instance()->setOpaqueDirty();
	else
		DisplayHandler::Instance()->setTransparentDirty();	
}

void DisplayObject::setTranslation( class Vec4& translation )
{
	m_translation = translation;
	informHandlerOfChange();
}

void DisplayObject::setRotation( float rotation )
{
	m_rotation = rotation;
	informHandlerOfChange();
}

float DisplayObject::getTransparency()
{
	return m_transparency;
}

void DisplayObject::setTransparency( float transparency )
{
	m_transparency = transparency;
	if (m_transparency>1.0f) m_transparency = 1.0f;
	if (m_transparency<0.0f) m_transparency = 0.0f;
	
	if (m_transparency > 0.f) 
	{
		if (m_isOpaque)
		{
			// inform display handler the moment this object becomes transparent
			m_isOpaque = false;
			DisplayHandler::Instance()->makeTransparent(this);
		}
	}
	else 
	{
		m_isOpaque = true;
		m_transparency = 0.f;
	}
	
	informHandlerOfChange();
}



