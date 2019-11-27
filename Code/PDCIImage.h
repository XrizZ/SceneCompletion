//============================================================================
// Name        : PDCIImage.h
// Author      : Christian Thurow
// Description : Defines the CPDCIImage class
//============================================================================

#include "cv.h"
#include "graphcut\graph.h"

#define NUM_X_TILES 4
#define NUM_Y_TILES 4
#define NUM_FREQS 4
#define NUM_ORIENTS 6
#define PADDING	64
#define WIDTH 256
#define HEIGHT 256
#define MAX_PEAK_FREQ 0.3
#define DELTA_FREQ_OCT 0.88752527 // frequency step size in octaves
#define BANDWIDTH_OCT DELTA_FREQ_OCT // bandwidth in octaves
#define ANGLE_FACTOR 1.0     // circular width factor
#define POLAR true           // use polar gabor filter construction
#define PREFILTER "none"	 // use prefilter (none, torralba)

#define M_PI 3.1415926535897932384626433832795

struct GistDescriptor
{
	float m_mean[NUM_FREQS][NUM_ORIENTS][NUM_Y_TILES][NUM_X_TILES];
	float m_variance[NUM_FREQS][NUM_ORIENTS][NUM_Y_TILES][NUM_X_TILES];
	std::string m_fileName;
	double m_dissimilarity;

	GistDescriptor()
	{
		m_dissimilarity = -1.0;
		m_fileName = "";
	};
};

class CPDCIImage
{
private:
	cv::Mat m_inputImage;
	cv::Mat m_mask;
	int m_maxNumSimilarImages;
	vector<cv::Mat> m_similarImages;
	vector<cv::Mat> m_similarImagesMasks;
	vector<cv::Mat> m_outputImages;
	std::vector<GistDescriptor*> m_GIST; //list of gist descriptors below a certain similarity boundary
	GistDescriptor* m_inputGIST;
	vector<CvPoint>* m_listOfBorderPoints;
	double m_maskOverlap[NUM_Y_TILES][NUM_X_TILES];

public: 
	CPDCIImage()
	{
		m_inputImage = NULL;
		m_mask = NULL;
		m_maxNumSimilarImages = 15;
		m_similarImages.clear();
		m_similarImagesMasks.clear();
		m_outputImages.clear();
		m_GIST.clear();
		m_inputGIST = NULL;
	};

	~CPDCIImage();

	void Blend();
	void CalcGISTofInput();
	double CalcSimilarity(GistDescriptor* descrA, GistDescriptor* descrB);
	bool Cleanup();
	cv::Mat GetBestCut(cv::Mat similarImage);
	void GetBestCuts();
	vector<cv::Point>* GetBorderPixelsFromMask(cv::Mat_<uchar> mask, int preErodeNum);
	vector<cv::Point>* GetBorderPixelsFromImage();
	cv::Point GetCentreOfMask();
	double GetDistance(int x, int y);
	double GetDistWeight(int x, int y);
	bool LoadImageFromFile(char* path);
	bool LoadMaskFromFile(char* path);
	bool OpenDescriptorFiles();
	void FillGapsInMasks();
	void FillGapsInMask(cv::Mat* mask);
	void FindSimilarImagesFromLargeDB();
	void FindSimilarImagesFromTinyDB();
	void InitMaskBorder();
	void InitMaskWeights();
	void InsertElem(GistDescriptor* currGist);
	void PrintSimilarImages();
	int ReadIntFromFile(std::ifstream* fileHandle);
	float ReadFloatFromFile(std::ifstream* fileHandle);
	long ReadLongFromFile(std::ifstream* fileHandle);
	GistDescriptor* ReadNextGistDescriptor();
	bool ReadInFile(std::string filename, std::vector<char>* buffer);
	bool ReadInFileAsLines(std::string filename, std::vector<char*>* buffer);
	bool ReadInFileNumBytes(std::ifstream* file, char* buffer, int num_bytes);
	void CloseFileHandles();
	void SaveMasks();
	void SaveResults();
	void SetSourceAndSink(Graph<int,int,int>* g);
	void ShowMasks();
	void ShowResults();
};
