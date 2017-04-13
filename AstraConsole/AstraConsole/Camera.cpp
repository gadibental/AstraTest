#include "stdafx.h"
#include "Camera.h"
#include <astra/astra.hpp>
#include "..\..\..\fruittracker\DensTracker\GeometricImageLib\CalibratedCamera.h"


using namespace GeometricImage_NS;

Camera::Camera(cv::Vec2f fieldOfViewDegrees, float paralax, int id)
	: m_fieldOfViewDegrees(fieldOfViewDegrees)
{
	Pose offset(cv::Point3f(paralax, 0, 0), cv::Point3f(0, 0, 0));
	m_theCamera.reset(new CalibratedCamera(offset, CameraIntrinsicParameters(), id));
}


Camera::~Camera()
{
}

void Camera::ReadFrame(const astra::DepthFrame & depthFrame)
{
	int width = depthFrame.width();
	int height = depthFrame.height();
	if (width != m_imageSize.width || height != m_imageSize.height)
	{
		buffer_D = buffer_ptrD(new int16_t[depthFrame.length()]);
		m_imageSize.width = width;
		m_imageSize.height = height;
		m_lastImage = cv::Mat(height, width, CV_16UC1);
		ResetCameraIntrinsics(); // because image size haschanged
	}
	depthFrame.copy_to(buffer_D.get());

	size_t index = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			short pixelValue = buffer_D[index];

			m_lastImage.at<ushort>(cv::Point(x, y)) = pixelValue;
			index++;
		}
	}
}

void Camera::ReadFrame(const astra::ColorFrame & colourFrame)
{
	int width = colourFrame.width();
	int height = colourFrame.height();
	if (width != m_imageSize.width || height != m_imageSize.height)
	{
		buffer_RGB = buffer_ptrRGB(new astra::RgbPixel[colourFrame.length()]);
		m_imageSize.width = width;
		m_imageSize.height = height;
		m_lastImage = cv::Mat(height, width, CV_8UC3);
		ResetCameraIntrinsics(); // because image size haschanged
	}
	colourFrame.copy_to(buffer_RGB.get());

	size_t index = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			astra::RgbPixel pixelValue = buffer_RGB[index];
			cv::Vec3b bgr(pixelValue.b, pixelValue.g, pixelValue.r);
			m_lastImage.at<cv::Vec3b>(cv::Point(x, y)) = bgr;
			index++;
		}
	}
}

cv::Mat Camera::GetLastFrame()
{
	return m_lastImage;
}

cv::Vec3f Camera::ImageToCamera(const cv::Point2f & pixel, float cameraZ)
{
	return m_theCamera->ImageToCamera(pixel, cameraZ);
}

cv::Point2f Camera::CameraToImage(cv::Vec3f vertex)
{
	return m_theCamera->CameraToImage(vertex);
}

cv::Vec3f Camera::ImageToWorld(const cv::Point2f & pixel, float cameraZ)
{
	return m_theCamera->ImageToWorld(pixel, cameraZ);
}

cv::Point2f Camera::WorldToImage(cv::Vec3f vertex)
{
	return m_theCamera->WorldToImage(vertex);
}

void Camera::ResetCameraIntrinsics()
{
	CameraIntrinsicParameters intrinsics = m_theCamera->GetInternalParams();
	Pose pose = m_theCamera->GetPose();
	int id = m_theCamera->GetId();

	cv::Vec2f imgSize(m_imageSize.width, m_imageSize.height);
	intrinsics.SetFromImageSizeAndFieldOfView(imgSize, m_fieldOfViewDegrees);


	m_theCamera.reset(new CalibratedCamera(pose, intrinsics, id));
}
