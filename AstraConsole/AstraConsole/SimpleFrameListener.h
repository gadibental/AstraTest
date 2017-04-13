#pragma once

#include <astra/astra.hpp>
#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <chrono>
#include "Camera.h"


class SimpleFrameListener : public astra::FrameListener
{

public:

	SimpleFrameListener();

	virtual void on_frame_ready(astra::StreamReader& reader, astra::Frame& frame) override;

	void SaveVertexMap(const astra::CoordinateMapper& mapper);

	int ShowDepth(const astra::DepthFrame& depthFrame);

	void ShowColour(const astra::ColorFrame& colorFrame);


	void check_fps();

private:
	using duration_type = std::chrono::duration < double >;
	duration_type frameDuration_{ 0.0 };

	using clock_type = std::chrono::system_clock;
	std::chrono::time_point<clock_type> lastTimepoint_;

	std::shared_ptr<Camera> m_depthCamera;
	std::shared_ptr<Camera> m_colourCamera;

};
