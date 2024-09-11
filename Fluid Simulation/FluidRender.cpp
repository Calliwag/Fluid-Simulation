#include "FluidRender.hpp"
#include <omp.h>
#include <chrono>
#include "opencv2/opencv.hpp"

// Constructor
FluidRender::FluidRender(std::shared_ptr<Fluid> _fluid, glm::dvec4 _baseDye, glm::dvec4 _barrierColor, int _renderScale, int _maxFrames, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize)
{
	fluid = _fluid;
	baseDye = _baseDye;
	barrierColor = _barrierColor;
	renderScale = _renderScale;
	maxFrames = _maxFrames;
	drawMode = _drawMode;
	drawMinMax = _drawMinMax;
	drawLines = _drawLines;
	lineSize = _lineSize;

	SetTraceLogLevel(5);
	raylib::Window window(100, 100, "Fluid Simulation");
	SetWindowSize(fluid->sizeX * renderScale, fluid->sizeY * renderScale);
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - fluid->sizeX * renderScale / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - fluid->sizeY * renderScale / 2);

	fluidRenderTexture = LoadRenderTexture(fluid->sizeX, fluid->sizeY);
	linesRenderTexture = LoadRenderTexture(fluid->sizeX * renderScale, fluid->sizeY * renderScale);
	screenRenderTexture = LoadRenderTexture(fluid->sizeX * renderScale, fluid->sizeY * renderScale);
}

void FluidRender::getGrids()
{
	if (drawLines)
	{
		flowGrid = fluid->flowGrid;
	}
	if (drawMode == 0)
	{
		dyeGrid = fluid->dye;
	}
	if (drawMode == 1)
	{
		drawGrid = fluid->pressureGrid;
	}
	else if (drawMode == 2)
	{
		drawGrid = fluid->curlGrid;
	}
}

// Main draw function
void FluidRender::draw()
{
	BeginTextureMode(fluidRenderTexture);
	ClearBackground(BLACK);
	for (int x = 0; x < fluid->sizeX; x++)
	{
		for (int y = 0; y < fluid->sizeY; y++)
		{
			Color cellColor = YELLOW;
			if (fluid->fluidField[x][y] == 0)
			{
				cellColor.r = barrierColor.x * 255;
				cellColor.g = barrierColor.y * 255;
				cellColor.b = barrierColor.z * 255;
				cellColor.a = barrierColor.w * 255;
			}
			else if (drawMode == 0)
			{
				cellColor.r = dyeGrid[x][y].x * 255;
				cellColor.g = dyeGrid[x][y].y * 255;
				cellColor.b = dyeGrid[x][y].z * 255;
				cellColor.a = dyeGrid[x][y].w * 255;
			}
			else if (drawMode == 1)
			{
				cellColor.r = glm::mix(0, 255, (glm::clamp(drawGrid[x][y], drawMinMax.x, drawMinMax.y) - drawMinMax.x) / (drawMinMax.y - drawMinMax.x));
				cellColor.g = 0;
				cellColor.b = glm::mix(255, 0, (glm::clamp(drawGrid[x][y], drawMinMax.x, drawMinMax.y) - drawMinMax.x) / (drawMinMax.y - drawMinMax.x));
				cellColor.a = 255;
			}
			else if (drawMode == 2)
			{
				cellColor.r = glm::mix(0, 255, (glm::clamp(drawGrid[x][y], drawMinMax.x, drawMinMax.y) - drawMinMax.x) / (drawMinMax.y - drawMinMax.x));
				cellColor.g = 0;
				cellColor.b = glm::mix(255, 0, (glm::clamp(drawGrid[x][y], drawMinMax.x, drawMinMax.y) - drawMinMax.x) / (drawMinMax.y - drawMinMax.x));
				cellColor.a = 255;
			}
			DrawPixel(x, y, cellColor);
		}
	}
	EndTextureMode();

	BeginTextureMode(linesRenderTexture);
	ClearBackground(BLANK);
	if (drawLines)
	{
		for (int x = 1; x < fluid->sizeX - 1; x += lineSize)
		{
			for (int y = 1; y < fluid->sizeY - 1; y += lineSize)
			{
				glm::dvec2 velocity = flowGrid[x][y];
				DrawLine(x * renderScale + renderScale / 2.0, y * renderScale + renderScale / 2.0, (x + velocity.x * 0.0625 * lineSize) * renderScale + renderScale / 2.0, (y + velocity.y * 0.0625 * lineSize) * renderScale + renderScale / 2.0, WHITE);
				DrawCircle((x + velocity.x * 0.0625 * lineSize) * renderScale + renderScale / 2.0, (y + velocity.y * 0.0625 * lineSize) * renderScale + renderScale / 2.0, 2, WHITE);
			}
		}
	}
	EndTextureMode();

	BeginTextureMode(screenRenderTexture);
	ClearBackground(BLACK);
	DrawTextureEx(fluidRenderTexture.texture, { 0,0 }, 0, renderScale, WHITE);
	DrawTexture(linesRenderTexture.texture, 0, 0, WHITE);
	EndTextureMode();

	window.BeginDrawing();
	window.ClearBackground(BLACK);
	DrawTextureRec(screenRenderTexture.texture, Rectangle{ 0,0,1.0f * fluid->sizeX * renderScale,1.0f * fluid->sizeY * renderScale }, { 0,0 }, WHITE);
	window.EndDrawing();
}

// Saves image to a vector of images
void FluidRender::storeScreenImage()
{
	images.push_back(LoadImageFromTexture(screenRenderTexture.texture));
}

void FluidRender::saveVideo()
{
	int width = images[0].width;
	int height = images[0].height;
	cv::Size frameSize(images[0].width, images[0].height);
	int codec = cv::VideoWriter::fourcc('H', '2', '6', '4');
	cv::VideoWriter outputVideo("simulationOutput.mp4", codec, 60.0, frameSize);
	for (int i = 0; i < images.size(); i++)
	{
		std::cout << "Writing frame " << i << " to video\n";
		cv::Mat inputMat(cv::Size(width, height), CV_8UC3);
		Color* imageArr = LoadImageColors(images[i]);
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				inputMat.at<cv::Vec3b>(y, x) = { imageArr[y * width + x].b,imageArr[y * width + x].g,imageArr[y * width + x].r };
			}
		}
		try
		{
			outputVideo.write(inputMat);
		}
		catch (cv::Exception& e)
		{
			std::cout << e.msg << std::endl;
		}
	}
	std::cout << "Saving video" << std::endl;
	outputVideo.release();
	system("ffmpeg -i simulationOutput.mp4 -vcodec libx264 -crf 18 -pix_fmt yuv420p simulationOutputCompressed.mp4");
	videoCaptured = 1;
}

// Main loop
void FluidRender::mainLoop()
{
	bool exitWindow = 0;
	std::jthread updateThread(&Fluid::updateLoop, fluid);
	while (!exitWindow)
	{
		fluid->m.lock();
		while (drawnFrames != fluid->frames && drawnFrames <= maxFrames)
		{
			drawnFrames++;
			std::cout << "Frame " << drawnFrames << " saved\n";
			storeScreenImage();
			if (drawnFrames == maxFrames && !videoCaptured) saveVideo();
		}
		getGrids();
		fluid->m.unlock();
		draw();

		if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
		{
			exitWindow = true;
			fluid->updateThreadShouldJoin = 1;
			updateThread.join();
			std::cout << "Simulation closed by user" << std::endl;
		}
	}
}