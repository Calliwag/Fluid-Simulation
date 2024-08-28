#include "Fluid.hpp"
raylib::Window window(100, 100, "Fluid Simulation");


fluid::fluid(int _sizeX, int _sizeY)
{
	sizeX = _sizeX;
	sizeY = _sizeY;
	dye.resize(_sizeX);
	fluidField.resize(_sizeX);
	for (int i = 0; i < _sizeX; i++)
	{
		dye[i].resize(_sizeY);
		fluidField[i].resize(_sizeY);
	}

	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			fluidField[x][y] = 1;
		}
	}

	flowX.resize(_sizeX + 1);
	for (int i = 0; i < _sizeX + 1; i++)
	{
		flowX[i].resize(_sizeY);
	}
	flowY.resize(_sizeX);
	for (int i = 0; i < _sizeX; i++)
	{
		flowY[i].resize(_sizeY + 1);
	}
	SetWindowSize(sizeX * renderScale, sizeY * renderScale);
}

void fluid::draw()
{
	window.BeginDrawing();
	window.ClearBackground(BLACK);
	for (int x = 1; x < sizeX - 1; x++)
	{
		for (int y = 1; y < sizeY - 1; y++)
		{
			Color cellColor = BLACK;
			cellColor.r = 255 * min(dye[x][y], 1.0);
			DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, cellColor);
			glm::dvec2 velocity = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
									flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
			//DrawLine(x * renderScale, y * renderScale, (x + velocity.x * 0.5) * renderScale, (y + velocity.y * 0.5) * renderScale, WHITE);
		}
	}
	window.EndDrawing();
}

void fluid::update()
{
	for (int i = 0; i < 5; i++)
	{
		project();
	}
	for (int y = 49; y <= 51; y += 1)
	{
		for (int x = 0; x < 2; x++)
		{
			int r = GetRandomValue(0, 3);
			flowX[x + 1][y] = 10 + r;
		}
		dye[2][y] = 0.7 + GetRandomValue(0, 30) / 100.0;
	}
	advect();
}

void fluid::gravity()
{
}

void fluid::project()
{
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
			double divergence = (flowX[x + 1][y] * fluidField[x + 1][y]) - 
								(flowX[x][y] * fluidField[x - 1][y]) + 
								(flowY[x][y + 1] * fluidField[x][y + 1]) - 
								(flowY[x][y] * fluidField[x][y - 1]);
			double fluidCount = fluidField[x + 1][y] + fluidField[x - 1][y] + fluidField[x][y + 1] + fluidField[x][y - 1];
			double correctionFactor = 1.0 * divergence / fluidCount;
			newFlowX[x + 1][y] -= correctionFactor;
			newFlowX[x][y] += correctionFactor;
			newFlowY[x][y + 1] -= correctionFactor;
			newFlowY[x][y] += correctionFactor;
		}
	}

	flowX = newFlowX;
	flowY = newFlowY;
}

void fluid::advect()
{
	vector<vector<double>> newDye = dye;
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
			glm::dvec2 velocity = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
									flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
			glm::dvec2 sourceFrac = glm::dvec2{ x,y } - velocity * timeStep;
			if (sourceFrac.x < 1)
			{
				sourceFrac.x = 1;
			}
			if (sourceFrac.x > sizeX - 2)
			{
				sourceFrac.x = sizeX - 2;
			}
			if (sourceFrac.y < 1)
			{
				sourceFrac.y = 1;
			}
			if (sourceFrac.y > sizeY - 2)
			{
				sourceFrac.y = sizeY - 2;
			}
			glm::ivec2 source = sourceFrac;
			sourceFrac -= source;
			double d1 = glm::mix(dye[source.x][source.y], dye[source.x][source.y + 1], sourceFrac.y);
			double d2 = glm::mix(dye[source.x + 1][source.y], dye[source.x + 1][source.y + 1], sourceFrac.y);
			double sourceDye = glm::mix(d1, d2, sourceFrac.x);
			//if (dye[x][y] != sourceDye)
			//{
			//	cout << "Movement!" << endl;
			//}
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
				sourceFrac.x = 1;
			}
			if (sourceFrac.x > flowX.size() - 2)
			{
				sourceFrac.x = flowX.size() - 2;
			}
			if (sourceFrac.y < 1)
			{
				sourceFrac.y = 1;
			}
			if (sourceFrac.y > flowX[x].size() - 2)
			{
				sourceFrac.y = flowX[x].size() - 2;
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
				sourceFrac.x = 1;
			}
			if (sourceFrac.x > flowY.size() - 2)
			{
				sourceFrac.x = flowY.size() - 2;
			}
			if (sourceFrac.y < 1)
			{
				sourceFrac.y = 1;
			}
			if (sourceFrac.y > flowY[x].size() - 2)
			{
				sourceFrac.y = flowY[x].size() - 2;
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