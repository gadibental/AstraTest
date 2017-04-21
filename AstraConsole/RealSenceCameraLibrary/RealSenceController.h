#pragma once

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <memory>

namespace Intel
{
	namespace RealSense
	{
		class SenseManager;
		class Projection;
		class Image;
	}
}

namespace ImgProcLib
{
	class BackgroundSubtractor;
}


class RealSenceController
{
public:
	RealSenceController();
	~RealSenceController();

	void Run(int numFrames, int saveVertexFrequency);
	void RunTillStopped();

	void SetShowIR(bool s) { m_ShowIr = s;  }
	void SetShowColour(bool s) { m_ShowColour = s; }
	void SetShowDepth(bool s) { m_ShowDepth = s; }
	void SetRemoveBg(bool s) { m_RemovBG = s; }
	void Stop() { m_Stop = true; }
	void SaveNextFrame(std::string fileName);

	void LearnBG();
	void UseBlueAsBG();

private:
	bool Initialise();
	void SaveVertexMap();
	void GetNextFrame();
	void Release();


private:
	Intel::RealSense::SenseManager* m_pipeline;
	Intel::RealSense::Projection* m_projection;
	Intel::RealSense::Image* m_lastDepthImage;
	Intel::RealSense::Image* m_lastColourImage;
	//short m_lowConfidanceDepthValue;

	bool m_ShowDepth;
	bool m_ShowColour;
	bool m_ShowIr;
	bool m_Stop;
	bool m_SaveNextFrame;
	bool m_RemovBG;
	int m_BgImageCounter;

	std::string m_SaveFileName;
	std::shared_ptr<ImgProcLib::BackgroundSubtractor> m_BgSubtractor;

};

