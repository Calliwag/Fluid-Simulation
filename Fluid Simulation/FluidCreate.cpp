#include "FluidCreate.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

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
	raylib::Window window(renderX, renderY + buttonHeight, "Fluid Simulation");
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - renderX / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - renderY / 2 - buttonHeight);

	drawType = 1;
	drawVel = { 0,0 };
	drawColor = { 1,0,0,1 };
}

void FluidCreate::loadInfo(FluidInfo info)
{
	sizeX = info.sizeX;
	sizeY = info.sizeY;
	renderScale = info.renderScale;

	fluidField = info.fluidField;
	dyeSource = info.dyeSource;
	flowSource = info.flowSource;
	baseDye = info.baseDye;
	barrierColor = info.barrierColor;

	int renderX = sizeX * renderScale;
	int renderY = sizeY * renderScale;
	SetWindowSize(renderX, renderY + buttonHeight);
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - renderX / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - renderY / 2 - buttonHeight);

	drawType = 1;
	drawVel = { 0,0 };
	drawColor = { 1,0,0,1 };
}

void FluidCreate::createLoop()
{
	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
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

		// Get input
		if (!(loading || saving)) input();

		// Making new file
		if (newing)
		{
			glm::dvec2 origin = { sizeX * renderScale / 2 - 250, sizeY * renderScale / 2 + buttonHeight - 100 };
			GuiPanel(Rectangle{ (float)origin.x,(float)origin.y,500,100 }, "New File: Enter horizontal size, vertical size, and render scale");
			GuiValueBox(Rectangle{ (float)origin.x + 50,(float)origin.y + 40,66,20 }, NULL, &newX, 1, 10000, 1);
			GuiValueBox(Rectangle{ (float)origin.x + 216,(float)origin.y + 40,66,20 }, NULL, &newY, 1, 10000, 1);
			GuiValueBox(Rectangle{ (float)origin.x + 382,(float)origin.y + 40,66,20 }, NULL, &newRenderScale, 1, 100, 1);
		}

		// Loading files
		if (fileDialogState.SelectFilePressed)
		{
			// Load sim file
			if (IsFileExtension(fileDialogState.fileNameText, ".sim"))
			{
				FluidInfo info(fileDialogState.fileNameText);
				loadInfo(info);
				loading = 0;
			}
			fileDialogState.SelectFilePressed = false;
		}
		if (fileDialogState.windowActive) GuiLock();
		if (loading)
		{
			fileDialogState.windowActive = true;
		}
		GuiUnlock();
		GuiWindowFileDialog(&fileDialogState);
		if (!fileDialogState.windowActive)
		{
			loading = 0;
		}

		//Saving files
		if (saving)
		{
			int result = GuiTextInputBox({ (float)sizeX * renderScale / 2, (float)sizeY * renderScale / 2 + buttonHeight, 500, 200 }, "Save File As", "Input File Name", "Cancel;Save", textBoxText, 64, NULL);
			if (result == 0 || result == 1)
			{
				saving = 0;
			}
			if (result == 2)
			{
				if (IsFileExtension(textBoxText, ".sim"))
				{
					FluidInfo info(*this);
					info.saveTo(textBoxText);
					saving = 0;
				}
			}
		}

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
			else if (drawType == 3)
			{
				cellColor.r = dyeSource[x][y].x * 255;
				cellColor.g = dyeSource[x][y].y * 255;
				cellColor.b = dyeSource[x][y].z * 255;
				cellColor.a = dyeSource[x][y].w * 255;
			}
			else if (flowSource[x][y] != glm::dvec2{ 0,0 } && drawType != 3)
			{
				cellColor = GREEN;
				cellColor.a = 128;
			}

			DrawRectangle(x * renderScale, y * renderScale + buttonHeight, renderScale, renderScale, cellColor);
		}
	}

	if (drawingRect)
	{
		glm::dvec2 mPos = mouseToGrid(GetMousePosition());
		int x = mPos.x;
		int y = mPos.y;
		Color cellColor = GRAY;
		cellColor.a = 128;
		DrawRectangle(glm::min(rectBase.x, mPos.x) * renderScale, glm::min(rectBase.y, mPos.y) * renderScale + buttonHeight, abs(mPos.x - rectBase.x) * renderScale, abs(mPos.y - rectBase.y) * renderScale, cellColor);
	}

	for (int x = 0; x < sizeX; x += 2)
	{
		for (int y = 0; y < sizeY; y += 2)
		{
			if (flowSource[x][y] != glm::dvec2{ 0,0 })
			{
				glm::dvec2 velocity = flowSource[x][y];
				DrawLine(x * renderScale + renderScale / 2.0, y * renderScale + renderScale / 2.0 + buttonHeight, (x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0 + buttonHeight, WHITE);
				DrawCircle((x + velocity.x * 0.125) * renderScale + renderScale / 2.0, (y + velocity.y * 0.125) * renderScale + renderScale / 2.0 + buttonHeight, 2, WHITE);
			}
		}
	}

	std::string textToDraw;
	if (drawType == 1) textToDraw = "Draw Mode: Obstacle";
	if (drawType == 2) textToDraw = "Draw Mode: Flow Source\nSource Velocity: " + glm::to_string(drawVel);
	if (drawType == 3) textToDraw = "Draw Mode: Dye Source\nSource Color(rgba): " + glm::to_string(drawColor);

	DrawText(textToDraw.c_str(), renderScale * 2, renderScale * 2 + buttonHeight, 12, GREEN);
}

void FluidCreate::input()
{
	glm::dvec2 mPos = mouseToGrid(GetMousePosition());
	double x = mPos.x;
	double y = mPos.y;
	if (!(x < 0 || x > sizeX - 1 || y < 0 || y > sizeY - 1))
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
			}
			else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			{
				if (!drawingRect)
				{
					rectBase = { x,y };
					drawingRect = 1;
					rectButton = MOUSE_BUTTON_RIGHT;
				}
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

	if (GuiButton(Rectangle{ 0, 0, buttonWidth, buttonHeight }, "#131#Run")) exitWindow = 1;
	if (GuiButton(Rectangle{ buttonWidth, 0, buttonWidth, buttonHeight }, "#8#New")) newing = 1;
	if (GuiButton(Rectangle{ buttonWidth * 2, 0, buttonWidth, buttonHeight }, "#1#Open")) loading = 1;
	if (GuiButton(Rectangle{ buttonWidth * 3, 0, buttonWidth, buttonHeight }, "#2#Save As")) saving = 1;

	if (IsKeyPressed(KEY_ONE)) drawType = 1;
	if (GuiButton(Rectangle{ buttonWidth * 4, 0, buttonWidth, buttonHeight }, "#79#Obstacle")) drawType = 1;
	if (IsKeyPressed(KEY_TWO)) drawType = 2;
	if (GuiButton(Rectangle{ buttonWidth * 5, 0, buttonWidth, buttonHeight }, "#62#Flow Source")) drawType = 2;
	if (IsKeyPressed(KEY_THREE)) drawType = 3;
	if (GuiButton(Rectangle{ buttonWidth * 6, 0, buttonWidth, buttonHeight }, "#26#Dye Source")) drawType = 3;

	if (IsKeyPressed(KEY_SPACE)) rectMode = !rectMode;
	if (GuiButton(Rectangle{ buttonWidth * 7, 0, buttonWidth, buttonHeight }, "#109#Rectangle Mode")) rectMode = !rectMode;
}

glm::dvec2 FluidCreate::mouseToGrid(Vector2 mousePos)
{
	glm::dvec2 gridPos = { mousePos.x / renderScale, (mousePos.y - buttonHeight) / renderScale};
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
