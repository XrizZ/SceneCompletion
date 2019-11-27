#include <QImage>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../types.hpp"

#include "tinylab.hpp"

namespace imdb {

// --------------------------------------------------------------------
// Note: Mathias has modified this descriptor, making the colorspace of
// the resulting tiny image a parameter.
// So this descriptor should now better be rename to tinyimage
// --------------------------------------------------------------------

tinylab_generator::tinylab_generator(const ptree& params)
 : GeneratorWithCopyClone<tinylab_generator>(params, Properties().add<vec_f32_t>("features")),
 _width (parse<size_t>(_parameters, "params.width" ,     16)), //width of thumbnail
 _height(parse<size_t>(_parameters, "params.height",     16)),  // height of thumbnail
 _colorspace(parse<string>  (_parameters, "params.colorspace", "lab"))
{}

void tinylab_generator::compute(anymap_t& data)
{

    using namespace std;
    using namespace cv;

    // ------------------------------------------------------------------------
    // Required input:
    //
    // this generator expects the image to be a CV_8UC3 with BGR channel order.
    // ------------------------------------------------------------------------

    Mat img = get<mat_8uc3_t>(data, "image");

    // resize to thumbnail size
    Mat imgScaled;
    resize(img, imgScaled, Size(_width, _height), 0, 0, INTER_AREA);


    imgScaled.convertTo(imgScaled, CV_32FC3, 1.0/255.0);

    vec_f32_t features;
    if (_colorspace == "grey") features.resize(_width * _height);
    else                       features.resize(_width * _height * 3);

    // convert to user-selected colorspace
    cv::Mat imageColorspace;
    if (_colorspace == "lab")       cv::cvtColor(imgScaled, imageColorspace, CV_RGB2Lab);
    else if (_colorspace == "grey") cv::cvtColor(imgScaled, imageColorspace, CV_RGB2GRAY);
    else if (_colorspace == "rgb")  imageColorspace = imgScaled;


    // copy image data to output feature
    if (_colorspace == "grey")
    {
        vec_f32_t::iterator di = features.begin();
        for (cv::MatConstIterator_<float> si = imgScaled.begin<float>(); si != imgScaled.end<float>(); ++si)
        {
            *di = (*si);
            ++di;
        }
    }
    else
    {
        vec_f32_t::iterator di = features.begin();
        for (cv::MatConstIterator_<cv::Vec3f> si = imgScaled.begin<cv::Vec3f>(); si != imgScaled.end<cv::Vec3f>(); ++si)
        {
            *di = (*si)[0]; ++di;
            *di = (*si)[1]; ++di;
            *di = (*si)[2]; ++di;
        }
    }

    data["features"] = features;
}

bool tinylab_registered = Generator::register_generator<tinylab_generator>("tinylab");

} // namespace imdb
