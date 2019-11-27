#include "utilities.hpp"

#include <numeric>
#include <opencv2/imgproc/imgproc.hpp>

namespace imdb {

void filterEmptyFeatures(const vec_vec_f32_t& features, const vector<cv::Point2f>& keypoints, const cv::Size& imageSize, vec_vec_f32_t& featuresFiltered, vec_vec_f32_t& keypointsFiltered)
{
    for (size_t i = 0; i < features.size(); i++) {
        double sum = std::accumulate(features[i].begin(), features[i].end(), 0.0f);
        if (sum > 0)
        {
            featuresFiltered.push_back(features[i]);

            std::vector<float> p(2);
            // the position is stored in relative coordinates of the image size
            p[0] = static_cast<float>(keypoints[i].x) / imageSize.width;
            p[1] = static_cast<float>(keypoints[i].y) / imageSize.height;
            keypointsFiltered.push_back(p);
        }
    }
}


double scaleToSideLength(const cv::Mat& image, int maxSideLength, cv::Mat& scaled)
{
    double scaling_factor = (image.size().width > image.size().height)
            ? static_cast<double>(maxSideLength) / image.size().width
            : static_cast<double>(maxSideLength) / image.size().height;

    // as of OpenCV version 2.2 we need to user INTER_AREA for downscaling as all
    // other options lead to severe aliasing!
    cv::resize(image, scaled, cv::Size(0, 0), scaling_factor, scaling_factor, cv::INTER_AREA);
    return scaling_factor;
}


}
