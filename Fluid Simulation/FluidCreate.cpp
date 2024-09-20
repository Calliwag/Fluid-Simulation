#include "FluidCreate.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

FluidCreate::FluidCreate(int _sizeX, int _sizeY, int _renderScale)
{
	sizeX = _sizeX;
	sizeY = _sizeY;
	renderScale = _renderScale;

	dyeSource.resize(sizeX, sizeY, baseDye);
	fluidField.resize(sizeX, sizeY, 0);
	flowSource.resize(sizeX, sizeX, glm::dvec2{ 0,0 });

	// Make all center cells into fluid
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			fluidField[x][y] = 1;
		}
	}

	int renderX = sizeX * renderScale;
	int renderY = sizeY * renderScale;
	SetTraceLogLevel(5);
	raylib::Window window(renderX, renderY + topBorder, "Fluid Simulation");
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - renderX / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - renderY / 2 - topBorder);

	drawType = 1;
	drawVel = { 0,0 };
	drawColor = { 1,0,0,1 };
}

void FluidCreate::createLoop()
{
	while (!exitWindow)
	{
		// Draw to screen
		window.BeginDrawing();
		window.ClearBackground(BLACK);
		draw();

		if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
		{
			exitWindow = true;
			std::cout << "Fluid creation closed by user\n";
		}

		// Get input, draw to grids
		input();

		window.EndDrawing();
	}
	// For some reason this doesn't actually close the window? Valve pls fix
	CloseWindow();
}

void FluidCreate::draw()
{
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			Color cellColor = BLANK;
			if (!fluidField[x][y])
			{
				cellColor.r = barrierColor.x * 255;
				cellColor.g = barrierColor.y * 255;
				cellColor.b = barrierColor.z * 255;
				cellColor.a = barrierColor.w * 255;
			}
			else
			{
				cellColor.r = dyeSource[x][y].x * 255;
				cellColor.g = dyeSource[x][y].y * 255;
				cellColor.b = dyeSource[x][y].z * 255;
				cellColor.a = dyeSource[x][y].w * 255;
			}

			DrawRectangle(x * renderScale, y * renderScale + topBorder, renderScale, renderScale, cellColor);

			if (flowSource[x][y] != glm::dvec2{ 0,0 } && drawType != 3)
			{
				cellColor = GREEN;
				cellColor.a = 128;
				DrawRectangle(x * renderScale, y * renderScale + topBorder, renderScale, renderScale, cellColor);
			}
		}
	}

	if (drawingRect)
	{
		glm::dvec2 mPos = mouseToGrid(GetMousePosition());
		int x = mPos.x;
		int y = mPos.y;
		Color cellColor = GRAY;
		cellColor.a = 128;
		DrawRectangle(glm::min(rectBase.x, mPos.x) * renderScale, glm::min(rectBase.y, mPos.y) * renderScale + topBorder, abs(mPos.x - rectBase.x) * renderScale, abs(mPos.y - rectBase.y) * renderScale, cellColor);
	}

	for (int x = 0; x < sizeX; x += 2)
	{
		for (int y = 0; y < sizeY; y += 2)
		{
			if (flowSource[x][y] != glm::dvec2{ 0,0 })
			{
				glm::dvec2 velocity = flowSource[x][y];
				DrawLine(x * renderScale + renderScale / 2.0, y * renderScale + renderScale / 2.0 + topBorder, (x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0 + topBorder, WHITE);
				DrawCircle((x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0 + topBorder, 2, WHITE);
			}
		}
	}

	std::string textToDraw;
	if (drawType == 1) textToDraw = "Draw Mode: Obstacle";
	if (drawType == 2) textToDraw = "Draw Mode: Flow Source\nSource Velocity: " + glm::to_string(drawVel);
	if (drawType == 3) textToDraw = "Draw Mode: Dye Source\nSource Color(rgba): " + glm::to_string(drawColor);

	DrawText(textToDraw.c_str(), renderScale * 2, renderScale * 2 + topBorder, 12, GREEN);
}

void FluidCreate::input()
{
	glm::dvec2 mPos = mouseToGrid(GetMousePosition());
	double x = mPos.x;
	double y = mPos.y;
	if (!(x < 1 || x > sizeX - 1 || y < 1 || y > sizeY - 1))
	{
		if (rectMode)
		{
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				if (!drawingRect)
				{
					rectBase = { x,y };
					drawingRect = 1;
					rectButton = MOUSE_BUTTON_LEFT;
				}
				return;
			}
			else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			{
				if (!drawingRect)
				{
					rectBase = { x,y };
					drawingRect = 1;
					rectButton = MOUSE_BUTTON_RIGHT;
				}
				return;
			}
			else if (drawingRect)
			{
				drawingRect = 0;

				for (int ix = glm::min(rectBase.x, x); ix < glm::max(rectBase.x, x); ix++)
				{
					for (int iy = glm::min(rectBase.y, y); iy < glm::max(rectBase.y, y); iy++)
					{
						if (drawType == 1)
						{
							if (rectButton == MOUSE_BUTTON_LEFT)
							{
								fluidField[ix][iy] = 0;
							}
							else if (rectButton == MOUSE_BUTTON_RIGHT)
							{
								fluidField[ix][iy] = 1;
							}
						}
						else if (drawType == 2)
						{
							if (rectButton == MOUSE_BUTTON_LEFT)
							{
								if (fluidField[ix][iy]) flowSource[ix][iy] = drawVel;
							}
							else if (rectButton == MOUSE_BUTTON_RIGHT)
							{
								flowSource[ix][iy] = { 0,0 };
							}
						}
						else if (drawType == 3)
						{
							if (rectButton == MOUSE_BUTTON_LEFT)
							{
								if (fluidField[ix][iy]) dyeSource[ix][iy] = drawColor;
							}
							else if (rectButton == MOUSE_BUTTON_RIGHT)
							{
								dyeSource[ix][iy] = baseDye;
							}
						}
					}
				}
				return;
			}
		}
		else
		{
			if (drawType == 1)
			{
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				{
					fluidField[x][y] = 0;
				}
				else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				{
					fluidField[x][y] = 1;
				}
			}
			else if (drawType == 2)
			{
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				{
					if (fluidField[x][y]) flowSource[x][y] = drawVel;
				}
				else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				{
					flowSource[x][y] = { 0,0 };
				}
			}
			else if (drawType == 3)
			{
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				{
					if (fluidField[x][y]) dyeSource[x][y] = drawColor;
				}
				else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				{
					dyeSource[x][y] = baseDye;
				}
			}
		}
	}

	if (drawType == 2)
	{
		if (IsKeyPressed(KEY_BACKSPACE)) drawVel = { 0,0 };
		else if (IsKeyPressed(KEY_UP)) drawVel.y -= 1;
		else if (IsKeyPressed(KEY_DOWN)) drawVel.y += 1;
		else if (IsKeyPressed(KEY_LEFT)) drawVel.x -= 1;
		else if (IsKeyPressed(KEY_RIGHT)) drawVel.x += 1;
	}
	else if (drawType == 3)
	{
		if (IsKeyPressed(KEY_R)) drawColor.r = 1 - drawColor.r;
		if (IsKeyPressed(KEY_G)) drawColor.g = 1 - drawColor.g;
		if (IsKeyPressed(KEY_B)) drawColor.b = 1 - drawColor.b;
		if (IsKeyPressed(KEY_A)) drawColor.a = 1 - drawColor.a;
	}

	if (IsKeyPressed(KEY_ONE)) drawType = 1;
	if (IsKeyPressed(KEY_TWO)) drawType = 2;
	if (IsKeyPressed(KEY_THREE)) drawType = 3;

	if (IsKeyPressed(KEY_SPACE)) rectMode = !rectMode;
	if (GuiButton(Rectangle{ 0, 0, 120, 24 }, "Run")) exitWindow = 1;
}

glm::dvec2 FluidCreate::mouseToGrid(Vector2 mousePos)
{
	glm::dvec2 gridPos = { mousePos.x / renderScale, (mousePos.y - topBorder) / renderScale};
	return gridPos;
}

FluidInfo::FluidInfo(FluidCreate data)
{
	sizeX = data.sizeX;
	sizeY = data.sizeY;
	renderScale = data.renderScale;

	fluidField = data.fluidField;
	dyeSource = data.dyeSource;
	flowSource = data.flowSource;
	baseDye = data.baseDye;
	barrierColor = data.barrierColor;
}

FluidInfo::FluidInfo(std::string fileName)
{
	std::ifstream fin(fileName);
	// Read size and render scale
	fin >> sizeX >> sizeY >> renderScale;

	// Read base dye
	fin >> baseDye.x >> baseDye.y >> baseDye.z >> baseDye.w;

	// Read barrier color
	fin >> barrierColor.x >> barrierColor.y >> barrierColor.z >> barrierColor.w;

	// Size grids
	fluidField.resize(sizeX, sizeY);
	dyeSource.resize(sizeX, sizeY);
	flowSource.resize(sizeX, sizeY);

	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			// Read fluid field
			fin >> fluidField[x][y];

			// Read dye source
			fin >> dyeSource[x][y].x;
			fin >> dyeSource[x][y].y;
			fin >> dyeSource[x][y].z;
			fin >> dyeSource[x][y].w;

			// Read flow source
			fin >> flowSource[x][y].x;
			fin >> flowSource[x][y].y;
		}
	}
}

void FluidInfo::saveTo(std::string fileName)
{
	std::ofstream fout(fileName);
	fout << sizeX << " " << sizeY << " "; // Write size, uint16_t
	fout << renderScale << " "; // Write render scale, uint16_t
	fout << baseDye.x << " " << baseDye.y << " " << baseDye.z << " " << baseDye.w << " "; // Write base dye
	fout << barrierColor.x << " " << barrierColor.y << " " << barrierColor.z << " " << barrierColor.w << " "; // Write barrier color
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			// Write fluid field, uint8_t
			fout << fluidField[x][y] << " ";

			// Write dye source
			fout << dyeSource[x][y].x << " ";
			fout << dyeSource[x][y].y << " ";
			fout << dyeSource[x][y].z << " ";
			fout << dyeSource[x][y].w << " ";

			// Write flow source
			fout << flowSource[x][y].x << " ";
			fout << flowSource[x][y].y << " ";
		}
	}
	fout.close();
}
