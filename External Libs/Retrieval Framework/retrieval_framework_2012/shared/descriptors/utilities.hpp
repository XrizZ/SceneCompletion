#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "../types.hpp"

namespace imdb {

// Removes all empty features, i.e. those that only contains zeros
void filterEmptyFeatures(const vec_vec_f32_t& features, const vector<cv::Point2f>& keypoints, const cv::Size& imageSize, vec_vec_f32_t& featuresFiltered, vec_vec_f32_t& keypointsFiltered);

// Uniformly scale the image such that the longer of its sides is
// scaled to exactly maxSideLength, the other one <= maxSideLength
double scaleToSideLength(const cv::Mat& image, int maxSideLength, cv::Mat& scaled);

} // namespace imdb

#endif // UTILITIES_HPP
