// AstraConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <astra/astra.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 
#include "SimpleFrameListener.h"


#include "key_handler.h"

astra::DepthStream configure_depth(astra::StreamReader& reader)
{
	auto depthStream = reader.stream<astra::DepthStream>();

	//We don't have to set the mode to start the stream, but if you want to here is how:
	astra::ImageStreamMode depthMode;

	depthMode.set_width(640);
	depthMode.set_height(480);
	depthMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_DEPTH_MM);
	depthMode.set_fps(30);

	depthStream.set_mode(depthMode);
	depthStream.enable_registration(true);

	return depthStream;
}

astra::InfraredStream configure_ir(astra::StreamReader& reader, bool useRGB)
{
	auto irStream = reader.stream<astra::InfraredStream>();

	astra::ImageStreamMode irMode;
	irMode.set_width(640);
	irMode.set_height(480);

	if (useRGB)
	{
		irMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
	}
	else
	{
		irMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_GRAY16);
	}

	irMode.set_fps(30);
	irStream.set_mode(irMode);

	return irStream;
}

astra::ColorStream configure_color(astra::StreamReader& reader)
{
	auto colorStream = reader.stream<astra::ColorStream>();

	astra::ImageStreamMode colorMode;
	colorMode.set_width(640);
	colorMode.set_height(480);
	colorMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
	colorMode.set_fps(30);

	colorStream.set_mode(colorMode);

	return colorStream;
}

int main(int argc, char** argv)
{
	astra::initialize();
	set_key_handler();


	astra::StreamSet streamSet;
	astra::StreamReader reader = streamSet.create_reader();

	SimpleFrameListener listener;

	auto depthStream = configure_depth(reader);
	depthStream.start();

	auto colorStream = configure_color(reader);
	colorStream.start();

	reader.stream<astra::DepthStream>().start();
	reader.stream<astra::ColorStream>().start();

	std::cout << "depthStream -- hFov: "
		<< reader.stream<astra::DepthStream>().hFov()
		<< " vFov: "
		<< reader.stream<astra::DepthStream>().vFov()
		<< std::endl;

	reader.add_listener(listener);


	do
	{
		astra_temp_update();
	} while (shouldContinue);

	reader.remove_listener(listener);

	astra::terminate();
}
