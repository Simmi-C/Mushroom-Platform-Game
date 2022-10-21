#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1024;
int DISPLAY_HEIGHT = 768;
int DISPLAY_SCALE = 1;

enum CharacterState
{
	STATE_START = 0,
	STATE_APPEAR,
	STATE_STANDING,
	STATE_WALKING,
	STATE_JUMPING,
	STATE_FALLING,
	STATE_DAMAGED,
	STATE_DEAD,
	STATE_LEVEL_COMPLETE,
};


struct GameState
{
	int score = 0;
	int lives = 3;
	bool facingright = true;
	CharacterState characterState = STATE_START;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_CHARACTER,
	TYPE_PLATFORM,
	TYPE_COLLECTABLE,
	TYPE_COLLECTED,
	TYPE_FLAG,
	TYPE_ENEMY,
	TYPE_DESTROYED,
	TYPE_JUMPPAD,
	TYPE_UI_STAR,
	TYPE_UI_HEART,
	TYPE_BACKGROUND
};

void DrawUI();
void DefinePlatformsandJumpPads();
void DrawPlatformsandJumpPads();

void DefineCollectables();
void UpdateCollectables();
void UpdateCollected();
void DefineEnemies();
void UpdateEnemies();
void UpdateDestroyed();

void Standing();
void Walking();
void Jumping();
void Falling();
void UpdateCharacter();

bool CheckPlatformCollisions(GameObject& character);
void FacingRight();

void CompletedLevel();


// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	//Play::LoadBackground("Data\\Backgrounds\\allbackgroundstars.png");
	int id2 = Play::CreateGameObject(TYPE_BACKGROUND, { 0, 0 }, 0, "sky");
	Play::CentreMatchingSpriteOrigins("mushroom");
	Play::CentreSpriteOrigin("star");
	Play::CentreSpriteOrigin("heart");
	Play::MoveSpriteOrigin("flag", 22, 39);
	

	DefinePlatformsandJumpPads();
	
	Play::CreateGameObject(TYPE_CHARACTER, { 300, 1600 }, 36, "mushroom_walk_right");
	int id = Play::CreateGameObject(TYPE_UI_STAR, { Play::cameraPos.x + 70, Play::cameraPos.y + 60 }, 0, "star");
	int id1 = Play::CreateGameObject(TYPE_UI_HEART, { Play::cameraPos.x + DISPLAY_WIDTH - 140, Play::cameraPos.y + 60 }, 0, "heart");

	
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	//Play::DrawBackground();
	Play::ClearDrawingBuffer(Play::cOrange );
	GameObject& background = Play::GetGameObjectByType(TYPE_BACKGROUND);
	Play::DrawObject(background);
	DrawPlatformsandJumpPads();
	UpdateCollectables();
	UpdateCollected();
	UpdateEnemies();
	UpdateDestroyed();
	DrawUI();
	UpdateCharacter(); 

	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void DrawUI()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	Play::GetCameraPosition();

	//stars
	GameObject& obj_star = Play::GetGameObjectByType(TYPE_UI_STAR);
	obj_star.pos = { Play::cameraPos.x + 70, Play::cameraPos.y + 60 };
	Play::UpdateGameObject(obj_star);
	Play::DrawObject(obj_star);
	Play::DrawFontText("64px", ":" + std::to_string(gameState.score), { Play::cameraPos.x + 110, Play::cameraPos.y + 32 }, Play::CENTRE);

	//frame
	//Play::DrawFontText("64px", "Obj_character frame:" + std::to_string(obj_character.frame), { Play::cameraPos.x + DISPLAY_WIDTH / 2, Play::cameraPos.y + 50 }, Play::CENTRE);

	//lives
	GameObject& obj_heart = Play::GetGameObjectByType(TYPE_UI_HEART);
	obj_heart.pos = { Play::cameraPos.x + DISPLAY_WIDTH - 140, Play::cameraPos.y + 60 };
	Play::UpdateGameObject(obj_heart);
	Play::DrawObject(obj_heart);
	Play::DrawFontText("64px", ":" + std::to_string(gameState.lives), {Play::cameraPos.x + DISPLAY_WIDTH - 100, Play::cameraPos.y + 32}, Play::CENTRE);
	
	//instructions
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE. SPACE TO JUMP", { Play::cameraPos.x + DISPLAY_WIDTH / 2, Play::cameraPos.y + DISPLAY_HEIGHT - 70 }, Play::CENTRE);

	//mouse position
	//Vector2f mousepos = Play::GetMousePos();
	//Play::DrawFontText("64px", std::to_string(mousepos.x) + ", " + std::to_string(mousepos.y), (Play::cameraPos + mousepos), Play::LEFT);
}

void DefinePlatformsandJumpPads()
{
	int platform_width = Play::GetSpriteWidth("platform");
	std::vector<Point2f> platform_coordinates = { { 0, 1800 }, { platform_width, 1800 }, { platform_width*2, 1800 }, { platform_width*3, 1800 },
		 { platform_width * 2, 1650 }, {150, 1500}, {600, 1350}, {-100, 1150}, {400,1150}, {750, 1150}, {650, 1000}, {0, 850},
		{DISPLAY_WIDTH - platform_width - 100, 550}, {100, 550}, {(DISPLAY_WIDTH - platform_width) / 2, 700}, {(DISPLAY_WIDTH - platform_width) / 2, 350}
	};

	for (Point2f coordinate : platform_coordinates)
	{
		Play::CreateGameObject(TYPE_PLATFORM, coordinate, 0, "platform");
	}

	Play::MoveSpriteOrigin("jumppad", 32, 20);
	std::vector<Point2f> jumppad_coordinates = { {300, (1500+15)}, {800,(550+15) } };

	for (Point2f coordinate : jumppad_coordinates)
	{
		Play::CreateGameObject(TYPE_JUMPPAD, coordinate, 27, "jumppad");
	}
	
}

void DrawPlatformsandJumpPads()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER); 
	
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		Play::DrawObject(obj_platform);
	}
	
	std::vector<int> vJumpPads = Play::CollectGameObjectIDsByType(TYPE_JUMPPAD);
	for (int id : vJumpPads)
	{
		GameObject& obj_jumppad = Play::GetGameObject(id);
		if (gameState.characterState == STATE_FALLING && Play::IsColliding(obj_character, obj_jumppad))
		{			
			if (obj_character.velocity.y < 10)
			{
				obj_character.velocity.y = -17;
			}
			else if (obj_character.velocity.y >= 10)
			{
				obj_character.velocity.y = -(17 + obj_character.velocity.y * 0.25);
			}
			gameState.characterState = STATE_JUMPING;
			Play::PlayAudio("Jumppad");
		}
		Play::DrawObject(obj_jumppad);
	}
	
}

void DefineCollectables()
{
	std::vector<Point2f> collectable_coordinates = { {100, 1600}, {850, 1500}, {750, 1275}, {300,1200}, {850, 1100}, {55, 1050}, {400, 950}, {750,720}, {DISPLAY_WIDTH / 2, 600}, {150, 400} };
	for (Point2f coordinate : collectable_coordinates)
	{
		int id = Play::CreateGameObject(TYPE_COLLECTABLE, coordinate, 20, "star13");
		Play::GetGameObject(id).animSpeed = 0.25f;
	}

	Play::CentreSpriteOrigin("star13");
	
	
	int id = Play::CreateGameObject(TYPE_FLAG, { DISPLAY_WIDTH/2 + 7 , 320 }, 25, "flag");
	
}

void UpdateCollectables()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	std::vector<int> vCollectables = Play::CollectGameObjectIDsByType(TYPE_COLLECTABLE);

	for (int id : vCollectables)
	{
		GameObject& obj_collectable = Play::GetGameObject(id);
		bool collected = false;

		if (Play::IsColliding(obj_collectable, obj_character))
		{
			gameState.characterState = STATE_JUMPING;
			obj_collectable.type = TYPE_COLLECTED;
			collected = true;
			gameState.score += 1;
			Play::PlayAudio("Collect_Star");
		}

		Play::UpdateGameObject(obj_collectable);
		Play::DrawObject(obj_collectable);
	}
}

void UpdateCollected()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	std::vector<int> vCollected = Play::CollectGameObjectIDsByType(TYPE_COLLECTED);

	for (int id_collected : vCollected)        
	{
		GameObject& obj_collected = Play::GetGameObject(id_collected);

	}

	GameObject& obj_flag = Play::GetGameObjectByType(TYPE_FLAG);
	Play::DrawObject(obj_flag);

	if (Play::IsColliding(obj_flag, obj_character))
	{
		
		if (vCollected.size() == 10 && gameState.characterState != STATE_LEVEL_COMPLETE) 
		{ 
			Play::DrawFontText("64px", "PRESS E TO RETURN STARS", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
			if (Play::KeyPressed('E'))
			{
				Play::PlayAudio("Collect_flag");
				gameState.characterState = STATE_LEVEL_COMPLETE;
			}
		}
		else if (vCollected.size() != 10)
		{
			int starsLeft = 10 - gameState.score;
			Play::DrawFontText("64px", std::to_string(starsLeft) + " STARS LEFT TO COLLECT!", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		}
	}

}

void DefineEnemies()
{
	std::vector<Point2f> enemy_coordinates = { {435 ,1135}, {635, 1635}, {35, 835}, {135, 535} };

	for (Point2f coordinate : enemy_coordinates)
	{
		int id = Play::CreateGameObject(TYPE_ENEMY, coordinate, 30, "snail_right");
		Play::GetGameObject(id).velocity.x = 1.0f;
	}

	Play::CentreSpriteOrigin("snail_right");
	Play::CentreSpriteOrigin("snail_left");
}

void UpdateEnemies()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	std::vector<int> vEnemies = Play::CollectGameObjectIDsByType(TYPE_ENEMY);

	for (int id : vEnemies)
	{
		GameObject& obj_enemy = Play::GetGameObject(id);
		obj_enemy.animSpeed = 0.05f;
		float enemyspeed = 1;

		if (obj_enemy.frame > 10)
		{
			if (obj_enemy.velocity.x > 0)
			{
				Play::SetSprite(obj_enemy, "snail_left", 0.05f);
				obj_enemy.velocity.x = -enemyspeed;
			}
			else if (obj_enemy.velocity.x < 0)
			{
				Play::SetSprite(obj_enemy, "snail_right", 0.05f);
				obj_enemy.velocity.x = enemyspeed;
			}
		}
		Play::UpdateGameObject(obj_enemy);

		if ((gameState.characterState == STATE_FALLING) && Play::IsColliding(obj_enemy, obj_character))
		{
			gameState.characterState = STATE_JUMPING;
			obj_character.velocity.y = -10;
			obj_enemy.type = TYPE_DESTROYED;
			Play::PlayAudio("HitSnail");
			
		}
		else if (!(gameState.characterState == STATE_FALLING) && Play::IsColliding(obj_enemy, obj_character) && gameState.lives !=0)
		{
			
			gameState.characterState = STATE_DAMAGED;
			gameState.lives -= 1;
		}
	
		Play::DrawObject(obj_enemy);
	}
}

void UpdateDestroyed()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_CHARACTER);
	std::vector<int> vDestroyed = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_destroyed : vDestroyed)
	{
		GameObject& obj_destroyed = Play::GetGameObject(id_destroyed);
		
		if (obj_destroyed.velocity.x > 0)
		{
			Play::SetSprite(obj_destroyed, "snail_right", 0.1f);
		}
		else if (obj_destroyed.velocity.x < 0)
		{
			Play::SetSprite(obj_destroyed, "snail_left", 0.1f);
		}
		obj_destroyed.velocity.x = 0;
		Play::UpdateGameObject(obj_destroyed);

		if (obj_destroyed.frame % 2) 
			Play::DrawObjectRotated(obj_destroyed, (10 - obj_destroyed.frame) / 10.0f); 

		if (!Play::IsVisible(obj_destroyed) || obj_destroyed.frame >= 10)  
			Play::DestroyGameObject(id_destroyed);
	}
}


bool CheckPlatformCollisions(GameObject& character)
{
	int platform_width = Play::GetSpriteWidth("platform");
	int platform_height = Play::GetSpriteHeight("platform");
	float character_width = Play::GetSpriteWidth("mushroom") - 36;
	float character_height = Play::GetSpriteHeight("mushroom");

	bool StandingOnPlatform = false;

	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);

		bool X_collision = (character.pos.x + character_width / 2 > obj_platform.pos.x)           //Testing Right Side
			&& (obj_platform.pos.x + platform_width > character.pos.x - character_width / 2);     //Testing Left Side
		bool Y_collision = (character.pos.y + character_height / 2 > obj_platform.pos.y)          //Testing Top Side
			&& (obj_platform.pos.y + platform_height > character.pos.y - character_height / 2);   //Testing Under Side

		if (X_collision && Y_collision)
		{
			StandingOnPlatform = true;
			if (character.pos.y < obj_platform.pos.y) 
			{ 
				character.pos.y = obj_platform.pos.y + 1 - character_height / 2; 
			}
			if (character.pos.y > obj_platform.pos.y)
			{
				character.pos.y = obj_platform.pos.y + platform_height + 1 + character_height / 2;
				gameState.characterState = STATE_FALLING;
			}
		}
	}

	return StandingOnPlatform;

}

void Standing()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	FacingRight();

	if (CheckPlatformCollisions(obj_character))
	{
		if (Play::KeyPressed(VK_LEFT) || Play::KeyPressed(VK_RIGHT) || Play::KeyDown(VK_LEFT) || Play::KeyDown(VK_RIGHT))
		{

			gameState.characterState = STATE_WALKING;
		}
		else if (Play::KeyPressed(VK_SPACE))
		{
			obj_character.velocity.y = -13;
			Play::PlayAudio("Jump");
			gameState.characterState = STATE_JUMPING;
		}
		else
		{
			obj_character.velocity.x = 0;
			if (gameState.facingright)
			{
				Play::SetSprite(obj_character, "mushroom_idle_right", 0.25f);
			}
			else
			{
				Play::SetSprite(obj_character, "mushroom_idle_left", 0.25f);
			}
		}
	}

	if (!CheckPlatformCollisions(obj_character))
	{
		gameState.characterState = STATE_FALLING;
	}

	Play::DrawObject(obj_character);
}

void Walking()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	int playerspeed = 7;
	FacingRight();
	if (Play::KeyDown(VK_LEFT))
	{
		obj_character.velocity = { -playerspeed,0 };
		Play::SetSprite(obj_character, "mushroom_walk_left", 0.25f);

	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		obj_character.velocity = { playerspeed,0 };
		Play::SetSprite(obj_character, "mushroom_walk_right", 0.25f);
	}
	else
	{
		obj_character.velocity.x = 0;
		gameState.characterState = STATE_STANDING;
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		Play::PlayAudio("Jump");
		obj_character.velocity.y = -13;
		gameState.characterState = STATE_JUMPING;
	}

			//Walking in to side of platform
			/*if (obj_character.velocity.x > 0)
				obj_character.pos.x = obj_platform.pos.x - character_width / 2;
			else if (obj_character.velocity.x < 0)
				obj_character.pos.x = obj_platform.pos.x + platform_width + character_width / 2;
			obj_character.velocity = { 0,0 };
			gameState.characterState = STATE_STANDING;*/
	
	if (!CheckPlatformCollisions(obj_character))
	{
		gameState.characterState = STATE_FALLING;
	}

	Play::DrawObject(obj_character);
	
}

void Jumping()
{
	float character_width = Play::GetSpriteWidth("mushroom");
	int platform_width = Play::GetSpriteWidth("platform");
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	obj_character.velocity.y += 0.5;
	FacingRight();

	if (gameState.facingright)
	{
		Play::SetSprite(obj_character, "mushroom_jump_right", 0.25f);
	}
	else
	{
		Play::SetSprite(obj_character, "mushroom_jump_left", 0.25f);
	}

	if (Play::KeyDown(VK_RIGHT))
	{
		Play::SetSprite(obj_character, "mushroom_jump_right", 0.25f);
		obj_character.velocity.x = 4;
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		Play::SetSprite(obj_character, "mushroom_jump_left", 0.25f);
		obj_character.velocity.x = -4;
	}

	if (obj_character.velocity.y >= 0)
	{
		obj_character.velocity.y = 0;
		gameState.characterState = STATE_FALLING;

	}


	/*std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);

		if (CheckPlatformCollisions(obj_character, obj_platform)) {
			obj_character.pos = obj_character.oldPos;

			float normalisedvector = sqrt(obj_character.velocity.x * obj_character.velocity.x + obj_character.velocity.y * obj_character.velocity.y);
			Point2f directionvector = obj_character.velocity * normalisedvector;
			obj_character.velocity = directionvector;

			if (CheckXAxisCollisions(obj_character, obj_platform) && !CheckYAxisCollisions(obj_character,obj_platform))
			{
				if (obj_character.velocity.x > 0)
					obj_character.pos.x = obj_platform.pos.x - character_width / 2;
				else if (obj_character.velocity.x < 0)
					obj_character.pos.x = obj_platform.pos.x + platform_width + character_width / 2;
				obj_character.velocity.x = 0;
			}

			else if (CheckYAxisCollisions(obj_character, obj_platform))
			{
				obj_character.velocity.y = 0;
				gameState.characterState = STATE_FALLING;
			}
		}
	}*/
	CheckPlatformCollisions(obj_character);

	Play::DrawObject(obj_character);
}

void Falling()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	if (gameState.facingright)
	{
		Play::SetSprite(obj_character, "mushroom_fall_right", 0.25f);
	}
	else
	{
		Play::SetSprite(obj_character, "mushroom_fall_left", 0.25f);
	}

	if (obj_character.velocity.y < 13)
	{
		obj_character.velocity.y += 0.5;
	}
	else {
		obj_character.velocity.y = 13;
	}

	if (Play::KeyDown(VK_RIGHT))
	{
		obj_character.velocity.x = 4;
		gameState.facingright = true;
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		obj_character.velocity.x = -4;
		gameState.facingright = false;
	}
	

	if (CheckPlatformCollisions(obj_character))
	{
		gameState.characterState = STATE_STANDING;
		obj_character.velocity.y = 0;
	}

	Play::DrawObject(obj_character);

}

void FacingRight()
{
	if (Play::KeyPressed(VK_RIGHT)) { gameState.facingright = true; }
	if (Play::KeyPressed(VK_LEFT)) { gameState.facingright = false; }
}



void CompletedLevel()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);
	obj_character.velocity = { 0,0 };
	Play::DrawObject(obj_character);
	Play::DrawFontText("64px", "You did it! Press enter to restart the level", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);

	std::vector<int> vCollected = Play::CollectGameObjectIDsByType(TYPE_COLLECTED);
	std::vector<Point2f> SparkleCoordinates = { {467, 50}, {557 ,50} ,{422 ,95}, {512 ,95}, {602 ,95}, {422 ,140}, {602 ,140}, {467 ,185}, {557 ,185}, {512 ,230} };

	for (int id_collected : vCollected)
	{
		GameObject& obj_collected = Play::GetGameObject(id_collected);
		Play::CentreSpriteOrigin("twinkle_star4");
		Play::SetSprite(obj_collected, "twinkle_star4", 0.05f);

		for (int i = 0; i < vCollected.size(); i++)
		{
			if (vCollected.at(i) == id_collected)
			{
				obj_collected.pos = SparkleCoordinates.at(i);

			}
		}

		Play::UpdateGameObject(obj_collected);
		if (obj_character.frame < 100) { Play::DrawObjectTransparent(obj_collected, obj_character.frame * 0.01); }
		else { Play::DrawObject(obj_collected); }
	}
}

void UpdateCharacter()
{
	GameObject& obj_character = Play::GetGameObjectByType(TYPE_CHARACTER);

	switch (gameState.characterState)
	{
	case STATE_START:
		Play::StartAudioLoop("sleepytime");
		obj_character.pos = { 300, 1600 };
		gameState.lives = 3;
		gameState.score = 0;
		DefineCollectables();
		DefineEnemies();
		 gameState.characterState = STATE_APPEAR;
		break;

	case STATE_APPEAR:
		gameState.characterState = STATE_FALLING;

	case STATE_STANDING:
		Standing();
		break;

	case STATE_WALKING:
		Walking();
		break;

	case STATE_JUMPING:
		Jumping();
		break;

	case STATE_FALLING:
		Falling();
		break;

	case STATE_DAMAGED:
		
		if (gameState.lives > 0) {
			Play::PlayAudio("Damage");
			obj_character.pos = { 300, 1600 };
			gameState.characterState = STATE_APPEAR;
		}
		else {
			Play::PlayAudio("Dead");
			gameState.characterState = STATE_DEAD;
		}
		break;
	case STATE_DEAD:
		Play::StopAudioLoop("sleepytime");
		Play::DrawFontText("64px", "Those pesky snails! Press enter to restart the level", { Play::cameraPos.x + DISPLAY_WIDTH / 2, Play::cameraPos.y + DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		if (Play::KeyPressed(VK_RETURN))
		{
			Play::DestroyGameObjectsByType(TYPE_FLAG);
			Play::DestroyGameObjectsByType(TYPE_COLLECTABLE);
			Play::DestroyGameObjectsByType(TYPE_COLLECTED);
			Play::DestroyGameObjectsByType(TYPE_ENEMY);

			gameState.characterState = STATE_START;
		}
		break;

	case STATE_LEVEL_COMPLETE:
		CompletedLevel();
		if (Play::KeyPressed(VK_RETURN))
		{
			Play::DestroyGameObjectsByType(TYPE_FLAG);
			Play::DestroyGameObjectsByType(TYPE_COLLECTABLE);
			Play::DestroyGameObjectsByType(TYPE_COLLECTED);
			Play::DestroyGameObjectsByType(TYPE_ENEMY);
			
			gameState.characterState = STATE_START;
		}
		break;

	
	}

	Play::SetCameraPosition({ 0, obj_character.pos.y - 500 });

	if (Play::cameraPos.y <= 0)
		Play::SetCameraPosition({ 0, 0 });

	if (Play::cameraPos.y >= 1145)
		Play::SetCameraPosition({ 0,1145 });

	if (Play::IsLeavingDisplayArea(obj_character, Play::HORIZONTAL))
	{
		if (!Play::IsVisible(obj_character))
		{
			obj_character.pos.x = DISPLAY_WIDTH - obj_character.oldPos.x;
		}
	}

	if (Play::IsLeavingDisplayArea(obj_character, Play::VERTICAL))
	{
		obj_character.pos.y = obj_character.oldPos.y;
	}
	
	Play::UpdateGameObject(obj_character);
}

/* TO DO
Week 6:
Start of level.
Level completion sequence ( stars fly up to space. Option to move to next level, or restart level)

Jumping up in to platforms, bounces off
Colliding with side of platform (while jumping or falling) will only stop x velocity. Currently makes it jump up a level
*/