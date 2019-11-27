#ifndef IO_HPP
#define IO_HPP

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <set>

#include <boost/cstdint.hpp>
#include <boost/array.hpp>

namespace imdb {

struct IO
{
    virtual size_t write(std::ostream& os, int8_t v) const = 0;
    virtual size_t write(std::ostream& os, int16_t v) const = 0;
    virtual size_t write(std::ostream& os, int32_t v) const = 0;
    virtual size_t write(std::ostream& os, int64_t v) const = 0;
    virtual size_t write(std::ostream& os, uint8_t v) const = 0;
    virtual size_t write(std::ostream& os, uint16_t v) const = 0;
    virtual size_t write(std::ostream& os, uint32_t v) const = 0;
    virtual size_t write(std::ostream& os, uint64_t v) const = 0;
    virtual size_t write(std::ostream& os, float v) const = 0;
    virtual size_t write(std::ostream& os, double v) const = 0;
    virtual size_t write(std::ostream& os, const std::string& v) const = 0;

    virtual size_t read(std::istream& is, int8_t& v) const = 0;
    virtual size_t read(std::istream& is, int16_t& v) const = 0;
    virtual size_t read(std::istream& is, int32_t& v) const = 0;
    virtual size_t read(std::istream& is, int64_t& v) const = 0;
    virtual size_t read(std::istream& is, uint8_t& v) const = 0;
    virtual size_t read(std::istream& is, uint16_t& v) const = 0;
    virtual size_t read(std::istream& is, uint32_t& v) const = 0;
    virtual size_t read(std::istream& is, uint64_t& v) const = 0;
    virtual size_t read(std::istream& is, float& v) const = 0;
    virtual size_t read(std::istream& is, double& v) const = 0;
    virtual size_t read(std::istream& is, std::string& v) const = 0;

    virtual size_t read_floats(std::istream& is, float* v, size_t n) const = 0;
};

class BinaryIO : public IO
{
    public:

    virtual size_t write(std::ostream& os, int8_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, int16_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, int32_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, int64_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, uint8_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, uint16_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, uint32_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, uint64_t v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, float v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, double v) const { return _write(os, v); }
    virtual size_t write(std::ostream& os, const std::string& v) const
    {
        size_t t = 0;
        t += write(os, static_cast<int32_t>(v.length()));
        for (size_t i = 0; i < v.length(); i++) t += write(os, reinterpret_cast<const int8_t&>(v[i]));
        return t;
    }

    virtual size_t read(std::istream& is, int8_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, int16_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, int32_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, int64_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, uint8_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, uint16_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, uint32_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, uint64_t& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, float& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, double& v) const { return _read(is, v); }
    virtual size_t read(std::istream& is, std::string& v) const
    {
        int32_t s;
        size_t t = 0;
        t += read(is, s);
        v.resize(s);
        for (size_t i = 0; i < v.length(); i++) t += read(is, reinterpret_cast<int8_t&>(v[i]));
        return t;
    }

    virtual size_t read_floats(std::istream& is, float* v, size_t n) const
    {
        is.read(reinterpret_cast<char*>(v), n*sizeof(float));
        return n*sizeof(float);
    }

    private:

    template <class T>
    size_t _write(std::ostream& os, T v) const
    {
        os.write(reinterpret_cast<char*>(&v), sizeof(v));
        return sizeof(v);
    }

    template <class T>
    size_t _read(std::istream& is, T& v) const
    {
        is.read(reinterpret_cast<char*>(&v), sizeof(v));
        return sizeof(v);
    }
};

namespace io
{
    // ----------------------------------------------------------------------------
    // Signatures -- we need them to allow arbitrary nestings in the following
    // implementation part. Otherwise the implementation for e.g. std::pair<T1,T2>
    // would need to be defined before the one of std::vector<T> when trying to
    // read/write a vector<pair<T1,T2>>
    // ----------------------------------------------------------------------------

    template <class T>
    size_t write(std::ostream& os, const T& v, const IO& io);

    template <class T>
    size_t read(std::istream& is, T& v, const IO& io);

    template <class T, std::size_t N>
    size_t write(std::ostream& os, const boost::array<T, N>& v, const IO& io);

    template <class T, std::size_t N>
    size_t read(std::istream& is, boost::array<T, N>& v, const IO& io);

    template <class T1, class T2>
    size_t write(std::ostream& os, const std::pair<T1, T2>& v, const IO& io);

    template <class T1, class T2>
    size_t read(std::istream& is, std::pair<T1, T2>& v, const IO& io);

    template <class T>
    size_t write(std::ostream& os, const std::vector<T>& v, const IO& io);

    template <class T>
    size_t read(std::istream& is, std::vector<T>& v, const IO& io);

    template <class T1, class T2>
    size_t write(std::ostream& os, const std::map<T1, T2>& v, const IO& io);

    template <class T1, class T2>
    size_t read(std::istream& is, std::map<T1, T2>& v, const IO& io);

    template <class T>
    size_t write(std::ostream& os, const std::set<T>& v, const IO& io);

    template <class T>
    size_t read(std::istream& is, std::set<T>& v, const IO& io);


    // ----------------------------------------------------------------------------
    // Implementations
    // ----------------------------------------------------------------------------

    template <class T>
    size_t write(std::ostream& os, const T& v, const IO& io)
    {
        return io.write(os, v);
    }

    template <class T>
    size_t read(std::istream& is, T& v, const IO& io)
    {
        return io.read(is, v);
    }

    template <class T, std::size_t N>
    size_t write(std::ostream& os, const boost::array<T, N>& v, const IO& io)
    {
        size_t t = 0;
        for (size_t i = 0; i < N; i++) t += write(os, v[i], io);
        return t;
    }

    template <class T, std::size_t N>
    size_t read(std::istream& is, boost::array<T, N>& v, const IO& io)
    {
        size_t t = 0;
        for (size_t i = 0; i < N; i++) t += read(is, v[i], io);
        return t;
    }

    template <class T>
    size_t write(std::ostream& os, const std::vector<T>& v, const IO& io)
    {
        size_t t = write(os, static_cast<int64_t>(v.size()), io);
        for (size_t i = 0; i < v.size(); i++) t += write(os, v[i], io);
        return t;
    }


    // Specialize variant of the function below which read in a
    // complete vector<float> with a single read. This gives us
    // about 4x performance for the descriptor vectors we
    // typically use.
    // Note that gcc requires (i686-apple-darwin11-llvm-gcc-4.2)
    // that this function comes before the more general to actually
    // invoke it when we read a vector<float>
    inline
    size_t read(std::istream& is, std::vector<float>& v, const IO& io)
    {
        int64_t size = 0;
        size_t t = read(is, size, io);
        v.resize(size);
        t += io.read_floats(is, &v[0], size);
        return t;
    }

    template <class T>
    size_t read(std::istream& is, std::vector<T>& v, const IO& io)
    {
        int64_t size = 0;
        size_t t = read(is, size, io);
        v.resize(size);
        for (int64_t i = 0; i < size; i++) t += read(is, v[i], io);
        return t;
    }

    template <class T1, class T2>
    size_t write(std::ostream& os, const std::pair<T1, T2>& v, const IO& io)
    {
        size_t s = 0;
        s += write(os, v.first, io);
        s += write(os, v.second, io);
        return s;
    }

    template <class T1, class T2>
    size_t read(std::istream& is, std::pair<T1, T2>& v, const IO& io)
    {
        size_t s = 0;
        s += read(is, v.first, io);
        s += read(is, v.second, io);
        return s;
    }


    template <class T>
    size_t write(std::ostream& os, const std::set<T>& v, const IO& io)
    {
        size_t s = 0;
        s += write(os, static_cast<int64_t>(v.size()), io);
        for (typename std::set<T>::const_iterator it = v.begin(); it != v.end(); ++it)
        {
            s += write(os, *it, io);
        }
        return s;
    }

    template <class T>
    size_t read(std::istream& is, std::set<T>& v, const IO& io)
    {
        size_t s = 0;
        v.clear();
        int64_t size = 0;
        s += read(is, size, io);
        for (int64_t i = 0; i < size; i++)
        {
            T x;
            s += read(is, x, io);
            v.insert(x);
        }
        return s;
    }


    template <class T1, class T2>
    size_t write(std::ostream& os, const std::map<T1, T2>& v, const IO& io)
    {
        size_t s = 0;
        s += write(os, static_cast<int64_t>(v.size()), io);
        for (typename std::map<T1, T2>::const_iterator it = v.begin(); it != v.end(); ++it)
        {
            s += write(os, *it, io);
        }
        return s;
    }

    template <class T1, class T2>
    size_t read(std::istream& is, std::map<T1, T2>& v, const IO& io)
    {
        size_t s = 0;
        v.clear();
        int64_t size = 0;
        s += read(is, size, io);
        for (int64_t i = 0; i < size; i++)
        {
            std::pair<T1, T2> x;
            s += read(is, x, io);
            v.insert(x);
        }
        return s;
    }
}

} // namespace imdb

#endif // IO_HPP
