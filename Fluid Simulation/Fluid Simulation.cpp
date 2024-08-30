#include "Fluid.hpp"

int main()
{
	fluid newFluid(200, 100);
	for (int x = 0; x < newFluid.sizeX; x++)
	{
		for (int y = 0; y < newFluid.sizeY; y++)
		{
			if (sqrt(pow((x - newFluid.sizeX / 2 + 50),2) + pow((y - newFluid.sizeY / 2),2)) < 9)
			{
				newFluid.fluidField[x][y] = 0;
			}
		}
	}

	for (int y = 0; y < newFluid.sizeY; y += 1)
	{
		for (int x = 1; x < 3; x++)
		{
			newFluid.sourceX[x][y] = 7;
			newFluid.sourceX[newFluid.sizeX - x][y] = 7;
		}
	}

	int streamWidth = 8;

	for (int y = newFluid.sizeY / 2 - streamWidth / 2; y <= newFluid.sizeY / 2 + streamWidth / 2; y++)
	{
		newFluid.dyeSource[2][y] = { 1,0,0,1 };
		newFluid.dyeSource[3][y] = { 1,0,0,1 };
	}

	while (true)
	{
		newFluid.update();
		newFluid.draw();
		//if (newFluid.frames <= 4320)
		//{
		//	std::string fileName = "frame" + std::to_string(newFluid.frames) + ".png";
		//	TakeScreenshot(fileName.c_str()); //ffmpeg -framerate 144 -i frame%d.png -vcodec libx264 -crf 18 -pix_fmt yuv420p output.mp4
		//}
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
