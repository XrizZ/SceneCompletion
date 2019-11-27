//============================================================================
// Name        : PDCIImage.cpp
// Author      : Christian Thurow
// Description : contains functions for the CPDCIImage class
//============================================================================
#pragma once

#include "stdafx.h"
#include "PDCIImage.h"
#include "highgui.h"
#include "PoissonBlending.h"
#include "retrieval_framework_2012\shared\descriptors\gist_helper.hpp"
#include <algorithm>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>

void InitFilter(std::vector<cv::Mat_<std::complex<double>>>* _filters);

std::ifstream m_meanFileHandle;
std::ifstream m_varianceFileHandle;
std::ifstream m_fileListHandle;

char* m_fileListBuffer;
char* m_meanFileBuffer;
char* m_varianceFileBuffer;

bool CPDCIImage::LoadImageFromFile(char* path)
{
	m_inputImage = cv::imread(path, 1 /*CV_LOAD_IMAGE_UNCHANGED*/);

	if(m_inputImage.data == NULL)
		return false;

	//show input image
	cv::namedWindow( "input image", CV_WINDOW_AUTOSIZE );
	cv::imshow( "input image", m_inputImage );

	return true;
}

//loads the mask from a file
//note: mask file needs to have same resolution as input image
//usage: black means, take pixels from input image, white means, 
//		 this is the region where we want to complete the scene from other images
bool CPDCIImage::LoadMaskFromFile(char* path)
{
	m_mask = cv::imread(path, CV_LOAD_IMAGE_GRAYSCALE);

	if(m_mask.data == NULL)
		return false;

	//show mask
	cv::namedWindow( "mask", CV_WINDOW_AUTOSIZE );
	cv::imshow( "mask", m_mask);
	//cvWaitKey(0);	//DEBUG

	return true;
}

bool CPDCIImage::Cleanup()
{	
	m_inputImage.release();
	m_mask.release();

	//clear the similar images
	for(vector<cv::Mat>::iterator it = m_similarImages.begin(); it != m_similarImages.end(); it++)
	{
		(*it).release();		
    }

	while(m_similarImages.size() != 0)
		m_similarImages.pop_back();

	//clear the output images
	for(vector<cv::Mat>::iterator it = m_outputImages.begin(); it != m_outputImages.end(); it++)
	{
		(*it).release();
    }

	while(m_outputImages.size() != 0)
		m_outputImages.pop_back();

	m_GIST.clear();

	return true;
}

double CPDCIImage::CalcSimilarity(GistDescriptor* descrA, GistDescriptor* descrB)
{
	double dissimilarity = 0.0;
	//L1 distance
	//loop: für jedes tile/patch
	for(int i=0; i<NUM_FREQS; i++)
		for(int j=0; j<NUM_ORIENTS; j++)
			for(int y=0; y<NUM_Y_TILES; y++)
				for(int x=0; x<NUM_X_TILES; x++)
				{
					dissimilarity += m_maskOverlap[y][x]*(abs(descrA->m_mean[i][j][y][x] - descrB->m_mean[i][j][y][x])
						+ abs(descrA->m_variance[i][j][y][x] - descrB->m_variance[i][j][y][x]));
				}

	descrA->m_dissimilarity = dissimilarity;	
	//cout << dissimilarity << endl; //DEBUG

	return dissimilarity;
}

void CPDCIImage::CloseFileHandles()
{
	m_fileListHandle.close();
	m_meanFileHandle.close();
	m_varianceFileHandle.close();
}

GistDescriptor* CPDCIImage::ReadNextGistDescriptor()
{
	int filenameLength = ReadIntFromFile(&m_fileListHandle);
	if (m_fileListHandle.get() == '\0')
	{
		CloseFileHandles();
		return NULL;
	}
	else m_fileListHandle.unget();

	char* currentFilename = new char[filenameLength+1];
	currentFilename[filenameLength] = '\0';
	//m_fileListHandle.get(currentFilename, filenameLength+1);		//ifstream.get(...) reads n-1 bytes, not n ... WTF?!
	ReadInFileNumBytes(&m_fileListHandle, currentFilename, filenameLength);

	long currentMeanDescrCount = ReadLongFromFile(&m_meanFileHandle);
	long currentVarDescrCount = ReadLongFromFile(&m_varianceFileHandle);

	//if (currentMeanDescrCount != currentVarDescrCount || currentMeanDescrCount != NUM_Y_TILES * NUM_X_TILES * NUM_FREQS * NUM_ORIENTS)
	//{
		//CloseFileHandles();
		//return NULL;
	//}

	GistDescriptor* retVal = new GistDescriptor();
	retVal->m_fileName = "h:\\";
	retVal->m_fileName += currentFilename;

	for(int i=0; i<NUM_FREQS; i++)
		for(int j=0; j<NUM_ORIENTS; j++)
			for(int y=0; y<NUM_Y_TILES; y++)
				for(int x=0; x<NUM_X_TILES; x++)
				{
					retVal->m_mean[i][j][y][x] = ReadFloatFromFile(&m_meanFileHandle);
					retVal->m_variance[i][j][y][x] = ReadFloatFromFile(&m_varianceFileHandle);
				}

	return retVal;	
}

void CPDCIImage::InitMaskWeights()
{
	double tileWidth = m_mask.cols/NUM_X_TILES;
	double tileHeight = m_mask.rows/NUM_Y_TILES;

	//gewichtung mit der maske!
	for(int y=0; y<NUM_Y_TILES; y++)
		for(int x=0; x<NUM_X_TILES; x++)
		{
			int pixelCount = 0;
			for(int w=0; w<tileWidth; w++)
				for(int h=0; h<tileHeight; h++)
				{
					if(m_mask.at<uchar>(y*tileHeight + h, x*tileWidth + w) < 255.0)
						pixelCount++;
				}

			m_maskOverlap[y][x] = (double)pixelCount/(double)(tileWidth*tileHeight);
		}

}

//finds images in large DB that are similar to m_inputImage
//stores the results in m_similarImages
//maximal number of similar images to store is: m_maxNumSimilarImages
void CPDCIImage::FindSimilarImagesFromLargeDB()
{
	OpenDescriptorFiles();
	
	GistDescriptor* currGist = NULL;

	InitMaskWeights();

	//DWORD start = ::GetTickCount();
	
	unsigned int count = 0;
	//Load each image according to file(that the script produced)
	while((currGist = ReadNextGistDescriptor()) != NULL)
	{
		CalcSimilarity(currGist, m_inputGIST);
		InsertElem(currGist);
		if(count++%100 == 0)
			cout << "| Read " << count-1 << " images from DB\r";
	}

	cout << "| Read " << count << " images from DB\n";

	//DWORD diff = ::GetTickCount() - start;

	//cout << "time for loading files: " << diff << endl;

	//cvWaitKey(0);

	//read in images
	for(int i=0; i<m_GIST.size(); i++)
	{
		const char* fileName = m_GIST.at(i)->m_fileName.c_str();

		IplImage* newImage = cvLoadImage(fileName, 1);
		
		//if newImage doesnt have same size as our input, then resize it!
		if((newImage->width != m_inputImage.cols) || (newImage->height != m_inputImage.rows))
		{
			IplImage* oldImageNewSize = cvCreateImage(cvSize(m_inputImage.cols, m_inputImage.rows), newImage->depth, newImage->nChannels);
			cvResize(newImage, oldImageNewSize);
			newImage = oldImageNewSize;
		}

		m_similarImages.push_back(newImage);
	}
}

void CPDCIImage::InsertElem(GistDescriptor* currGist)
{
	static double max = numeric_limits<double>::max();
	if(m_GIST.size() >= m_maxNumSimilarImages && currGist->m_dissimilarity > max)
	{
		delete currGist;
		return;
	}

	//insert
	vector<GistDescriptor*>::iterator it;
	for(it = m_GIST.begin(); it != m_GIST.end(); it++)
	{
		GistDescriptor* here = (*it);
		if(here->m_dissimilarity > currGist->m_dissimilarity)
			break;
	}
	m_GIST.insert(it, currGist);

	if(m_GIST.size() > m_maxNumSimilarImages)
		m_GIST.pop_back();

	max = m_GIST.back()->m_dissimilarity;
}

//creates the gist descriptor of the input image, for later comparision with other images
void CPDCIImage::CalcGISTofInput()
{
    // ------------------------------------------------------------------------
    // Required input:
    //
    // this generator expects the image to be a CV_8UC3 with BGR channel order.
    // ------------------------------------------------------------------------

    cv::Mat image;
	cv::cvtColor(m_inputImage, image, CV_BGR2GRAY);

	unsigned int _realwidth = WIDTH;
	unsigned int _realheight = HEIGHT;

	unsigned int _height = _realheight + PADDING;
	unsigned int _width = _realwidth + PADDING;

    // uniformly scale the image such that it has no side that is larger than the filter's size
	double scaling_factor = (image.size().width > image.size().height)
                          ? static_cast<double>(_realwidth) / image.size().width
                          : static_cast<double>(_realheight) / image.size().height;


    // need to use INTER_AREA for downscaling as only this performs correct antialiasing
    cv::Mat scaled;
    cv::resize(image, scaled, cv::Size(), scaling_factor, scaling_factor, cv::INTER_AREA);

    cv::Mat_<unsigned char> padded(_height, _width);
    symmetric_pad(cv::Mat_<unsigned char>(scaled), padded);

    // if exists, apply prefilter on image buffer
    // for example the torralba prefilter    
	//torralba_prefilter(_width, _height, 4.0 /* * _width / _realwidth*/);
	
    cv::Mat_<std::complex<double>> src(_height, _width);
    cv::MatConstIterator_<unsigned char> sit = padded.begin();
    cv::MatIterator_<std::complex<double>> dit = src.begin();

    while (sit != padded.end())
    {
        *dit++ = (float) *sit++ * (1.0/255.0);
    }

    // transform image into fourier space
    cv::Mat_<std::complex<double>> fts;
    cv::dft(src, fts);

    //std::vector<float> means;
    //std::vector<float> sdevs;

    // shouldn't we better use scaled.width and scaled.height?
    int tilewidth = scaling_factor * image.size().width / NUM_X_TILES;
    int tileheight = scaling_factor * image.size().height / NUM_Y_TILES;

	std::vector<cv::Mat_<std::complex<double>> > _filters;
	InitFilter(&_filters);

	m_inputGIST = new GistDescriptor();

    // convolve image with each filter in fourier space
    for (size_t i = 0; i < _filters.size(); i++)
    {
        // multiply sprectrums == convolve
        cv::Mat_<std::complex<double>> ftd;
        cv::mulSpectrums(fts, _filters[i], ftd, 0);

        // transform back to image space
        cv::Mat_<std::complex<double>> dst;
        cv::idft(ftd, dst, cv::DFT_SCALE);

        // compute the response magnitude
        cv::Mat_<float> mag(_height, _width);
        for (int r = 0; r < dst.rows; r++)
			for (int c = 0; c < dst.cols; c++)
			{
				float real = dst(r, c).real();
				float imag = dst(r, c).imag();
				mag(r, c) = std::sqrt(real*real + imag*imag);
			}

        // get mean and standard deviation of tile contents
        for (size_t y = 0; y < NUM_Y_TILES; y++)
			for (size_t x = 0; x < NUM_X_TILES; x++)
			{
				cv::Mat tile = mag(cv::Rect(x * tilewidth, y * tileheight, tilewidth, tileheight));
				cv::Scalar m, d;
				cv::meanStdDev(tile, m, d);

				//means.push_back(m[0]);
				m_inputGIST->m_mean[i/NUM_ORIENTS][i%NUM_ORIENTS][y][x] = m.val[0];

				//sdevs.push_back(d[0]*d[0]);
				m_inputGIST->m_variance[i/NUM_ORIENTS][i%NUM_ORIENTS][y][x] = d[0]*d[0];
			}
    }
	int blub = 42;	//DEBUG
}

void InitFilter(std::vector<cv::Mat_<std::complex<double>>>* _filters)
{
    const double delta_freq = std::pow(2.0, DELTA_FREQ_OCT);
    const double bandwidth = std::pow(2.0, BANDWIDTH_OCT);
    const double delta_omega = (M_PI / static_cast<double>(NUM_ORIENTS));
	unsigned int _height = HEIGHT + PADDING;
	unsigned int _width = WIDTH + PADDING;
    const double max_extend = std::max(_width, _height);
    const double pad_max_peak_freq = max_extend * MAX_PEAK_FREQ / (max_extend + static_cast<double>(PADDING));

    // compute gabor filter in regular formation
    for (size_t i = 0; i < NUM_FREQS; i++)
    {
        for (size_t k = 0; k < NUM_ORIENTS; k++)
        {
            // compute params
            const double curPeak = pad_max_peak_freq / std::pow(delta_freq, static_cast<double>(i));
            const double curOmega = k * delta_omega;

            // generate filter
            cv::Mat_<std::complex<double>> filter(_width, _height);
            //if (_polar)
            //{
                generate_polargabor_filter(filter, curPeak, bandwidth, curOmega, delta_omega * ANGLE_FACTOR);
            //}
            //else
            //{
            //    generate_gabor_filter(filter, curPeak, bandwidth, curOmega, delta_omega * _angle_factor);
            //}

            // kill dc
            filter(0, 0) = 0;

            // add
            _filters->push_back(filter);
        }
    }
}

//just takes all images from the small local DB
//stores the results in m_similarImages
//maximal number of similar images to store is: m_maxNumSimilarImages
void CPDCIImage::FindSimilarImagesFromTinyDB()
{
	m_similarImages.clear();
	for(int i=0; i<m_maxNumSimilarImages; i++)
	{
		//this only works for up to 10 images:

		string path = "Images\\tinyDB\\";
		char* file = new char[7];
		file[0] = (i>8) ? '1' : '0';
		file[1] = (i>8) ? '0' : i+49;
		file[2] = '.';
		file[3] = 'j';
		file[4] = 'p';
		file[5] = 'g';
		file[6] = '\0';
		path += file;

		CvMat* newImg = cvLoadImageM(path.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		m_similarImages.push_back(newImg);
		delete file;
	}
}

//puts out all similar images to the screen
void CPDCIImage::PrintSimilarImages()
{
	int i=0;
	for(vector<cv::Mat>::iterator it = m_similarImages.begin(); it != m_similarImages.end(); it++, i++)
	{
		string name = "similar image: ";
		name += i+48;
		cv::namedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
		cv::imshow(name.c_str(), (*it));
    }
}

bool IsMaskBlack(cv::Mat mask)
{
	int whiteCount = 0;

	for(int x=0; x<mask.cols; x++)
		for(int y=0; y<mask.rows; y++)
			if(mask.at<uchar>(y, x) > 0)
				whiteCount++;

	if(whiteCount > 0)
		return false;
	else
		return true;
}

//implements the blending of the input image with the similarImages found according to the mask
void CPDCIImage::Blend()
{
	int i = 1;
	for(vector<cv::Mat>::iterator it = m_similarImages.begin(); it != m_similarImages.end(); it++, i++)
	{
		cout << "| Blending Image: " << i << " of " << m_maxNumSimilarImages << "\r";
		cv::Mat newImg;

		if(IsMaskBlack(m_similarImagesMasks.at(i-1)))
			newImg = m_similarImagesMasks.at(i-1).clone();
		else
			newImg = PoissonBlend((*it), m_inputImage, m_similarImagesMasks.at(i-1));

		m_outputImages.push_back(newImg);
	}
	cout << endl;
}

//shows the output images found on the screen
void CPDCIImage::ShowResults()
{
	int i=0;
	for(vector<cv::Mat>::iterator it = m_outputImages.begin(); it != m_outputImages.end(); it++, i++)
	{
		string name = "Resulting image after Blending: ";
		int length = log((double)m_maxNumSimilarImages) + 2;
		char* number = new char[length];
		itoa(i+1, number, 10);
		name += number;
		cv::namedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
		cv::imshow(name.c_str(), (*it));
    }
}

void CPDCIImage::ShowMasks()
{	
	for(int i=0; i<m_similarImagesMasks.size(); i++)
	{
		string name = "Resulting Mask after Smoothing: ";
		int length = log((double)m_maxNumSimilarImages) + 2;
		char* number = new char[length];
		itoa(i+1, number, 10);
		name += number;
		cv::namedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
		cv::imshow(name.c_str(), m_similarImagesMasks.at(i));
    }
}

//saves the outputImages array into seperate files
void CPDCIImage::SaveResults()
{
	string path = "Images\\Results\\";
	string type = ".png";

	int i=0;
	for(vector<cv::Mat>::iterator it = m_outputImages.begin(); it != m_outputImages.end(); it++, i++)
	{
		string imageType = "result";
		string name = imageType;
		int length = log((double)m_maxNumSimilarImages) + 2;
		char* number = new char[length];
		itoa(i, number, 10);
		name += number;
		name += type;
		cv::imwrite(path + name, (*it));
	}
}

//saves the outputImages array into seperate files
void CPDCIImage::SaveMasks()
{
	string path = "Images\\Results\\";
	string type = ".png";

	int i=0;
	for(vector<cv::Mat>::iterator it = m_similarImagesMasks.begin(); it != m_similarImagesMasks.end(); it++, i++)
	{
		string imageType = "mask";
		string name = imageType;
		int length = log((double)m_maxNumSimilarImages) + 2;
		char* number = new char[length];
		itoa(i, number, 10);
		name += number;
		name += type;
		cv::imwrite(path + name, (*it));
	}
}

void CPDCIImage::FillGapsInMasks()
{
	int i=1;
	for(vector<cv::Mat>::iterator it = m_similarImagesMasks.begin(); it != m_similarImagesMasks.end(); it++, i++)
	{
		cout << "| Smoothing Mask: " << i << " of " << m_maxNumSimilarImages << "\r";
		FillGapsInMask(&(*it));
	}

	cout << endl;
}

void CPDCIImage::FillGapsInMask(cv::Mat* mask)
{
	int white = 255;
	for(int x=0; x<m_mask.cols; x++)
		for(int y=0; y<m_mask.rows; y++)
		{
			//if already mask
			if(mask->at<uchar>(y, x) > 0)
				continue;

			//if pixel is on border, then skip
			for(int i=0; i>m_listOfBorderPoints->size(); i++)
			{
				CvPoint curr = m_listOfBorderPoints->at(i);
				if(curr.x == x && curr.y == y)
					break;
			}

			//pixel is not on border
			int leftVal = white;
			if(x > 0) leftVal = mask->at<uchar>(y, x-1);

			int rightVal = white;
			if(x < m_mask.cols-1) rightVal = mask->at<uchar>(y, x+1);

			int topVal = white;
			if(y > 0) topVal = mask->at<uchar>(y-1, x);

			int downVal = white;
			if(y < m_mask.rows-1) downVal = mask->at<uchar>(y+1, x);

			int sum = leftVal + rightVal + topVal + downVal;

			if(sum >= 3*white)
				mask->at<uchar>(y, x) = white;
		}

	//cv::imshow("refined->holes filled", mask);
	//cvWaitKey(0);

	//erase all white pixels outside mask!
	int black = 0;
	for(int x=0; x<m_mask.cols; x++)
		for(int y=0; y<m_mask.rows; y++)
		{
			//if already mask
			if(mask->at<uchar>(y, x) == black)
				continue;

			//if pixel is on border, then skip
			for(int i=0; i>m_listOfBorderPoints->size(); i++)
			{
				CvPoint curr = m_listOfBorderPoints->at(i);
				if(curr.x == x && curr.y == y)
					break;
			}

			//pixel is not on border
			int leftVal = black;
			if(x > 0) leftVal = mask->at<uchar>(y, x-1);

			int rightVal = black;
			if(x < m_mask.cols-1) rightVal = mask->at<uchar>(y, x+1);

			int topVal = black; 
			if(y > 0) topVal = mask->at<uchar>(y-1, x);

			int downVal = black;
			if(y < m_mask.rows-1) downVal = mask->at<uchar>(y+1, x);

			int sum = leftVal + rightVal + topVal + downVal;

			if(sum <= 1*white) //>= 3*black ^^
				mask->at<uchar>(y, x) = black;
		}

	//cv::imshow("refined->islands erased", mask);
	//cvWaitKey(0);

}

void CPDCIImage::GetBestCuts()
{
	InitMaskBorder();

	int i = 1;
	for(vector<cv::Mat>::iterator it = m_similarImages.begin(); it != m_similarImages.end(); it++, i++)
	{
		cout << "| Calculating Best Cut for Mask: " << i << " of " << m_maxNumSimilarImages << "\r";
		cv::Mat currMask = GetBestCut((*it));
		m_similarImagesMasks.push_back(currMask);
	}

	cout << endl;

	//FillGapsInMasks();
}

double GetGreyValueAt(int x, int y, cv::Mat image)
{
	uchar* inputRow = image.ptr(y)+x*image.channels();
	int inputR = (*inputRow);
	int inputG = (*inputRow)+1;
	int inputB = (*inputRow)+2;

	double grey = 0.299*inputR + 0.587*inputG + 0.114*inputB;
	return grey;
}

void CPDCIImage::InitMaskBorder()
{
	CvSize size = cvSize(m_mask.cols, m_mask.rows);
	IplImage* inputMask = cvCreateImage(size, IPL_DEPTH_8U, 1);
	//copy mask into an iplImage
	for(int i=0; i<m_mask.cols; i++) //width
		for(int j=0; j<m_mask.rows; j++)
			cvSet2D(inputMask, j, i, cvScalar(m_mask.at<uchar>(j, i)));

	IplImage* erosionResult = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplConvKernel* kernel = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_CROSS);
	cvErode(inputMask, erosionResult, kernel);
	IplImage* border = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSub(inputMask, erosionResult, border);

	m_listOfBorderPoints = new vector<CvPoint>;

	//add border points to list
	for(int x=0; x<m_mask.cols; x++)
		for(int y=0; y<m_mask.rows; y++)
		{
			if(cvGetReal2D(border, y, x) > 0)
				m_listOfBorderPoints->push_back(cvPoint(x,y));
		}
}

vector<cv::Point>* CPDCIImage::GetBorderPixelsFromMask(cv::Mat_<uchar> mask, int preErodeNum)
{
	CvSize size = cvSize(mask.cols, mask.rows);
	
	//IplImage* inputMask = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cv::Mat_<bool> kernel(3, 3);
	cv::Mat_<bool> eroded(size);
	cv::Mat_<bool> tmp(size);

	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
		{
			if (x == 1 || y == 1) kernel(y, x) = 1;
			else kernel(y, x) = 0;
		}
	
	cv::erode(mask, eroded, kernel, cv::Point(-1, -1), preErodeNum);
	cv::erode(eroded, tmp, kernel, cv::Point(-1, -1), 1);

	cv::subtract(eroded, tmp, tmp);

	vector<cv::Point>* borderPointsList = new vector<cv::Point>;

	//add border points to list
	for(int x=0; x<tmp.cols; x++)
		for(int y=0; y<tmp.rows; y++)
		{
			if(tmp(y, x) > 0)
				borderPointsList->push_back(cv::Point(x,y));
		}

	return borderPointsList;
}

//returns the distance of a point to the nearest mask point, if the point is wihtin the mask, the negative distance to the nearest border point is returned
double CPDCIImage::GetDistance(int x, int y)
{
	double distance = 0.0;
	int white = 255;

	//if point within mask
	uchar maskVal = m_mask.at<uchar>(y, x);
	if(maskVal == white)
	{
		//point is part of the mask!
		double negMinDist = sqrt(double( (x - m_listOfBorderPoints->at(0).x)*(x - m_listOfBorderPoints->at(0).x) + 
		(y - m_listOfBorderPoints->at(0).y)*(y - m_listOfBorderPoints->at(0).y) 	));

		//checks every second pixel on the border
		for(int i=1; i < m_listOfBorderPoints->size(); i+=2)
		{
			int currY = m_listOfBorderPoints->at(i).y;
			int currX = m_listOfBorderPoints->at(i).x;
			double currDist = sqrt(double( (x - currX)*(x - currX) + (y - currY)*(y - currY) ));

			if(negMinDist > currDist)
				negMinDist = currDist;

			if(negMinDist == 0.0)
				return 0.0;

			if(negMinDist <= 1.5)
				return -1.0;
		}
		return -negMinDist;
	}

	//point is outside the mask

	double minDist = sqrt(double( (x - m_listOfBorderPoints->at(0).x)*(x - m_listOfBorderPoints->at(0).x) + 
		(y - m_listOfBorderPoints->at(0).y)*(y - m_listOfBorderPoints->at(0).y) 	));

	//checks every second pixel on the border
	for(int i=1; i < m_listOfBorderPoints->size(); i+=2)
	{
		int currY = m_listOfBorderPoints->at(i).y;
		int currX = m_listOfBorderPoints->at(i).x;
		double currDist = sqrt(double( (x - currX)*(x - currX) + (y - currY)*(y - currY) ));

		if(currDist < minDist)
			currDist = minDist;

		if(minDist <= 1.5)
			return 1.0;

		if(minDist == 0.0)
			return 0.0;
	}

	distance = minDist;
	return distance;
}

double CPDCIImage::GetDistWeight(int x, int y)
{
	double weighting = 0.0;
	//double k = 0.002;	//original
	double k = 0.01;
	//double k = 0.02;
	double distance = GetDistance(x, y);
	if(distance < 0)
		weighting = 999999999;
		//weighting = pow((k*10)*(-distance), 3);
	else 
		weighting = pow(k*distance, 3);

	return weighting;
}

//returns all image border pixels that are not covered by the mask
vector<cv::Point>* CPDCIImage::GetBorderPixelsFromImage()
{
	vector<cv::Point>* border = new vector<cv::Point>;
	int black = 0;
	//top border
	for(int x=0; x<m_mask.cols-1; x++)
	{
		if(m_mask.at<uchar>(0, x) == black)
			border->push_back(cv::Point(x, 0));
	}

	//right border
	for(int y=0; y<m_mask.rows-1; y++)
	{
		if(m_mask.at<uchar>(y, m_mask.cols-1) == black)
			border->push_back(cv::Point(m_mask.cols-1, y));
	}

	//bottom border
	for(int x=1; x<m_mask.cols; x++)
	{
		if(m_mask.at<uchar>(m_mask.rows-1, x) == black)
			border->push_back(cv::Point(x, m_mask.rows-1));
	}

	//left border
	for(int y=1; y<m_mask.rows; y++)
	{
		if(m_mask.at<uchar>(y, 0) == black)
			border->push_back(cv::Point(0, y));
	}

	return border;
}

void CPDCIImage::SetSourceAndSink(Graph<int,int,int>* g)
{
	//get current border of mask
	//check image borders
	//mask covers left image border
	int white = 255;
	int black = 0;
	bool isLeftMask = true;
	bool isLeftImage = true;
	for(int y=0; y<m_mask.rows; y++)
	{
		if(m_mask.at<uchar>(y, 0) < white)
			isLeftMask = false;
		
		if(m_mask.at<uchar>(y, 0) > black)
			isLeftImage = false;
	}

	//right
	bool isRightMask = true;
	bool isRightImage = true;
	for(int y=0; y<m_mask.rows; y++)
	{
		if(m_mask.at<uchar>(y, m_mask.cols-1) < white)
			isRightMask = false;
		
		if(m_mask.at<uchar>(y, m_mask.cols-1) > black)
			isRightImage = false;
	}

	//top
	bool isTopMask = true;
	bool isTopImage = true;
	for(int x=0; x<m_mask.cols; x++)
	{
		if(m_mask.at<uchar>(0, x) < white)
			isTopMask = false;
		
		if(m_mask.at<uchar>(0, x) > black)
			isTopImage = false;
	}

	//bottom
	bool isBottomMask = true;
	bool isBottomImage = true;
	for(int x=0; x<m_mask.cols; x++)
	{
		if(m_mask.at<uchar>(m_mask.rows-1, x) < white)
			isBottomMask = false;
		
		if(m_mask.at<uchar>(m_mask.rows-1, x) > black)
			isBottomImage = false;
	}

	//auswerten der bools

	if(isLeftMask && isRightImage)
	{
		//sink
		for(int i=0; i<m_mask.rows; i++)
			g->add_tweights((i)*m_mask.cols+0, 0, 65536);

		//source
		for(int i=0; i<m_mask.rows; i++)
			g->add_tweights((i)*m_mask.cols+m_mask.cols-1, 65536, 0);
	}
	else if(isLeftImage && isRightMask)
	{
		//sink
		for(int i=0; i<m_mask.rows; i++)
			g->add_tweights((i)*m_mask.cols+m_mask.cols-1, 0, 65536);

		//source
		for(int i=0; i<m_mask.rows; i++)
			g->add_tweights((i)*m_mask.cols+0, 65536, 0);
	}
	else if(isTopMask && isBottomImage)
	{
		//sink
		for(int i=0; i<m_mask.cols; i++)
			g->add_tweights(i + m_mask.cols*(0), 0, 65536);

		//source
		for(int i=0; i<m_mask.cols; i++)
			g->add_tweights(i + m_mask.cols*(m_mask.rows-1), 65536, 0);
	}
	else if(isTopImage && isBottomMask)
	{
		//sink
		for(int i=0; i<m_mask.cols; i++)
			g->add_tweights(i + m_mask.cols*(m_mask.rows-1), 0, 65536);

		//source
		for(int i=0; i<m_mask.cols; i++)
			g->add_tweights(i + m_mask.cols*(0), 65536, 0);
	}
	else
	{
		vector<cv::Point>* currentBorder = GetBorderPixelsFromMask(m_mask, 10);

		//set sink:
		//sink
		int numSinks = currentBorder->size();
		for(int i=0; i<numSinks; i++)
		{
			cv::Point currentPoint = currentBorder->at(i);
			g->add_tweights(currentPoint.x + m_mask.cols*(currentPoint.y), 0, 65536);
		}

		//source
		vector<cv::Point>* imageBorder = GetBorderPixelsFromImage();
		int sourcesLeft = numSinks;
		while(sourcesLeft && imageBorder->size())
		{
			int randNum = rand() % imageBorder->size();
			cv::Point currBorderPoint = imageBorder->at(randNum);
			g->add_tweights(currBorderPoint.x + m_mask.cols*(currBorderPoint.y), 65536, 0);
			imageBorder->erase(imageBorder->begin()+randNum);			
			sourcesLeft--;
		}

		currentBorder->clear();
		imageBorder->clear();
	}
}

cv::Mat CPDCIImage::GetBestCut(cv::Mat similarImage)
{
	static int count = 1;
	cv::Mat currMask = cvCreateMat(m_mask.rows, m_mask.cols, m_mask.type());

	int size = similarImage.cols*similarImage.rows;
	typedef Graph<int,int,int> GraphType;
	GraphType *g = new GraphType(size, 2*size);

	g->add_node(size);

	for (int y = 0; y < similarImage.rows; y++)
	{
		for (int x = 0; x < similarImage.cols; x++)
		{
			double inputGrey = GetGreyValueAt(x, y, m_inputImage);
			double similarGrey = GetGreyValueAt(x, y, similarImage);

			double inputGreyPosRight = 0.0;
			double similarGreyPosRight = 0.0;
			if(x+1 < similarImage.cols)
			{
				inputGreyPosRight = GetGreyValueAt(x+1, y, m_inputImage);
				similarGreyPosRight = GetGreyValueAt(x+1, y, similarImage);
			}

			double inputGreyPosDown = 0.0;
			double similarGreyPosDown = 0.0;
			if(y+1 < similarImage.rows)
			{
				inputGreyPosDown = GetGreyValueAt(x, y+1, m_inputImage);
				similarGreyPosDown = GetGreyValueAt(x, y+1, similarImage);
			}

			//proposal: |A(t)-B(t)| + |A(s)-B(s)| , s = curr, t = neighbor, A = image1, B = image2

			double diffInputHor = abs(inputGreyPosRight - inputGrey);
			double diffInputVer = abs(similarGreyPosDown - inputGrey);

			double diffSimilarHor = abs(similarGreyPosRight - similarGrey);
			double diffSimilarVer = abs(similarGreyPosDown - similarGrey);

			double diffHor = abs(diffInputHor - diffSimilarHor); //plus or minus??
			double diffVer = abs(diffInputVer - diffSimilarVer);

			double currWeight = GetDistWeight(x, y);
			int diffHorWeight = diffHor + currWeight;
			int diffVerWeight = diffVer + currWeight;

			int pos = y*similarImage.cols+x;

			//weights heigh, means cutting costs high -> pixels are more likely to stay as they are
			// cut will be applied where costs are low!

			if(x+1 < similarImage.cols)
				g->add_edge(pos, pos+1,                 diffHorWeight, diffHorWeight);
			
			if(y+1 < similarImage.rows)
				g->add_edge(pos, pos+similarImage.cols, diffVerWeight, diffVerWeight);
		}
	}

	//sink and source
	SetSourceAndSink(g);

	g->maxflow();

	//read out result
	for(int x = 0; x < m_mask.cols; x++)
		for(int y = 0; y < m_mask.rows; y++)
			currMask.at<uchar>(y, x) = (uchar)(g->what_segment(y*similarImage.cols + x) != GraphType::SOURCE)*255;

	delete g;

	return currMask;
}

cv::Point CPDCIImage::GetCentreOfMask()
{
	cv::Point tl(m_mask.size()), br(-1,-1);
	
	for(int y=0; y<m_mask.rows; ++y)
	{
		uchar *p = m_mask.ptr(y);
		for(int x=0; x<m_mask.cols; ++x,++p)
		{
			if(*p==0) continue;
			if(tl.x>x) tl.x=x;
			if(tl.y>y) tl.y=y;
			if(br.x<x) br.x=x;
			if(br.y<y) br.y=y;
		}
	}
	br.x += 1;
	br.y += 1;

	double posX = (br.x + tl.x) / 2.0;
	double posY = (br.y + tl.y) / 2.0;
	cv::Point retVal = cvPoint((int)posX, (int)posY);

	return retVal;
}

//returns 0 if file could not be read, 1 otherwise
bool CPDCIImage::ReadInFile(std::string filename, std::vector<char>* buffer)
{
	buffer->clear();

	std::ifstream file;
	file.open(filename, std::ios::in | ios::binary);

	if(!file.fail())
	{ 
		while(!file.eof())
		{
			char currChar;
			file.get(currChar);
			buffer->push_back(currChar);
		}
		file.close();
    } 
	else
       return false;

	return true;
}

//returns 0 if file could not be read, 1 otherwise
bool CPDCIImage::ReadInFileAsLines(std::string filename, std::vector<char*>* buffer)
{
	buffer->clear();

	std::ifstream file;
	file.open(filename, std::ios::in | ios::binary);

	if(!file.fail())
	{ 
		while(!file.eof())
		{
			char* currLine = new char[10000];
			file.getline(currLine, 10000);
			buffer->push_back(currLine);
		}
		file.close();
    } 
	else
       return false;

	return true;
}

//returns 0 if file could not be read, 1 otherwise
bool CPDCIImage::ReadInFileNumBytes(std::ifstream* currentFile, char* buffer, int num_bytes)
{
	if(!currentFile->fail())
	{
		//file->get(buffer, num_bytes);
		for (int i = 0; i < num_bytes; i++) { *buffer = currentFile->get(); buffer++; }
    } 
	else
       return false;

	return true;
}

bool CPDCIImage::OpenDescriptorFiles()
{
	std::string fileListName = "huge_filelist";
	std::string gistlistfeature_meanName = "huge_gistfeatures_mean";
	std::string gistlistfeature_varianceName = "huge_gistfeatures_variance";
	//std::string gistlistparameterName = "retrieval_framework_2012\\gistlistparameter";

	m_meanFileHandle.open(gistlistfeature_meanName, std::ios::in | ios::binary);
	m_varianceFileHandle.open(gistlistfeature_varianceName, std::ios::in | ios::binary);
	m_fileListHandle.open(fileListName, std::ios::in | ios::binary);

	if(m_meanFileHandle.fail() || m_varianceFileHandle.fail() || m_fileListHandle.fail())
		return false;
	
	m_fileListBuffer = new char[65536];
	m_meanFileBuffer = new char[65536];
	m_varianceFileBuffer = new char[65536];
	
	m_fileListHandle.rdbuf()->pubsetbuf(m_fileListBuffer, 65536);
	m_meanFileHandle.rdbuf()->pubsetbuf(m_meanFileBuffer, 65536);
	m_varianceFileHandle.rdbuf()->pubsetbuf(m_varianceFileBuffer, 65536);

	return true;
}

float CPDCIImage::ReadFloatFromFile(std::ifstream* fileHandle)
{
	char buffer[4];
	ReadInFileNumBytes(fileHandle, buffer, 4);

	if ((int*) buffer == 0) return 0.0;
	else return *(float*)buffer;
}

int CPDCIImage::ReadIntFromFile(std::ifstream* fileHandle)
{
	char buffer[4];
	ReadInFileNumBytes(fileHandle, buffer, 4);

	return *(unsigned int*)buffer;
}

long CPDCIImage::ReadLongFromFile(std::ifstream* fileHandle)
{
	char buffer[8];
	ReadInFileNumBytes(fileHandle, buffer, 8);

	return *(unsigned long*)buffer;
}

