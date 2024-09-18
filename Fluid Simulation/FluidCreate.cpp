#include "FluidCreate.hpp"

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

	sourceX.resize(sizeX + 1, sizeY);
	sourceY.resize(sizeX, sizeY + 1);

	int renderX = sizeX * renderScale;
	int renderY = sizeY * renderScale;
	SetTraceLogLevel(5);
	raylib::Window window(renderX, renderY, "Fluid Simulation");
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - renderX / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - renderY / 2);

	drawType = 0;
	drawVel = { 0,0 };
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
		}
	}

	for (int x = 0; x < sizeX; x += 4)
	{
		for (int y = 0; y < sizeY; y += 4)
		{
			if (flowSource[x][y] != glm::dvec2{ 0,0 })
			{
				glm::dvec2 velocity = flowSource[x][y];
				DrawLine(x * renderScale + renderScale / 2.0, y * renderScale + renderScale / 2.0, (x + velocity.x * 0.25) * renderScale + renderScale / 2.0, (y + velocity.y * 0.25) * renderScale + renderScale / 2.0, WHITE);
				DrawCircle((x + velocity.x * 0.25) * renderScale + renderScale / 2.0, (y + velocity.y * 0.25) * renderScale + renderScale / 2.0, 2, WHITE);
			}
		}
	}

	window.EndDrawing();
}

void FluidCreate::input()
{
	glm::dvec2 mPos = mouseToGrid(GetMousePosition());
	int x = mPos.x;
	int y = mPos.y;
	if (drawType == 0)
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
	if (drawType == 1)
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			flowSource[x][y] = drawVel;
		}
		else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			flowSource[x][y] = { 0,0 };
		}
	}
}

glm::ivec2 FluidCreate::mouseToGrid(Vector2 mousePos)
{
	glm::ivec2 gridPos = { mousePos.x / renderScale, mousePos.y / renderScale };
	return gridPos;
}
