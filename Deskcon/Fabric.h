//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Fabric.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

class ServiceCollection 
{
private:
	vector<IService*> serviceVector;

public:
	ServiceCollection();

	~ServiceCollection();

public:
	
void RegisterService( IService* ppService );

template<class T>
T* GetService( void );

}; 

class FabricApp
{
private:
	static FabricApp* instance;
	ServiceCollection services;

public:
	static FabricApp* Instance()
	{
		if( instance == NULL )
		{
			instance = new FabricApp();
		}

		return instance;
	}

	ServiceCollection Services()
	{
		return services;
	}
};