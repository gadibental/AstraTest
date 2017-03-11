// AstraConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <astra/astra.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 

#include "key_handler.h"

class SampleFrameListener : public astra::FrameListener
{
private:
	using buffer_ptrD = std::unique_ptr<int16_t[]>;
	buffer_ptrD buffer_D;
	unsigned int lastWidth_D;
	unsigned int lastHeight_D;

	using buffer_ptrRGB = std::unique_ptr<astra::RgbPixel[]>;
	buffer_ptrRGB buffer_RGB;
	unsigned int lastWidth_RGB;
	unsigned int lastHeight_RGB;

public:
	virtual void on_frame_ready(astra::StreamReader& reader,
		astra::Frame& frame) override
	{
		const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();
		if (depthFrame.is_valid())
		{
//			print_depth(depthFrame,
//				reader.stream<astra::DepthStream>().coordinateMapper());
			check_fps();
			ShowDepth(depthFrame);
		}

		const astra::ColorFrame colorFrame = frame.get<astra::ColorFrame>();
		if (colorFrame.is_valid())
		{
//			print_color(colorFrame);
			ShowColour(colorFrame);
		}

		cv::waitKey(1); // refresh
	}


	void ShowDepth(const astra::DepthFrame& depthFrame)
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
		cv::Mat depthImage = cv::Mat(height, width, CV_8UC1);
		size_t index = 0;
//		float maxDepth = 0;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				short pixelValue = buffer_D[index];
//				maxDepth = max(maxDepth, pixelValue);

				depthImage.at<byte>(cv::Point(x, y)) = (byte)(pixelValue * 255 / maxDepth);
				index++;
			}
		}
		//if (maxDepth > 1.0f)
		//{
		//	depthImage = depthImage / maxDepth;
		//}

		cv::imshow("depth", depthImage);
		cv::imwrite("depth.png", depthImage);
	}

	void ShowColour(const astra::ColorFrame& colorFrame)
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
		cv::imwrite("colour.png", colourImage);
	}


	void print_color(const astra::ColorFrame& colorFrame)
	{
		if (colorFrame.is_valid())
		{
			int width = colorFrame.width();
			int height = colorFrame.height();
			int frameIndex = colorFrame.frame_index();

			if (width != lastWidth_RGB || height != lastHeight_RGB) {
				buffer_RGB = buffer_ptrRGB(new astra::RgbPixel[colorFrame.length()]);
				lastWidth_RGB = width;
				lastHeight_RGB = height;
			}
			colorFrame.copy_to(buffer_RGB.get());

			size_t index = (size_t)((width * (height / 2.0f)) + (width / 2.0f));
			astra::RgbPixel middle = buffer_RGB[index];

			std::cout << "color frameIndex: " << frameIndex
				<< " size :" << width << " x " << height
				<< " r: " << static_cast<int>(middle.r)
				<< " g: " << static_cast<int>(middle.g)
				<< " b: " << static_cast<int>(middle.b)
				<< std::endl;
		}
	}
	void print_depth(const astra::DepthFrame& depthFrame,
		const astra::CoordinateMapper& mapper)
	{
		if (depthFrame.is_valid())
		{
			int width = depthFrame.width();
			int height = depthFrame.height();
			int frameIndex = depthFrame.frame_index();

			//determine if buffer needs to be reallocated
			if (width != lastWidth_D || height != lastHeight_D)
			{
				buffer_D = buffer_ptrD(new int16_t[depthFrame.length()]);
				lastWidth_D = width;
				lastHeight_D = height;
			}
			depthFrame.copy_to(buffer_D.get());

			size_t index = (size_t)((width * (height / 2.0f)) + (width / 2.0f));
			short middle = buffer_D[index];

			float worldX, worldY, worldZ;
			float depthX, depthY, depthZ;
			mapper.convert_depth_to_world(width / 2.0f, height / 2.0f, middle, &worldX, &worldY, &worldZ);
			mapper.convert_world_to_depth(worldX, worldY, worldZ, &depthX, &depthY, &depthZ);

			std::cout << "depth frameIndex: " << frameIndex
				<< " size :" << width << " x " << height
				<< " value: " << middle
				<< " wX: " << worldX
				<< " wY: " << worldY
				<< " wZ: " << worldZ
				<< " dX: " << depthX
				<< " dY: " << depthY
				<< " dZ: " << depthZ
				<< std::endl;
		}
	}

	void check_fps()
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

private:
	using duration_type = std::chrono::duration < double >;
	duration_type frameDuration_{ 0.0 };

	using clock_type = std::chrono::system_clock;
	std::chrono::time_point<clock_type> lastTimepoint_;
};

int main(int argc, char** argv)
{
	astra::initialize();
	set_key_handler();


	astra::StreamSet streamSet;
	astra::StreamReader reader = streamSet.create_reader();

	SampleFrameListener listener;

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
