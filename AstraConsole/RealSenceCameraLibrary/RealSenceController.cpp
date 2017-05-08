#include "stdafx.h"
#include "RealSenceController.h"
#include "IImageDisplayer.h"
#include "CameraCalibrationExporter.h"
#include "ImageSaver3D.h"
#include "..\ImageProcessingLib\BackgroundSubtractor.h"

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
	, m_ShowColour(false)
	, m_ShowDepth(false)
	, m_ShowIr(false)
	, m_Stop(false)
	, m_SaveNextFrame(false)
	, m_SaveFileName("RealVertex.csv")
	, m_BgImageCounter(-1)
//	, m_lowConfidanceDepthValue(0)
{
	m_BgSubtractor.reset(new ImgProcLib::BackgroundSubtractor);
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

void RealSenceController::RunTillStopped()
{
	if (!Initialise())
	{
		return;
	}

	while (!m_Stop)
	{
		if (m_SaveNextFrame)
		{
			SaveVertexMap();
			m_SaveNextFrame = false;
		}
		GetNextFrame();
	}
	Release();
}





void RealSenceController::SaveVertexMap()
{
	if (NULL == m_lastDepthImage || NULL == m_lastColourImage)
	{
		return;
	}

	std::shared_ptr<ImageSaver3D> saver = ImageSaver3D::CreateSaver(m_SaveFileName);
	saver->SaveImage(m_lastDepthImage, m_lastColourImage, m_projection, m_RemovBG, m_BgSubtractor);

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
			if (m_ShowIr && (NULL != sample->ir))
			{
				cv::Mat image = IImageDisplayer::GetImageForDisplay(sample->ir);
				cv::imshow("IR", image);
			}
			if (NULL != sample->depth)
			{
				m_lastDepthImage = sample->depth;
				if (m_ShowDepth)
				{
					cv::Mat image = IImageDisplayer::GetImageForDisplay(sample->depth);
					cv::imshow("Depth", image);
				}

			}
			if (NULL != sample->color)
			{
				m_lastColourImage = sample->color;
				if (m_ShowColour)
				{
					cv::Mat colourImage = IImageDisplayer::GetImageForDisplay(sample->color);
					if (m_RemovBG)
					{
						colourImage = m_BgSubtractor->SubtractBg(colourImage);
					}
					cv::imshow("Colour", colourImage);
					if (m_BgImageCounter > 0)
					{
						m_BgSubtractor->AddBgImage(colourImage);
						m_BgImageCounter--;
					}
				}
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


void RealSenceController::SaveNextFrame(std::string fileName)
{ 
	m_SaveFileName = fileName;
	m_SaveNextFrame = true; 
}

void RealSenceController::LearnBG()
{
	m_BgSubtractor->Clear();
	m_BgImageCounter = 50;
}

void RealSenceController::UseBlueAsBG()
{
	m_BgSubtractor->UseBlueBg();
}

void RealSenceController::SaveCalibration(std::string fileName)
{
	CameraCalibrationExporter exporter;
	exporter.Export(m_projection, fileName);
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



	m_pipeline->EnableStream(Capture::STREAM_TYPE_COLOR, 640, 480, 0, StreamOption::STREAM_OPTION_STRONG_STREAM_SYNC);
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

