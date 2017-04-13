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
{
	// FOV numbers are from astra document
	m_depthCamera.reset( new Camera(cv::Vec2f(58.4f, 45.5f), 0, 1));
	m_colourCamera.reset( new Camera(cv::Vec2f(62.7f, 49.0f), 0, 2));
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
	cv::Mat depth = m_depthCamera->GetLastFrame();
	cv::Mat colour = m_colourCamera->GetLastFrame();
	if ((depth.cols == 0) || (colour.cols == 0))
	{
		return; // no data
	}


	std::ofstream file("vertices.csv", std::ios::out);

	file << "X,Y,Z,R,G,B\n";

	int depthIndex = 0;
	for (int y = 0; y < depth.rows; ++y)
	{
		for (int x = 0; x < depth.cols; ++x)
		{
			cv::Point position(x, y);
			ushort depthValue = depth.at<ushort>(position);
			if (depthValue != 0 && depthValue < 1000)
			{
				cv::Vec3f vertex = m_depthCamera->ImageToWorld(position, depthValue);
				cv::Point2f pixel = m_colourCamera->WorldToImage(vertex);

				if (pixel.x >= 0 && pixel.x < colour.cols && pixel.y >= 0 && pixel.y < colour.rows)
				{
					for (int c = 0; c < vertex.rows; c++)
					{
						file << vertex[c] << ',';
					}
					cv::Vec3b colourValue = colour.at<cv::Vec3b>(pixel);
					for (int c = 0; c < colourValue.rows; c++)
					{
						file << (int)colourValue[colourValue.rows - c - 1] << ',';
					}
				}


				//astra::Vector3f depthPixel((float)x, (float)y, depthValue);
				//astra::Vector3f vertex = mapper.convert_depth_to_world(depthPixel);
				//file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
				//// get the corresponding colour value
				//int cx = x * colour.cols / depth.cols;
				//int cy = y * colour.rows / depth.rows;
				//cv::Vec3b colourValue = colour.at<cv::Vec3b>(position);
				//for (int c = 0; c < colourValue.rows; c++)
				//{
				//	file << (int)colourValue[colourValue.rows - c - 1] << ',';
				//}

				file << "\n";
			}
			depthIndex++;
		}
	}

}



int SimpleFrameListener::ShowDepth(const astra::DepthFrame& depthFrame)
{
	m_depthCamera->ReadFrame(depthFrame);
	cv::Mat showImage;// = cv::Mat(m_depthCamera->GetImageSize(), CV_8UC1);
	const float maxDepth = 1000;
	cv::convertScaleAbs(m_depthCamera->GetLastFrame(), showImage, 255.0 / maxDepth, 0);

	cv::imshow("depth", showImage);
	//		cv::imwrite("depth.png", showImage);
	//SaveMat("depth.txt", depthImage);

	return depthFrame.frame_index();
}

void SimpleFrameListener::ShowColour(const astra::ColorFrame& colorFrame)
{
	m_colourCamera->ReadFrame(colorFrame);


	cv::imshow("colour", m_colourCamera->GetLastFrame());
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
