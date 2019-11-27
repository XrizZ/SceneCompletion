#ifndef DESCRIPTORS__TINYLAB_H
#define DESCRIPTORS__TINYLAB_H

#include <cstdlib>

#include "../types.hpp"
#include "../generator.hpp"

/**
  * One of the most simple image descripors possible,
  * this one only store a downscaled version of the
  * original image it represents.
  * The data is stored as pixels in L*a*b colorspace
  * since Euclidean distances in that colorspace
  * match human perception of color differences quite well.
  */

namespace imdb {

class tinylab_generator : public GeneratorWithCopyClone<tinylab_generator>
{
    public:

    tinylab_generator(const ptree& params);

    void compute(anymap_t& data);

    private:

    const std::size_t _width;
    const std::size_t _height;
    const std::string _colorspace;
};

} // namespace imdb

#endif // DESCRIPTORS__TINYLAB_H
