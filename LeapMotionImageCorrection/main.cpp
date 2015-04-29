/**
*	Leap Motion Image Correction
*	By: Andres Bejarano <abejara@purdue.edu>
*
*	- Captures the images from the Leap Motion
*	- Corrects the images using warp
*	- Stores the images in OpenCV format
*	- Crops the corrected images
*	- Save images (original, corrected and cropped)
*/

#define	CORR_WIDTH	640			// Width of the corrected image
#define	CORR_HEIGHT	640			// Height of the corrected image

#include <iostream>
#include <Leap.h>
#include <opencv2\opencv.hpp>

/**
*	main function (no arguments required)
*/
int main(int argc, char**argv)
{

	//	Initiate the Leap Motion controller and set the image policy
	Leap::Controller controller;
	controller.setPolicy(Leap::Controller::POLICY_IMAGES);

	//	Wait until the Leap Motion is connected
	std::cout << "Waiting for a Leap Motion..." << std::endl;
	while (!controller.isConnected()) 
	{
		// DO NOTHING, JUST WAIT!
	}
	std::cout << "Leap Motion found" << std::endl;

	//	Define the windows where images will be shown
	//	Only cropped images will be displayed
	std::string lcrWindow = "left_cropped";
	std::string rcrWindow = "right_cropped";
	cv::namedWindow(lcrWindow, cv::WINDOW_AUTOSIZE);
	cv::namedWindow(rcrWindow, cv::WINDOW_AUTOSIZE);

	//	File names for the left images
	std::stringstream lFilename;
	std::stringstream lCorrFilename;
	std::stringstream lCropFilename;
	
	//	File names for the right images
	std::stringstream rFilename;
	std::stringstream rCorrFilename;
	std::stringstream rCropFilename;

	int currentImage = 0;

	//	Region of interest for cropping images
	cv::Rect lRoi(100, 185, 402, 273);
	cv::Rect rRoi(110, 184, 402, 273);

	//	Repeat indefinitely until ESC is pressed
	while (true)
	{
		//	Get the images from the Leap Motion
		Leap::ImageList images = controller.images();
		Leap::Image lImage = images[0];
		Leap::Image rImage = images[1];

		//	Check if both images contain valid data
		if (!(lImage.isValid() && rImage.isValid()))
		{
			continue;
		}

		//	Save images in OpenCV format
		cv::Mat lCam = cv::Mat(lImage.height(), lImage.width(), CV_8UC1);
		lCam.data = (unsigned char*)lImage.data();
		cv::Mat rCam = cv::Mat(rImage.height(), rImage.width(), CV_8UC1);
		rCam.data = (unsigned char*)rImage.data();

		//	Get the calibration maps
		cv::Mat lcCam = cv::Mat(CORR_WIDTH, CORR_HEIGHT, CV_8UC1);
		cv::Mat rcCam = cv::Mat(CORR_WIDTH, CORR_HEIGHT, CV_8UC1);
		unsigned char brightness;

		for (int i = 0; i < CORR_WIDTH; i += 1)
		{
			for (int j = 0; j < CORR_HEIGHT; j += 1)
			{

				//	Normalize i, j values to range [0..1]
				Leap::Vector lInput((float)i / CORR_WIDTH, (float)j / CORR_HEIGHT, 0);
				Leap::Vector rInput((float)i / CORR_WIDTH, (float)j / CORR_HEIGHT, 0);

				//	Convert from normalized [0..1] to slope [-4..4] the left input
				lInput.x = (lInput.x - lImage.rayOffsetX()) / lImage.rayScaleX();
				lInput.y = (lInput.y - lImage.rayOffsetY()) / lImage.rayScaleY();

				//	Convert from normalized [0..1] to slope [-4..4] the right input
				rInput.x = (rInput.x - rImage.rayOffsetX()) / rImage.rayScaleX();
				rInput.y = (rInput.y - rImage.rayOffsetY()) / rImage.rayScaleY();

				//	Get the corrected value for both current left and right pixels
				Leap::Vector lPixel = lImage.warp(lInput);
				Leap::Vector rPixel = rImage.warp(rInput);

				//	Apply correction on the left image
				if (lPixel.x >= 0 && lPixel.x < lImage.width() && lPixel.y >= 0 && lPixel.y < lImage.height())
				{
					int index = floor(lPixel.y) * lImage.width() + floor(lPixel.x);
					brightness = lImage.data()[index];
				}
				else
				{
					brightness = 255;
				}
				lcCam.at<unsigned char>(j, i) = brightness;

				//	Apply correction on the right image
				if (rPixel.x >= 0 && rPixel.x < rImage.width() && rPixel.y >= 0 && rPixel.y < rImage.height())
				{
					int index = floor(rPixel.y) * rImage.width() + floor(rPixel.x);
					brightness = rImage.data()[index];
				}
				else
				{
					brightness = 255;
				}
				rcCam.at<unsigned char>(j, i) = brightness;

			}
		}

		cv::Mat lCropped = lcCam(lRoi);
		cv::Mat rCropped = rcCam(rRoi);

		lFilename.str("");
		lFilename.clear();
		lCorrFilename.str("");
		lCorrFilename.clear();
		lCropFilename.str("");
		lCropFilename.clear();

		rFilename.str("");
		rFilename.clear();
		rCorrFilename.str("");
		rCorrFilename.clear();
		rCropFilename.str("");
		rCropFilename.clear();

		lFilename     << "images/lImg" << currentImage << ".orig.jpg";
		lCorrFilename << "images/lImg" << currentImage << ".corr.jpg";
		lCropFilename << "images/lImg" << currentImage << ".crop.jpg";

		rFilename     << "images/rImg" << currentImage << ".orig.jpg";
		rCorrFilename << "images/rImg" << currentImage << ".corr.jpg";
		rCropFilename << "images/rImg" << currentImage << ".crop.jpg";

		cv::imwrite(lFilename.str(), lCam);
		cv::imwrite(lCorrFilename.str(), lcCam);
		cv::imwrite(lCropFilename.str(), lCropped);

		cv::imwrite(rFilename.str(), rCam);
		cv::imwrite(rCorrFilename.str(), rcCam);
		cv::imwrite(rCropFilename.str(), rCropped);

		currentImage += 1;

		//	Show cropped images
		cv::imshow(lcrWindow, lCropped);
		cv::imshow(rcrWindow, rCropped);

		//	Press ESC to exit
		if (cv::waitKey(30) == 27) {
			break;
		}

	}

	return 0;

}
