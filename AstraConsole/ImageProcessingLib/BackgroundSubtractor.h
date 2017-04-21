#pragma once

#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp> // for BackgroundSubtractor


namespace ImgProcLib
{
	class BackgroundSubtractor
	{
		enum class bgType { none, learn, blue };
	public:
		BackgroundSubtractor();
		~BackgroundSubtractor();


		cv::Mat SubtractBg(cv::Mat image);
		void AddBgImage(cv::Mat image);
		void Clear();
		void UseBlueBg();

	private:
		cv::Mat CreateForgroundMask(cv::Mat image);


	private:
		cv::Ptr<cv::BackgroundSubtractor> m_subtractor;
		bgType m_HowToIdentifyBg;

	};

}

