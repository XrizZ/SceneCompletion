#include "imagefiles.h"

#include <QDir>
#include <QDirIterator>
#include <boost/random.hpp>


namespace imdb {

    ImageFiles::ImageFiles(const std::string& image_dir) : _image_dir(image_dir) {}

    const std::string& ImageFiles::image_dir() const
    {
        return _image_dir;
    }

    void ImageFiles::set_image_dir(const std::string& image_dir)
    {
        // TODO: check that folder actually exists
        _image_dir = image_dir;
    }

    const std::vector<std::string>& ImageFiles::files() const
    {
        return _files;
    }

    size_t ImageFiles::size() const
    {
        return _files.size();
    }

    std::string ImageFiles::get_filename(size_t index) const
    {
        if (index >= _files.size()) throw std::range_error("image file index out of range");
        return _image_dir + QDir::separator().toAscii() + _files[index];
    }

    std::string ImageFiles::get_relative_filename(size_t index) const
    {
        if (index >= _files.size()) throw std::range_error("image file index out of range");
        return _files[index];
    }


    const filenames_t& ImageFiles::filenames() const
    {
        return _files;
    }

    void ImageFiles::lookup_dir(callback_fn callback, FileType fileType)
    {        
        QStringList nameFilters;
        switch(fileType)
        {
        case Images: nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp"; break;
        case Off: nameFilters << "*.off"; break;
        case Obj: nameFilters << "*.obj"; break;
        case XForm: nameFilters << "*.xf"; break;
        case SVG: nameFilters << "*.svg"; break;
        }

        std::vector<std::string> files;

        QDir root(_image_dir.c_str());
        QDirIterator it(_image_dir.c_str(),
                        nameFilters,
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);

        for (size_t i = 0; it.hasNext(); i++)
        {
            files.push_back(root.relativeFilePath(it.next()).toStdString());
            if (callback) callback(i);
        }

        _files = files;
    }

    void ImageFiles::load(const std::string& filename)
    {
        std::vector<std::string> files;

        property_t().load(files, filename);

        _files = files;
    }

    void ImageFiles::store(const std::string& filename) const
    {
        boost::shared_ptr<PropertyWriter> writer = property_t().create_writer(filename);
        for (size_t i = 0; i < _files.size(); i++) writer->push_back(_files[i]);
    }

    void ImageFiles::random_sample(size_t new_size, size_t seed)
    {
        if (new_size >= _files.size()) return;

        std::vector<size_t> indices(_files.size());
        for (size_t i = 0; i < indices.size(); i++) indices[i] = i;

        typedef boost::mt19937 rng_t;
        typedef boost::uniform_int<size_t> uniform_t;
        rng_t rng(seed);
        boost::variate_generator<rng_t&, uniform_t> random(rng, uniform_t(0, indices.size() - 1));

        std::random_shuffle(indices.begin(), indices.end(), random);
        indices.resize(new_size);
        std::sort(indices.begin(), indices.end());

        std::vector<std::string> new_files(new_size);
        for (size_t i = 0; i < new_files.size(); i++) new_files[i] = _files[indices[i]];
        _files = new_files;
    }
}

