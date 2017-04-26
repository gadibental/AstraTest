#include "stdafx.h"
#include "CameraCalibrationExporter.h"

#include "RealSense/SenseManager.h"

#include <iostream>
#include <fstream>

using namespace Intel::RealSense;

void OutputPose(std::ofstream& file, const Intel::RealSense::StreamTransform& pose)
{
	file << "Rotation: \n";
	for (int y = 0; y < 3; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			float r = pose.rotation[x][y];
			file << r << ", ";
		}
		file << "\n";
	}
	file << "Translation: ";
	for (int x = 0; x < 3; ++x)
	{
		file << pose.translation[x] << ", ";
	}
	file << "\n";

}

void OutputCalibration(std::ofstream& file, const Intel::RealSense::StreamCalibration& calib)
{
	file << "focalLength: " << calib.focalLength.x << ", " << calib.focalLength.y << "\n";
	file << "principalPoint: " << calib.principalPoint.x << ", " << calib.principalPoint.y << "\n";
	file << "radialDistortion: ";
	for (int x = 0; x < 3; ++x)
	{
		file << calib.radialDistortion[x] << ", ";
	}
	file << "\n";
	file << "tangentialDistortion: ";
	for (int x = 0; x < 2; ++x)
	{
		file << calib.tangentialDistortion[x] << ", ";
	}
	file << "\n";

	file << "device: " << Capture::DeviceModelToString(calib.model) <<"\n";
}


CameraCalibrationExporter::CameraCalibrationExporter()
{
}


CameraCalibrationExporter::~CameraCalibrationExporter()
{
}

void CameraCalibrationExporter::Export(Intel::RealSense::Projection * projection, std::string fileName)
{
	Calibration* calibration = projection->QueryCalibration();
	StreamCalibration depthCalib;
	StreamTransform depthPose;
	calibration->QueryStreamProjectionParameters(StreamType::STREAM_TYPE_DEPTH, &depthCalib, &depthPose);

	StreamCalibration colourCalib;
	StreamTransform colourPose;
	calibration->QueryStreamProjectionParameters(StreamType::STREAM_TYPE_COLOR, &colourCalib, &colourPose);

	std::ofstream file(fileName, std::ios::out);
	file << "Depth camera:\n";
	OutputPose(file, depthPose);
	OutputCalibration(file, depthCalib);

	file << "Colour camera:\n";
	OutputPose(file, colourPose);
	OutputCalibration(file, colourCalib);

}


