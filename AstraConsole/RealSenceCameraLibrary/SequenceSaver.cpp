#include "stdafx.h"
#include "SequenceSaver.h"
#include "ImageSaver3D.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "RealSense/SenseManager.h"


SequenceSaver::SequenceSaver(int expectedNumFrames, std::string fileName)
	: m_fileName(fileName)
{
	m_theSequance.reserve(expectedNumFrames);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	m_TimerFrequancy = (double)freq.QuadPart;
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	m_startTime = startTime.QuadPart;
}


SequenceSaver ::~SequenceSaver()
{
}

void SequenceSaver::AddImage(Intel::RealSense::Image * depthImage, Intel::RealSense::Image * colourImage, Intel::RealSense::Projection * projection, bool removeBg, std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor)
{
	ImageSaverRGBD matCreator(" ");
	matCreator.CreateProjection(depthImage, colourImage, projection);
	cv::Mat image = matCreator.CreateImageToSave();

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	double HiResTime = (double)(endTime.QuadPart-m_startTime);
	HiResTime /= m_TimerFrequancy;
	double frameTime = HiResTime * 1000.0; // convert from sec to mSec

	m_theSequance.push_back(std::make_pair(frameTime, image));
}

void SequenceSaver::Save(Intel::RealSense::Projection* projection)
{
	SaveXml(projection);
	SaveImages();
}

void SequenceSaver::SaveXml(Intel::RealSense::Projection* projection)
{
	std::ofstream file(m_fileName, std::ios::out);

	cv::Mat firstFrame = (m_theSequance.size() > 0) ? m_theSequance[0].second : cv::Mat::zeros(0, 0, CV_8UC1);

	std::vector<float> cameraIntrinsics;
	std::vector<float> cameraDistoration;
	GetCameraCalibration(projection, cameraIntrinsics, cameraDistoration);

	file << "< ? xml version = " << '"' << "1.0" << '"' << " encoding = " << '"' << "utf-8" << '"' << " ? >\n";
	file << "<data>\n";
	file << "<units>\n";
	file << "<length >millimeters</length>\n";
	file << "<angle >degrees</angle>\n";
	file << "</units>\n";
	file << "<volume-model>\n";
	file << "<edge-length>100</edge-length>\n";
	file << "<maximum-voxel-edge-length>0.4</maximum-voxel-edge-length>\n";
	file << "<noise-model>\n";
	file << "<depth type = " << '"' << "clipped-linear" << '"' << ">\n";
	file << "<standard-error>0.2</standard-error>\n";
	file << "</depth>\n";
	file << "</noise-model>\n";
	file << "</volume-model>\n";
	file << "<views>\n";
	file << "<view id = '1'>\n";
	file << "<name>Top</name>\n";
	file << "<type>depth + texture</type>\n";
	file << "<include-in-tracking>true</include-in-tracking>\n";
	file << "<pose convention = " << '"' << "camera-to-world" << '"' << "> \n";
	file << "<euler-rotation>\n";
	file << "<x>180</x>\n";
	file << "</euler-rotation>\n";
	file << "<matrix order = " << '"' << "column major" << '"' << " rows = " << '"' << "4" << '"' << ">1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 350, 1</matrix>\n";
	file << "</pose>\n";
	file << "<perspective-viewer>\n";
	file << "<matrix order = " << '"' << "column major" << '"' << " rows = " << '"' << "3" << '"' << ">" << cameraIntrinsics[0];
	file << ", 0, 0, 0," << cameraIntrinsics[1] << ", 0, " << cameraIntrinsics[2] << "," << cameraIntrinsics[3] <<", 1</matrix>\n";
	file << "</perspective-viewer>\n";
	file << "<image-correction>\n";
	file << "<distortion type = " << '"' << "open-cv" << '"' << ">";
	for (size_t i = 0; i < cameraDistoration.size(); i++)
	{
		file << cameraDistoration[i];
		if (i < cameraDistoration.size() - 1)
		{
			file << ",";
		}
	}
	file << "</distortion>\n";
	file << "</image-correction>\n";
	file << "<channels >\n";
	file << "<first >0</first>\n";
	file << "<count >4</count>\n";
	file << "</channels>\n";
	file << "<image>\n";
	file << "<width>" << firstFrame.cols << "</width>\n";
	file << "<height>" << firstFrame.rows << "</height>\n";
	file << "</image>\n";
	file << "<sensor>\n";
	file << "<width unit = " << '"' << "mm" << '"' << ">11.2512</width>\n";
	file << "</sensor>\n";
	file << "<scale >\n";
	file << "<z0 >0</z0>\n";
	file << "<z1 >655.36</z1>\n";
	file << "</scale>\n";
	file << "</view>\n";
	file << "</views>\n";
	file << "<captures>\n";
	file << "<folder>\n";
	file << "<name>//Frames//</name>\n";
	file << "</folder>\n";

	for (size_t i = 0; i < m_theSequance.size(); i++)
	{
		double frameTime = m_theSequance[i].first;
		std::string frameName = GetFrameName(frameTime);

		file << "<capture>\n"; 
		file << "<time>" << i + 1 << "</time>\n";
		file << "<image view = " << '"' << "1" << '"' << ">" << frameName << "</image>\n";
		file << "</capture>\n";
	}

	file << "</captures>\n";
	file << "</data>\n";
}

void SequenceSaver::SaveImages()
{
	size_t slash = m_fileName.find_last_of('\\');
	std::string folder = (std::string::npos == slash) ? "" : m_fileName.substr(0, slash);
	folder += "\\frames\\";

	CreateFolderIfNeeded(folder);

	for (size_t i = 0; i < m_theSequance.size(); i++)
	{
		cv::Mat frame = m_theSequance[i].second;
		double frameTime = m_theSequance[i].first;
		std::string frameName = folder + GetFrameName(frameTime);
		cv::imwrite(frameName, frame);
	}
}

std::string SequenceSaver::GetFrameName(double time)
{
	std::stringstream stream;
	stream << "image" << (int)time << ".png";

	return stream.str();
}

void SequenceSaver::CreateFolderIfNeeded(std::string folder)
{
	DWORD attributes = ::GetFileAttributes(folder.c_str());
	if (attributes == 0xffffffff)
	{
		::CreateDirectory(folder.c_str(), NULL);
	}
}

void SequenceSaver::GetCameraCalibration(Intel::RealSense::Projection * projection, std::vector<float>& cameraIntrinsics, std::vector<float>& cameraDistoration)
{
	using namespace Intel::RealSense;
	Calibration* calibration = projection->QueryCalibration();
	StreamCalibration depthCalib;
	StreamTransform depthPose;
	calibration->QueryStreamProjectionParameters(StreamType::STREAM_TYPE_DEPTH, &depthCalib, &depthPose);

	cameraIntrinsics.clear();
	cameraIntrinsics.push_back(depthCalib.focalLength.x);
	cameraIntrinsics.push_back(depthCalib.focalLength.y);
	cameraIntrinsics.push_back(depthCalib.principalPoint.x);
	cameraIntrinsics.push_back(depthCalib.principalPoint.y);

	cameraDistoration.clear();
	for (int x = 0; x < 3; ++x)
	{
		cameraDistoration.push_back(depthCalib.radialDistortion[x]);
	}
	for (int x = 0; x < 2; ++x)
	{
		cameraDistoration.push_back(depthCalib.tangentialDistortion[x]);
	}
}
