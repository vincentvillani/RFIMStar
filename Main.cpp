/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <iostream>

#include "Header/RFIMStar.h"
#include "Header/UnitTests.h"
#include "tclap/CmdLine.h"
#include "Header/RFIMConfiguration.h"


//TODO: Add the ability to generate a mask back in
//TODO: Add the ability to use the Z-values to detect outliers or not (uncomment code and have a switch basically)
//TODO: Calculate and write out signal statistics
	//Mean, std-dev, variance - all per beam
//TODO: Add flag to only do the first x samples
//TODO: Per sample resolution?
//TODO: Set the nbits (and other stuff?) for mask file in the sigproc header
//TODO: Figure put a better way to set/use a threshold?
//TODO: insert test signals into the actual filterbank files
//TODO: Add the ability to read in and write out different nbit values
//TODO: (Probably not anymore) Do a normalisation step before computing the covariance matrix, to see if there are any huge outliers, if there are, replace it with white noise or zeroes?
		//Median absolute deviation (MAD) to normalise
int main(int argc, char** argv)
{

	//Unit tests
	//RunAllUnitTests();


	RFIMConfiguration configuration;

	// Wrap everything in a try block.  Do this every time,
	// because exceptions will be thrown for problems when parsing
	try
	{


		TCLAP::CmdLine cmd("RFIMStar removes RFI!", ' ', "0.1");


		TCLAP::UnlabeledValueArg<std::string> inputFilepath("inputFilesPath", "The path to the input files.",
				true, "Unset", "A string containing the path to the input files. The path must contain folders in the standard "
						"Swinburne format. I.E. folders called 01 ~ beamnumber (default is 13). "
						"Each folder must contain filterbank files");

		TCLAP::UnlabeledValueArg<std::string> outputFilepath("outputFilesPath", "The path to the output files.",
				false, "", "A string containing the path to the output files.");

		TCLAP::ValueArg<uint32_t> beamNumber("b", "beams", "Sets the number of filterbank files that RFIMStar will open.", false, 13,
				"An unsigned integer containing the number of filterbank files to open");

		TCLAP::ValueArg<uint32_t> windowSize("w", "window",
				"Sets the number of samples that will be integrated before running the algorithm. Shorted window sizes will be better for impulsive RFI, while longer window sizes will be better for red/brown noise. Should probably be at least beamNumber squared!"
				, true, 169,
				"An unsigned integer containing the number of samples to integrate before running the algorithm");

		TCLAP::ValueArg<float> powerThreshold("p", "powerThreshold",
				"Sets the power threshold for removing RFI. Set it to be low if you want to be more aggressive, higher if you want to be gentle."
				, true, 2.0f,
				"A floating point number indicating the RFI strength needed in order for it to be removed");

		TCLAP::ValueArg<int> rfimMode("m", "mode",
				"Sets the mode of RFIM. Controls what it does when it finds RFIM"
				, false, ATTENUATE,
				"An enum value that indicates the mode for RFIM to run in");

		uint32_t defaultThreadNum = std::thread::hardware_concurrency();

		if(defaultThreadNum == 0)
			defaultThreadNum = 2;

		TCLAP::ValueArg<uint32_t> threadNum("t", "threads",
				"Sets the number of worker threads. More worker threads means more ram will be used!", false, defaultThreadNum,
				"An unsigned integer that contains the number of worker threads to use during computation");


		TCLAP::ValueArg<uint32_t> rawBlockNum("r", "iobuffers", "Sets the number of input/output buffers", false, 6,
				"An unsigned integer that sets the number of iobuffers. Set this to be a higher number of you think your bound by i/o");

		TCLAP::SwitchArg shouldGenerateMask("m", "mask", "Set if you want to generate a window size mask instead of doing de-projection",
				false);


		// Add the argument nameArg to the CmdLine object. The CmdLine object
		// uses this Arg to parse the command line.
		cmd.add(inputFilepath);
		cmd.add(outputFilepath);
		cmd.add(beamNumber);
		cmd.add(windowSize);
		cmd.add(powerThreshold);
		cmd.add(threadNum);
		cmd.add(rawBlockNum);
		cmd.add(shouldGenerateMask);

		// Parse the argv array.
		cmd.parse( argc, argv );



		std::string inputFilesString = inputFilepath.getValue();

		//Figure out the name of the actual file
		size_t substringPosition = inputFilesString.find_last_of('/');

		//Assume that the whole string is the filename
		if(substringPosition == std::string::npos)
		{
			configuration.inputFilenamePrefix = "";
			configuration.inputFilenamePostfix = inputFilesString;
		}
		else
		{
			configuration.inputFilenamePrefix = inputFilesString;
			configuration.inputFilenamePostfix = inputFilesString.substr(substringPosition + 1, inputFilesString.length() - substringPosition) + ".fil";
		}

		//if(outputFilepath.getValue().compare(".") == 0)
		configuration.outputFilenamePrefix = outputFilepath.getValue();
		configuration.outputFilenamePostfix = inputFilesString.substr(substringPosition + 1, inputFilesString.length() - substringPosition) + "RFIMStar.fil";

		configuration.beamNum = beamNumber.getValue();
		configuration.windowSize = windowSize.getValue();
		configuration.numberOfWorkerThreads = threadNum.getValue();
		configuration.rawDataBlockNum = rawBlockNum.getValue();
		configuration.generatingMask = shouldGenerateMask.getValue();
		configuration.powerThreshold = powerThreshold.getValue();
		configuration.rfimMode = (RFIMMode)rfimMode.getValue();


	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		exit(2);
	}


	std::cout << "Input prefix: " << configuration.inputFilenamePrefix << std::endl;
	std::cout << "Input postfix: " << configuration.inputFilenamePostfix << std::endl;
	std::cout << "Output filename prefix: " << configuration.outputFilenamePrefix << std::endl;
	std::cout << "Output filename postfix: " << configuration.outputFilenamePostfix << std::endl;
	std::cout << "Beam number: " << configuration.beamNum << std::endl;
	std::cout << "Window size: " << configuration.windowSize << std::endl;
	std::cout << "Worker threads " << configuration.numberOfWorkerThreads << std::endl;
	std::cout << "Raw data blocks: " << configuration.rawDataBlockNum << std::endl;
	std::cout << "Generating mask?: " << configuration.generatingMask << std::endl;
	std::cout << "Power threshold: " << configuration.powerThreshold << std::endl;

	if(configuration.rfimMode == ZERO)
	{
		std::cout << "RFIM Mode: Zero" << std::endl;
	}
	else if(configuration.rfimMode == ATTENUATE)
	{
		std::cout << "RFIM Mode: Attenuate" << std::endl;
	}


	/*
	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 10;
	uint32_t numberOfWorkerThreads = 15;
	uint32_t windowSize = 169;
	uint32_t dimensionsToReduce = 1;
	bool generatingMask = true;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2014-12-06-04:36:26/";
	std::string inputFilenamePostfix = "/2014-12-06-04:36:26.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/badObservation/";
	std::string outputFilenamePostfix = "pna1-169W.fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, dimensionsToReduce, rawDataBlockNum,
			inputFilenamePrefix, inputFilenamePostfix,
			outputFilenamePrefix, outputFilenamePostfix, generatingMask);
	*/

	RFIMStarRoutine(&configuration);

	std::cout << "Done!" << std::endl;







	return 0;
}
