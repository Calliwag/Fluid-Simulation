#pragma once
#include "Fluid.hpp"
#include "FluidRender.hpp"
#include "FluidCreate.hpp"
#include <argparse/argparse.hpp>

int main(int argc, char* argv[])
{
	/*
	Basic Commands ".\Fluid Simulation.exe" Input.png --DyeSourceImage DyeInput.png --DrawMode 0 --DrawMinMax -10 10 --DrawLines 1 4 --RenderScale 6
	Additional Commands --MaxFrames 3600
	*/

	argparse::ArgumentParser program("FluidSimulation");

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
		.default_value(std::vector<int>{0,1});

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
	std::shared_ptr<Fluid> fluid;
	FluidCreate create(200, 75, 6);
	create.createLoop();
	FluidInfo info(create);
	fluid = std::make_shared<Fluid>(info, program.get<double>("--Vorticity"), program.get<int>("--RelaxationSteps"));

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