

#include "ModelLibrary.h"


ModelLibrary* ModelLibrary::_instance = NULL;

ModelLibrary* ModelLibrary::Instance()
{
	if (_instance == NULL)
	{
		_instance = new ModelLibrary;
	}
	return _instance;
}

ModelLibrary::ModelLibrary()
{

};



void ModelLibrary::FreeModel(std::string inName)
{
	Model *model = NULL;
	ModelContainer::const_iterator iter = m_models.find(inName);
	if(iter != m_models.end()) 
	{
		model = iter->second;
		
		delete model;
		m_models.erase(inName);
	}
}

/* return pointer to a model, loading the model if necessary */
Model* ModelLibrary::GetModel(std::string inName, shading_modes sm) const
{
	Model *model = NULL;
	ModelContainer::const_iterator iter = m_models.find(inName);
	
	if(iter != m_models.end())
	{
		//model already loaded, just return a pointer to it
		model = iter->second;

	}
	else
	{				
		//load model in file inName (from model folder)
		model = new Model(inName.c_str(), sm);
		m_models[inName] = model;
	}
	
	return model;
}



