#include "Fluid.hpp"
#include <omp.h>
#include <chrono>
#include "opencv2/opencv.hpp"

using namespace std;

// Basic constructor, unused for now
Fluid::Fluid(int _sizeX, int _sizeY)
{
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
	flowY.resize(sizeX, sizeY + 1);
	flowSource.resize(sizeX, sizeY);
}

// Constructor with only layout image
Fluid::Fluid(Image layoutImage, glm::dvec4 dyeColor, double _vorticity, int _relaxationSteps)
{
	vorticity = _vorticity;
	relaxationSteps = _relaxationSteps;

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
	flowY.resize(sizeX, sizeY + 1);
	flowSource.resize(sizeX, sizeY);

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
				flowSource[x][y] = glm::dvec2{r / 255.0 * (2 * b / 255.0 - 1) * 10, g / 255.0 * (2 * b / 255.0 - 1) * 10};
			}
			else if (r == 0 && g == 0 && layoutImageColors[x][y].b != 0)
			{
				dyeSource[x][y] = layoutImageColors[x][y].b / 255.0 * dyeColor;
			}
		}
	}

	frame = 0;
}

// Constructor with layout image and dye source image
Fluid::Fluid(Image layoutImage, Image dyeImage, double _vorticity, int _relaxationSteps)
{
	vorticity = _vorticity;
	relaxationSteps = _relaxationSteps;

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
	flowY.resize(sizeX, sizeY + 1);
	flowSource.resize(sizeX, sizeY);

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
				flowSource[x][y] = glm::dvec2{ r / 255.0 * (2 * b / 255.0 - 1) * 10, g / 255.0 * (2 * b / 255.0 - 1) * 10 };
			}
			if (dyeImageColors[x][y].a != 0)
			{
				dyeSource[x][y] = glm::dvec4(dyeImageColors[x][y]) / 255.0;
			}
		}
	}

	frame = 0;
}

Fluid::Fluid(FluidInfo info, double _vorticity, int _relaxationSteps)
{
	sizeX = info.sizeX;
	sizeY = info.sizeY;
	dyeSource = info.dyeSource;
	fluidField = info.fluidField;
	flowSource = info.flowSource;
	densitySource = info.densitySource;
	for (int i = 0; i < sizeX; i++)
	{
		for (int j = 0; j < sizeY; j++)
		{
			if (dyeSource[i][j] == glm::dvec4{0, 0, 0, 1}) dyeSource[i][j] = baseDye;
		}
	}

	density.resize(sizeX, sizeY, baseDensity);
	dye.resize(sizeX, sizeY, baseDye);
	pressureGrid.resize(sizeX, sizeY);
	flowGrid.resize(sizeX, sizeY);
	curlGrid.resize(sizeX, sizeY);
	flowX.resize(sizeX + 1, sizeY);
	flowY.resize(sizeX, sizeY + 1);

	vorticity = _vorticity;
	relaxationSteps = _relaxationSteps;

	frame = 0;
}

void Fluid::updateLoop()
{
	updateThreadShouldJoin = 0;
	updateFlowAndCurl();
	while (!updateThreadShouldJoin)
	{
		updateMutex.lock();
		frame++;
		auto t1 = std::chrono::high_resolution_clock::now();
		update();
		updateMutex.unlock();
		auto t2 = std::chrono::high_resolution_clock::now();
		auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
		cout << "Frame " << frame << ", at " << int(1000.0 / ms_int.count()) << " fps\n";
	}
	cout << "Update thread joining\n";
}

void Fluid::update()
{
	// Vorticity Confinement Step
	vorticityConfinement();

	if (!compressible)
	{
		// Solve Incompressibility
		solveIncompressibility();
	}
	else
	{
		// Solve compressibility
		solveCompressibility();
	}

	// Advect Velocity
	advectVelocity();

	// Advect density
	if (compressible) advectDensity();

	//Advect Dye
	updateFlowAndCurl();
	updateDyeSources();
	advectDye();

	// Dye Decay and Diffuse Step
	decayDye();
	diffuseDye();
}

// Updating all sources of velocity
void Fluid::updateFlowSources()
{
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (flowSource[x][y] != glm::dvec2{ 0, 0 })
			{
				flowX[x][y] = 0.5 * (flowSource[x - 1][y].x + flowSource[x][y].x);
				flowX[x + 1][y] = 0.5 * (flowSource[x + 1][y].x + flowSource[x][y].x);
				flowY[x][y] = 0.5 * (flowSource[x][y - 1].y + flowSource[x][y].y);
				flowY[x][y + 1] = 0.5 * (flowSource[x][y + 1].y + flowSource[x][y].y);
				if (compressible)
				{
					density[x][y] = baseDensity;
				}
			}
		}
	}
}

void Fluid::updateDensitySources()
{
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			if (densitySource[x][y] != baseDensity)
			{
				density[x][y] = densitySource[x][y];
			}
		}
	}
}

// Updating all sources of dye
void Fluid::updateDyeSources()
{
#pragma omp parallel for num_threads(12)
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

//Updating flow and curl grid
void Fluid::updateFlowAndCurl()
{
#pragma omp parallel for num_threads(12)
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			flowGrid[x][y] = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
							   flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
			curlGrid[x][y] = fluidField[x][y + 1] * flowGrid[x][y + 1].x -
							 fluidField[x][y - 1] * flowGrid[x][y - 1].x +
							 fluidField[x - 1][y] * flowGrid[x - 1][y].y -
							 fluidField[x + 1][y] * flowGrid[x + 1][y].y;;
		}
	}
}

// Decaying dye with 'decayValue'
void Fluid::decayDye()
{
#pragma omp parallel for num_threads(12)
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 1)
			{
				dye[x][y] = baseDye - (1 - decayValue) * (baseDye - dye[x][y]);
			}
		}
	}
}

// Diffusing dye with 'diffuseValue'
void Fluid::diffuseDye()
{
#pragma omp parallel for num_threads(12)
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

// Solve incompressibility of the fluid
void Fluid::solveIncompressibility()
{
#pragma omp parallel for num_threads(12)
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
#pragma omp parallel for num_threads(12)
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
#pragma omp parallel for num_threads(12)
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

// Solve incompressibility at a specific grid cell
void Fluid::solveIncompressibilityAt(int x, int y)
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
}

void Fluid::solveCompressibility()
{
	for (int i = 0; i < relaxationSteps; i++)
	{
		updateFlowSources();
		updateDensitySources();
//#pragma omp parallel for num_threads(12)
		for (int x = 1; x < sizeX - 1; x++)
		{
			for (int y = 1; y < sizeY - 1; y++)
			{
				if (fluidField[x][y] == 1 && flowSource[x][y] == glm::dvec2{ 0, 0 })
				{
					double divergence = (flowX[x + 1][y] * fluidField[x + 1][y]) -
						(flowX[x][y] * fluidField[x - 1][y]) +
						(flowY[x][y + 1] * fluidField[x][y + 1]) -
						(flowY[x][y] * fluidField[x][y - 1]);
					double fluidCount = fluidField[x + 1][y] + fluidField[x - 1][y] + fluidField[x][y + 1] + fluidField[x][y - 1];
					density[x][y] -= timeStep * divergence;
					pressureGrid[x][y] = density[x][y];
				}
				else
				{
					density[x][y] = baseDensity;
				}
			}
		}
#pragma omp parallel for num_threads(12)
		for (int x = 2; x < sizeX - 2; x++)
		{
			for (int y = x % 2 + 1; y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 1 && flowSource[x][y] == glm::dvec2{ 0, 0 })
				{
					solveCompressibilityAt(x, y);
				}
			}
		}
#pragma omp parallel for num_threads(12)
		for (int x = 2; x < sizeX - 2; x++)
		{
			for (int y = 2 - (x % 2); y < sizeY - 1; y += 2)
			{
				if (fluidField[x][y] == 1 && flowSource[x][y] == glm::dvec2{ 0, 0 })
				{
					solveCompressibilityAt(x, y);
				}
			}
		}
	}
}

void Fluid::solveCompressibilityAt(int x, int y)
{
	double ptDensity = density[x][y];
	double fluidCount = fluidField[x + 1][y] + fluidField[x - 1][y] + fluidField[x][y + 1] + fluidField[x][y - 1];
	double avgDensity = density[x - 1][y] * fluidField[x - 1][y] +
		density[x + 1][y] * fluidField[x + 1][y] +
		density[x][y - 1] * fluidField[x][y - 1] +
		density[x][y + 1] * fluidField[x][y + 1];
	avgDensity = avgDensity / fluidCount;
	double pressure = ptDensity/* - avgDensity*/;
	double push = (1.0 - compressibility) * timeStep * pressure / fluidCount;
	flowX[x + 1][y] += push * fluidField[x + 1][y];
	flowX[x][y] -= push * fluidField[x - 1][y];
	flowY[x][y + 1] += push * fluidField[x][y + 1];
	flowY[x][y] -= push * fluidField[x][y - 1];
}

// Advecting(moving) velocity
void Fluid::advectVelocity()
{
	Grid<double> newFlowX = flowX;
	Grid<double> newFlowY = flowY;
#pragma omp parallel for num_threads(12)
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
			if (abs(sourceFrac.x) < timeStep / 100) sourceFrac.x = 0;
			if (abs(sourceFrac.y) < timeStep / 100) sourceFrac.y = 0;
			double d1 = glm::mix(flowX[source.x][source.y], flowX[source.x][source.y + 1], sourceFrac.y);
			double d2 = glm::mix(flowX[source.x + 1][source.y], flowX[source.x + 1][source.y + 1], sourceFrac.y);
			double sourceFlow = glm::mix(d1, d2, sourceFrac.x);
			newFlowX[x][y] = sourceFlow;
		}
	}
#pragma omp parallel for num_threads(12)
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
			if (sourceFrac.y > sizeY - 1)
			{
				double ratio = (sizeY - 1 - y) / (sourceFrac.y - y);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{ x, y };
			}
			glm::ivec2 source = sourceFrac;
			sourceFrac -= source;
			if (abs(sourceFrac.x) < timeStep / 100) sourceFrac.x = 0;
			if (abs(sourceFrac.y) < timeStep / 100) sourceFrac.y = 0;
			double d1 = glm::mix(flowY[source.x][source.y], flowY[source.x][source.y + 1], sourceFrac.y);
			double d2 = glm::mix(flowY[source.x + 1][source.y], flowY[source.x + 1][source.y + 1], sourceFrac.y);
			double sourceFlow = glm::mix(d1, d2, sourceFrac.x);
			newFlowY[x][y] = sourceFlow;
		}
	}
	flowX = newFlowX;
	flowY = newFlowY;
}

// Advecting(moving) dye
void Fluid::advectDye()
{
	Grid<glm::dvec4> newDye = dye;
#pragma omp parallel for num_threads(12)
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

void Fluid::advectDensity()
{
	Grid<double> newDensity = density;
#pragma omp parallel for num_threads(12)
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
			double sourceDensity;
			double d1;
			double d2;

			d1 = glm::mix(density[source.x][source.y], density[source.x][source.y + 1], sourceFrac.y);
			d2 = glm::mix(density[source.x + 1][source.y], density[source.x + 1][source.y + 1], sourceFrac.y);
			sourceDensity = glm::mix(d1, d2, sourceFrac.x);

			newDensity[x][y] = sourceDensity;
		}
	}
	density = newDensity;
}

// Amplifying vortices
void Fluid::vorticityConfinement()
{
	Grid<double> newFlowX = flowX;
	Grid<double> newFlowY = flowY;
#pragma omp parallel for num_threads(12)
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