#include "Fluid.hpp"

int main()
{
	Image inputImage = LoadImage("Input.png");
	fluid newFluid(inputImage, {1,0,0,1});

	//int streamWidth = 2;

	//for (int x = newFluid.sizeX / 2 - streamWidth / 2; x <= newFluid.sizeX / 2 + streamWidth / 2; x += 1)
	//{
	//	for (int y = 1; y < 3; y++)
	//	{
	//		newFluid.sourceY[x][y] = 5;
	//	}
	//	newFluid.dyeSource[x][2] = { 1,1,0,1 };
	//}
	//newFluid.pressureMinMax = { -30.0,15.0 };

	//for (int y = newFluid.sizeY / 2 - streamWidth / 2; y <= newFluid.sizeY / 2 + streamWidth / 2; y += 1)
	//{
	//	for (int x = 1; x < 3; x++)
	//	{
	//		newFluid.sourceX[x][y] = 6;
	//		newFluid.sourceX[newFluid.sizeX - x][y] = -6;
	//	}
	//	newFluid.dyeSource[2][y] = { 1,0,0,1 };
	//	newFluid.dyeSource[newFluid.sizeX - 3][y] = { 0,1,0,1 };
	//}
	//for (int x = newFluid.sizeX / 2 - streamWidth / 2; x <= newFluid.sizeX / 2 + streamWidth / 2; x += 1)
	//{
	//	for (int y = 1; y < 3; y++)
	//	{
	//		newFluid.sourceY[x][y] = 6;
	//		newFluid.sourceY[x][newFluid.sizeY - y] = -6;
	//	}
	//	newFluid.dyeSource[x][2] = { 0,0,1,1 };
	//	newFluid.dyeSource[x][newFluid.sizeY - 3] = { 1,1,0,1 };
	//}

	//for (int x = 0; x < newFluid.sizeX; x++)
	//{
	//	for (int y = 0; y < newFluid.sizeY; y++)
	//	{
	//		if (sqrt(pow((x - newFluid.sizeX / 2 + newFluid.sizeX / 3),2) + pow((y - newFluid.sizeY / 2),2)) < 9)
	//		{
	//			newFluid.fluidField[x][y] = 0;
	//		}
	//	}
	//}
	//for (int y = 0; y < newFluid.sizeY; y++)
	//{
	//	for (int x = 1; x < 3; x++)
	//	{
	//		double r = GetRandomValue(-10, 10) / 1000.0;
	//		newFluid.sourceX[x][y] = 5 + r;
	//		newFluid.sourceX[newFluid.sizeX - x][y] = 5 + r;
	//	}
	//}
	//for (int y = newFluid.sizeY / 2 - 2; y <= newFluid.sizeY / 2 + 2; y++)
	//{
	//	newFluid.dyeSource[2][y] = { 1,0,0,1 };
	//}
	//newFluid.pressureMinMax = { -15.0,15.0 };
	//newFluid.curlMinMax = { -10,10 };

	while (true)
	{
		newFluid.update();
		newFluid.draw();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
