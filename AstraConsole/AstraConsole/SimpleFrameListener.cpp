#include "stdafx.h"
#include "SimpleFrameListener.h"

#include <astra/astra.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 

SimpleFrameListener::SimpleFrameListener()
	: lastWidth_RGB(0)
	, lastHeight_RGB(0)
	, lastWidth_D(0)
	, lastHeight_D(0)
{

}

void SimpleFrameListener::on_frame_ready(astra::StreamReader& reader, astra::Frame& frame)
{
	const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();
	if (depthFrame.is_valid())
	{
		int frameCount = ShowDepth(depthFrame);

		if ((frameCount % 10) == 0)
		{
			SaveVertexMap(reader.stream<astra::DepthStream>().coordinateMapper());
		}
	}

	const astra::ColorFrame colorFrame = frame.get<astra::ColorFrame>();
	if (colorFrame.is_valid())
	{
		ShowColour(colorFrame);
	}

	check_fps();
	cv::waitKey(1); // refresh
}

void SimpleFrameListener::SaveVertexMap(const astra::CoordinateMapper& mapper)
{
	if (lastWidth_D == 0 || lastHeight_D == 0 ||
		lastWidth_RGB == 0 || lastHeight_RGB == 0)
	{
		return; // no data
	}


	std::ofstream file("vertices.csv", std::ios::out);

	file << "X,Y,Z,R,G,B\n";

	int depthIndex = 0;
	for (unsigned int y = 0; y < lastHeight_D; ++y)
	{
		for (unsigned int x = 0; x < lastWidth_D; ++x)
		{
			ushort depthValue = buffer_D[depthIndex];
			if (depthValue != 0)
			{
				astra::Vector3f depthPixel((float)x, (float)y, depthValue);
				astra::Vector3f vertex = mapper.convert_depth_to_world(depthPixel);
				file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
				// get the corresponding colour value
				int cx = x * lastWidth_RGB / lastWidth_D;
				int cy = y * lastHeight_RGB / lastHeight_D;
				int colourIndex = cy * lastWidth_RGB + cx;
				astra::RgbPixel colourValue = buffer_RGB[colourIndex];
				file << (int)colourValue.r << ',' << (int)colourValue.g << ',' << (int)colourValue.b << ',';

				file << "\n";
			}
			depthIndex++;
		}
	}

}



int SimpleFrameListener::ShowDepth(const astra::DepthFrame& depthFrame)
{
	int width = depthFrame.width();
	int height = depthFrame.height();
	if (width != lastWidth_D || height != lastHeight_D)
	{
		buffer_D = buffer_ptrD(new int16_t[depthFrame.length()]);
		lastWidth_D = width;
		lastHeight_D = height;
	}
	depthFrame.copy_to(buffer_D.get());

	const float maxDepth = 1000.0f;
	cv::Mat depthImage = cv::Mat(height, width, CV_16UC1);
	cv::Mat showImage = cv::Mat(height, width, CV_8UC1);
	size_t index = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			short pixelValue = buffer_D[index];

			depthImage.at<ushort>(cv::Point(x, y)) = pixelValue;
			showImage.at<uchar>(cv::Point(x, y)) = (uchar)(pixelValue * 255 / maxDepth);
			index++;
		}
	}

	cv::imshow("depth", showImage);
	//		cv::imwrite("depth.png", showImage);
	//SaveMat("depth.txt", depthImage);

	return depthFrame.frame_index();
}

void SimpleFrameListener::ShowColour(const astra::ColorFrame& colorFrame)
{
	int width = colorFrame.width();
	int height = colorFrame.height();
	if (width != lastWidth_RGB || height != lastHeight_RGB)
	{
		buffer_RGB = buffer_ptrRGB(new astra::RgbPixel[colorFrame.length()]);
		lastWidth_RGB = width;
		lastHeight_RGB = height;
	}
	colorFrame.copy_to(buffer_RGB.get());

	cv::Mat colourImage = cv::Mat(height, width, CV_8UC3);
	size_t index = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			astra::RgbPixel pixelValue = buffer_RGB[index];
			cv::Vec3b bgr(pixelValue.b, pixelValue.g, pixelValue.r);
			colourImage.at<cv::Vec3b>(cv::Point(x, y)) = bgr;
			index++;
		}
	}

	cv::imshow("colour", colourImage);
	//		cv::imwrite("colour.png", colourImage);
}





void SimpleFrameListener::check_fps()
{
	const double frameWeight = 0.2;

	auto newTimepoint = clock_type::now();
	auto frameDuration = std::chrono::duration_cast<duration_type>(newTimepoint - lastTimepoint_);

	frameDuration_ = frameDuration * frameWeight + frameDuration_ * (1 - frameWeight);
	lastTimepoint_ = newTimepoint;

	double fps = 1.0 / frameDuration_.count();

	auto precision = std::cout.precision();
	std::cout << std::fixed
		<< std::setprecision(1)
		<< fps << " fps ("
		<< std::setprecision(2)
		<< frameDuration.count() * 1000 << " ms)"
		<< std::setprecision(precision)
		<< std::endl;
}
