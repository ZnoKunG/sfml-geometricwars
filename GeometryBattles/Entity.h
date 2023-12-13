#pragma once

#include "Components.h";
#include <memory>
#include <string>

class Entity
{
	friend class EntityManager;

	bool m_isActive = true;
	size_t m_id = 0;
	std::string m_tag = "default";

	// private constructor (prevent others to instantiate this object rather than EntityManager)
	Entity(const size_t id, const std::string& tag);

public:
	
	// component pointers (this version, it is just listing all components which could be improved later)
	std::shared_ptr<CTransform> cTransform;
	std::shared_ptr<CShape> cShape;
	std::shared_ptr<CCollision> cCollision;
	std::shared_ptr<CInput> cInput;
	std::shared_ptr<CScore> cScore;
	std::shared_ptr<CLifespan> cLifespan;

	bool isActive() const;
	const std::string& getTag() const;
	const size_t id() const;
	void destroy();
};

