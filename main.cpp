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
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

		TileX = ScreenWidth() / 5;			//Переменные середины дисплея
		TileY = ScreenHeight() / 5;
		MidleX = ScreenWidth() / 2;
		MidleY = ScreenHeight() / 2;

		std::cout << "size of world = "<< sizeof(World) <<std::endl; 

		BuildMod = false;

		std::cout << "Tile x = " << TileX << std::endl;		//Типо дебаг
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
		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override // вызов каждый божий кадр
	{
		

		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		//std::cout << "AccumulatedTime = "<< fAccumulatedTime<<std::endl;
		//std::cout << "ElapsedTime = " << fElapsedTime<<std::endl;
	


		// ввод с клавы

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

		if (GetKey(olc::Key::F).bPressed)
		{
			std::cout << "Enter Player statement" << std::endl;
			std::cin >> Player.state;
			if (Player.state == 3)
			{
				frame = 0;
			}
		}

		if(GetKey(olc::Key::ENTER).bPressed)
		{
			EnterPressed = true;
		}
		else if (GetKey(olc::Key::LEFT).bHeld)
		{
			LPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::RIGHT).bHeld)
		{
			RPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::UP).bHeld)
		{
			UpPressed = true;
			KeyPressed = true;
		}
		else if (GetKey(olc::Key::DOWN).bHeld)
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

		//обработка логики мира
		switch (Player.state)
		{
		case State::NOT_STARTED:

			Player.state = 1; // загатовка для главного меню
			break;

		case State::STARTED:

			Player.state = 2; // загатовка для катсцени или типо того

			break;

		case State::OVERWORLD: //режим открытого мира

			Clear(olc::DARK_BLUE); // очистка экран на сплошной цвет

			OverworldControl(fElapsedTime);
			DisplayWorld();
			DisplayPlayer();

			if(BuildMod == true)
			{
				DrawBuildHud();
			}
			if((World[int(Player.x)][int(Player.y)].obj == 6))
			{
				DisplaySign();
			}

			break;
		case State::PRE_FIGHT:
			Clear(olc::DARK_BLUE);
			DisplayWorld();
			DisplayPlayer();
			for (int i = 0; i <= 480; i++)
			{
				DrawLine(i - 2300 + ((frame* frame) / 8), i, (i + 20) + ((frame * frame) / 8), i, olc::BLACK); // переделать функцию
			}
			if (frame == 60)
			{
				frame = 0;
				Player.state = State::FIGHT;
			}
			frame++;
			break;
		case State::FIGHT:// режим битвы
			
			Clear(olc::BLACK);
			DrawString(MidleX,	MidleY, "FIGHT MODE", olc::WHITE, 2);
			break;
		}

		return true;
	}

private:

	// variables
	Actor Player;
	std::unique_ptr<olc::Sprite> bridgew;
	std::unique_ptr<olc::Sprite> alisaw;
	std::unique_ptr<olc::Sprite> grassw;
	std::unique_ptr<olc::Sprite> waterw;
	std::unique_ptr<olc::Sprite> signw;
	std::unique_ptr<olc::Sprite> BrokenRoad;
	std::unique_ptr<olc::Sprite> OldRoad;
	std::unique_ptr<olc::Sprite> alisawRight;
	std::unique_ptr<olc::Sprite> treew;
	std::unique_ptr<olc::Sprite> NULLw;

	float fTargetFrameTime;
	float fAccumulatedTime;

	short frame;
	short KeyFrame;

	Map World[256][256];

	unsigned TileX;
	unsigned TileY;
	unsigned MidleX;
	unsigned MidleY;

	unsigned BlockTypes;
	unsigned CurrentBlock;

	bool KeyPressed;
	bool BuildMod;

	bool LPressed;
	bool RPressed;
	bool UpPressed;
	bool DownPressed;
	bool EnterPressed;
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
					SetPixelMode(olc::Pixel::MASK);
					DrawSprite((j - (int(Player.x) - 2)) * TileX, (i - (int(Player.y) - 2)) * TileY, GetSprite(World[j][i].obj), ((TileX / 48) * 1.0f), 0);
					SetPixelMode(olc::Pixel::NORMAL);
				}
			}
		}
	}

	void OverworldControl(float fElapsedTime)
	{
		if (KeyPressed == true)		//Управление/Перемещение по миру
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
		}
	}

	void DisplayPlayer()		//Рисование
	{
		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(MidleX - 16, MidleY - 16, alisaw.get(), (TileX / 16) / 2);
		SetPixelMode(olc::Pixel::NORMAL);
	}

	void DrawBuildHud()
	{
		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(440,20, GetSprite(CurrentBlock), 0.5f, 0);
		SetPixelMode(olc::Pixel::NORMAL);
	}

	void DisplaySign()
	{
		FillRect(20,360,440,440,olc::BLACK);
		DrawString(30,370, GetSign(), olc::WHITE, 2);
	}

	olc::Sprite* GetSprite(unsigned SpriteID)
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
			case Tile::NULLW:
				return NULLw.get();
			default:
				return waterw.get();
		}
	}
	

	bool CollisionDetection(unsigned block)
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

	std::string GetSign()
	{
		FILE* file;
		Actor buffer;
		std::string note;
		if(fopen_s(&file, "signs.bin", "r") == 0)
		{
			int count = _filelength(_fileno(file)) / (sizeof(unsigned) + sizeof(unsigned) + sizeof(std::string));
			for(int i = 0; i < count; i++)
			{
				fread(&buffer.x, sizeof(unsigned), 1, file);
				fread(&buffer.y, sizeof(unsigned), 1, file);
				fread(&note, sizeof(std::string), 1, file);
				if((buffer.x == int(Player.x)) && (buffer.y == int(Player.y)))
				{
					return note;
				}
			}
		}
		note = "I am a sign!";
		return note;
	}

	bool IsBlock(unsigned block)
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
	Game go;
	if (go.Construct(480, 480, 2, 2))		//Задает размеры видимой области игрового мира, а так же включает vsync
	{
		go.Start();
	}

	return 0;
}