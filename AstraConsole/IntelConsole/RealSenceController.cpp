#include "stdafx.h"
#include "RealSenceController.h"

#include "RealSense/SenseManager.h"
#include "RealSense/SampleReader.h"

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 

#include <iostream>
#include <fstream>

using namespace Intel::RealSense;

RealSenceController::RealSenceController()
	: m_pipeline(NULL)
	, m_lastColourImage(NULL)
	, m_lastDepthImage(NULL)
	, m_projection(NULL)
//	, m_lowConfidanceDepthValue(0)
{
	m_lastColourMat = cv::Mat::zeros(0, 0, CV_8UC3);
}


RealSenceController::~RealSenceController()
{
}

void RealSenceController::Run(int numFrames)
{
	if (!Initialise())
	{
		return;
	}

	for (int frameCount = 0; frameCount < numFrames; frameCount++)
	{
		if ((NULL != m_lastDepthImage && NULL != m_lastColourImage) && ((frameCount % 20) == 0))
		{
			SaveVertexMap();
		}
		GetNextFrame();
		std::cout << "Frame: " << frameCount << "\n";
	}
	Release();
}

void RealSenceController::GotDepthImage(Intel::RealSense::Image * depthImage)
{
	m_lastDepthImage = depthImage;
	ImageInfo dI = depthImage->QueryInfo();
	cv::Mat depthMat = cv::Mat::zeros(dI.height, dI.width, CV_16UC1);
	ImageData ddata;
	if (depthImage->AcquireAccess(Image::ACCESS_READ, &ddata) >= PXC_STATUS_NO_ERROR)
	{
		memcpy(depthMat.data, ddata.planes[0], dI.height*dI.width * 2);
	}
	depthImage->ReleaseAccess(&ddata);

	const float maxDepth = 1000;
	cv::Mat showImage;
	cv::convertScaleAbs(depthMat, showImage, 255.0 / maxDepth, 0);

	cv::imshow("depth", showImage);
}

void RealSenceController::GotColourImage(Intel::RealSense::Image * colourImage)
{
	m_lastColourImage = colourImage;
	ImageInfo cI = colourImage->QueryInfo();
	m_lastColourMat = cv::Mat::zeros(cI.height, cI.width, CV_8UC3);
	ImageData ddata;
	if (colourImage->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_BGR, &ddata) >= PXC_STATUS_NO_ERROR)
	{
		memcpy(m_lastColourMat.data, ddata.planes[0], cI.height*cI.width * 3);
	}
	colourImage->ReleaseAccess(&ddata);

	cv::imshow("colour", m_lastColourMat);
}

void RealSenceController::SaveVertexMap()
{
	ImageInfo dI = m_lastDepthImage->QueryInfo();

	int numPoints = dI.width * dI.height;
	std::vector<Point3DF32> vertices(numPoints);
	Status status = m_projection->QueryVertices(m_lastDepthImage, &vertices[0]);
	std::vector<PointF32> uvMap(numPoints);
	status = m_projection->QueryUVMap(m_lastDepthImage, &uvMap[0]);


	std::ofstream file("RealVertex.csv", std::ios::out);

	file << "X,Y,Z,R,G,B\n";

	for (int p = 0; p < numPoints; ++p)
	{
		Point3DF32 vertex = vertices[p];
		if (vertex.z != 0)
		{
			cv::Point pixel((int)(uvMap[p].x *m_lastColourMat.cols), (int)(uvMap[p].y * m_lastColourMat.rows));
			if (pixel.x >= 0 && pixel.x < m_lastColourMat.cols && pixel.y >= 0 && pixel.y < m_lastColourMat.rows)
			{
				file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
				cv::Vec3b colourValue = m_lastColourMat.at<cv::Vec3b>(pixel);
				for (int c = 0; c < colourValue.rows; c++)
				{
					file << (int)colourValue[colourValue.rows - c - 1] << ',';
				}
				file << "\n";
			}
		}
	}
}

void RealSenceController::GetNextFrame()
{
	/* Waits until new frame is available and locks it for application processing */
	Status status = m_pipeline->AcquireFrame(false);

	if (status == Status::STATUS_NO_ERROR)
	{
		/* Render streams, unless -noRender is selected */
		const Capture::Sample *sample = m_pipeline->QuerySample();
		if (sample)
		{
			if (NULL != sample->depth)
			{
				GotDepthImage(sample->depth);

			}
			if (NULL != sample->color)
			{
				GotColourImage(sample->color);
			}

			cv::waitKey(1); // refresh

		}
	}


	/* Releases lock so pipeline can process next frame */
	m_pipeline->ReleaseFrame();
}

void RealSenceController::Release()
{
	m_pipeline->Release();
}


bool RealSenceController::Initialise()
{
	/* Creates an instance of the SenseManager */
	m_pipeline = SenseManager::CreateInstance();
	if (!m_pipeline)
	{
		std::cout << L"Unable to create the SenseManager\n";
		return false;
	}


	m_pipeline->EnableStream(Capture::STREAM_TYPE_COLOR, 640, 480);
	m_pipeline->EnableStream(Capture::STREAM_TYPE_DEPTH);
	Status status = m_pipeline->Init();

	DataDesc desc = {};
	desc.deviceInfo.streams = Capture::STREAM_TYPE_COLOR | Capture::STREAM_TYPE_DEPTH;

	Capture::Device *device = m_pipeline->QueryCaptureManager()->QueryDevice();
	device->ResetProperties(Capture::STREAM_TYPE_ANY);

	//m_lowConfidanceDepthValue = m_pipeline->QueryCaptureManager()->QueryDevice()->QueryDepthLowConfidenceValue();
	m_projection = m_pipeline->QueryCaptureManager()->QueryDevice()->CreateProjection();

	return (STATUS_NO_ERROR == status);
}
