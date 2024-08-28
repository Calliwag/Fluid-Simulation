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
		dye[i].resize(_sizeY, WHITE);
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
			DrawRectangle(x * renderScale, y * renderScale, renderScale, renderScale, dye[x][y]);
		}
	}
	//for (int x = 1; x < sizeX - 1; x++)
	//{
	//	for (int y = 1; y < sizeY - 1; y++)
	//	{
	//		glm::dvec2 velocity = { flowX[x + 1][y] * fluidField[x + 1][y] + flowX[x][y] * fluidField[x - 1][y],
	//								flowY[x][y + 1] * fluidField[x][y + 1] + flowY[x][y] * fluidField[x][y - 1] };
	//		DrawLine(x * renderScale + renderScale / 2, y * renderScale + renderScale / 2, (x + velocity.x * 0.5) * renderScale + renderScale / 2, (y + velocity.y * 0.5) * renderScale + renderScale / 2, WHITE);
	//	}
	//}
	window.EndDrawing();
}

void fluid::update()
{
	for (int i = 0; i < 2; i++)
	{
		project();
	}
	for (int y = 45; y <= 55; y += 1)
	{
		for (int x = 1; x < 3; x++)
		{
			double r1 = GetRandomValue(0, 100) / 30.0;
			flowX[x][y] = 0 + r1;
			double r2 = GetRandomValue(0, 100) / 30.0;
			flowX[sizeX - x][y] = -(0 + r2);
		}
		dye[2][y] = RED;
		dye[sizeX - 3][y] = BLUE;
	}
	for (int x = 45; x <= 55; x += 1)
	{
		for (int y = 1; y < 3; y++)
		{
			double r1 = GetRandomValue(0, 100) / 30.0;
			flowY[x][y] = 0 + r1;
			//double r2 = GetRandomValue(0, 100) / 30.0;
			//flowY[x][sizeY - y] = -(5 + r2);
		}
		dye[x][2] = GREEN;
		//dye[x][sizeY - 3] = BLACK;
	}
	advect();
	decayDye();
}

void fluid::decayDye()
{
	for (int x = 0; x < sizeX; x++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			dye[x][y].a = 255 - (255 - dye[x][y].a) * decayValue;
			dye[x][y].r = 255 - (255 - dye[x][y].r) * decayValue;
			dye[x][y].g = 255 - (255 - dye[x][y].g) * decayValue;
			dye[x][y].b = 255 - (255 - dye[x][y].b) * decayValue;
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
	vector<vector<Color>> newDye = dye;
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
			glm::ivec2 source = sourceFrac;
			sourceFrac -= source;
			Color sourceDye = BLACK;
			double d1 = 0;
			double d2 = 0;

			//Alpha blend
			d1 = glm::mix(dye[source.x][source.y].a, dye[source.x][source.y + 1].a, sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y].a, dye[source.x + 1][source.y + 1].a, sourceFrac.y);
			sourceDye.a = glm::mix(d1, d2, sourceFrac.x);

			//Red blend
			d1 = glm::mix(dye[source.x][source.y].r, dye[source.x][source.y + 1].r, sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y].r, dye[source.x + 1][source.y + 1].r, sourceFrac.y);
			sourceDye.r = glm::mix(d1, d2, sourceFrac.x);

			//Green blend
			d1 = glm::mix(dye[source.x][source.y].g, dye[source.x][source.y + 1].g, sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y].g, dye[source.x + 1][source.y + 1].g, sourceFrac.y);
			sourceDye.g = glm::mix(d1, d2, sourceFrac.x);

			//Blue blend
			d1 = glm::mix(dye[source.x][source.y].b, dye[source.x][source.y + 1].b, sourceFrac.y);
			d2 = glm::mix(dye[source.x + 1][source.y].b, dye[source.x + 1][source.y + 1].b, sourceFrac.y);
			sourceDye.b = glm::mix(d1, d2, sourceFrac.x);

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
			if (sourceFlow > 100)
			{
				cout << "BAD" << endl;
			}
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