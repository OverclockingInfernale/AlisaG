#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <io.h>

struct Actor// упрощения хранения координат игрока
{
	float x;
	float y;
	unsigned state; // отвечает за текущее состояние игрока (enum Player) 
	
};

struct Map
{
	unsigned block;// ячейка хранения блоков
	unsigned obj;// ячейка хранения объектов
};

class Game : public olc::PixelGameEngine
{
public:

	enum Tile // енам мира
	{
		EMPTY = 0,
		GRASS = 1,
		WATER = 2,
		BRIDGE = 3,
		ROAD = 4,
		BROKENROAD = 5,
		SIGN = 6,
		NULLW = 7,
		TREE = 8,
	};

	enum Battle
	{
		MAIN = 0,
		ATTACK = 1,
		HEAL = 2,
		FLEE = 3,
	};
	enum Icon
	{
		ATTACKICON = 0,
		BLOCKICON = 1,
		DODGEICON = 2,
	};

	enum Player // енам состояния игрока
	{
		FREE = 0,
		IN_MENU = 1,
		IN_FIGHT = 2,
		DIED = 3,
	};

	enum State // енам состояния игры
	{
		NOT_STARTED = 0,
		STARTED = 1,
		OVERWORLD = 2,
		PRE_FIGHT = 3,
		FIGHT = 4,
	};

	Game() // конструктор класса
	{
		BlockTypes = 8;
		CurrentBlock = 0;


		Menu.x = 1;
		Menu.y = 1;
		Menu.state = 0;
		Player.x = 128;
		Player.y = 128;
		Player.state = 0;

		FILE* file;
		if (fopen_s(&file, "map.bin", "r") == 0)
		{
			for (int i = 0; i < 256; i++) 
			{
				for (int j = 0; j < 256; j++)
				{
					fread(&World[i][j].block, sizeof(unsigned), 1, file);
					fread(&World[i][j].obj, sizeof(unsigned), 1, file);
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
					World[i][j].obj = Tile::NULLW;
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
		fTargetFrameTime = 1.0f / 60.0f;
		fAccumulatedTime = 0.0f;
		sAppName = "Example";
		Player.state = 0;

		

		LPressed = false;
		RPressed = false;
		UpPressed = false;
		DownPressed = false;

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

		TileX = ScreenWidth() / 5;			//Переменные середины дисплея
		TileY = ScreenHeight() / 5;
		MidleX = ScreenWidth() / 2;
		MidleY = ScreenHeight() / 2;

		std::cout << "size of world = "<< sizeof(World) <<std::endl; 

		BuildMod = false;

		std::cout << "Tile x = " << TileX << std::endl;	//Типо дебаг
		std::cout << "Tile y = " << TileY << std::endl;
		std::cout << "Midle X = " << MidleX << std::endl;
		std::cout << "Midle Y = " << MidleY << std::endl;
		std::cout << "Player.x = " << Player.x << std::endl;
		std::cout << "Player.y = " << Player.y << std::endl;
		std::cout << "Player pos = " << MidleX - (TileX / 4) << "   " << MidleY - (TileY / 4) << "   " << MidleX << "   " << MidleY << std::endl;

		// указание пути размещения спрайтов

		alisaw = std::make_unique<olc::Sprite>("./Sprites/player/playerc.png");
		waterw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/waterw.png");
		grassw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/grassw.png");
		bridgew = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/bridgew.png");
		signw = std::make_unique<olc::Sprite>("./Sprites/ObjSpr/signw.png");
		BrokenRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/BrokenRoad.png");
		OldRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/OldRoad.png");
		treew = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/Treew.png");
		NULLw = std::make_unique<olc::Sprite>("./Sprites/ObjSpr/NULLw.png");
		alisaFight = std::make_unique<olc::Sprite>("./Sprites/player/BattlePlayer.png");
		floppa = std::make_unique<olc::Sprite>("./Sprites/Enemies/Floppa.png");
		attackIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconAttack.png");
		blockIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconBlock.png");
		dodgeIcon = std::make_unique<olc::Sprite>("./Sprites/BattleIcons/IconDodge.png");
		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override // вызов каждый божий кадр
	{
		

		//std::cout << "AccumulatedTime = "<< fAccumulatedTime<<std::endl;
		//std::cout << "ElapsedTime = " << fElapsedTime<<std::endl;
	


		// ввод с клавы

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
						fwrite(&World[i][j].block, sizeof(unsigned), 1, file);
						fwrite(&World[i][j].obj, sizeof(unsigned), 1, file);
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
		}
		if (GetKey(olc::Key::R).bPressed && BuildMod)
		{
			if(CurrentBlock == BlockTypes)
			{
				CurrentBlock = 0;
			}
			else
			{
				CurrentBlock++;
			}
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

		//обработка логики мира
		switch (Player.state)
		{
		case State::NOT_STARTED:

			Player.state = State::STARTED; // Главное меню
			break;

		case State::STARTED:
			
			StartMenu();
			break;

		case State::OVERWORLD: //режим открытого мира

			Clear(olc::DARK_BLUE); // очистка экран на сплошной цвет

			if(GetKey(olc::Key::I).bPressed)
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
			}

			if((EnterPressed == true) && (BuildMod == true))
			{
				if(IsBlock(CurrentBlock) == true)
				{
					World[int(Player.x)][int(Player.y)].block = CurrentBlock;
				}
				else
				{	
					if (CurrentBlock == Tile::NULLW)
					{
						World[int(Player.x)][int(Player.y)].obj = NULL;
					}
					else
					{
						World[int(Player.x)][int(Player.y)].obj = CurrentBlock;
					}
				}
				
				EnterPressed = false;
				
			}

			OverworldControl(fElapsedTime);
			DisplayWorld();
			DisplayPlayer();

			if(BuildMod == true)		//Отображение выбранного блока в режиме редактирования
			{
				DrawBuildHud();
			}
			if((World[int(Player.x)][int(Player.y)].obj == 6))
			{
				DisplaySign();
			}

			break;

		case State::PRE_FIGHT:		// всплывающий круг перед битвой

			
			Clear(olc::DARK_BLUE);
			DisplayWorld();
			DisplayPlayer();
			FillCircle(MidleX,MidleY, ((timer + sin(timer)) * 4) * 40, olc::BLACK);
			if (timer >= 2.0f)
			{
				timer = 0;
				Player.state = State::FIGHT;
			}
			timer += fElapsedTime;
			break;

		case State::FIGHT:		// режим битвы (В РАЗРАБОТКЕ)
			
			Clear(olc::BLACK);
			DisplayFight();

			break;
		}
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
	std::unique_ptr<olc::Sprite> attackIcon;
	std::unique_ptr<olc::Sprite> blockIcon;
	std::unique_ptr<olc::Sprite> dodgeIcon;

	float fTargetFrameTime;			//Предсказуемое время кадров
	float fAccumulatedTime;			//Время с момента запуска
	float timer;					//Принимает значение ElapsedTime в момент вызова

	short KeyFrame;

	Map World[256][256];		//Матрица для записи мира в файл
	Actor Player;
	Actor Menu;
	
	unsigned TileX;			//Размеры одного тайла, размещенного на дисплее
	unsigned TileY;
	unsigned MidleX;		//Центр прорисовки мира
	unsigned MidleY;

	unsigned BlockTypes;		//Типы блоков (не спрайтов), доступные в режим редактирования
	unsigned CurrentBlock;		//Текущий блок в режиме редактирования

	bool KeyPressed;		//Нажатие любой клавиши
	bool BuildMod;			//Режим редактирования

	bool LPressed;			//Нажатие клавиш
	bool RPressed;
	bool UpPressed;
	bool DownPressed;
	bool EnterPressed;


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
					DrawSprite((j - (int(Player.x) - 2)) * TileX, (i - (int(Player.y) - 2)) * TileY, GetSprite(World[j][i].obj), ((TileX / 48) * 1.0f), 0);
				}
			}
		}
	}

	void DisplayFight()
	{
		
		DisplayPlayer();
		DisplayEnemy();

		DrawRect(2, 350, 477, 435, olc::WHITE);
		DrawRect(3, 351, 476, 435, olc::WHITE);
		
		
		switch (Menu.state)
		{
			case Battle::MAIN:
				switch(int(Menu.y))
				{
					case 1:
						DrawString(40, 360, "ATTACK", olc::YELLOW, 2);
						DrawString(32, 380, "HEAL", olc::WHITE, 2);
						DrawString(32, 400, "FLEE", olc::WHITE, 2);
						DrawSprite(12, 360, GetIcon(Icon::ATTACKICON), 2);
						break;
					case 2:
						DrawString(32, 360, "ATTACK", olc::WHITE, 2);
						DrawString(40, 380, "HEAL", olc::YELLOW, 2);
						DrawString(32, 400, "FLEE", olc::WHITE, 2);
						break;
					case 3:
						DrawString(32, 360, "ATTACK", olc::WHITE, 2);
						DrawString(32, 380, "HEAL", olc::WHITE, 2);
						DrawString(40, 400, "FLEE", olc::YELLOW, 2);
						break;
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
					if(EnterPressed == true)
					{
						Menu.state = int(Menu.y);
						EnterPressed = false;
					}
					KeyPressed = false;
				}
			break;
		}
	}

	olc::Sprite* GetIcon(unsigned SpriteID)
	{
		switch(SpriteID)
		{
			case Icon::ATTACKICON:
				return attackIcon.get();
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
			DrawString(10, MidleY - 20, "Game Developed by Vladimir_Maks, Overclocking_Infernale", olc::WHITE, 1);
			DrawString(10, MidleY, "PixelGameEngine by OneLineCoder", olc::WHITE, 1);
			DrawString(25, MidleY - 60, "Press Enter to return", olc::YELLOW, 1);
			if(EnterPressed == true)
			{
				InCredits = false;
				EnterPressed = false;
			}
		}
		else
		{
			DrawString(MidleX - 110, MidleY - 80, "AlisaG", olc::GREY, 5);
			DrawString(MidleX - 40, MidleY, "PLAY", olc::WHITE, 2);
			DrawString(MidleX - 40, MidleY + 40, "CREDITS", olc::WHITE, 2);
			DrawString(MidleX - 40, MidleY + 80, "QUIT", olc::WHITE, 2);
			DrawSprite(MidleX + 110, MidleY - 79, alisaw.get(), 1, 0);
		
			if (Menu.y >= 2.5f)
			{
				DrawString(MidleX - 40, MidleY + 80, "QUIT", olc::YELLOW, 2);
			} 
			else if (Menu.y >= 1.5f)
			{
				DrawString(MidleX - 40, MidleY + 40, "CREDITS", olc::YELLOW, 2);
			}
			else if (Menu.y >= 0.5f)
			{
				DrawString(MidleX - 40, MidleY, "PLAY", olc::YELLOW, 2);
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

			if( World[int(Player.x)][int(Player.y)].block == Tile::GRASS)
			{
				unsigned random = rand() % 999 + 1;
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
		
		if(Player.state == State::OVERWORLD)
		{
			DrawSprite(MidleX - 16, MidleY - 16, alisaw.get(), (TileX / 16) / 2);
		}
		else if(Player.state == State::FIGHT)
		{
			DrawSprite(MidleX - (2 * TileX), MidleY - (TileY / 48), alisaFight.get(), TileX / 18);
		}
	}
	
	void DisplayEnemy()
	{
		DrawSprite(MidleX + TileX, MidleY - (TileY * 2), floppa.get(), TileX/32);
	}

	void DrawBuildHud()			//Отображает спрайт выбранного блока в режиме редактирования/Показывает координаты игрока
	{
		
		FillRect(0, 0, 480, 40, olc::BLACK);
		DrawString(5,20, "Plr.X = " + std::to_string(int(Player.x)), olc::WHITE, 1.0f);
		DrawString(5,30, "Plr.Y = " + std::to_string(int(Player.y)), olc::WHITE, 1.0f);
		FillRect(416,17, 440, 57 , olc::DARK_GREEN);
		DrawSprite(420, 20, GetSprite(CurrentBlock), 0.5f, 0);
		
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
			case Tile::SIGN:
				return signw.get();
				break;
			case Tile::TREE:
				return treew.get();
				break;
			case Tile::NULLW: // Нуллв это спрайт никакого объекта тоесть ну ничего
				return NULLw.get();
			default:
				return waterw.get();
		}
	}
	

	bool CollisionDetection(unsigned block)			//Проверка на коллизии/отключение коллизий в режиме редактирования
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

	std::string GetSign()			//Возвращает текст таблички из файла
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
			case Tile::SIGN:
			case Tile::NULLW:
				return false;
			break;
			default:
				return true;
			break;
		}
	}
	
};

int main()
{
	srand(time(0));
	Game go;
	if (go.Construct(480, 480, 2, 2))		//Задает размеры видимой области игрового мира, а так же включает vsync
	{
		go.Start();
	}

	return 0;
}