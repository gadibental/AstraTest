// IntelConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\RealSenceCameraLibrary\RealSenceController.h"


int main()
{
	const int numFramesToRun = 100;
	const int saveImageFrequency = 20;
	RealSenceController controller;
	controller.SetShowColour(true);
	controller.SetShowDepth(true);
	controller.SetShowIR(false);
	controller.Run(numFramesToRun, saveImageFrequency);

    return 0;
}

