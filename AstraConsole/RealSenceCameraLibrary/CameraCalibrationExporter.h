#pragma once

#include <memory>
#include <string>

namespace Intel
{
	namespace RealSense
	{
		class Projection;
	}
}

class CameraCalibrationExporter
{
public:
	CameraCalibrationExporter();
	~CameraCalibrationExporter();

	void Export(Intel::RealSense::Projection* projection, std::string fileName);

};

