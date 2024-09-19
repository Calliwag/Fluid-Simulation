#include "FluidCreate.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"

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
	raylib::Window window(renderX, renderY, "Fluid Simulation");
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - renderX / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - renderY / 2);

	drawType = 1;
	drawVel = { 0,0 };
	drawColor = { 1,0,0,1 };
}

void FluidCreate::createLoop()
{
	bool exitWindow = 0;
	while (!exitWindow)
	{
		// Get input, draw to grids
		input();

		// Draw to screen
		draw();
		if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
		{
			exitWindow = true;
			std::cout << "Fluid creation closed by user\n";
		}
	}
	// For some reason this doesn't actually close the window? Valve pls fix
	CloseWindow();
}

void FluidCreate::draw()
{
	window.BeginDrawing();
	window.ClearBackground(BLACK);
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

			DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);

			if (flowSource[x][y] != glm::dvec2{ 0,0 } && drawType != 3)
			{
				cellColor = GREEN;
				cellColor.a = 128;
				DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);
			}
		}
	}

	for (int x = 0; x < sizeX; x += 2)
	{
		for (int y = 0; y < sizeY; y += 2)
		{
			if (flowSource[x][y] != glm::dvec2{ 0,0 })
			{
				glm::dvec2 velocity = flowSource[x][y];
				DrawLine(x * renderScale + renderScale / 2.0, y * renderScale + renderScale / 2.0, (x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0, WHITE);
				DrawCircle((x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0, 2, WHITE);
			}
		}
	}

	std::string textToDraw;
	if (drawType == 1) textToDraw = "Draw Mode: Obstacle";
	if (drawType == 2) textToDraw = "Draw Mode: Flow Source\nSource Velocity: " + glm::to_string(drawVel);
	if (drawType == 3) textToDraw = "Draw Mode: Dye Source\nSource Color(rgba): " + glm::to_string(drawColor);

	DrawText(textToDraw.c_str(), renderScale * 2, renderScale * 2, 12, GREEN);

	window.EndDrawing();
}

void FluidCreate::input()
{
	glm::dvec2 mPos = mouseToGrid(GetMousePosition());
	int x = mPos.x;
	int y = mPos.y;
	if (x < 1 || x > sizeX - 1 || y < 1 || y > sizeY - 1)
	{
		return;
	}
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
			if(fluidField[x][y]) flowSource[x][y] = drawVel;
		}
		else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			flowSource[x][y] = { 0,0 };
		}
		if (IsKeyPressed(KEY_BACKSPACE)) drawVel = { 0,0 };
		else if (IsKeyPressed(KEY_UP)) drawVel.y -= 1;
		else if (IsKeyPressed(KEY_DOWN)) drawVel.y += 1;
		else if (IsKeyPressed(KEY_LEFT)) drawVel.x -= 1;
		else if (IsKeyPressed(KEY_RIGHT)) drawVel.x += 1;
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
		if (IsKeyPressed(KEY_R)) drawColor.r = 1 - drawColor.r;
		if (IsKeyPressed(KEY_G)) drawColor.g = 1 - drawColor.g;
		if (IsKeyPressed(KEY_B)) drawColor.b = 1 - drawColor.b;
		if (IsKeyPressed(KEY_A)) drawColor.a = 1 - drawColor.a;
	}
	if (IsKeyPressed(KEY_ONE)) drawType = 1;
	if (IsKeyPressed(KEY_TWO)) drawType = 2;
	if (IsKeyPressed(KEY_THREE)) drawType = 3;
}

glm::ivec2 FluidCreate::mouseToGrid(Vector2 mousePos)
{
	glm::ivec2 gridPos = { mousePos.x / renderScale, mousePos.y / renderScale };
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

void FluidInfo::saveTo(std::string fileName)
{
	//https://uscilab.github.io/cereal/
}
