#include "stdafx.h"
#include "IImageDisplayer.h"

#include "RealSense/SenseManager.h"
#include "RealSense/SampleReader.h"

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 

using namespace Intel::RealSense;


IImageDisplayer::IImageDisplayer()
{
}


IImageDisplayer::~IImageDisplayer()
{
}

std::shared_ptr<IImageDisplayer> IImageDisplayer::CreateDisplayer(Intel::RealSense::Image * image)
{
	std::shared_ptr<IImageDisplayer> displayer;
	switch (image->QueryInfo().format)
	{
	case PixelFormat::PIXEL_FORMAT_YUY2:
		displayer.reset(new ImageDisplayerColour);
		break;
	case PixelFormat::PIXEL_FORMAT_DEPTH:
		displayer.reset(new ImageDisplayerDepth);
		break;
	case PixelFormat::PIXEL_FORMAT_Y16:
		displayer.reset(new ImageDisplayerIR);
		break;

	default:
		break;
	}

	return displayer;
}

void IImageDisplayer::ShowImage(Intel::RealSense::Image * image, std::string imageName)
{
	std::shared_ptr<IImageDisplayer> displayer = IImageDisplayer::CreateDisplayer(image);
	displayer->DoShowImage(image, imageName);
}

void IImageDisplayer::DoShowImage(Intel::RealSense::Image * image, std::string imageName)
{
	ImageInfo info = image->QueryInfo();
	cv::Mat imageMat = cv::Mat::zeros(info.height, info.width, GetMatType());
	ImageData ddata;
	if (image->AcquireAccess(Image::ACCESS_READ, (PixelFormat)GetAcquireType(), &ddata) >= PXC_STATUS_NO_ERROR)
	{
		memcpy(imageMat.data, ddata.planes[0], info.height*info.width * GetBytesPerPixel());
	}
	image->ReleaseAccess(&ddata);

	cv::Mat showImage = ScaleImageForDisplay(imageMat);

	cv::imshow(imageName, showImage);
}

int ImageDisplayerDepth::GetMatType()
{
	return CV_16UC1;
}

int ImageDisplayerDepth::GetAcquireType()
{
	return (int)PixelFormat::PIXEL_FORMAT_DEPTH;
}

int ImageDisplayerDepth::GetBytesPerPixel()
{
	return 2;
}

cv::Mat ImageDisplayerDepth::ScaleImageForDisplay(cv::Mat image)
{
	cv::Mat scaled;
	const float maxDepth = 1000;
	cv::convertScaleAbs(image, scaled, 255.0 / maxDepth, 0);
	return scaled;
}

int ImageDisplayerColour::GetMatType()
{
	return CV_8UC3;
}

int ImageDisplayerColour::GetAcquireType()
{
	return (int)PixelFormat::PIXEL_FORMAT_BGR;
}

int ImageDisplayerColour::GetBytesPerPixel()
{
	return 3;
}

cv::Mat ImageDisplayerColour::ScaleImageForDisplay(cv::Mat image)
{
	return image;
}

int ImageDisplayerIR::GetMatType()
{
	return CV_8UC1;
}

int ImageDisplayerIR::GetAcquireType()
{
	return (int)PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE;
}

int ImageDisplayerIR::GetBytesPerPixel()
{
	return 1;
}

cv::Mat ImageDisplayerIR::ScaleImageForDisplay(cv::Mat image)
{
	return image;
}
