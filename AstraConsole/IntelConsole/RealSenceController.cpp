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
}


RealSenceController::~RealSenceController()
{
}

void RealSenceController::Run(int numFrames, int saveVertexFrequency)
{
	if (!Initialise())
	{
		return;
	}

	for (int frameCount = 0; frameCount < numFrames; frameCount++)
	{
		if ((frameCount % saveVertexFrequency) == 0)
		{
			SaveVertexMap();
		}
		GetNextFrame();
		std::cout << "Frame: " << frameCount << "\n";
	}
	Release();
}


void RealSenceController::GotImage(Intel::RealSense::Image * image, std::string imageName)
{
	ImageInfo info = image->QueryInfo();
	int matType = 0;
	int bytesPerPixel = 0;
	bool needsScaling = false;
	PixelFormat whatTypeToAskFor = PixelFormat::PIXEL_FORMAT_DEPTH;

	switch (info.format)
	{
	case PixelFormat::PIXEL_FORMAT_YUY2:
		matType = CV_8UC3;
		bytesPerPixel = 3;
		needsScaling = false;
		whatTypeToAskFor = PixelFormat::PIXEL_FORMAT_BGR;
		break;
	case PixelFormat::PIXEL_FORMAT_DEPTH:
		matType = CV_16UC1;
		bytesPerPixel = 2;
		needsScaling = true;
		whatTypeToAskFor = PixelFormat::PIXEL_FORMAT_DEPTH;
		break;
	case PixelFormat::PIXEL_FORMAT_Y16:
		matType = CV_8UC1;
		bytesPerPixel = 1;
		needsScaling = false;
		whatTypeToAskFor = PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE;
		break;

	default:
		std::wcout << "Unknown image format: " << std::wstring(Image::PixelFormatToString(info.format)) << "\n";
		break;
	}

	cv::Mat imageMat = cv::Mat::zeros(info.height, info.width, matType);
	ImageData ddata;
	if (image->AcquireAccess(Image::ACCESS_READ, whatTypeToAskFor, &ddata) >= PXC_STATUS_NO_ERROR)
	{
		memcpy(imageMat.data, ddata.planes[0], info.height*info.width * bytesPerPixel);
	}
	image->ReleaseAccess(&ddata);

	const float maxDepth = 1000;
	cv::Mat showImage = imageMat;
	if (needsScaling)
	{
		cv::convertScaleAbs(imageMat, showImage, 255.0 / maxDepth, 0);
	}

	cv::imshow(imageName, showImage);
}



void RealSenceController::SaveVertexMap()
{
	if (NULL == m_lastDepthImage || NULL == m_lastColourImage)
	{
		return;
	}

	ImageInfo dI = m_lastDepthImage->QueryInfo();

	int numPoints = dI.width * dI.height;
	std::vector<Point3DF32> vertices(numPoints);
	Status status = m_projection->QueryVertices(m_lastDepthImage, &vertices[0]);
	std::vector<PointF32> uvMap(numPoints);
	status = m_projection->QueryUVMap(m_lastDepthImage, &uvMap[0]);

	ImageInfo cI = m_lastColourImage->QueryInfo();
	ImageData ddata;
	m_lastColourImage->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGBA, &ddata);


	std::ofstream file("RealVertex.csv", std::ios::out);

	file << "X,Y,Z,R,G,B\n";

	int* colourPixel = (int*)ddata.planes[0];
	for (int p = 0; p < numPoints; ++p)
	{
		Point3DF32 vertex = vertices[p];
		if (vertex.z != 0)
		{
			if (uvMap[p].x >= 0 && uvMap[p].x < 1.0f && uvMap[p].y >= 0 && uvMap[p].y < 1.0f)
			{
				int x = (int)(uvMap[p].x * cI.width);
				int y = (int)(uvMap[p].y * cI.height);
				file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
				int pixelValue = *(colourPixel + y*cI.width + x);
				for (int c = 0; c < 3; c++)
				{
					int colour = (int)(pixelValue & 0xFF);
					file << colour << ',';
					pixelValue = pixelValue >> 8;
				}
				file << "\n";
			}
		}
	}
	m_lastColourImage->ReleaseAccess(&ddata);
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
			if (NULL != sample->ir)
			{
				GotImage(sample->ir, "IR");
			}
			if (NULL != sample->depth)
			{
				m_lastDepthImage = sample->depth;
				GotImage(sample->depth, "Depth");

			}
			if (NULL != sample->color)
			{
				m_lastColourImage = sample->color;
				GotImage(sample->color, "Colour");
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
	m_pipeline->EnableStream(Capture::STREAM_TYPE_IR);
	Status status = m_pipeline->Init();
	if (STATUS_NO_ERROR != status)
	{
		return false;
	}

	DataDesc desc = {};
	desc.deviceInfo.streams = Capture::STREAM_TYPE_COLOR | Capture::STREAM_TYPE_DEPTH;

	Capture::Device *device = m_pipeline->QueryCaptureManager()->QueryDevice();
	device->ResetProperties(Capture::STREAM_TYPE_ANY);

	//m_lowConfidanceDepthValue = m_pipeline->QueryCaptureManager()->QueryDevice()->QueryDepthLowConfidenceValue();
	m_projection = m_pipeline->QueryCaptureManager()->QueryDevice()->CreateProjection();

	return (STATUS_NO_ERROR == status);
}

