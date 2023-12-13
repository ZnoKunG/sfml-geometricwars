#include "Game.h"
#include <iostream>
#include <fstream>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	std::ifstream config(path);

	std::string identifier;
	while (config >> identifier)
	{
		if (identifier == "Window")
		{
			float wWidth = 0;
			float wHeight = 0;
			int frameLimit = 0;
			bool isFullscreen = false;
			config >> wWidth >> wHeight >> frameLimit >> isFullscreen;

			window.create(sf::VideoMode(wWidth, wHeight), "Geometry Wars", isFullscreen ? sf::Style::Fullscreen : sf::Style::Default);
			window.setFramerateLimit(frameLimit);

			std::cout << "Config Window completed\n";
		}

		if (identifier == "Font")
		{
			std::string fontFile;
			int fontSize = 0;
			sf::Color fontColor;
			int r = 0;
			int g = 0;
			int b = 0;
			config >> fontFile >> fontSize >> r >> g >> b;

			if (!m_font.loadFromFile(fontFile))
			{
				std::cout << "Cannot download font from file!\n";
				exit(-1);
			}

			m_text = sf::Text("Score: ", m_font, fontSize);
			m_text.setFillColor(sf::Color(r, g, b));
			std::cout << "Config Font completed\n";
		}

		if (identifier == "Player")
		{
			config >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >>
					  m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >> m_playerConfig.S;

			std::cout << "Config Player completed\n";
		}

		if (identifier == "Enemy")
		{
			config >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >>
					  m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX;

			std::cout << "Config Enemy completed\n";
		}

		if (identifier == "Bullet")
		{
			config >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >>
					  m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L >> m_bulletConfig.S;

			std::cout << "Config Bullet completed\n";
		}

		if (identifier == "Special")
		{
			config >> m_specialConfig.SR >> m_specialConfig.CR >> m_specialConfig.FR >> m_specialConfig.FG >> m_specialConfig.FB >> m_specialConfig.OR >>
				m_specialConfig.OG >> m_specialConfig.OB >> m_specialConfig.OT >> m_specialConfig.V >> m_specialConfig.TD >> m_specialConfig.TR >> m_specialConfig.CD >> m_specialConfig.S;

			std::cout << "Config Special completed\n";
		}
	}

	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		// Running systems
		if (!m_paused)
		{
			sEnemySpawner();
			sUserInput();
			sCollision();
			sLifespan();
			sMovement();
		}

		sRender();

		m_currentFrame++;
	}
}


void Game::setPaused(bool paused)
{
	m_paused = paused;
}

void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");

	Vec2 randDir = Vec2(rand() % 2, rand() % 2).normalize();
	entity->cTransform = std::make_shared<CTransform>(Vec2(window.getSize().x / 2.0f, window.getSize().y / 2.0f), randDir * m_playerConfig.S, 0);

	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	entity->cInput = std::make_shared<CInput>();

	m_player = entity;
}

void Game::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");

	srand(time(NULL));
	int randSpawnX = m_enemyConfig.SR + (rand() % (1 + (window.getSize().x - m_enemyConfig.SR) - m_enemyConfig.SR));
	int randSpawnY = m_enemyConfig.SR + (rand() % (1 + (window.getSize().y - m_enemyConfig.SR) - m_enemyConfig.SR));
	Vec2 randDir = Vec2((rand() % 2) + 1, (rand() % 2) + 1).normalize();
	float randSpeed = m_enemyConfig.SMIN + (rand() % (1 + m_enemyConfig.SMAX - m_enemyConfig.SMIN));
	std::cout << "random speed " << randSpeed << "\n";
	entity->cTransform = std::make_shared<CTransform>(Vec2(randSpawnX, randSpawnY), randDir * randSpeed, 0);

	sf::Color randColor(rand() % 256, rand() % 256, rand() % 256);
	int randVertices = m_enemyConfig.VMIN + (rand() % (1 + m_enemyConfig.VMAX - m_enemyConfig.VMIN));
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, randVertices, randColor,
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
}

void Game::sRender()
{
	window.clear(sf::Color::Black);

	for (auto e : m_entities.getEntities())
	{
		if (e->cShape && e->cTransform)
		{
			e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
			e->cTransform->angle += 1.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
			window.draw(e->cShape->circle);
		}
	}

	m_text.setPosition(10, 10);
	std::string specialText = m_currentFrame - m_lastSpecialWeaponUsedTime >= m_specialConfig.CD ? " [SPECIAL]" : "";
	m_text.setString("Score: " + std::to_string(m_score) + specialText);
	window.draw(m_text);

	window.display();
}

void Game::sCollision()
{
	for (auto e1 : m_entities.getEntities())
	{
		if (!e1->isActive()) continue;

		if (e1->cTransform && e1->cCollision)
		{
			// Check if collide with the window
			if (e1->cTransform->pos.x <= e1->cCollision->radius || e1->cTransform->pos.x >= window.getSize().x - e1->cCollision->radius)
			{
				if (e1->cInput)
				{
					if (e1->cInput->left && e1->cTransform->pos.x <= e1->cCollision->radius)
					{
						e1->cInput->left = false;
					}
					if (e1->cInput->right && e1->cTransform->pos.x >= window.getSize().x - e1->cCollision->radius)
					{
						e1->cInput->right = false;
					}
				}
				else
				{
					if (e1->getTag() != "bullet")
					{
						e1->cTransform->velocity = Vec2(e1->cTransform->velocity.x * -1, e1->cTransform->velocity.y);
					}
				}
			}

			if (e1->cTransform->pos.y <= e1->cCollision->radius || e1->cTransform->pos.y >= window.getSize().y - e1->cCollision->radius)
			{
				if (e1->cInput)
				{
					if (e1->cInput->up && e1->cTransform->pos.y <= e1->cCollision->radius)
					{
						e1->cInput->up = false;
					}
					if (e1->cInput->down && e1->cTransform->pos.y >= window.getSize().y - e1->cCollision->radius)
					{
						e1->cInput->down = false;
					}
				}
				else
				{
					if (e1->getTag() != "bullet")
					{
						e1->cTransform->velocity = Vec2(e1->cTransform->velocity.x, e1->cTransform->velocity.y * -1);
					}
				}
			}

			// Check if collide with others
			for (auto e2 : m_entities.getEntities())
			{
				if (!e2->isActive()) continue;

				if (e2->cTransform && e2->cCollision && e2->id() != e1->id())
				{
					if (e1->cTransform->pos.dist(e2->cTransform->pos) <= e1->cCollision->radius + e2->cCollision->radius)
					{
						// If collision occurs! we could add functionality here
						if (e1->cInput)
						{
							e1->cTransform->velocity = Vec2(0, 0); // Prevent movement from inputs when collided

							if (e2->getTag() == "enemy")
							{
								e2->destroy();
								spawnSmallEmemies(e2);
								m_player->cTransform->pos = Vec2(window.getSize().x / 2, window.getSize().y / 2);
							}
						}
						else
						{
							if (e1->getTag() == "bullet" && e2->getTag() == "enemy")
							{
								e1->destroy();
								e2->destroy();
								spawnSmallEmemies(e2);
								m_score += 100;
							}

							if (e1->getTag() == "enemy" && e2->getTag() == "bullet")
							{
								e1->destroy();
								e2->destroy();
								spawnSmallEmemies(e1);
								m_score += 100;
							}

							if (e1->getTag() == "bullet" && e2->getTag() == "smallEnemy")
							{
								e1->destroy();
								e2->destroy();
								spawnParticleEmemies(e2);
								m_score += 200;
							}

							if (e1->getTag() == "smallEnemy" && e2->getTag() == "bullet")
							{
								e1->destroy();
								e2->destroy();
								spawnParticleEmemies(e1);
								m_score += 200;
							}

							// Prevent small enemies from colliding each other
							if ((e1->getTag() == "smallEnemy" || e2->getTag() == "smallEnemy") ||
								(e1->getTag() == "player" && e2->getTag() == "bullet") ||
								(e1->getTag() == "bullet" && e2->getTag() == "player") ||
								e1->getTag() == "special" || e2->getTag() == "special") break;

							Vec2 collideDir = (e2->cTransform->pos - e1->cTransform->pos).normalize();
							e1->cTransform->velocity = collideDir * -1 * e1->cTransform->velocity.length();
							break;
						}
					}
				}
			}
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI)
	{
		spawnEnemy();
		m_lastEnemySpawnTime = m_currentFrame;
	}
}

void Game::sMovement()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cTransform)
		{
			if (e->cInput)
			{
				float xInput = 0.0f;
				float yInput = 0.0f;

				if (e->cInput->right || e->cInput->left && !(e->cInput->right && e->cInput->left))
				{
					xInput = e->cInput->right ? 1.0f : -1.0f;
				}

				if (e->cInput->up || e->cInput->down && !(e->cInput->up && e->cInput->down))
				{
					yInput = e->cInput->down ? 1.0f : -1.0f;
				}

				Vec2 moveDir = Vec2(xInput, yInput).normalize();
				e->cTransform->velocity = moveDir * m_playerConfig.V;
			}

			e->cTransform->pos += e->cTransform->velocity;

			if (e->getTag() == "special")
			{
				std::cout << e->cTransform->pos.dist(m_specialConfig.TriggerPos) << "\n";
				if (e->cTransform->pos.dist(m_specialConfig.TriggerPos) <= 10.0f)
				{
					if (!isTriggeringSpecial)
					{
						isTriggeringSpecial = true;
						m_startTriggerSpecialTime = m_currentFrame;

						auto s = m_entities.addEntity("specialVisual");
						s->cTransform = std::make_shared<CTransform>(e->cTransform->pos, Vec2(0, 0), 0.0f);
						s->cShape = std::make_shared<CShape>(m_specialConfig.TR, m_specialConfig.V, sf::Color(m_specialConfig.FR, m_specialConfig.FG, m_specialConfig.FB, 0),
							sf::Color(m_specialConfig.OR, m_specialConfig.OG,  m_specialConfig.OB), m_specialConfig.OT);
						e->destroy();
					}
				}
			}

			if (e->getTag() == "specialVisual")
			{
				TriggerSpecial(e);
			}
		}
	}
}

void Game::sLifespan()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cLifespan)
		{
			if (e->cShape)
			{
				int alpha = int(((float)e->cLifespan->remaining / e->cLifespan->total) * 255);

				e->cShape->circle.setFillColor(sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g,
														 e->cShape->circle.getFillColor().b, alpha));

				e->cShape->circle.setOutlineColor(sf::Color(e->cShape->circle.getOutlineColor().r, e->cShape->circle.getOutlineColor().g,
															e->cShape->circle.getOutlineColor().b, alpha));
			}

			if (e->cLifespan->remaining <= 0.0f)
			{
				e->destroy();
			}

			e->cLifespan->remaining -= 1.0f;
		}
	}
}

void Game::sUserInput()
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed || event.type == sf::Keyboard::Escape)
		{
			m_running = false;
			window.close();
		}
		
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::W)
			{
				m_player->cInput->up = true;
				std::cout << "Pressed W\n";
			}

			if (event.key.code == sf::Keyboard::S)
			{
				m_player->cInput->down = true;
				std::cout << "Pressed S\n";
			}

			if (event.key.code == sf::Keyboard::A)
			{
				m_player->cInput->left = true;
				std::cout << "Pressed A\n";
			}

			if (event.key.code == sf::Keyboard::D)
			{
				m_player->cInput->right = true;
				std::cout << "Pressed D\n";
			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::W)
			{
				m_player->cInput->up = false;
				std::cout << "Released W\n";
			}

			if (event.key.code == sf::Keyboard::S)
			{
				m_player->cInput->down = false;
				std::cout << "Released S\n";
			}

			if (event.key.code == sf::Keyboard::A)
			{
				m_player->cInput->left = false;
				std::cout << "Released A\n";
			}

			if (event.key.code == sf::Keyboard::D)
			{
				m_player->cInput->right = false;
				std::cout << "Released D\n";
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				std::cout << "Mouse click at " << event.mouseButton.x << ", " << event.mouseButton.y << "\n";
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right && m_currentFrame - m_lastSpecialWeaponUsedTime >= m_specialConfig.CD)
			{
				m_lastSpecialWeaponUsedTime = m_currentFrame;
				spawnSpecialWeapon(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
		}
	}
}

void Game::spawnSmallEmemies(std::shared_ptr<Entity> baseEnemyEntity)
{
	for (int i = 0; i < baseEnemyEntity->cShape->circle.getPointCount(); i++)
	{
		float shootAngle = 360.0f * i / baseEnemyEntity->cShape->circle.getPointCount();
		double pi = 22 / 7;
		Vec2 shootDir = Vec2(cosf(shootAngle * pi / 180), sinf(shootAngle * pi / 180)).normalize();
		
		auto e = m_entities.addEntity("smallEnemy");
		e->cTransform = std::make_shared<CTransform>(baseEnemyEntity->cTransform->pos, shootDir * baseEnemyEntity->cTransform->velocity.length(),
													 baseEnemyEntity->cTransform->angle + shootAngle);

		e->cShape = std::make_shared<CShape>(baseEnemyEntity->cShape->circle.getRadius() / 2, baseEnemyEntity->cShape->circle.getPointCount(),
											 baseEnemyEntity->cShape->circle.getFillColor(), baseEnemyEntity->cShape->circle.getOutlineColor(),
											 baseEnemyEntity->cShape->circle.getOutlineThickness());

		e->cCollision = std::make_shared<CCollision>(baseEnemyEntity->cShape->circle.getRadius() / 2);
		e->cLifespan = std::make_shared<CLifespan>(60);
	}
}

void Game::spawnParticleEmemies(std::shared_ptr<Entity> baseEnemyEntity)
{
	for (int i = 0; i < baseEnemyEntity->cShape->circle.getPointCount(); i++)
	{
		float shootAngle = 360.0f * i / baseEnemyEntity->cShape->circle.getPointCount();
		double pi = 22 / 7;
		Vec2 shootDir = Vec2(cosf(shootAngle * pi / 180), sinf(shootAngle * pi / 180)).normalize();

		auto e = m_entities.addEntity("particle");
		e->cTransform = std::make_shared<CTransform>(baseEnemyEntity->cTransform->pos, shootDir * baseEnemyEntity->cTransform->velocity.length(),
													 baseEnemyEntity->cTransform->angle + shootAngle);

		e->cShape = std::make_shared<CShape>(baseEnemyEntity->cShape->circle.getRadius() / 2, baseEnemyEntity->cShape->circle.getPointCount(),
			baseEnemyEntity->cShape->circle.getFillColor(), baseEnemyEntity->cShape->circle.getOutlineColor(),
			baseEnemyEntity->cShape->circle.getOutlineThickness());

		e->cLifespan = std::make_shared<CLifespan>(30);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> spawnerEntity, const Vec2& mousePos)
{
	auto bullet = m_entities.addEntity("bullet");

	Vec2 shootDir = (mousePos - spawnerEntity->cTransform->pos).normalize();
	bullet->cTransform = std::make_shared<CTransform>(spawnerEntity->cTransform->pos, shootDir * m_bulletConfig.S, atan2f(shootDir.y, shootDir.x));

	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
					 sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> spawnerEntity, const Vec2& mousePos)
{
	auto bullet = m_entities.addEntity("special");

	Vec2 shootDir = (mousePos - spawnerEntity->cTransform->pos).normalize();
	bullet->cTransform = std::make_shared<CTransform>(spawnerEntity->cTransform->pos, shootDir * m_specialConfig.S, atan2f(shootDir.y, shootDir.x));

	bullet->cShape = std::make_shared<CShape>(m_specialConfig.SR, m_specialConfig.V, sf::Color(m_specialConfig.FR, m_specialConfig.FG, m_specialConfig.FB, 0),
		sf::Color(m_specialConfig.OR, m_specialConfig.OG, m_specialConfig.OB), m_specialConfig.OT);

	bullet->cCollision = std::make_shared<CCollision>(m_specialConfig.CR);
	m_specialConfig.TriggerPos = mousePos;
	
}

void Game::TriggerSpecial(std::shared_ptr<Entity> specialEntity)
{
	float durationRatio = (float)(m_currentFrame - m_startTriggerSpecialTime) / m_specialConfig.TD;
	std::cout << (int)(durationRatio * 255) << "\n";
	specialEntity->cShape->circle.setFillColor(sf::Color(specialEntity->cShape->circle.getFillColor().r, specialEntity->cShape->circle.getFillColor().g,
														 specialEntity->cShape->circle.getFillColor().b, (int)(durationRatio * 128)));

	for (auto e : m_entities.getEntitiesFromTag("enemy"))
	{
		if (specialEntity->cTransform->pos.dist(e->cTransform->pos) <= m_specialConfig.TR)
		{
			e->destroy();
			spawnSmallEmemies(e);
			m_score += 100;
		}
	}

	if (m_currentFrame - m_startTriggerSpecialTime >= m_specialConfig.TD)
	{
		isTriggeringSpecial = false;

		for (auto e : m_entities.getEntitiesFromTag("enemy"))
		{
			if (specialEntity->cTransform->pos.dist(e->cTransform->pos) <= m_specialConfig.TR)
			{
				e->destroy();
				spawnSmallEmemies(e);
				m_score += 100;
			}
		}

		for (auto e : m_entities.getEntitiesFromTag("smallEnemy"))
		{
			if (specialEntity->cTransform->pos.dist(e->cTransform->pos) <= m_specialConfig.TR)
			{
				e->destroy();
				spawnParticleEmemies(e);
				m_score += 200;
			}
		}

		specialEntity->destroy();
	}
}

