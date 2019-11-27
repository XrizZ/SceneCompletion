#ifndef IMAGEFILES_H
#define IMAGEFILES_H

#include <string>
#include <vector>

#include <boost/function.hpp>

#include "property.hpp"

namespace imdb {

    class ImageFiles
    {
    public:

        enum FileType { Images, SVG, Off, Obj, XForm };

        typedef PropertyT<std::string>         property_t;
        typedef boost::function<void (size_t)> callback_fn;

        ImageFiles(const std::string& image_dir = ".");

        const std::string& image_dir() const;

        void set_image_dir(const std::string& image_dir);
        const std::vector<std::string>& files() const;

        size_t size() const;

        std::string get_filename(size_t index) const;

        std::string get_relative_filename(size_t index) const;

        void lookup_dir(callback_fn callback = callback_fn(), FileType = Images);

        void load(const std::string& filename);

        void store(const std::string& filename) const;

        void random_sample(size_t new_size, size_t seed);

        const filenames_t& filenames() const;

    private:

        std::string              _image_dir;
        filenames_t              _files;
    };
} // end namespace

#endif // IMAGEFILES_H
