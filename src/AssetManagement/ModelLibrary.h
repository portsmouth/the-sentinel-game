/*
 *  ModelLibrary.h
 *  The Sentinel
 *
 *  Created by Jamie Portsmouth on 28/04/2007.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MODELLIBRARY_H
#define MODELLIBRARY_H

#include <string>
#include <map>

#include "Model.h"

class ModelLibrary 
{
	
protected:
	
	// SINGLETON
	ModelLibrary();
	
private:
	
	static ModelLibrary* _instance;
	
public:	
	
	static ModelLibrary* Instance();
	
	
	Model* GetModel(std::string inName, shading_modes sm) const;
	void FreeModel(std::string inName);
	
private:
	
	typedef std::map<std::string, Model*> ModelContainer;
	mutable ModelContainer m_models;
	
};

#endif //MODELLIBRARY_H

