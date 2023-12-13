#pragma once
#include <memory>
#include <vector>
#include <map>
#include "Entity.h"
#include <iostream>

typedef std::vector <std::shared_ptr<Entity>> EntityVector;
typedef std::map<std::string, EntityVector> EntityMap;
class EntityManager
{
	EntityVector m_entities;
	EntityMap m_entityMap;
	EntityVector m_toAdd;
	EntityVector m_toRemove;
	size_t m_totalEntities = 0;

public:
	EntityManager();

	void update();

	std::shared_ptr<Entity> addEntity(const std::string& tag);
	EntityVector& getEntities();
	EntityVector& getEntitiesFromTag(const std::string& tag);
};

