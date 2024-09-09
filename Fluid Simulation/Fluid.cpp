#include "Fluid.hpp"
#include <omp.h>
#include <chrono>
#include "opencv2/opencv.hpp"

using namespace std;

// Basic constructor, unused for now
fluid::fluid(int _sizeX, int _sizeY)
{
	raylib::Window window(100, 100, "Fluid Simulation");
	sizeX = _sizeX;
	sizeY = _sizeY;

	dye.resize(sizeX, sizeY, baseDye);
	dyeSource.resize(sizeX, sizeY, baseDye);
	pressureGrid.resize(sizeX, sizeY);
	fluidField.resize(sizeX, sizeY);
	flowGrid.resize(sizeX, sizeY);
	curlGrid.resize(sizeX, sizeY);

	// Yes, this is needed.
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			fluidField[x][y] = 1;
		}
	}

	flowX.resize(sizeX + 1, sizeY);
	sourceX.resize(sizeX + 1, sizeY);
	flowY.resize(sizeX, sizeY + 1);
	sourceY.resize(sizeX, sizeY + 1);

	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
	fluidRenderTexture = LoadRenderTexture(sizeX, sizeY);
	linesRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);
	screenRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);
}

// Constructor with only layout image
fluid::fluid(Image layoutImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, glm::dvec4 dyeColor, int _maxFrames)
{
	renderScale = _renderScale;
	drawMode = _drawMode;
	drawMinMax = _drawMinMax;
	drawLines = _drawLines;
	lineSize = _lineSize;
	maxFrames = _maxFrames;
	Color* layoutImageColorsArr = LoadImageColors(layoutImage);
	vector<vector<glm::ivec4>>  layoutImageColors = {};
	layoutImageColors.resize(layoutImage.width);
	for (int x = 0; x < layoutImage.width; x++)
	{
		layoutImageColors[x].resize(layoutImage.height);
	}
	for (int x = 0; x < layoutImage.width; x++)
	{
		for (int y = 0; y < layoutImage.height; y++)
		{
			layoutImageColors[x][y].x = layoutImageColorsArr[(y * layoutImage.width) + x].r;
			layoutImageColors[x][y].y = layoutImageColorsArr[(y * layoutImage.width) + x].g;
			layoutImageColors[x][y].z = layoutImageColorsArr[(y * layoutImage.width) + x].b;
			layoutImageColors[x][y].w = layoutImageColorsArr[(y * layoutImage.width) + x].a;
		}
	}
	delete[] layoutImageColorsArr;

	sizeX = layoutImage.width;
	sizeY = layoutImage.height;

	dye.resize(sizeX, sizeY, baseDye);
	dyeSource.resize(sizeX, sizeY, baseDye);
	pressureGrid.resize(sizeX, sizeY);
	fluidField.resize(sizeX, sizeY, 1);
	flowGrid.resize(sizeX, sizeY);
	curlGrid.resize(sizeX, sizeY);

	flowX.resize(sizeX + 1, sizeY);
	sourceX.resize(sizeX + 1, sizeY);
	flowY.resize(sizeX, sizeY + 1);
	sourceY.resize(sizeX, sizeY + 1);

	SetTraceLogLevel(5);
	raylib::Window window(100, 100, "Fluid Simulation");
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - sizeX * renderScale / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - sizeY * renderScale / 2);

	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (layoutImageColors[x][y] == glm::ivec4{ 0,0,0,255 })
			{
				fluidField[x][y] = 0;
				continue;
			}

			int r = layoutImageColors[x][y].r;
			int g = layoutImageColors[x][y].g;
			int b = layoutImageColors[x][y].b;
			int a = layoutImageColors[x][y].a;

			if ((r != 0 || g != 0) && b != 128)
			{
				sourceX[x][y] = r / 255.0 * (2 * b / 255.0 - 1) * 10;
				sourceX[x + 1][y] = r / 255.0 * (2 * b / 255.0 - 1) * 10;
				sourceY[x][y] = g / 255.0 * (2 * b / 255.0 - 1) * 10;
				sourceY[x][y + 1] = g / 255.0 * (2 * b / 255.0 - 1) * 10;
			}
			else if (r == 0 && g == 0 && layoutImageColors[x][y].b != 0)
			{
				dyeSource[x][y] = layoutImageColors[x][y].b / 255.0 * dyeColor;
			}
		}
	}

	fluidRenderTexture = LoadRenderTexture(sizeX, sizeY);
	linesRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);
	screenRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);

	images.reserve(maxFrames);
}

// Constructor with layout image and dye source image
fluid::fluid(Image layoutImage, Image dyeImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, int _maxFrames)
{
	renderScale = _renderScale;
	drawMode = _drawMode;
	drawMinMax = _drawMinMax;
	drawLines = _drawLines;
	lineSize = _lineSize;
	maxFrames = _maxFrames;

	if (!(layoutImage.width == dyeImage.width && layoutImage.height == dyeImage.height))
	{
		cout << "Layout image must have same dimensions as dye image." << endl;
		return;
	}

	Color* layoutImageColorsArr = LoadImageColors(layoutImage);
	Color* dyeImageColorsArr = LoadImageColors(dyeImage);
	vector<vector<glm::ivec4>> layoutImageColors = {};
	vector<vector<glm::ivec4>> dyeImageColors = {};
	layoutImageColors.resize(layoutImage.width);
	dyeImageColors.resize(layoutImage.width);
	for (int x = 0; x < layoutImage.width; x++)
	{
		layoutImageColors[x].resize(layoutImage.height);
		dyeImageColors[x].resize(layoutImage.height);
	}
	for (int x = 0; x < layoutImage.width; x++)
	{
		for (int y = 0; y < layoutImage.height; y++)
		{
			layoutImageColors[x][y].x = layoutImageColorsArr[(y * layoutImage.width) + x].r;
			layoutImageColors[x][y].y = layoutImageColorsArr[(y * layoutImage.width) + x].g;
			layoutImageColors[x][y].z = layoutImageColorsArr[(y * layoutImage.width) + x].b;
			layoutImageColors[x][y].w = layoutImageColorsArr[(y * layoutImage.width) + x].a;
			dyeImageColors[x][y].x = dyeImageColorsArr[(y * layoutImage.width) + x].r;
			dyeImageColors[x][y].y = dyeImageColorsArr[(y * layoutImage.width) + x].g;
			dyeImageColors[x][y].z = dyeImageColorsArr[(y * layoutImage.width) + x].b;
			dyeImageColors[x][y].w = dyeImageColorsArr[(y * layoutImage.width) + x].a;
		}
	}
	delete [] layoutImageColorsArr;
	delete [] dyeImageColorsArr;

	sizeX = layoutImage.width;
	sizeY = layoutImage.height;

	dye.resize(sizeX, sizeY, baseDye);
	dyeSource.resize(sizeX, sizeY, baseDye);
	pressureGrid.resize(sizeX, sizeY);
	fluidField.resize(sizeX, sizeY, 1);
	flowGrid.resize(sizeX, sizeY);
	curlGrid.resize(sizeX, sizeY);

	flowX.resize(sizeX + 1, sizeY);
	sourceX.resize(sizeX + 1, sizeY);
	flowY.resize(sizeX, sizeY + 1);
	sourceY.resize(sizeX, sizeY + 1);

	SetTraceLogLevel(5);
	raylib::Window window(100, 100, "Fluid Simulation");
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
	SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - sizeX * renderScale / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - sizeY * renderScale / 2);

	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (layoutImageColors[x][y] == glm::ivec4{ 0,0,0,255 })
			{
				fluidField[x][y] = 0;
				continue;
			}

			int r = layoutImageColors[x][y].r;
			int g = layoutImageColors[x][y].g;
			int b = layoutImageColors[x][y].b;
			int a = layoutImageColors[x][y].a;

			if ((r != 0 || g != 0) && b != 128)
			{
				sourceX[x][y] = r / 255.0 * (2 * b / 255.0 - 1) * 10;
				sourceX[x + 1][y] = r / 255.0 * (2 * b / 255.0 - 1) * 10;
				//sourceY[x][y] = g / 255.0 * (2 * b / 255.0 - 1) * 10;
				//sourceY[x][y + 1] = g / 255.0 * (2 * b / 255.0 - 1) * 10;
				sourceY[x][y] = 0;
				sourceY[x][y + 1] = 0;
			}
			if (dyeImageColors[x][y].a != 0)
			{
				dyeSource[x][y] = glm::dvec4(dyeImageColors[x][y]) / 255.0;
			}
		}
	}

	fluidRenderTexture = LoadRenderTexture(sizeX, sizeY);
	linesRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);
	screenRenderTexture = LoadRenderTexture(sizeX * renderScale, sizeY * renderScale);

	images.reserve(maxFrames);
}

// Destructor, unused
fluid::~fluid()
{

}

// Main draw function
void fluid::draw()
{
	if (drawMode == 1)
	{
		drawGrid = pressureGrid;
	}
	else if (drawMode == 2)
	{
		drawGrid = curlGrid;
	}
	BeginTextureMode(fluidRenderTexture);
	ClearBackground(BLACK);
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			Color cellColor = YELLOW;
			if(fluidField[x][y] == 0)
			{
				cellColor.r = barrierColor.x * 255;
				cellColor.g = barrierColor.y * 255;
				cellColor.b = barrierColor.z * 255;
				cellColor.a = barrierColor.w * 255;
			}
			else if (drawMode == 0)
			{
				cellColor.r = dye[x][y].x * 255;
				cellColor.g = dye[x][y].y * 255;
				cellColor.b = dye[x][y].z * 255;
				cellColor.a = dye[x][y].w * 255;
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
		for (int x = 1; x < sizeX - 1; x += lineSize)
		{
			for (int y = 1; y < sizeY - 1; y += lineSize)
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
	DrawTextureRec(screenRenderTexture.texture, Rectangle{ 0,0,1.0f * sizeX * renderScale,1.0f * sizeY * renderScale }, { 0,0 }, WHITE);
	window.EndDrawing();
}

// Saves image to a vector of images
void fluid::storeScreenImage()
{
	images.push_back(LoadImageFromTexture(screenRenderTexture.texture));
	unsavedFrame = 0;
}

void fluid::mainLoop()
{
	bool exitWindow = 0;
	jthread updateThread(&fluid::updateLoop, this);
	while (!exitWindow)
	{
		while (drawnFrames != frames && frames <= maxFrames)
		{
			unsavedFrame = 1;
			cout << "Frame " << drawnFrames << " saved" << endl;
			storeScreenImage();
			int width = images[0].width;
			int height = images[0].height;
			bool drawn = 0;
			if (frames == maxFrames && !videoCaptured)
			{
				isMakingVideo = 1;
				cv::Size frameSize(images[0].width, images[0].height);
				int codec = cv::VideoWriter::fourcc('H', '2', '6', '4');
				cv::VideoWriter outputVideo("simulationOutput.mp4", codec, 60.0, frameSize);
				for (int i = 0; i < images.size(); i++)
				{
					cout << "Writing frame " << i << " to video" << endl;
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
						cout << e.msg;
					}
				}
				cout << "Saving video" << endl;
				outputVideo.release();
				system("ffmpeg -i simulationOutput.mp4 -vcodec libx264 -crf 18 -pix_fmt yuv420p simulationOutputCompressed.mp4");
				videoCaptured = 1;
				isMakingVideo = 0;
			}
			drawnFrames++;
		}
		draw();
		if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
		{
			exitWindow = true;
			updateThreadShouldJoin = 1; 
			updateThread.join();
			std::cout << "Simulation closed by user" << std::endl;
		}
	}
}

void fluid::updateLoop()
{
	while (!updateThreadShouldJoin)
	{
		while (unsavedFrame || isMakingVideo)
		{
			WaitTime(0.001);
		}
		frames++;
		auto t1 = std::chrono::high_resolution_clock::now();
		update();
		auto t2 = std::chrono::high_resolution_clock::now();
		auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
		cout << "Frame " << frames << ", at " << int(1000.0 / ms_int.count()) << " fps" << endl;
	}
}

void fluid::update()
{
	// Vorticity Confinement Step
	updateFlowGrid();
	updateCurlGrid();
	vorticityConfinement();

	// Incompressibility Step
	solveIncompressibility();

	// Advection Step
	advectVelocity();
	if (drawMode == 0)
	{
		updateFlowGrid();
		updateDyeSources();
		advectDye();
	}

	// Dye Decay and Diffuse Step
	if (drawMode == 0)
	{
		decayDye();
		diffuseDye();
	}

	// Update Flow Grid for Rendering
	updateFlowGrid();
	updateCurlGrid();
}

void fluid::updateFlowSources()
{
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 0; x < sizeX + 1; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (sourceX[x][y] != 0)
			{
				flowX[x][y] = sourceX[x][y];
			}
		}
	}
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY + 1; y++)
		{
			if (sourceY[x][y] != 0)
			{
				flowY[x][y] = sourceY[x][y];
			}
		}
	}
}

void fluid::updateDyeSources()
{
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (dyeSource[x][y] != baseDye)
			{
				dye[x][y] = dyeSource[x][y];
			}
		}
	}
}

void fluid::updateFlowGrid()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			flowGrid[x][y] = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
							   flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
		}
	}
}

void fluid::decayDye()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 1)
			{
				dye[x][y] = baseDye - decayValue * (baseDye - dye[x][y]);
			}
		}
	}
}

void fluid::diffuseDye()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 1)
			{
				int fluidCount = fluidField[x + 1][y] + fluidField[x - 1][y] + fluidField[x][y + 1] + fluidField[x][y - 1];

				glm::dvec4 rDye = (double)fluidField[x + 1][y] * dye[x + 1][y];
				glm::dvec4 lDye = (double)fluidField[x - 1][y] * dye[x - 1][y];
				glm::dvec4 uDye = (double)fluidField[x][y - 1] * dye[x][y - 1];
				glm::dvec4 dDye = (double)fluidField[x][y + 1] * dye[x][y + 1];
				glm::dvec4 avgDye = (1.0 / fluidCount) * (rDye + lDye + uDye + dDye);
				dye[x][y] = (1.0 - diffuseValue) * dye[x][y] + avgDye * diffuseValue;
			}
		}
	}
}

void fluid::solveIncompressibility()
{
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			pressureGrid[x][y] = 0;
		}
	}
	for (int i = 0; i < relaxationSteps; i++)
	{
		updateFlowSources();
#pragma omp parallel for num_threads(12) collapse(2)
		for (int x = 1; x < sizeX - 1; x++)
		{
			for (int y = x % 2 + 1; y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					solveIncompressibilityAt(x, y);
				}
			}
		}
		updateFlowSources();
#pragma omp parallel for num_threads(12) collapse(2)
		for (int x = 1; x < sizeX - 1; x++)
		{
			for (int y = 2 - (x % 2); y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					solveIncompressibilityAt(x, y);
				}
			}
		}
	}
}

bool fluid::solveIncompressibilityAt(int x, int y)
{
	double divergence = (flowX[x + 1][y] * fluidField[x + 1][y]) -
		(flowX[x][y] * fluidField[x - 1][y]) +
		(flowY[x][y + 1] * fluidField[x][y + 1]) -
		(flowY[x][y] * fluidField[x][y - 1]);
	double fluidCount = fluidField[x + 1][y] + fluidField[x - 1][y] + fluidField[x][y + 1] + fluidField[x][y - 1];
	double correctionFactor = 1.9 * divergence / fluidCount;
	flowX[x + 1][y] -= correctionFactor * fluidField[x + 1][y];
	flowX[x][y] += correctionFactor * fluidField[x - 1][y];
	flowY[x][y + 1] -= correctionFactor * fluidField[x][y + 1];
	flowY[x][y] += correctionFactor * fluidField[x][y - 1];
	pressureGrid[x][y] -= divergence / (fluidCount * timeStep);

	return 1;
}

void fluid::advectVelocity()
{
	Grid<double> newFlowX = flowX;
	Grid<double> newFlowY = flowY;
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 1; x < sizeX; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			glm::dvec2 velocity = { flowX[x][y], (flowY[x][y] + flowY[x][y + 1] + flowY[x - 1][y] + flowY[x - 1][y + 1]) / 4.0 };
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;
			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.x > sizeX - 1)
			{
				double ratio = (sizeX - 1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y < 1)
			{
				double ratio = (1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y > sizeY - 2)
			{
				double ratio = (sizeY - 2 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			glm::ivec2 source = sourceFrac;
			sourceFrac -= source;
			double d1 = glm::mix(flowX[source.x][source.y], flowX[source.x][source.y + 1], sourceFrac.y);
			double d2 = glm::mix(flowX[source.x + 1][source.y], flowX[source.x + 1][source.y + 1], sourceFrac.y);
			double sourceFlow = glm::mix(d1, d2, sourceFrac.x);
			newFlowX[x][y] = sourceFlow;
		}
	}
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY; y++)
		{
			glm::dvec2 velocity = { (flowX[x][y] + flowX[x + 1][y] + flowX[x][y - 1] + flowX[x + 1][y - 1]) / 4.0,flowY[x][y] };
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;

			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.x > sizeX - 2)
			{
				double ratio = (sizeX - 2 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y < 1)
			{
				double ratio = (1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y > sizeX - 1)
			{
				double ratio = (sizeX - 1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			glm::ivec2 source = sourceFrac;
			sourceFrac -= source;
			double d1 = glm::mix(flowY[source.x][source.y], flowY[source.x][source.y + 1], sourceFrac.y);
			double d2 = glm::mix(flowY[source.x + 1][source.y], flowY[source.x + 1][source.y + 1], sourceFrac.y);
			double sourceFlow = glm::mix(d1, d2, sourceFrac.x);
			newFlowY[x][y] = sourceFlow;
		}
	}
	flowX = newFlowX;
	flowY = newFlowY;
}

void fluid::advectDye()
{
	Grid<glm::dvec4> newDye = dye;
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 0)
			{
				continue;
			}
			glm::dvec2 velocity = flowGrid[x][y];
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep * 0.5;
			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.x > sizeX - 2)
			{
				double ratio = (sizeX - 2 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y < 1)
			{
				double ratio = (1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y > sizeY - 2)
			{
				double ratio = (sizeY - 2 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			glm::ivec2 source = glm::ivec2{ floor(sourceFrac.x),floor(sourceFrac.y) };
			sourceFrac -= source;
			glm::dvec4 sourceDye{ 1,1,1,1 };
			glm::dvec4 d1 = { 1,1,1,1 };
			glm::dvec4 d2 = { 1,1,1,1 };

			d1 = glm::mix(dye[source.x][source.y], dye[source.x][source.y + 1], sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y], dye[source.x + 1][source.y + 1], sourceFrac.y);
			sourceDye = glm::mix(d1, d2, sourceFrac.x);

			newDye[x][y] = sourceDye;
		}
	}
	dye = newDye;
}

void fluid::updateCurlGrid()
{
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			curlGrid[x][y] = fluidField[x][y + 1] * flowGrid[x][y + 1].x -
							 fluidField[x][y - 1] * flowGrid[x][y - 1].x +
							 fluidField[x - 1][y] * flowGrid[x - 1][y].y -
							 fluidField[x + 1][y] * flowGrid[x + 1][y].y;;
		}
	}
}

void fluid::vorticityConfinement()
{
	Grid<double> newFlowX = flowX;
	Grid<double> newFlowY = flowY;
#pragma omp parallel for num_threads(12) collapse(2)
	for (int x = 3; x < sizeX - 3; x++)
	{
		for (int y = 3; y < sizeY - 3; y++)
		{
			glm::dvec2 direction = { abs(curlGrid[x][y - 1]) - abs(curlGrid[x][y + 1]),
									 abs(curlGrid[x + 1][y]) - abs(curlGrid[x - 1][y]) };

			direction = vorticity / (length(direction) + 1e-5f) * direction;

			newFlowX[x][y] += curlGrid[x][y] * direction.x * timeStep / 2;
			newFlowX[x + 1][y] += curlGrid[x][y] * direction.x * timeStep / 2;
			newFlowY[x][y] += curlGrid[x][y] * direction.y * timeStep / 2;
			newFlowY[x][y + 1] += curlGrid[x][y] * direction.y * timeStep / 2;
		}
	}
	flowX = newFlowX;
	flowY = newFlowY;
}
