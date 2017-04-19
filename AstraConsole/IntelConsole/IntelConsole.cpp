// IntelConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RealSenceController.h"


int main()
{
	const int numFramesToRun = 100;
	const int saveImageFrequency = 20;
	RealSenceController controller;
	controller.Run(numFramesToRun, saveImageFrequency);

    return 0;
}

