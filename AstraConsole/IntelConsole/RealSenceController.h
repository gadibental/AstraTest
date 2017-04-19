#pragma once

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality

namespace Intel
{
	namespace RealSense
	{
		class SenseManager;
		class Projection;
		class Image;
	}
}


class RealSenceController
{
public:
	RealSenceController();
	~RealSenceController();

	void Run(int numFrames, int saveVertexFrequency);

private:
	bool Initialise();
	void GotImage(Intel::RealSense::Image* image, std::string imageName);
	void SaveVertexMap();
	void GetNextFrame();
	void Release();


private:
	Intel::RealSense::SenseManager* m_pipeline;
	Intel::RealSense::Projection* m_projection;
	Intel::RealSense::Image* m_lastDepthImage;
	Intel::RealSense::Image* m_lastColourImage;
	//short m_lowConfidanceDepthValue;
};

