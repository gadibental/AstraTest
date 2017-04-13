// IntelConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "RealSense/SenseManager.h"
#include "RealSense/SampleReader.h"

#include "opencv2/highgui.hpp" // basic opencv header. needed for all openCV functionality
#include <opencv2/imgproc.hpp> // needed for overlay on images 

#include <iostream>
#include <fstream>

using namespace Intel::RealSense;

int main()
{
	/* Creates an instance of the SenseManager */
	SenseManager *pipeline = SenseManager::CreateInstance();
	if (!pipeline) 
	{
		wprintf_s(L"Unable to create the SenseManager\n");
		return 3;
	}


	pipeline->EnableStream(Capture::STREAM_TYPE_COLOR, 640, 480);
	pipeline->EnableStream(Capture::STREAM_TYPE_DEPTH);
	Status status = pipeline->Init();

	DataDesc desc = {};
	desc.deviceInfo.streams = Capture::STREAM_TYPE_COLOR | Capture::STREAM_TYPE_DEPTH;

	Capture::Device *device = pipeline->QueryCaptureManager()->QueryDevice();
	device->ResetProperties(Capture::STREAM_TYPE_ANY);

	short invalids[2];
	invalids[0] = 0;// pipeline->QueryCaptureManager()->QueryDevice()->QueryDepthSaturationValue();
	invalids[1] = pipeline->QueryCaptureManager()->QueryDevice()->QueryDepthLowConfidenceValue();
	Projection* projection = pipeline->QueryCaptureManager()->QueryDevice()->CreateProjection();

	Image* depthImage = NULL;
	Image* colourImage = NULL;


	cv::Mat colourMat, depthMat;
	const int maxFrames = 1000;
	for (int nframes = 0; nframes < maxFrames; nframes++)
	{
		if ((NULL != depthImage && NULL != colourImage) && ((nframes % 20) == 0))
		{
			ImageInfo dI = depthImage->QueryInfo();
			int numPoints = dI.width * dI.height;
			std::vector<Point3DF32> vertices(numPoints);
			//Point3DF32* vertices = new Point3DF32[numPoints];
			status = projection->QueryVertices(depthImage, &vertices[0]);
			std::vector<PointF32> pixels(numPoints);
			projection->ProjectCameraToColor(numPoints, &vertices[0], &pixels[0]);
			std::ofstream file("RealVertex.csv", std::ios::out);

			file << "X,Y,Z,R,G,B\n";

			for (int p = 0; p < numPoints; ++p)
			{
				Point3DF32 vertex = vertices[p];
				if (vertex.z != 0 && vertex.z < 1000)
				{
					cv::Point pixel((int)pixels[p].x, (int)pixels[p].y);
					if (pixel.x >= 0 && pixel.x < colourMat.cols && pixel.y >= 0 && pixel.y < colourMat.rows)
					{
						file << vertex.x << ',' << vertex.y << ',' << vertex.z << ',';
						cv::Vec3b colourValue = colourMat.at<cv::Vec3b>(pixel);
						for (int c = 0; c < colourValue.rows; c++)
						{
							file << (int)colourValue[colourValue.rows - c - 1] << ',';
						}
					}

					file << "\n";
				}
			}
		}

		/* Waits until new frame is available and locks it for application processing */
		status = pipeline->AcquireFrame(false);

		if (status < Status::STATUS_NO_ERROR)
		{
			if (status == Status::STATUS_STREAM_CONFIG_CHANGED) 
			{
				wprintf_s(L"Stream configuration was changed, re-initilizing\n");
				pipeline->Close();
			}
			break;
		}

		/* Render streams, unless -noRender is selected */
		const Capture::Sample *sample = pipeline->QuerySample();
		if (sample) 
		{
			if (NULL != sample->depth)
			{
				depthImage = sample->depth;
				ImageInfo dI = depthImage->QueryInfo();
				depthMat = cv::Mat::zeros(dI.height, dI.width, CV_16UC1);
				ImageData ddata;
				if (depthImage->AcquireAccess(Image::ACCESS_READ, &ddata) >= PXC_STATUS_NO_ERROR)
				{
					memcpy(depthMat.data, ddata.planes[0], dI.height*dI.width * 2);
				}
				depthImage->ReleaseAccess(&ddata);

				const float maxDepth = 1000;
				cv::Mat showImage;
				cv::convertScaleAbs(depthMat, showImage, 255.0 / maxDepth, 0);

				cv::imshow("depth", showImage);
				//		cv::imwrite("depth.png", showImage);
				//SaveMat("depth.txt", depthImage);



			}
			if (NULL != sample->color)
			{
				colourImage = sample->color;
				ImageInfo cI = colourImage->QueryInfo();
				colourMat = cv::Mat::zeros(cI.height, cI.width, CV_8UC3);
				ImageData ddata;
				if (colourImage->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_BGR, &ddata) >= PXC_STATUS_NO_ERROR)
				{
					memcpy(colourMat.data, ddata.planes[0], cI.height*cI.width*3);
				}
				colourImage->ReleaseAccess(&ddata);

				cv::imshow("colour", colourMat);
			}

			cv::waitKey(1); // refresh

		}

		/* Releases lock so pipeline can process next frame */
		pipeline->ReleaseFrame();
	}

	pipeline->Release();


    return 0;
}

