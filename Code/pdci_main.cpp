//============================================================================
// Name        : pdci_main.cpp
// Author      : Christian Thurow
// Description : Main class for the Scene completion algorithm
//============================================================================

#include "stdafx.h"
#include "highgui.h"
#include "PDCIImage.h"
#include <iostream>

using namespace std;

// usage: path to image in argv[1]
// usage: path to mask in argv[2]
// this is the start function, it calls all necessary sub functions
int main(int argc, char** argv)
{
	cout << "|===================START===================|" <<endl;

	//init time measurement
	DWORD start = GetTickCount();

	//create container for the image
	CPDCIImage* imageData = new CPDCIImage();

	cout << "| Loading and Preprocessing Input" << endl;

	//load the input
	imageData->LoadImageFromFile(argv[1]);
	imageData->LoadMaskFromFile(argv[2]);

	imageData->CalcGISTofInput();

	cout << "|===========================================|" <<endl;
	double deltaInS = (GetTickCount() - start) / 1000.0;
	cout << "| " << deltaInS << " seconds passed since program start" << endl;
	cout << "|===========================================|" <<endl;

	cout << "| Search for Similar Images in DB" << endl;

	//find similar images in large db
	imageData->FindSimilarImagesFromLargeDB(); //this is for the large image db
	//imageData->FindSimilarImagesFromTinyDB();

	cout << "|===========================================|" <<endl;
	deltaInS = (GetTickCount() - start) / 1000.0;
	cout << "| " << deltaInS << " seconds passed since program start" << endl;
	cout << "|===========================================|" <<endl;

	//debug: show the similar images found
	//imageData->PrintSimilarImages();

	cout << "| Calc Best Cuts" << endl;

	//test all results with input image and mask:
	imageData->GetBestCuts();

	cout << "|===========================================|" <<endl;
	deltaInS = (GetTickCount() - start) / 1000.0;
	cout << "| " << deltaInS << " seconds passed since program start" << endl;
	cout << "|===========================================|" <<endl;

	//imageData->ShowMasks();
	imageData->SaveMasks();
	//cvWaitKey(0);

	cout << "| Poisson Blend Images" << endl;

	//poison blend images
	imageData->Blend();

	cout << "|===========================================|" <<endl;
	deltaInS = (GetTickCount() - start) / 1000.0;
	cout << "| " << deltaInS << " seconds passed since program start" << endl;
	cout << "|===================END=====================|" <<endl;

	//debug: show output images on screen
	imageData->ShowResults();

	//save images to disk
	imageData->SaveResults();

	//cvWaitKey(0);		//DEBUG for batch processing

	//cleanup data to avoid memory leaks
	imageData->Cleanup();
}
