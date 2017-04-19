#pragma once

#include <string>
#include <memory>
#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality

namespace Intel
{
	namespace RealSense
	{
		class Image;
	}
}

class IImageDisplayer
{
public:
	IImageDisplayer();
	virtual ~IImageDisplayer();

	static void ShowImage(Intel::RealSense::Image* image, std::string imageName);

private:
	static std::shared_ptr<IImageDisplayer> CreateDisplayer(Intel::RealSense::Image* image);
	void DoShowImage(Intel::RealSense::Image * image, std::string imageName);

protected:
	virtual int GetMatType() = 0;
	virtual int GetAcquireType() = 0;
	virtual int GetBytesPerPixel() = 0;
	virtual cv::Mat ScaleImageForDisplay(cv::Mat image) = 0;
};


class ImageDisplayerDepth : public IImageDisplayer
{
public:
	ImageDisplayerDepth() {};
	virtual ~ImageDisplayerDepth() {};

protected:
	virtual int GetMatType() override;
	virtual int GetAcquireType() override;
	virtual int GetBytesPerPixel() override;
	virtual cv::Mat ScaleImageForDisplay(cv::Mat image) override;
};

class ImageDisplayerColour : public IImageDisplayer
{
public:
	ImageDisplayerColour() {};
	virtual ~ImageDisplayerColour() {};

protected:
	virtual int GetMatType() override;
	virtual int GetAcquireType() override;
	virtual int GetBytesPerPixel() override;
	virtual cv::Mat ScaleImageForDisplay(cv::Mat image) override;
};

class ImageDisplayerIR : public IImageDisplayer
{
public:
	ImageDisplayerIR() {};
	virtual ~ImageDisplayerIR() {};

protected:
	virtual int GetMatType() override;
	virtual int GetAcquireType() override;
	virtual int GetBytesPerPixel() override;
	virtual cv::Mat ScaleImageForDisplay(cv::Mat image) override;
};

