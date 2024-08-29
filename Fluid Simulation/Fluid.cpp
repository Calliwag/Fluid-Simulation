#include "Fluid.hpp"
raylib::Window window(100, 100, "Fluid Simulation");

using namespace std;

fluid::fluid(int _sizeX, int _sizeY)
{
	sizeX = _sizeX;
	sizeY = _sizeY;

	dye.resize(sizeX);
	fluidField.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		dye[i].resize(sizeY, glm::dvec4{0,0,0,1});
		fluidField[i].resize(sizeY);
	}

	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			fluidField[x][y] = 1;
		}
	}

	flowX.resize(sizeX + 1);
	for (int i = 0; i < sizeX + 1; i++)
	{
		flowX[i].resize(sizeY);
	}
	flowY.resize(sizeX);
	for (int i = 0; i < sizeX; i++)
	{
		flowY[i].resize(sizeY + 1);
	}
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
}

void fluid::draw()
{
	window.BeginDrawing();
	window.ClearBackground(WHITE);
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			Color cellColor = WHITE;
			cellColor.r = dye[x][y].x * 255;
			cellColor.g = dye[x][y].y * 255;
			cellColor.b = dye[x][y].z * 255;
			cellColor.a = dye[x][y].w * 255;
			DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);
		}
	}
	for (int x = 1; x < sizeX - 1; x += 4)
	{
		for (int y = 1; y < sizeY - 1; y += 4)
		{
			glm::dvec2 velocity = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
									flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
			DrawLine(x * renderScale + renderScale / 2, y * renderScale + renderScale / 2, (x + velocity.x * 0.5) * renderScale + renderScale / 2, (y + velocity.y * 0.5) * renderScale + renderScale / 2, WHITE);
		}
	}
	window.EndDrawing();
}

void fluid::update()
{
	frames++;
	int streamWidth = 16;
	for (int i = 0; i < 5; i++)
	{
		project();
	}

	for (int y = sizeY / 2 - streamWidth / 2; y <= sizeY / 2 + streamWidth / 2; y += 1)
	{
		for (int x = 1; x < 3; x++)
		{
			double r1 = GetRandomValue(0, 200) / 30.0;
			flowX[x][y] = 0 + r1;
			double r2 = GetRandomValue(0, 200) / 30.0;
			flowX[sizeX - x][y] = -(0 + r2);
		}
		dye[2][y] = { 1,0,0,1 };
		dye[sizeX - 3][y] = { 0,1,0,1 };
	}
	for (int x = sizeX / 2 - streamWidth / 2; x <= sizeX / 2 + streamWidth / 2; x += 1)
	{
		for (int y = 1; y < 3; y++)
		{
			double r1 = GetRandomValue(0, 200) / 30.0;
			flowY[x][y] = 0 + r1;
			double r2 = GetRandomValue(0, 200) / 30.0;
			flowY[x][sizeY - y] = -(0 + r2);
		}
		dye[x][2] = { 0,0,1,1 };
		dye[x][sizeY - 3] = {0,0,0,1};
	}
	advect();
	decayDye();
	vorticityConfinement();
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

			dye[x][y] = glm::dvec4{ 0,0,0,1 } - decayValue * (glm::dvec4{ 0,0,0,1 } - dye[x][y]);

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
	vector<vector<double>> newFlowX = flowX;
	vector<vector<double>> newFlowY = flowY;

	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
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
			double correctionFactor = 1.0 * divergence / fluidCount;
			newFlowX[x + 1][y] -= correctionFactor * fluidField[x + 1][y];
			newFlowX[x][y] += correctionFactor * fluidField[x - 1][y];
			newFlowY[x][y + 1] -= correctionFactor * fluidField[x][y + 1];
			newFlowY[x][y] += correctionFactor * fluidField[x][y - 1];
		}
	}

	flowX = newFlowX;
	flowY = newFlowY;
}

void fluid::advect()
{
	vector<vector<glm::dvec4>> newDye = dye;
	vector<vector<double>> newFlowX = flowX;
	vector<vector<double>> newFlowY = flowY;
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			if (fluidField[x][y] == 0)
			{
				continue;
			}
			glm::dvec2 velocity = getGridVelocity(x, y);
			if (length(velocity) * timeStep < 0.01)
			{
				continue;
			}
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;
			if (sourceFrac.x < 1)
			{
				double ratio = (1 - x) / (sourceFrac.x - x);
				sourceFrac = ratio * (sourceFrac - glm::dvec2{ x, y }) + glm::dvec2{x, y};
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

			//Alpha blend
			d1 = glm::mix(dye[source.x][source.y], dye[source.x][source.y + 1], sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y], dye[source.x + 1][source.y + 1], sourceFrac.y);
			sourceDye = glm::mix(d1, d2, sourceFrac.x);

			newDye[x][y] = sourceDye;
		}
	}
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
	dye = newDye;
	flowX = newFlowX;
	flowY = newFlowY;
}

double fluid::curl(int x, int y)
{
	double curl = getGridVelocity(x,y+1).x - 
				  getGridVelocity(x,y-1).x + 
				  getGridVelocity(x-1,y).y - 
				  getGridVelocity(x+1,y).y;
	//cout << curl << endl;
	return curl;
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
			direction.x = abs(curl(x, y - 1)) - abs(curl(x, y + 1));
			direction.y = abs(curl(x + 1, y)) - abs(curl(x - 1, y));

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
