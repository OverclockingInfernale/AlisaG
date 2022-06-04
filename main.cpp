﻿#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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
		ROAD = 3,
		BRIDGE = 4,
		BROKENROAD = 5,
		SIGN = 6,
	};
	enum Player // енам состояния игрока
	{
		FREE = 0,
		IN_MENU = 1,
		DIED = 2,
	};

	enum State // енам состояния игры
	{
		NOT_STARTED = 0,
		STARTED = 1,
		OVERWORLD = 2,
		FIGHT = 3,
	};

	Game() // конструктор класса
	{
		BlockTypes = 6;
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
				}
			}
			fclose(file);
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				for (int j = 0; j < 256; j++)
				{
					World[i][j].block = 2;
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

		World[int(Player.x) - 4][int(Player.y)].obj = 6;

		std::cout << "Tile x = " << TileX << std::endl;		//Типо дебаг
		std::cout << "Tile y = " << TileY << std::endl;
		std::cout << "Midle X = " << MidleX << std::endl;
		std::cout << "Midle Y = " << MidleY << std::endl;
		std::cout << "Player.x = " << Player.x << std::endl;
		std::cout << "Player.y = " << Player.y << std::endl;
		std::cout << "Player pos = " << MidleX - (TileX / 4) << "   " << MidleY - (TileY / 4) << "   " << MidleX << "   " << MidleY << std::endl;

		FILE* file;
		if (fopen_s(&file, "map.bin", "r") == 0) // чтения мира из файла
		{
			for (int i = 0; i < 256; i++)
			{
				for (int j = 0; j < 256; j++)
				{
					fread(&World[i][j].block, sizeof(unsigned), 1, file);
				}
			}
			fclose(file);
		}
		else
		{
			std::cout << "MAP FILE READ ERROR" << std::endl;
		}

		// указание пути размещения спрайтов

		alisaw = std::make_unique<olc::Sprite>("./Sprites/player/playerc.png");
		waterw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/waterw.png");
		grassw = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/grassw.png");
		bridgew = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/bridgew.png");
		signw = std::make_unique<olc::Sprite>("./Sprites/ObjSpr/signw.png");
		BrokenRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/BrokenRoad.png");
		OldRoad = std::make_unique<olc::Sprite>("./Sprites/WorldSpr/OldRoad.png");
		

		return true;

	}

	bool OnUserUpdate(float fElapsedTime) override // вызов каждый божий кадр
	{
		Clear(olc::DARK_BLUE); // очистка экран на сплошной цвет

		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		//std::cout << "AccumulatedTime = "<< fAccumulatedTime<<std::endl;
		//std::cout << "ElapsedTime = " << fElapsedTime<<std::endl;
	


		// ввод с клавы

		if (GetKey(olc::Key::I).bPressed)
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
				World[int(Player.x)][int(Player.y)].obj = CurrentBlock;
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


		if (KeyPressed == true)		//Управление/Перемещение по миру
		{
			if (LPressed == true && (CollisionDetection(World[int(Player.x - 1)][int(Player.y)].block) != false) && (frame % 15 == 0))
			{
				Player.x -= 3.5f * fElapsedTime;
				LPressed = false;
			}
			else if (RPressed == true && (CollisionDetection(World[int(Player.x + 1)][int(Player.y)].block) != false) && (frame % 15 == 0))
			{
				Player.x += 3.5f * fElapsedTime;
				RPressed = false;
			}
			else if (UpPressed == true && (CollisionDetection(World[int(Player.x)][int(Player.y - 1)].block) != false) && (frame % 15 == 0))
			{
				Player.y -= 3.5f * fElapsedTime;
				UpPressed = false;
			}
			else if (DownPressed == true && (CollisionDetection(World[int(Player.x)][int(Player.y + 1)].block) != false) && (frame % 15 == 0))
			{
				Player.y += 3.5f * fElapsedTime;
				DownPressed = false;
			}
			KeyPressed = false;
		}

		//обработка логики мира
		switch (PlayerState)
		{
		case State::NOT_STARTED:

			PlayerState = 1;

			break;

		case State::STARTED:

			PlayerState = 2;

			break;

		case State::OVERWORLD:
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

	float fTargetFrameTime;
	float fAccumulatedTime;

	unsigned PlayerState;

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

	void DisplayPlayer()		//Рисование
	{
		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(MidleX - 16, MidleY - 16, alisaw.get(), (TileX / 16) / 2);
		SetPixelMode(olc::Pixel::NORMAL);
	}

	void DrawBuildHud()
	{
		DrawSprite(440,20, GetSprite(CurrentBlock), 0.5f, 0);
	}

	void DisplaySign()
	{
		FillRect(20,360,420,440,olc::BLACK);
		DrawString(30,370, "Im a sign!", olc::WHITE, 2);
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
		Player buffer;
		if(fopen_s(&file, "signs.bin", "r") == 0)
		{
			fread(&buffer.x, sizeof(unsigned), 1, file);
			fread(&buffer.y, sizeof(unsigned), 1, file)
		}
		else
		{
			return "I am a sign!";
		}
	}

	bool IsBlock(unsigned block)
	{
		switch(block)
		{
			case Tile::SIGN:
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
	if (go.Construct(480, 480, 2, 2, false, true))		//Задает размеры видимой области игрового мира, а так же включает vsync
	{
		go.Start();
	}

	return 0;
}