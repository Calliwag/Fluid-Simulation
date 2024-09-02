#include "Fluid.hpp"
raylib::Window window(100, 100, "Fluid Simulation");

using namespace std;

fluid::fluid(int _sizeX, int _sizeY)
{
	sizeX = _sizeX;
	sizeY = _sizeY;

	dye.resize(sizeX);
	dyeSource.resize(sizeX);
	pressureGrid.resize(sizeX);
	fluidField.resize(sizeX);
	flowGrid.resize(sizeX);
	curlGrid.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		dye[i].resize(sizeY, baseDye);
		dyeSource[i].resize(sizeY, baseDye);
		pressureGrid[i].resize(sizeY);
		fluidField[i].resize(sizeY);
		flowGrid[i].resize(sizeY);
		curlGrid[i].resize(sizeY);
	}

	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			fluidField[x][y] = 1;
		}
	}

	flowX.resize(sizeX + 1);
	sourceX.resize(sizeX + 1);
	for (int i = 0; i < sizeX + 1; i++)
	{
		flowX[i].resize(sizeY);
		sourceX[i].resize(sizeY);
	}
	flowY.resize(sizeX);
	sourceY.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		flowY[i].resize(sizeY + 1);
		sourceY[i].resize(sizeY + 1);
	}
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
}

fluid::fluid(Image layoutImage, glm::dvec4 dyeColor)
{
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

	dye.resize(sizeX);
	dyeSource.resize(sizeX);
	pressureGrid.resize(sizeX);
	fluidField.resize(sizeX);
	flowGrid.resize(sizeX);
	curlGrid.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		dye[i].resize(sizeY, baseDye);
		dyeSource[i].resize(sizeY, baseDye);
		pressureGrid[i].resize(sizeY);
		fluidField[i].resize(sizeY,1);
		flowGrid[i].resize(sizeY);
		curlGrid[i].resize(sizeY);
	}

	flowX.resize(sizeX + 1);
	sourceX.resize(sizeX + 1);
	for (int i = 0; i < sizeX + 1; i++)
	{
		flowX[i].resize(sizeY);
		sourceX[i].resize(sizeY);
	}
	flowY.resize(sizeX);
	sourceY.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		flowY[i].resize(sizeY + 1);
		sourceY[i].resize(sizeY + 1);
	}
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);

	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (layoutImageColors[x][y] == glm::ivec4{ 255,255,255,255 })
			{
				fluidField[x][y] = 0;
				continue;
			}
			if (layoutImageColors[x][y].r == 255 && layoutImageColors[x][y].g == 0)
			{
				sourceX[x][y] = layoutImageColors[x][y].b / 25.5;
				sourceX[x + 1][y] = layoutImageColors[x][y].b / 25.5;
			}
			else if (layoutImageColors[x][y].g == 255 && layoutImageColors[x][y].r == 0)
			{
				sourceY[x][y] = layoutImageColors[x][y].b / 25.5;
				sourceY[x][y + 1] = layoutImageColors[x][y].b / 25.5;
			}
			else if (layoutImageColors[x][y].r == 0 && layoutImageColors[x][y].g == 0 && layoutImageColors[x][y].b != 0)
			{
				dyeSource[x][y] = layoutImageColors[x][y].b / 255.0 * dyeColor;
			}
		}
	}
}

void fluid::draw()
{
	window.BeginDrawing();
	window.ClearBackground(BLACK);
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			Color cellColor;
			if(fluidField[x][y] == 0)
			{
				cellColor.r = barrierColor.x * 255;
				cellColor.g = barrierColor.y * 255;
				cellColor.b = barrierColor.z * 255;
				cellColor.a = barrierColor.w * 255;
				DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);
				continue;
			}
			if (drawType == 0)
			{
				cellColor.r = dye[x][y].x * 255;
				cellColor.g = dye[x][y].y * 255;
				cellColor.b = dye[x][y].z * 255;
				cellColor.a = dye[x][y].w * 255;
			}
			if (drawType == 1)
			{
				cellColor.r = glm::mix(0, 255, (glm::clamp(pressureGrid[x][y], pressureMinMax.x, pressureMinMax.y) - pressureMinMax.x) / (pressureMinMax.y - pressureMinMax.x));
				cellColor.g = 0;
				cellColor.b = glm::mix(255, 0, (glm::clamp(pressureGrid[x][y], pressureMinMax.x, pressureMinMax.y) - pressureMinMax.x) / (pressureMinMax.y - pressureMinMax.x));
				cellColor.a = 255;
			}
			if (drawType == 2)
			{
				cellColor.r = glm::mix(0, 255, (glm::clamp(curlGrid[x][y], curlMinMax.x, curlMinMax.y) - curlMinMax.x) / (curlMinMax.y - curlMinMax.x));
				cellColor.g = 0;
				cellColor.b = glm::mix(255, 0, (glm::clamp(curlGrid[x][y], curlMinMax.x, curlMinMax.y) - curlMinMax.x) / (curlMinMax.y - curlMinMax.x));
				cellColor.a = 255;
			}
			DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);
		}
	}
	
	if (drawLines)
	{
		for (int x = 1; x < sizeX - 1; x += lineSize)
		{
			for (int y = 1; y < sizeY - 1; y += lineSize)
			{
				glm::dvec2 velocity = flowGrid[x][y];
				DrawLine(x * renderScale + renderScale / 2, y * renderScale + renderScale / 2, (x + velocity.x * 1.0 / lineSize) * renderScale + renderScale / 2, (y + velocity.y * 1.0 / lineSize) * renderScale + renderScale / 2, WHITE);
				DrawCircle((x + velocity.x * 1.0 / lineSize) * renderScale + renderScale / 2, (y + velocity.y * 1.0 / lineSize) * renderScale + renderScale / 2, 2, WHITE);
			}
		}
	}
	window.EndDrawing();
	if (frames <= maxDrawFrames)
	{
		std::string fileName = "frame" + std::to_string(frames) + ".png";
		TakeScreenshot(fileName.c_str()); //ffmpeg -framerate 60 -i frame%d.png -vcodec libx264 -crf 18 -pix_fmt yuv420p output.mp4
	}
}

void fluid::update()
{
	frames++;
	cout << "frame " << frames << ", at " << window.GetFPS() << " fps" << endl;
	updateFlowGrid();
	updateCurlGrid();
	vorticityConfinement();
	project();
	updateSources();
	updateDyeSources();
	updateFlowGrid();
	advect();
	decayDye();
}

void fluid::updateSources()
{
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

glm::dvec2 fluid::getGridVelocity(int x, int y)
{
	glm::dvec2 velocity = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
									flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
	return velocity;
}

void fluid::decayDye()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 0)
			{
				continue;
			}

			dye[x][y] = baseDye - decayValue * (baseDye - dye[x][y]);

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

void fluid::project()
{
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			pressureGrid[x][y] = 0;
		}
	}
	for (int i = 0; i < relaxationSteps; i++)
	{
		updateSources();
		for (int x = 1; x < sizeX - 1; x++)
		{
			for (int y = x % 2 + 1; y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 0)
				{
					continue;
				}
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
			}
		}
		updateSources();
		for (int x = 1; x < sizeX - 1; x++)
		{
			for (int y = 2 - (x % 2); y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 0)
				{
					continue;
				}
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
			}
		}
	}
}

void fluid::advect()
{
	vector<vector<glm::dvec4>> newDye = dye;
	vector<vector<double>> newFlowX = flowX;
	vector<vector<double>> newFlowY = flowY;
	for (int x = 1; x < flowX.size() - 1; x++)
	{
		for (int y = 1; y < flowX[x].size() - 1; y++)
		{
			glm::dvec2 velocity = { flowX[x][y], (flowY[x][y] + flowY[x][y + 1] + flowY[x - 1][y] + flowY[x - 1][y + 1]) / 4.0 };
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;
			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.x > flowX.size() - 2)
			{
				double ratio = (flowX.size() - 2 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y < 1)
			{
				double ratio = (1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y > flowX[x].size() - 2)
			{
				double ratio = (flowX[x].size() - 2 - y) / (sourceFrac.y - y);
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
	for (int x = 1; x < flowY.size() - 1; x++)
	{
		for (int y = 1; y < flowY[x].size() - 1; y++)
		{
			glm::dvec2 velocity = { (flowX[x][y] + flowX[x + 1][y] + flowX[x][y - 1] + flowX[x + 1][y - 1]) / 4.0,flowY[x][y] };
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;

			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.x > flowY.size() - 2)
			{
				double ratio = (flowY.size() - 2 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y < 1)
			{
				double ratio = (1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			if (sourceFrac.y > flowY[x].size() - 2)
			{
				double ratio = (flowY[x].size() - 2 - y) / (sourceFrac.y - y);
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

double fluid::curl(int x, int y)
{
	double curl = fluidField[x][y + 1] * flowGrid[x][y + 1].x -
				  fluidField[x][y - 1] * flowGrid[x][y - 1].x +
			   	  fluidField[x - 1][y] * flowGrid[x - 1][y].y -
				  fluidField[x + 1][y] * flowGrid[x + 1][y].y;
	return curl;
}

void fluid::updateCurlGrid()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			curlGrid[x][y] = curl(x, y);
		}
	}
}

void fluid::vorticityConfinement()
{
	vector<vector<double>> newFlowX = flowX;
	vector<vector<double>> newFlowY = flowY;
	for (int x = 3; x < sizeX - 3; x++)
	{
		for (int y = 3; y < sizeY - 3; y++)
		{
			glm::dvec2 direction;
			direction.x = abs(curlGrid[x][y - 1]) - abs(curlGrid[x][y + 1]);
			direction.y = abs(curlGrid[x + 1][y]) - abs(curlGrid[x - 1][y]);

			direction = vorticity / (length(direction) + 1e-5f) * direction;

			newFlowX[x][y] += curl(x, y) * direction.x * timeStep / 2;
			newFlowX[x + 1][y] += curl(x, y) * direction.x * timeStep / 2;
			newFlowY[x][y] += curl(x, y) * direction.y * timeStep / 2;
			newFlowY[x][y + 1] += curl(x, y) * direction.y * timeStep / 2;
		}
	}
	flowX = newFlowX;
	flowY = newFlowY;
}
