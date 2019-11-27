#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>
#include <fstream>
#include <stdexcept>

#include <boost/any.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>

#include "types.hpp"
#include "io.hpp"

#include <iostream>

namespace imdb {

// Base classes Property, PropertyReader, PropertyWriter //////////////////////

template <class T>
struct PropertyReaderT
{
    // construction / destruction
    virtual ~PropertyReaderT() {}

    // accessors
    virtual void get(T&, index_t) const = 0;

    T operator[] (index_t index) const
    {
        T r;
        get(r, index);
        return r;
    }

    // properties
    // TODO: shouldn't this be a size_t i.e. an unsigned type?
    virtual index_t size() const = 0;

    // iterators
    class const_iterator : public boost::iterator_facade<const_iterator, T const, std::random_access_iterator_tag>
    {
        public:

        const_iterator() : _reader(0), _index(0), _isvalid(false) {}

        private:

        const_iterator(const PropertyReaderT& reader, index_t index)
            : _reader(&reader)
            , _index(index)
            , _isvalid(false)
        {}

        friend class boost::iterator_core_access;
        friend class PropertyReaderT;

        void increment() { _index++; _isvalid = false; }
        void decrement() { _index--; _isvalid = false; }
        void advance(index_t n) { _index += n; _isvalid = false; }

        index_t distance_to(const const_iterator& other) const
        {
            return other._index - _index;
        }

        bool equal(const const_iterator& other) const
        {
            return (_reader == other._reader && _index == other._index);
        }

        const T& dereference() const
        {
            assert(_reader != 0);

            if (!_isvalid)
            {
                _reader->get(_current, _index);
                _isvalid = true;
            }

            return _current;
        }

        const PropertyReaderT* _reader;
        index_t                _index;
        mutable T              _current;
        mutable bool           _isvalid;
    };

    const_iterator begin() { return const_iterator(*this, 0); }
    const_iterator end() { return const_iterator(*this, this->size()); }

    virtual const strmap_t& map() const = 0;

};

template <class T, class S = T>
class PropertyReaderElementProxy : public PropertyReaderT<T>
{
    public:

    typedef boost::shared_ptr<PropertyReaderT<S> > reader_sp;
    typedef boost::function<T (const S&)>         proxy_fn;

    PropertyReaderElementProxy(reader_sp reader, proxy_fn proxyfn)
        : _reader(reader), _proxyfn(proxyfn)
    {}

    void get(T& v, index_t index) const
    {
        S s;
        _reader->get(s, index);
        v = _proxyfn(s);
    }

    index_t size() const
    {
        return _reader->size();
    }

    private:

    reader_sp _reader;
    proxy_fn  _proxyfn;
};

struct PropertyWriter
{
    virtual ~PropertyWriter() {}
    virtual bool push_back(const boost::any&) = 0;
    virtual bool insert(const boost::any& element, size_t pos) = 0;
};

struct Property
{
    virtual boost::shared_ptr<PropertyWriter> create_writer(const std::string& filename) const = 0;
    virtual const std::string& name() const = 0;
};

// Default Property implementation (includes Reader and Writer) ///////////////

template <class T>
class PropertyT : public Property
{
    public:

    PropertyT() : _name() {}
    PropertyT(const std::string& name) : _name(name) {}

    class writer : public PropertyWriter, boost::noncopyable
    {
        public:

        writer(const std::string& filename)
         : _ofs(filename.c_str(), std::ofstream::binary|std::ofstream::trunc)
        {
            if (!_ofs.is_open()) throw std::runtime_error("could not open file " + filename);

            _map["__version"] = boost::lexical_cast<std::string>(PropertyT::version());
        }

        ~writer()
        {
            int64_t p_features = 0;
            _map["__features"] = boost::lexical_cast<std::string>(p_features);

            int64_t p_offsets = _ofs.tellp();
            _map["__offsets"] = boost::lexical_cast<std::string>(p_offsets);
            io::write(_ofs, _offset, _io);

            int64_t p_map = _ofs.tellp();
            io::write(_ofs, _map, _io);
            io::write(_ofs, p_map, _io);

            _ofs.close();
        }

        bool push_back(const boost::any& element)
        {
            _offset.push_back(_ofs.tellp());
            io::write(_ofs, boost::any_cast<T>(element), _io);
            return true;
        }

        bool insert(const boost::any& element, size_t pos)
        {
            if (_offset.size() <= pos) _offset.resize(pos + 1, -1);
            _offset[pos] = _ofs.tellp();
            io::write(_ofs, boost::any_cast<T>(element), _io);
            return true;
        }

        bool insert_map_entry(const std::string& key, const std::string& value)
        {
            return _map.insert(typename strmap_t::value_type(key, value)).second;
        }

        private:

        std::ofstream        _ofs;
        std::vector<int64_t> _offset;
        strmap_t             _map;
        BinaryIO             _io;
    };

    boost::shared_ptr<PropertyWriter> create_writer(const std::string& filename) const
    {
        return boost::shared_ptr<PropertyWriter>(new writer(filename));
    }

    class reader : public PropertyReaderT<T>, boost::noncopyable
    {
        public:

        reader(const std::string& filename)
         : _ifs(filename.c_str(), std::ifstream::binary)
         , _offset(new std::vector<int64_t>())
         , _map(new strmap_t())
        {
            if (!_ifs.is_open()) throw std::runtime_error("could not open file " + filename);

            _ifs.seekg(-static_cast<int>(sizeof(int64_t)), std::ios::end);
            int64_t p_map;
            io::read(_ifs, p_map, _io);

            _ifs.seekg(p_map);
            if (!_ifs.good()) throw std::runtime_error("error while reading file " + filename);
            io::read(_ifs, *_map, _io);

            if (!_map->count("__features") || !_map->count("__offsets") || !_map->count("__version"))
            {
                throw std::runtime_error("error while reading map in file " + filename);
            }

            int64_t p_features = boost::lexical_cast<int64_t>((*_map)["__features"]);
            int64_t p_offsets  = boost::lexical_cast<int64_t>((*_map)["__offsets"]);
            int     p_version  = boost::lexical_cast<int>((*_map)["__version"]);

            if (p_version > PropertyT::version())
            {
                throw std::runtime_error("version of file " + filename + " is higher than program version");
            }

            _ifs.seekg(p_offsets);
            if (!_ifs.good()) throw std::runtime_error("error while reading file " + filename);
            io::read(_ifs, *_offset, _io);

            if (!_ifs.good()) throw std::runtime_error("error while reading file " + filename);

            _p_features = p_features;
        }

        void get(T& r, index_t index) const
        {
            int64_t p = _p_features + (*_offset)[index];
            if (p != _ifs.tellg()) _ifs.seekg(p);
            io::read(_ifs, r, _io);
        }

        index_t size() const
        {
            return _offset->size();
        }

        const strmap_t& map() const
        {
            return *_map;
        }

        private:

        mutable std::ifstream                    _ifs;
        boost::shared_ptr<std::vector<int64_t> > _offset;
        boost::shared_ptr<strmap_t>              _map;
        BinaryIO                                 _io;
        int64_t                                  _p_features;
    };

    boost::shared_ptr<PropertyReaderT<T> > create_reader(const std::string& filename)
    {
        return boost::shared_ptr<PropertyReaderT<T> >(new reader(filename));
    }

    void load(std::vector<T>& v, const std::string& filename) const
    {
        reader rd(filename);
        v.resize(rd.size());
        for (index_t i = 0; i < rd.size(); i++) rd.get(v[i], i);
    }

    void store(const std::vector<T>& v, const std::string& filename) const
    {
        writer wr(filename);
        for (index_t i = 0; i < v.size(); i++) wr.push_back(v[i]);
    }

    const std::string& name() const
    {
        return _name;
    }

    static int version()
    {
        return 1;
    }

    private:

    std::string _name;
};

class Properties
{
    public:

    typedef std::vector<boost::shared_ptr<Property> > properties_t;

    template <class T>
    Properties& add(const std::string& name)
    {
        _properties.push_back(boost::shared_ptr<Property>(new PropertyT<T>(name)));
        return *this;
    }

    const properties_t& get() const
    {
        return _properties;
    }

    private:

    properties_t _properties;
};

} // namespace imdb

#endif // PROPERTY_HPP
