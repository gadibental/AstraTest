#include "stdafx.h"
#include "BackgroundSubtractor.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

using namespace ImgProcLib;

ImgProcLib::BackgroundSubtractor::BackgroundSubtractor()
	: m_HowToIdentifyBg(bgType::none)
{
	m_subtractor = cv::createBackgroundSubtractorKNN(50);
}


ImgProcLib::BackgroundSubtractor::~BackgroundSubtractor()
{
}

cv::Mat ImgProcLib::BackgroundSubtractor::SubtractBg(cv::Mat image)
{
	cv::Mat mask = CreateForgroundMask(image);

//	cv::imshow("mask", mask);


	std::vector<cv::Mat> channels;
	cv::split(image, channels);
	for ( auto& c : channels)
	{
		c = c & mask; // mask out BG
	}
	cv::Mat result;
	cv::merge(channels, result);

	return result;
}

void ImgProcLib::BackgroundSubtractor::AddBgImage(cv::Mat image)
{
	cv::Mat unUsedMask;
	m_subtractor->apply(image, unUsedMask, -1); // -1 learn BG using default rate
}

void ImgProcLib::BackgroundSubtractor::Clear()
{
	m_subtractor = cv::createBackgroundSubtractorKNN(50);
	m_HowToIdentifyBg = bgType::learn;
}

void ImgProcLib::BackgroundSubtractor::UseBlueBg()
{
	m_HowToIdentifyBg = bgType::blue;
}

cv::Mat ImgProcLib::BackgroundSubtractor::CreateForgroundMask(cv::Mat image)
{
	cv::Mat mask;
	switch (m_HowToIdentifyBg)
	{
	case bgType::none:
		mask = cv::Mat::ones(image.size(), CV_8UC1) * 255;
		break;
	case bgType::learn:
		m_subtractor->apply(image, mask, 0); // 0 for not updating BG
		cv::dilate(mask, mask, cv::Mat::ones(5, 5, CV_8UC1));
		break;
	case bgType::blue:
	{
		std::vector<cv::Mat> channels;
		cv::split(image, channels);
		cv::Mat maxRG = cv::max(channels[1], channels[2]); 
		mask = channels[0] < (maxRG - 5);
		cv::erode(mask, mask, cv::Mat::ones(7, 7, CV_8UC1));

	}
		break;
	default:
		break;
	}
	return mask;
}
