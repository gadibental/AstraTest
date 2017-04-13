#pragma once
#include <memory>
#include <astra/astra.hpp>

#include "..\..\..\fruittracker\DensTracker\GeometricImageLib\geometricImage.h"


class Camera
{
public:
	Camera(cv::Vec2f fieldOfViewDegrees, float paralax, int id);
	~Camera();

	using buffer_ptrD = std::unique_ptr<int16_t[]>;
	using buffer_ptrRGB = std::unique_ptr<astra::RgbPixel[]>;

	void ReadFrame(const astra::DepthFrame& depthFrame);
	void ReadFrame(const astra::ColorFrame& colourFrame);

	cv::Mat GetLastFrame();

	cv::Vec3f ImageToCamera(const cv::Point2f& pixel, float cameraZ);
	cv::Point2f CameraToImage(cv::Vec3f vertex);
	cv::Vec3f ImageToWorld(const cv::Point2f& pixel, float cameraZ);
	cv::Point2f WorldToImage(cv::Vec3f vertex);


private:
	void ResetCameraIntrinsics(); 

private:
	buffer_ptrD buffer_D;
	buffer_ptrRGB buffer_RGB;
	std::shared_ptr<GeometricImage_NS::CalibratedCamera> m_theCamera;
	cv::Size m_imageSize;
	cv::Mat m_lastImage;

	cv::Vec2f m_fieldOfViewDegrees;
};


