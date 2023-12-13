#include "EntityManager.h"

bool IsToDestroyEntity(std::shared_ptr<Entity> e)
{
	return !e->isActive();
}

EntityManager::EntityManager() {}

void EntityManager::update()
{
	for (auto const e : m_toAdd)
	{
		m_entities.push_back(e);
		m_entityMap[e->getTag()].push_back(e);
	}

	for (auto const e : m_entities)
	{
		if (!e->isActive())
		{
			m_toRemove.push_back(e);
		}
	}

	for (auto const e : m_toRemove)
	{
		auto toRemoveEntity = std::find(m_entities.begin(), m_entities.end(), e);
		m_entities.erase(toRemoveEntity);

		toRemoveEntity = std::find(m_entityMap[e->getTag()].begin(), m_entityMap[e->getTag()].end(), e);
		m_entityMap[e->getTag()].erase(toRemoveEntity);
	}

	m_toAdd.clear();
	m_toRemove.clear();
}


std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	std::shared_ptr<Entity> e = std::make_shared<Entity>(Entity(m_totalEntities++, tag));
	m_toAdd.push_back(e);
	return e;
}

EntityVector& EntityManager::getEntities()
{
	return m_entities;
}

EntityVector& EntityManager::getEntitiesFromTag(const std::string& tag)
{
	return m_entityMap[tag];
}

