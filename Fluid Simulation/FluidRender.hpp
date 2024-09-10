#pragma once
#include "Fluid.hpp"
#include <thread>

class FluidRender
{
private:
	// Fluid
	std::shared_ptr<Fluid> fluid;

	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	// Rendering
	raylib::Window window;
	int renderScale;
	int drawMode; // 0 = dye, 1 = pressure, 2 = curl
	glm::dvec2 drawMinMax;
	bool drawLines;
	int lineSize;
	int maxFrames;

	Grid<double> drawGrid;

	RenderTexture2D fluidRenderTexture;
	RenderTexture2D linesRenderTexture;
	RenderTexture2D screenRenderTexture;

	std::vector<Image> images = {};
	bool videoCaptured = 0;

	int drawnFrames = 0;

public:
	// Constructor
	FluidRender(std::shared_ptr<Fluid> _fluid, glm::dvec4 _baseDye, glm::dvec4 _barrierColor, int _renderScale, int _maxFrames, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize);

	// Main Loop
	void mainLoop();

	// Drawing
	void draw();
	void storeScreenImage();

};