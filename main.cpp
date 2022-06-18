#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <io.h>
#include <fstream>
#include <filesystem>

struct Actor	//Player
{
	float x;	//Player's coordinates
	float y;
	unsigned state; //State of the game
	int CurrentHP;	//Player stats
	unsigned maxHP;
	unsigned dmg;
	unsigned block;
	unsigned lvl;
};

struct Rival	//Enemy stats
{
	unsigned CurrentEnemy;
	int hp;
	unsigned maxhp;
	unsigned dmg;
	unsigned def;
	unsigned lvl;
};

struct Map	
{
	unsigned short block;	// block storage cell
	unsigned short obj;	// object storage cell
};

class Game : public olc::PixelGameEngine
{
public:

	enum Enemy
	{
		FLOPPA = 0,
		BINGUS = 1,
		SOGGA = 2,
	};

	enum Tile // Blocks
	{
		EMPTY = 0,
		GRASS = 1,
		WATER = 2,
		BRIDGE = 3,
		ROAD = 4,
		BROKENROAD = 5,
		TREE = 6,
	};

	enum Object
	{
		NULLW = 0,
		SIGN = 1,
	};

	enum Battle		//main menu in battle and its submenus
	{
		MAIN = 0,
		ATTACK = 1,
		HEAL = 2,
		FLEE = 3,
	};

	enum Icon		//Icons of actions in battle
	{
		ATTACKICON = 0,
		HEALICON = 1,
		FLEEICON = 2,
		BITEICON = 3,
		CLAWICON = 4,
		BLOCKICON = 5,
		DODGEICON = 6,
	};

	enum State // State of the game
	{
		NOT_STARTED = 0,
		STARTED = 1,
		OVERWORLD = 2,
		OVERWORLD_MENU = 3,
		PRE_FIGHT = 4,
		FIGHT = 5,
		POST_FIGHT = 6,
		DIED = 7,
	};

	Game() // Class constuctor
	{

		std::ifstream in("./configs/options.cfg");

		if (in.is_open())
		{
			std::cout << "cfg file opened" << std::endl;

			ConfigurationLoad = true;
			std::string param;
			unsigned int value;
			while (!in.eof())
			{
				in >> param;
				in >> value;

				std::cout << "checking : " << param << " which is " << value << std::endl;

				CheckParam(param, value);
			}
			in.close();
			
		}
		else
		{
			BuildMod = false;
			FPS = 60;
			ConfigurationLoad = false;
		}

		TileTypes = 0;

		std::filesystem::path p1{ "./Sprites/WorldSpr" };

		for (auto& p : std::filesystem::directory_iterator(p1))
		{
			++TileTypes;
		}

		ObjTypes = 0;

		std::filesystem::path p2{"./Sprites/ObjSpr"};

		for (auto& p : std::filesystem::directory_iterator(p2))
		{
			++ObjTypes;
		}
		
		BlockTypes = ObjTypes + TileTypes;

		CurrentBlock = 0;

		Enemy.CurrentEnemy = 0;
		Enemy.def = 0;
		Enemy.hp = 0;
		Enemy.lvl = 1;
		Enemy.dmg = 0;
		Enemy.maxhp = 0;

		Menu.x = 1;
		Menu.y = 1;
		Menu.state = 0;
		
		Player.x = 128;
		Player.y = 128;		
		Player.state = 0;
		
		Player.maxHP = 100;
		Player.CurrentHP = Player.maxHP;
		Player.dmg = 12;
		Player.lvl = 6;

		Menu.maxHP = 0;
		Menu.CurrentHP = 0;
		Menu.dmg = 0;
		Menu.lvl = 0;

		FILE* file;
		if (fopen_s(&file, "map.bin", "r") == 0)
		{
			for (int i = 0; i < 256; i++) 
			{
				for (int j = 0; j < 256; j++)
				{
					fread(&World[i][j].block, sizeof(short), 1, file);
					fread(&World[i][j].obj, sizeof(short), 1, file);
				}
			}
			fclose(file);
		}
		else
		{
			std::cout << "MAP READ ERROR" << std::endl;
			for (int i = 0; i < 256; i++)
			{
				for (int j = 0; j < 256; j++)
				{
					World[i][j].block = Tile::WATER;
					World[i][j].obj = Object::NULLW;
				}
			}
			for (int i = 16; i < 240; i++)		
			
			{
				for (int j = 16; j < 240; j++)
				{
					World[i][j].block = 1;
				}
			}

			for (int j = int(Player.y) + 2; j < int(Player.y) + 5; j++)
			{
				for (int i = int(Player.x); i < int(Player.x) + 10; i++)
				{
					World[i][j].block = Tile::WATER;
				}
			}
			for (int j = int(Player.y) + 2; j < int(Player.y) + 5; j++)
			{
				for (int i = int(Player.x) + 4; i < int(Player.x) + 6; i++)
				{
					World[i][j].block = Tile::BRIDGE;
				}
			}
		}
		fAccumulatedTime = 0.0f;
		//sAppName = "Example";
		Player.state = 0;

		BattleChance = 15 / (fTargetFrameTime);

		LPressed = false;
		RPressed = false;
		UpPressed = false;
		DownPressed = false;
		EscPressed = false;
		IPressed = false;
		
		KeyPressed = false;
		
	}

	~Game() // дуструктор класса
	{
	
	}

	bool OnUserCreate() override // вызов при создании окна
	{
		InCredits = false;

		SetPixelMode(olc::Pixel::MASK);

		FillRect(0, 0, ScreenWidth() + 20, ScreenHeight() + 20, olc::BLACK);	//

		TileX = ScreenWidth() / 5;			//Middle of a display
		TileY = ScreenHeight() / 5;
		MiddleX = ScreenWidth() / 2;
		MiddleY = ScreenHeight() / 2;

		std::cout << "size of world = "<< sizeof(World) <<std::endl;
		std::cout << "TileTypes = " << TileTypes << std::endl;
		std::cout << "ObjTypes = " << ObjTypes << std::endl;
// Pointing path for the sprites
		
//Player
		alisaw = std::make_unique<olc::Sprite>("./Sprites/player/playerc.png");	
		alisaFight = std::make_unique<olc::Sprite>("./Sprites/player/BattlePlayer.png");	

//Blocks
		waterw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/waterw.png");		
		grassw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/grassw.png");
		bridgew = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/bridgew.png");
		
		BrokenRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/BrokenRoad.png");
		OldRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/OldRoad.png");
		treew = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/Treew.png");

//Objects		
		signw = std::make_unique<olc::Sprite>("./Sprites/ObjSpr/signw.png");
		NULLw = std::make_unique<olc::Sprite>("./Sprites/ObjSpr/NULLw.png");
		
//Enemies
		floppa = std::make_unique<olc::Sprite>("./Sprites/Enemies/Floppa.png");
		bingus = std::make_unique<olc::Sprite>("./Sprites/Enemies/Bingus.png");
		sogga = std::make_unique<olc::Sprite>("./Sprites/Enemies/Sogga.png");

//Icons
		attackIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconAttack.png");
		blockIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconBlock.png");
		dodgeIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconDodge.png");
		healIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconHeal.png");
		biteIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconBite.png");
		clawIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconClaw.png");
		fleeIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconFlee.png");
		
// Вывод захапанной памяти всей игрой
		std::cout<< "size of game in memory "<< sizeof(*this)<<std::endl;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override // Summons every frame
	{
		// Keyboard controls

		if(GetKey(olc::Key::UP).bPressed)
		{
			UpPressed = true;
			KeyPressed = true;
		}
		if(GetKey(olc::Key::DOWN).bPressed)
		{
			DownPressed = true;
			KeyPressed = true;
		}

		if (GetKey(olc::Key::F).bPressed)
		{
			std::cout << "Enter Player statement" << std::endl;
			std::cin >> Player.state;
			if (Player.state == 3)
			{
				timer = 0;
			}
		}

		if(GetKey(olc::Key::ESCAPE).bPressed)
		{
			EscPressed = true;
			
		}

		if(GetKey(olc::Key::ENTER).bPressed)
		{
			EnterPressed = true;
		}
		else if (GetKey(olc::Key::LEFT).bHeld && (Player.state == State::OVERWORLD))
		{
			LPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::RIGHT).bHeld && (Player.state == State::OVERWORLD))
		{
			RPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::UP).bHeld && (Player.state == State::OVERWORLD))
		{
			UpPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::DOWN).bHeld && (Player.state == State::OVERWORLD))
		{
			DownPressed = true;
			KeyPressed = true;
		}

		if (GetKey(olc::Key::S).bPressed)
		{
			FILE* file;
			if (fopen_s(&file, "map.bin", "w") == 0)
			{
				for (int i = 0; i < 256; i++)
				{
					for (int j = 0; j < 256; j++)
					{
						fwrite(&World[i][j].block, sizeof(short), 1, file);
						fwrite(&World[i][j].obj, sizeof(short), 1, file);
					}
				}
				fclose(file);
				std::cout << "World Saved!" << std::endl;
			}
			else
			{
				std::cout << "MAP FILE WRITE ERROR" << std::endl;
				return false;
			}
		}
		else if (GetKey(olc::Key::I).bPressed)
		{
			IPressed = true;
		}

		// Редактирование мира
		if (GetKey(olc::Key::E).bPressed && (BuildMod == true))
		{
			if(CurrentBlock == 0)
			{
				CurrentBlock = BlockTypes;
			}
			else
			{
				CurrentBlock--;
			}
			std::cout << "cuurent block id = " << CurrentBlock << std::endl;
		}
		if (GetKey(olc::Key::R).bPressed && (BuildMod == true))
		{
			if(CurrentBlock == BlockTypes)
			{
				CurrentBlock = 0;
			}
			else
			{
				CurrentBlock++;
			}
			std::cout << "cuurent block id = " << CurrentBlock << std::endl;
		}

		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else
		{
			return true;
		}
		Clear(olc::BLACK);

		//основной свитч прорисовки игры

		switch (Player.state)
		{
		case State::NOT_STARTED:

			Player.state = State::STARTED; // Главное меню
			break;

		case State::STARTED:
			
			StartMenu();
			break;

		case State::OVERWORLD: //режим открытого мира

			Player.CurrentHP = Player.maxHP;

			Clear(olc::DARK_BLUE); // очистка экран на сплошной цвет

			
			if (IPressed == true)
			{
				if (BuildMod == false)
				{
					BuildMod = true;
					std::cout << "BuildMod activated" << std::endl;
				}
				else if (BuildMod = true)
				{
					BuildMod = false;
					std::cout << "BuildMod DIACTIVATED" << std::endl;
				}
				IPressed = false;
			}
			

			if((EnterPressed == true) && (BuildMod == true))
			{
				if(CurrentBlock < TileTypes)
				{
					World[int(Player.x)][int(Player.y)].block = CurrentBlock;
				}
				else
				{	
					if (CurrentBlock - TileTypes == Object::NULLW)
					{
						World[int(Player.x)][int(Player.y)].obj = NULL;
					}
					else
					{
						World[int(Player.x)][int(Player.y)].obj = CurrentBlock - TileTypes;
					}
				}
				EnterPressed = false;
			} 
			else if(EnterPressed == true)
			{
				Player.state = State::OVERWORLD_MENU;
				EnterPressed = false;
				BuildMod = false;
				Menu.y = 0;
			}

			OverworldControl(fElapsedTime);
			DisplayWorld();
			DisplayPlayer();

			if(BuildMod == true)		//Отображение выбранного блока в режиме редактирования
			{
				DrawBuildHud();
			}
			if((World[int(Player.x)][int(Player.y)].obj == Object::SIGN))
			{
				DisplaySign();
			}

		break;

		case State::OVERWORLD_MENU:		//Пауза
			DisplayWorld();
			DisplayPlayer();
			DisplayMenu();
			
		break;

		case State::PRE_FIGHT:		// Circle transition

			Clear(olc::DARK_BLUE);
			DisplayWorld();
			DisplayPlayer();
			FillCircle(MiddleX, MiddleY, ((timer + sin(timer)) * 4) * 40, olc::BLACK);
			if (timer >= 2.0f)
			{
				PrepareFight();
				timer = 0;
				Player.state = State::FIGHT;
			}
			timer += fElapsedTime;
			break;

		case State::FIGHT:	
			
			Clear(olc::BLACK);
			DisplayFight();

			break;

		case State::POST_FIGHT:		//Победный экран либо умер

			Player.state = State::POST_FIGHT;
			DisplayVictory();
			
		break;
		case State::DIED:
			DisplayDEATH();
		}

		IPressed = false;
		EnterPressed = false;

		return true;
	}

private:

	// variables
	
	std::unique_ptr<olc::Sprite> bridgew;		//Инициализация спрайтов
	std::unique_ptr<olc::Sprite> alisaw;
	std::unique_ptr<olc::Sprite> grassw;
	std::unique_ptr<olc::Sprite> waterw;
	std::unique_ptr<olc::Sprite> signw;
	std::unique_ptr<olc::Sprite> BrokenRoad;
	std::unique_ptr<olc::Sprite> OldRoad;
	std::unique_ptr<olc::Sprite> alisawRight;
	std::unique_ptr<olc::Sprite> treew;
	std::unique_ptr<olc::Sprite> NULLw;
	std::unique_ptr<olc::Sprite> alisaFight;
	std::unique_ptr<olc::Sprite> floppa;
	std::unique_ptr<olc::Sprite> bingus;
	std::unique_ptr<olc::Sprite> sogga;
	std::unique_ptr<olc::Sprite> attackIcon;
	std::unique_ptr<olc::Sprite> blockIcon;
	std::unique_ptr<olc::Sprite> dodgeIcon;
	std::unique_ptr<olc::Sprite> healIcon;
	std::unique_ptr<olc::Sprite> biteIcon;
	std::unique_ptr<olc::Sprite> clawIcon;
	std::unique_ptr<olc::Sprite> fleeIcon;


	float fTargetFrameTime;			//Предсказуемое время кадров
	float fAccumulatedTime;			//Время с момента запуска
	float timer;					//Принимает значение ElapsedTime в момент вызова

	short KeyFrame;		//Делает ничего

	Map World[256][256];		//Матрица для записи мира в файл
	Actor Player;
	Actor Menu;
	Rival Enemy;
	
	unsigned TileX;			//Размеры одного тайла в пикселях ( размер окна / 5 ), размещенного на дисплее
	unsigned TileY;
	unsigned MiddleX;		//Центр прорисовки мира
	unsigned MiddleY;

	unsigned BattleChance;

	
	unsigned TileTypes; // кол во типов тайлов
	unsigned ObjTypes; // кол во типов сущностей

	unsigned BlockTypes;		//Типы блоков (не спрайтов), доступные в режим редактирования

	unsigned CurrentBlock;		//Текущий блок в режиме редактирования

	bool KeyPressed;		//Нажатие любой клавиши
	bool BuildMod;			//Режим редактирования

	bool LPressed;			//Нажатие клавиш
	bool RPressed;
	bool UpPressed;
	bool DownPressed;
	bool EnterPressed;
	bool EscPressed;
	bool IPressed;

	bool ConfigurationLoad;

	unsigned FPS;

	bool InCredits;			//Контрольная переменная для показа титров в главном меню
	//void functions

	void DisplayWorld() // рисование мира
	{
		for (int i = Player.y - 2; i <= Player.y + 2; i++)
		{
			for (int j = Player.x - 2; j <= Player.x + 2; j++)
			{
				DrawSprite((j - (int(Player.x) - 2)) * TileX, (i - (int(Player.y) - 2)) * TileY, GetSprite(World[j][i].block), ((TileX / 48) * 1.0f), 0);
				if(World[j][i].obj != NULL)
				{
					DrawSprite((j - (int(Player.x) - 2)) * TileX, (i - (int(Player.y) - 2)) * TileY, GetObj(World[j][i].obj), ((TileX / 48) * 1.0f), 0);
				}
			}
		}
	}

	void DisplayMenu()	//Пауза в State::OVERWORLD
	{
		FillRect(50, 100 , 150, 250, olc::BLACK);

		if(EscPressed == true)
		{
			Player.state = State::OVERWORLD;
			EscPressed = false;
		}
	}

	void DisplayDEATH()
	{
		DrawString(MiddleX - 50, MiddleY - 30, "YOU DIED", olc::RED, 2);
		DrawString(MiddleX - 50, MiddleY, "ENTER to continue", olc::YELLOW, 1);
		DrawString(MiddleX - 50, MiddleY + 30, "ESC to exit", olc::YELLOW, 1);
			if(EnterPressed == true)
			{
				Player.state = State::STARTED;
				EnterPressed = true;
			}
			if(EscPressed == true)
			{
				exit(0);
			}
	}

	void DisplayVictory()		//Вызов в Player.state = State::POSTFIGHT
	{
		DrawString(MiddleX - 50, MiddleY - 20, "Victory", olc::WHITE, 2);
		DrawString(MiddleX - 100, MiddleY + 10, "Press Enter to continue", olc::YELLOW, 1);
		if(EnterPressed == true)
		{
			Player.state = State::OVERWORLD;
			EnterPressed = false;
		}
	}

	void DisplayFightHud(std::string first, std::string second, std::string third)		//Подсвечивает выбранное действие
	{
		switch(int(Menu.y))
				{
					case 1:
						DrawString(40, 360, first, olc::YELLOW, 2);
						DrawString(32, 380, second, olc::WHITE, 2);
						DrawString(32, 400, third, olc::WHITE, 2);
						break;
					case 2:
						DrawString(32, 360, first, olc::WHITE, 2);
						DrawString(40, 380, second, olc::YELLOW, 2);
						DrawString(32, 400, third, olc::WHITE, 2);
						break;
					case 3:
						DrawString(32, 360, first, olc::WHITE, 2);
						DrawString(32, 380, second, olc::WHITE, 2);
						DrawString(40, 400, third, olc::YELLOW, 2);
						break;
				}
	}

	void PrepareFight()		//Рандомный enemy и выставление около рандомных статов
	{
		
		Enemy.CurrentEnemy = rand() % 3 + 0;
		Menu.state = Battle::MAIN;
		Menu.y = 2.0f;
		std::cout << "Current enemy: ";
		Enemy.lvl = rand() % 4 + 1;
		switch(Enemy.CurrentEnemy)
		{
			case Enemy::FLOPPA:
				std::cout<<"Floppa"<<std::endl;
				Enemy.def = Enemy.lvl * 3;
				Enemy.hp = Enemy.lvl * 10 + (rand() % 10 + 0);
				Enemy.dmg = Enemy.lvl * 3 + (rand() % 6 + 0);
				Enemy.maxhp = Enemy.hp;
			break;

			case Enemy::BINGUS:
				std::cout<<"Bingus"<<std::endl;
				Enemy.def = 0;
				Enemy.hp = Enemy.lvl * 3 + (rand() % 10 + 0);
				Enemy.dmg = Enemy.lvl * 6 + (rand() % 10 + 0);
				Enemy.maxhp = Enemy.hp;
			break;

			case Enemy::SOGGA:
				std::cout<<"Sogga"<<std::endl;
				Enemy.def = int(Enemy.lvl * 1.5f) - 1;
				Enemy.hp = Enemy.lvl * 10 + (rand() % 10 + 0);
				Enemy.dmg = (Enemy.lvl * 6)/2 + (rand() % 2 + 0);
				Enemy.maxhp = Enemy.hp;
			break;
		}
		std::cout << "level : "<<Enemy.lvl<<std::endl;
		std::cout << "health point : "<<Enemy.hp<<std::endl;
		std::cout << "defense : "<<Enemy.def<<std::endl;
		std::cout << "damage : "<<Enemy.dmg<<std::endl;
		
	}

	void DisplayFight()		
	{
    	DisplayPlayer();
		DisplayEnemy();

		DrawRect(2, 350, 477, 100, olc::WHITE);		//UI
		DrawRect(3, 351, 476, 100, olc::WHITE);
		FillRect(355, 361, Player.CurrentHP, 19 , olc::RED);	//Player stats
		for (int i = 0; i < Player.lvl; i++)
		{
			FillRect(356 + (i*4) + i, 391, 4, 19, olc::BLUE);
		}
		DrawRect(355, 360, 100, 20, olc::WHITE);
		DrawRect(355, 390, 100, 20, olc::WHITE);
		
		DrawRect(200, 36, (Enemy.maxhp * 3) + 2, 20, olc::WHITE);		//Enemy stats
		FillRect(201, 37, (Enemy.hp * 3), 19, olc::RED);
		DrawString(250, 40, "lvl " + std::to_string(Enemy.lvl), olc::YELLOW, 1);		

		DrawString(280, 370, std::to_string(Player.CurrentHP) + "/" + std::to_string(Player.maxHP) + "HP", olc::WHITE, 1);	//More Player stats
		DrawString(300, 400, "lvl " + std::to_string(Player.lvl), olc::WHITE, 1);
		
		switch (Menu.state)
		{
			case Battle::MAIN:
				DisplayFightHud("ATTACK", "HEAL", "FLEE");
				DrawSprite(12, 360, GetIcon(Icon::ATTACKICON), 1);
				DrawSprite(12, 380, GetIcon(Icon::HEALICON), 1);
				DrawSprite(12, 400, GetIcon(Icon::FLEEICON), 1);
				
			break;
			case Battle::ATTACK:
				DisplayFightHud("SLASH", "BITE", "CLAW");
				DrawSprite(12, 360, GetIcon(Icon::ATTACKICON), 1);
				DrawSprite(12, 380, GetIcon(Icon::BITEICON), 1);
				DrawSprite(12, 400, GetIcon(Icon::CLAWICON), 1);
			break;
			case Battle::HEAL:
				DisplayFightHud("Small Potion", "Medium Potion", "Huge Potion");
				DrawSprite(12, 360, GetIcon(Icon::HEALICON), 1);
			
		}

		if(KeyPressed == true)		//Выбор меню
		{
			if((UpPressed == true) && ((Menu.y - 1.0f) > 0.0f))
			{
				Menu.y -= 1.0f;
				UpPressed = false;
			}
			if((DownPressed == true) && ((Menu.y + 1.0f) < 4.0f))
		    {
				Menu.y += 1.0f;
				DownPressed = false;
			}
			KeyPressed = false;
		}

		if(EnterPressed == true)		//Действия и подменю, нанесение урона и т.д. и т.п.
		{
			switch(Menu.state)
			{
				case Battle::MAIN:
					Menu.state = int(Menu.y);	
					if(Menu.state == Battle::FLEE)
					{
						Menu.state = Battle::MAIN;
						Player.state = State::OVERWORLD;
					}		
				break;	
					
				case Battle::ATTACK:
					Menu.state = Battle::MAIN;
					switch(int(Menu.y))
					{
						case 1: // Slash
							Enemy.hp -= ((Player.dmg * 3)/2) - (Enemy.def / 2);
							Player.CurrentHP -= Enemy.dmg;
						break;

						case 2: // Bite
							if(Enemy.def == 0)
							{
								Enemy.hp -= Player.dmg;
								Player.CurrentHP -= Enemy.dmg;
							}
							else
							{
								Player.CurrentHP -= Enemy.dmg;
							}
						break;

						case 3: // Claw
							Enemy.hp -=Player.dmg - Enemy.def;
							Player.CurrentHP -= Enemy.dmg;
						break;
					}
					Menu.state = Battle::MAIN;
					if(Enemy.hp < 0)
					{
						Player.state = State::POST_FIGHT;
					}
					if(Player.CurrentHP <= 0)
					{
						Player.state = State::DIED;
					}
				break;
				case Battle::HEAL:
					Menu.state = Battle::MAIN;
					switch(int(Menu.y))
					{
						case 1:	//Small Potion
							if(Player.CurrentHP + 10 < Player.maxHP)
							{
								Player.CurrentHP += 10;
							} else 
							{
								Menu.state = Battle::MAIN;
							}
							
							break;
						case 2:		//Medium Potion
							if(Player.CurrentHP + 25 < Player.maxHP)
							{
							Player.CurrentHP += 25;
							} else 
							{
								Menu.state = Battle::MAIN;
							}
							break;
						case 3:		//Huge Potion
							if(Player.CurrentHP + 50 < Player.maxHP)
							{
							Player.CurrentHP += 50;
							} else 
							{
								Menu.state = Battle::MAIN;
							}
							break;
							
					}
						if(Player.CurrentHP > Player.maxHP)
						{
							Player.state = State::DIED;
						}
				break;
			}
			EnterPressed = false;
		}	
	}

	olc::Sprite* GetEnemy(unsigned SpriteID)		//Enemy sprite
	{
		switch(SpriteID)
		{
			case Enemy::FLOPPA:
				return floppa.get();
			break;
			case Enemy::BINGUS:
				return bingus.get();
			break;
			case Enemy::SOGGA:
				return sogga.get();
			break;
			default:
				return NULLw.get();
			break;
		}
	}

	olc::Sprite* GetIcon(unsigned SpriteID)		//Возвращает (нет блин отбирает) спрайты иконок в битве
	{
		switch(SpriteID)
		{
			case Icon::ATTACKICON:
				return attackIcon.get();
				break;
			case Icon::HEALICON:
				return healIcon.get();
				break;
			case Icon::FLEEICON:
				return fleeIcon.get();
				break;
			case Icon::BITEICON:
				return biteIcon.get();
				break;
			case Icon::CLAWICON:
				return clawIcon.get();
				break;
			case Icon::BLOCKICON:
				return blockIcon.get();
				break;
			case Icon::DODGEICON:
				return dodgeIcon.get();
				break;
			default:
				return NULLw.get();
				break;
		}
	}

	void StartMenu()		//Отображает меню
	{
		Clear(olc::BLACK);

		if(InCredits == true)
		{
			DrawString(10, MiddleY - 20, "Developed by Overclocking_Infernale with help of Vladimir_Maks", olc::WHITE, 1);
			DrawString(10, MiddleY, "PixelGameEngine by OneLineCoder", olc::WHITE, 1);
			DrawString(25, MiddleY - 60, "Press Enter to return", olc::YELLOW, 1);
			if(EnterPressed == true)
			{
				InCredits = false;
				EnterPressed = false;
			}
		}
		else
		{
			DrawString(MiddleX - 110, MiddleY - 80, "AlisaG", olc::GREY, 5);
			DrawString(MiddleX - 40, MiddleY, "PLAY", olc::WHITE, 2);
			DrawString(MiddleX - 40, MiddleY + 40, "CREDITS", olc::WHITE, 2);
			DrawString(MiddleX - 40, MiddleY + 80, "QUIT", olc::WHITE, 2);
			DrawSprite(MiddleX + 110, MiddleY - 79, alisaw.get(), 1, 0);
		
			if (Menu.y >= 2.5f)
			{
				DrawString(MiddleX - 40, MiddleY + 80, "QUIT", olc::YELLOW, 2);
			} 
			else if (Menu.y >= 1.5f)
			{
				DrawString(MiddleX - 40, MiddleY + 40, "CREDITS", olc::YELLOW, 2);
			}
			else if (Menu.y >= 0.5f)
			{
				DrawString(MiddleX - 40, MiddleY, "PLAY", olc::YELLOW, 2);
			}
		}
		if(KeyPressed == true)		//Выбор меню
		{
			if((UpPressed == true) && ((Menu.y - 1.0f) > 0.0f))
			{
				Menu.y -= 1;
				UpPressed = false;
			}
			if((DownPressed == true) && ((Menu.y + 1.0f) < 4.0f))
			{
				Menu.y += 1;
				DownPressed = false;
			}
		}
		if((EnterPressed == true) && ((Menu.y >= 0.5f) && (Menu.y < 1.5f)))
		{
			Player.state = 2;
			EnterPressed = false;
		}
		if((EnterPressed == true) && ((Menu.y >= 1.5f) && (Menu.y < 2.5f)))
		{
			InCredits = true;
			EnterPressed = false;
		}
		if((EnterPressed == true) && ((Menu.y >= 2.5f) && (Menu.y < 4.0f)))
		{
			exit(0);
		}
	}

	void OverworldControl(float fElapsedTime)	//Управление/Перемещение по миру
	{
		if (KeyPressed == true)		
		{
			if (LPressed == true && (CollisionDetection(World[int(Player.x - 1)][int(Player.y)].block) != false))
			{
				Player.x -= 2.5f * fElapsedTime;
				LPressed = false;
			}
			else if (RPressed == true && (CollisionDetection(World[int(Player.x + 1)][int(Player.y)].block) != false))
			{
				Player.x += 2.5f * fElapsedTime;
				RPressed = false;
			}
			else if (UpPressed == true && (CollisionDetection(World[int(Player.x)][int(Player.y - 1)].block) != false))
			{
				Player.y -= 2.5f * fElapsedTime;
				UpPressed = false;
			}
			else if (DownPressed == true && (CollisionDetection(World[int(Player.x)][int(Player.y + 1)].block) != false))
			{
				Player.y += 2.5f * fElapsedTime;
				DownPressed = false;
			}
			KeyPressed = false;

			if(World[int(Player.x)][int(Player.y)].block == Tile::GRASS && (BuildMod == false))
			{
				unsigned random = rand() % BattleChance + 1;
				std::cout << random << std::endl;
				if(random <= 1)
				{
					Player.state = State::PRE_FIGHT;
				}	
			}
		}
	}

	void DisplayPlayer()		//Отображение спрайта игрока
	{
		if(Player.state == State::FIGHT)
		{
			DrawSprite(MiddleX - (2 * TileX), MiddleY - (TileY / 48), alisaFight.get(), TileX / 18);
		}
		else
		{
			DrawSprite(MiddleX - 16, MiddleY - 16, alisaw.get(), (TileX / 16) / 2);
		}
	}
	
	void DisplayEnemy()		//Дисплеит энеми
	{
		DrawSprite(MiddleX + TileX, MiddleY - (TileY * 2), GetEnemy(Enemy.CurrentEnemy), TileX/32);
	}

	void DrawBuildHud()			//Отображает спрайт выбранного блока в режиме редактирования/Показывает координаты игрока
	{
		
		FillRect(0, 0, 480, 40, olc::BLACK);
		DrawString(5,20, "Plr.X = " + std::to_string(int(Player.x)), olc::WHITE, 1.0f);
		DrawString(5,30, "Plr.Y = " + std::to_string(int(Player.y)), olc::WHITE, 1.0f);
		DrawString(100, 20, "CurBlock = " + std::to_string(int(World[int(Player.x)][int(Player.y)].block)), olc::WHITE, 1.0f);
		DrawString(100, 30, "CurObj = " + std::to_string(int(World[int(Player.x)][int(Player.y)].obj)), olc::WHITE, 1.0f);
		FillRect(416,17, 56, 56 , olc::DARK_GREEN);
		
		if (CurrentBlock < TileTypes)
		{
			DrawSprite(420, 20, GetSprite(CurrentBlock), 0.5f, 0);
		}
		else
		{
			DrawSprite(420, 20, GetObj(CurrentBlock - TileTypes), 0.5f, 0);
		}
		
	}

	void DisplaySign()		//Отображает текст таблички/энтити в специальном окне
	{
		FillRect(20,360,440,440,olc::BLACK);
		DrawString(30,370, GetSign(), olc::WHITE, 2);
	}

	olc::Sprite* GetSprite(unsigned SpriteID)		//Возвращает спрайт при прорисовке мира
	{
		switch(SpriteID)
		{
			case Tile::EMPTY:
				return waterw.get();
				break;
			case Tile::GRASS:
				return grassw.get();
				break;
			case Tile::WATER:
				return waterw.get();
				break;
			case Tile::BRIDGE:
				return bridgew.get();
				break;
			case Tile::ROAD:
				return OldRoad.get();
				break;
			case Tile::BROKENROAD:
				return BrokenRoad.get();
				break;
			case Tile::TREE:
				return treew.get();
				break;
			default:
				return waterw.get();
		}
	}

	olc::Sprite* GetObj(unsigned SpriteID)		//Возвращает спрайт при прорисовке мира
	{
		switch (SpriteID)
		{
		case Object::SIGN:
			return signw.get();
			break;
		default:
			return NULLw.get();
			break;
		}
	}


	bool CollisionDetection(unsigned block)		//Проверка на коллизии/отключение коллизий в режиме редактирования
	{
		if (BuildMod == true)
		{
			return true;
		}

		switch (block)
		{
		case Tile::WATER:
		case Tile::EMPTY:
		case Tile::TREE:
			return false;
			break;

		default:
			return true;
			break;
		}
	}

	std::string GetSign()			//Возвращает текст таблички из файла(?)
	{
		std::string note = "Hello, im a sign!";
		if ((int(Player.x) == 128) && (int(Player.y) == 126))
		{
			note = "Arigato";
		}
		return note;
		
	}

	bool IsBlock(unsigned block)		// Удаляет объекты/энтити в режиме редактирования
	{
		switch(block)
		{
			case Object::NULLW:
			case Object::SIGN:
				return false;
			break;
			default:
				return true;
			break;
		}
	}

	void CheckParam(std::string param, unsigned int value)
	{
		if (param == "fps")
		{
			if ((value > 0) && (value < 1000))
			{
				fTargetFrameTime = 1.0f / float(value);
			}
			else
			{
				fTargetFrameTime = 1.0f / 60.0f;
			}
		}
		else if (param == "debug_mode")
		{
			if ((value >= 0) && (value <= 1))
			{
				BuildMod = value;
			}
		}
	}
	
};

int main()
{
	srand(time(0));
	Game go;
	if (go.Construct(480, 480, 2, 2))		//Creates a window with certain size
	{
		go.Start();
	}

	return 0;
}