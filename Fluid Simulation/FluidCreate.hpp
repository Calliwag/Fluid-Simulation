#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include "glm/glm.hpp"
#include "Grid.hpp"
#include "raylib-cpp.hpp"

class FluidInfo;

class FluidCreate
{
private:
	int sizeX;
	int sizeY;
	int renderScale;

	Grid<uint8_t> fluidField = {};
	Grid<glm::dvec4> dyeSource = {};
	Grid<glm::dvec2> flowSource = {};

	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	raylib::Window window;

	// Drawing
	int drawType;
	bool rectMode = 0;
	glm::dvec2 drawVel;
	glm::dvec4 drawColor;
	bool drawingRect = 0;
	glm::dvec2 rectBase;
	MouseButton rectButton; // 1 = left, 2 = right
	bool exitWindow = 0;
	float buttonHeight = 24;
	float buttonWidth = 100;

	bool loading = 0;
	bool saving = 0;
	bool newing = 0;

	int newX = 100;
	int newY = 100;
	int newRenderScale = 4;
	bool editnX = 0;
	bool editnY = 0;
	bool editRS = 0;

	std::string fileToLoad = "";
	char textBoxText[256];
	bool textBoxEditMode = 0;

public:
	FluidCreate(int _sizeX, int _sizeY, int _renderScale);
	void loadInfo(FluidInfo info);
	void createLoop();
	void draw();
	void input();
	glm::dvec2 mouseToGrid(Vector2 mousePos);

	friend class FluidInfo;

};

class FluidInfo
{
public:
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t renderScale;

	Grid<uint8_t> fluidField;
	Grid<glm::dvec4> dyeSource;
	Grid<glm::dvec2> flowSource;
	glm::dvec4 baseDye;
	glm::dvec4 barrierColor;

public:
	// Construct from FluidCreate
	FluidInfo(FluidCreate data);

	// Construct from sizeX, sizeY, renderScale
	FluidInfo(int _sizeX, int _sizeY, int _renderScale);

	// Load from file
	FluidInfo(std::string fileName);

	// Save to file
	void saveTo(std::string fileName);

};