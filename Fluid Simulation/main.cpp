#include "Fluid.hpp"
#include "FluidRender.hpp"
#include <argparse/argparse.hpp>

std::shared_ptr<Fluid> fluid;

int main(int argc, char* argv[])
{
	/*
	Basic Commands ".\Fluid Simulation.exe" Input.png --DyeSourceImage DyeInput.png --DrawMode 0 --DrawMinMax -10 10 --DrawLines 1 4 --RenderScale 6
	Additional Commands --MaxFrames 3600
	*/

	argparse::ArgumentParser program("FluidSimulation");

	program.add_argument("--SourceImage")
		.help("Specify name of image source file (including file extension).");

	program.add_argument("--DyeSourceImage")
		.help("Image to determine dye sources. Not strictly needed for dye mode.");

	program.add_argument("--Vorticity")
		.help("Set the vorticity of the fluid.")
		.scan<'g', double>()
		.default_value(0.0);

	program.add_argument("--RelaxationSteps")
		.help("Set the number of relaxation steps for solving incompressibility. 25 recommended.")
		.scan<'i', int>()
		.default_value(25);

	program.add_argument("--DrawMode")
		.help("What the simulation renders: 0 = Dye, 1 = Pressure, 2 = Vorticity. --DrawMinMax should be used if --DrawMode is set to 1 or 2.")
		.scan<'i', int>()
		.default_value(0);

	program.add_argument("--DrawMinMax")
		.help("Min and max values for rendering pressure/vorticity. Does nothing if DrawMode is set to 0.")
		.nargs(2)
		.scan<'i', int>()
		.default_value(std::vector<int>{0, 0});

	program.add_argument("--RenderScale")
		.help("What scale the simulation is rendered at.")
		.scan<'i', int>()
		.default_value(1);

	program.add_argument("--DrawLines")
		.help("Should velocity lines be rendered and at what size (boolean, int).")
		.nargs(2)
		.scan<'i', int>()
		.default_value(std::vector<int>{0});

	program.add_argument("--MaxFrames")
		.help("How many frames should be saved as .png files")
		.scan<'i', int>()
		.default_value(0);

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	// Construct fluid(shared_ptr)
	Image inputImage = LoadImage(program.get<>("SourceImage").c_str());
	if (program.is_used("--SourceImage"))
	{
		if (program.is_used("--DyeSourceImage"))
		{
			Image dyeInputImage = LoadImage(program.get<>("--DyeSourceImage").c_str());
			fluid = std::make_shared<Fluid>(inputImage, dyeInputImage,
				program.get<double>("--Vorticity"),
				program.get<int>("--RelaxationSteps"));
		}
		else
		{
			fluid = std::make_shared<Fluid>(inputImage, glm::dvec4{ 1,0,0,1 },
				program.get<double>("--Vorticity"),
				program.get<int>("--RelaxationSteps"));
		}
	}
	else
	{

	}

	//Construct fluidRender
	FluidRender fluidRender(fluid, 
		glm::dvec4{0,0,0,1}, glm::dvec4{1,1,1,1}, 
		program.get<int>("--RenderScale"), 
		program.get<int>("--MaxFrames"), 
		program.get<int>("--DrawMode"), 
		glm::dvec2{ program.get<std::vector<int>>("--DrawMinMax")[0],program.get<std::vector<int>>("--DrawMinMax")[1] },
		program.get<std::vector<int>>("--DrawLines")[0],
		program.get<std::vector<int>>("--DrawLines")[1]);

	// Main loop of fluid simulation
	fluidRender.mainLoop();

	return 0;
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
