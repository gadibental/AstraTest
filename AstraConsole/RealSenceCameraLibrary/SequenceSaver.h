#pragma once

#include <string>
#include <memory>
#include <vector>
#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include "RealSense/SampleReader.h"


namespace ImgProcLib
{
	class BackgroundSubtractor;
}

class SequenceSaver
{
public:
	SequenceSaver(int expectedNumFrames, std::string fileName);
	~SequenceSaver();


	void AddImage(Intel::RealSense::Image* depthImage,
		Intel::RealSense::Image*colourImage,
		Intel::RealSense::Projection* projection,
		bool removeBg,
		std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor);
	void Save(Intel::RealSense::Projection* projection);


private:
	void SaveXml(Intel::RealSense::Projection* projection);
	void SaveImages();
	std::string GetFrameName(double time);
	void CreateFolderIfNeeded(std::string folder);

	void GetCameraCalibration(Intel::RealSense::Projection* projection, std::vector<float>& cameraIntrinsics, std::vector<float>& cameraDistoration);

private:
	std::string m_fileName;
	std::vector<std::pair<double, cv::Mat>> m_theSequance;
	long long m_startTime;
	double m_TimerFrequancy;
};

