#pragma once

#include "EntityManager.h"
#include <SFML/Graphics.hpp>

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI, SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
struct SpecialConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, TD, TR, CD; float S; Vec2 TriggerPos; };

class Game
{
	sf::RenderWindow window;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	SpecialConfig m_specialConfig;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	int m_startTriggerSpecialTime = 0;
	int m_lastSpecialWeaponUsedTime = 0;
	bool isTriggeringSpecial = false;
	bool m_paused = false;
	bool m_running = true;

	std::shared_ptr<Entity> m_player;

	void init(const std::string& path);
	void setPaused(bool paused);

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEmemies(std::shared_ptr<Entity> baseEnemyEntity);
	void spawnParticleEmemies(std::shared_ptr<Entity> baseEnemyEntity);
	void spawnBullet(std::shared_ptr<Entity> spawnerEntity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> spawnerEntity, const Vec2& mousePos);
	void TriggerSpecial(std::shared_ptr<Entity> specialEntity);

public:

	Game(const std::string& config);

	void run();
};

