#ifndef DESCRIPTORS__GIST_HPP
#define DESCRIPTORS__GIST_HPP

#include <string>
#include <complex>

#include <boost/function.hpp>

#include "../types.hpp"
#include "../generator.hpp"

namespace imdb
{

class gist_generator : public GeneratorWithCopyClone<gist_generator>
{
    typedef float                 float_t;
    typedef std::complex<float_t> complex_t;

    public:

    gist_generator(const ptree& params);

    void compute(anymap_t& data);

    private:

    void init_filter();

    const size_t _padding;

    const size_t _realwidth;
    const size_t _realheight;

    const size_t _num_x_tiles;
    const size_t _num_y_tiles;

    const size_t _num_freqs;
    const size_t _num_orients;

    const double _max_peak_freq;
    const double _delta_freq_oct;
    const double _bandwidth_oct;
    const double _angle_factor;

    const bool   _polar;

    const std::string _prefilter_str;

    const size_t _width;
    const size_t _height;

    boost::function<void (cv::Mat&)> _prefilter_ocv;
    std::vector<cv::Mat_<complex_t> > _filters;
};

} // namespace imdb

#endif // DESCRIPTORS__GIST_HPP
