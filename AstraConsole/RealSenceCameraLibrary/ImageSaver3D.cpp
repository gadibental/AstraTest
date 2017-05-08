#include "stdafx.h"
#include "ImageSaver3D.h"
#include "IImageDisplayer.h"

// utility librarues
#include "..\ImageProcessingLib\BackgroundSubtractor.h"

// STD
#include <vector>
#include <iostream>
#include <fstream>

// intel library
#include "RealSense/SampleReader.h"

// openCV
#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality

using namespace Intel::RealSense;



ImageSaver3D::ImageSaver3D(std::string fileName)
	: m_fileName(fileName)
{
}


ImageSaver3D::~ImageSaver3D()
{
}

std::shared_ptr<ImageSaver3D> ImageSaver3D::CreateSaver(std::string fileName)
{
	size_t dot = fileName.find_last_of('.');
	std::string extension = (std::string::npos == dot) ? "" :fileName.substr(dot, fileName.length());
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	std::shared_ptr<ImageSaver3D> saver;
	if (0 == extension.compare(".csv"))
	{
		saver.reset(new ImageSaverCsv(fileName));
	}
	else
	{
		saver.reset(new ImageSaverRGBD(fileName));
	}


	return saver;
}

void ImageSaver3D::SaveImage(Intel::RealSense::Image* depthImage, 
	Intel::RealSense::Image*colourImage, Intel::RealSense::Projection* projection,
	bool removeBg, std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor)
{
	CreateProjection(depthImage, colourImage, projection);
	if (removeBg)
	{
		RemoveBg(bgSubtractor);
	}
	SaveTheData();
}

void ImageSaver3D::CreateProjection(Intel::RealSense::Image* depthImage,
	Intel::RealSense::Image*colourImage, Intel::RealSense::Projection* projection)
{
	ImageInfo dI = depthImage->QueryInfo();

	int numPoints = dI.width * dI.height;
	m_vertices.resize(numPoints);
	Status status = projection->QueryVertices(depthImage, &m_vertices[0]);
	m_uvMap.resize(numPoints);
	status = projection->QueryUVMap(depthImage, &m_uvMap[0]);

	m_depthUv.resize(numPoints);
	projection->ProjectCameraToDepth(numPoints, &m_vertices[0], &m_depthUv[0]);


	ImageInfo cI = colourImage->QueryInfo();
	//ImageData ddata;
	//m_lastColourImage->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGBA, &ddata);
	m_colourMat = IImageDisplayer::GetImageForDisplay(colourImage);
}

void ImageSaver3D::RemoveBg(std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor)
{
	m_colourMat = bgSubtractor->SubtractBg(m_colourMat);
}


ImageSaverCsv::ImageSaverCsv(std::string fileName)
	: ImageSaver3D(fileName)
{
}

ImageSaverCsv::~ImageSaverCsv()
{
}

void ImageSaverCsv::SaveTheData()
{
	std::ofstream file(m_fileName, std::ios::out);

	file << "X,Y,Z,R,G,B,x,y\n";

	//	int* colourPixel = (int*)ddata.planes[0];
	for (int p = 0; p < (int)m_vertices.size(); ++p)
	{
		Point3DF32 vertex = m_vertices[p];
		if (vertex.z != 0)
		{
			if (m_uvMap[p].x >= 0 && m_uvMap[p].x < 1.0f && m_uvMap[p].y >= 0 && m_uvMap[p].y < 1.0f)
			{
				int x = (int)(m_uvMap[p].x * m_colourMat.cols);
				int y = (int)(m_uvMap[p].y * m_colourMat.rows);
				cv::Point pixelPosition(x, y);
				cv::Vec3b pixelValue = m_colourMat.at<cv::Vec3b>(pixelPosition);
				if (pixelValue[0] != 0)
				{
					file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
					for (int c = 0; c < 3; c++)
					{
						file << (int)pixelValue[2 - c] << ','; // note BGR to RGB
					}
					// get the source XY coordinate in the depth image
					int xd = (int)(m_depthUv[p].x);
					int yd = (int)(m_depthUv[p].y);
					file << xd << ',' << yd << ',';
					file << "\n";
				}
			}
		}
	}
}

ImageSaverRGBD::ImageSaverRGBD(std::string fileName)
	: ImageSaver3D(fileName)
{
}

ImageSaverRGBD::~ImageSaverRGBD()
{
}

void ImageSaverRGBD::SaveTheData()
{
	cv::Mat theImage = cv::Mat::zeros(m_colourMat.size(), CV_32FC(4));

	for (int p = 0; p < (int)m_vertices.size(); ++p)
	{
		Point3DF32 vertex = m_vertices[p];
		if (vertex.z != 0)
		{
			if (m_uvMap[p].x >= 0 && m_uvMap[p].x < 1.0f && m_uvMap[p].y >= 0 && m_uvMap[p].y < 1.0f)
			{
				int x = (int)(m_uvMap[p].x * m_colourMat.cols);
				int y = (int)(m_uvMap[p].y * m_colourMat.rows);
				cv::Point pixelPosition(x, y);
				cv::Vec3b colourValue = m_colourMat.at<cv::Vec3b>(pixelPosition);
				if (colourValue[0] != 0)
				{
					cv::Vec<float, 4> values;
					values[3] = vertex.z * 100; // get 2 decimal points
					for (int c = 0; c < 3; ++c)
					{
						values[c] = colourValue[c] * 256.0f; // 8 bits to 16
					}
					int xd = (int)(m_depthUv[p].x);
					int yd = (int)(m_depthUv[p].y);
					cv::Point pixel(xd, yd);
					theImage.at<cv::Vec<float, 4>>(pixel) = values;
				}
			}
		}
	}

	cv::Mat converted;
	theImage.convertTo(converted, CV_16U); // convert to 16 bit int image
	cv::imwrite(m_fileName, converted);
}
