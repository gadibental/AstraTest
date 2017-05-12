#pragma once

#include <string>
#include <memory>
#include <vector>
#include "RealSense/SampleReader.h"
#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality


namespace ImgProcLib
{
	class BackgroundSubtractor;
}

class ImageSaver3D
{
public:
	ImageSaver3D(std::string fileName);
	virtual ~ImageSaver3D();

	static std::shared_ptr<ImageSaver3D> CreateSaver(std::string fileName);

	void SaveImage(Intel::RealSense::Image* depthImage, 
		Intel::RealSense::Image*colourImage,
		Intel::RealSense::Projection* projection,
		bool removeBg,
		std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor);

	void CreateProjection(Intel::RealSense::Image* depthImage,
		Intel::RealSense::Image*colourImage,
		Intel::RealSense::Projection* projection);

protected:
	void RemoveBg(std::shared_ptr<ImgProcLib::BackgroundSubtractor> bgSubtractor);

	virtual void SaveTheData() = 0;


protected:
	std::vector<Intel::RealSense::Point3DF32> m_vertices;
	std::vector<Intel::RealSense::PointF32> m_uvMap;
	std::vector<Intel::RealSense::PointF32> m_depthUv;
	cv::Mat m_colourMat;
	std::string m_fileName;
	std::shared_ptr<ImgProcLib::BackgroundSubtractor> m_BgSubtractor;
};

class ImageSaverCsv : public ImageSaver3D
{
public:
	ImageSaverCsv(std::string fileName);
	~ImageSaverCsv();

protected:
	virtual void SaveTheData() override;

private:

};

class ImageSaverRGBD : public ImageSaver3D
{
public:
	ImageSaverRGBD(std::string fileName);
	~ImageSaverRGBD();

	cv::Mat CreateImageToSave();

protected:
	virtual void SaveTheData() override;

private:

};

