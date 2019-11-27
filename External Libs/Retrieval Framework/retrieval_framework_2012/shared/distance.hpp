#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include <cmath>
#include <limits>
#include <iterator>

#include <boost/function.hpp>

#include "types.hpp"
#include "registry.hpp"


// ----------------------------------------------------------------
// Note: only distance metrics allowed here, i.e. functions that
// describe the distance between two descriptors and thus smaller
// distances denote more similar objects. The dotproduct itself
// e.g. is NOT a distance measure, it is a similarity measure
// ----------------------------------------------------------------


namespace imdb
{

template <class T, class R = float>
struct l1norm
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            R d = static_cast<R>(*ai) - static_cast<R>(*bi);
            s += std::abs(d);
        }
        return s;
    }
};


template <class T, class R = float>
struct l2norm_squared
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            R d = static_cast<R>(*ai) - static_cast<R>(*bi);
            s += d*d;
        }
        return s;
    }
};


// experimental distance function that simulates the behaviour
// of an inverted index search when the tf/idf functions are constant
template <class T, class R = float>
struct dist_tmp
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        float la = 0;
        float lb = 0;

        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            int aa = ((*ai) != 0);
            int bb = ((*bi) != 0);

            la += aa;
            lb += bb;

            s += aa*bb;
        }
        return std::sqrt(la)*std::sqrt(lb) / (s + std::numeric_limits<float>::epsilon());
    }
};


template <class T, class R = float>
struct l2norm
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    l2norm_squared<T,R> n;

    R operator() (const T& a, const T& b) const
    {
        return std::sqrt(n(a,b));
    }
};


// assumes a and b are two vectors in R^d
// computes <a,b>
//template <class T, class R = float>
//struct dotproduct
//{
//    // stl
//    typedef T first_argument_type;
//    typedef T second_argument_type;
//    typedef R result_type;

//    R operator() (const T& a, const T& b) const
//    {
//        R s = 0;
//        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
//        {
//            s += static_cast<R>(*ai) * static_cast<R>(*bi);
//        }
//        return s;
//    }
//};


// assumes a and b are two vectors in R^d and have unit length
// computes 1 - <a,b>
template <class T, class R = float>
struct one_minus_dot
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            s += static_cast<R>(*ai) * static_cast<R>(*bi);
        }
        return (1.0 - s);
    }
};

//template <class T, class R = float>
//struct dotproduct_premult
//{
//    // stl
//    typedef T first_argument_type;
//    typedef T second_argument_type;
//    typedef R result_type;

//    // ap and bp are the premultiplied versions of a and b
//    R operator() (const T& a, const T& b, const T& ap, const T& bp) const
//    {
//        R s = 0;
//        s +=     _dp(ap, a);
//        s -= 2 * _dp(ap, b);
//        s +=     _dp(bp, b);
//        return std::sqrt(s);
//    }
//    dotproduct<T, R> _dp;
//};


// computes the total distance value
// as the sum of l2 distances between
// three consecutive data points.
// this is useful e.g. for the TinyLab
// descriptor where three consecutive
// datapoints represent a pixel in L*a*b
// colorspace
template <class T, class R = float>
struct l2norminterleaved
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R dist = 0;

        // We assume that the descriptor size is divisible by three.
        // Otherwise we will just miss one or two entries at the end...
        //std::size_t numPixels = std::distance(a.begin(), a.end()) / 3;
        //unsigned int index = 0;

        typename T::const_iterator ai = a.begin();
        typename T::const_iterator bi = b.begin();
        while (ai != a.end())
        {
            int i = 0;
            R dsq = 0;
            for (; i < 3 && ai != a.end(); i++, ++ai, ++bi)
            {
                dsq += (*ai - *bi) * (*ai - *bi);
            }

            if (i < 3) break;

            dist += std::sqrt(dsq);
        }
        return dist;
    }
};

template <class T, class R = float>
struct jsd
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            R v0 = *ai;
            R v1 = *bi;
            R n = 2.0 / (v0 + v1);

            s += ((v0 > 0.0) ? v0 * std::log(v0*n):0.0) + ((v1 > 0.0) ? v1 * std::log(v1*n):0.0);
        }
        return s;
    }
};



// TODO: test me!!!
// chi squared distance function between two vectors k and h
// d = sum_i[(ki - hi)^2 / (ki+hi)]
// avoid division through zero by adding a tiny value to the denominator
// 1. if ki == hi == 0 then the nominator is zero and the overall result is zero as desired
// 2. if ki or hi != 0 then ki + hi == ki + hi + float_min
template <class T, class R = float>
struct chi2
{

    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        R s = 0;
        for (typename T::const_iterator ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi)
        {
            R v0 = *ai;
            R v1 = *bi;
            R nom = v0-v1;
            R denom = v0+v1+std::numeric_limits<R>::epsilon();
            s += nom*nom / denom;
        }
        return s;
    }
};



template <class T, class R = float>
struct dist_frobenius
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    R operator() (const T& a, const T& b) const
    {
        assert(a.size() % 3 == 0);
        assert(a.size() == b.size());
        uint numTensors = a.size() / 3;
        uint index = 0;
        R dist = 0;

        for (uint i = 0; i < numTensors; i++) {
            R dE = a[index] - b[index]; index++;
            R dF = a[index] - b[index]; index++;
            R dG = a[index] - b[index]; index++;
            dist += std::sqrt(dE*dE + 2*dF*dF + dG*dG);
        }
        assert(index == a.size());

        // normalization not required since the number of cells is defined by the query
        // sketch and thus is the same for all comparison required for one query
        return dist;
    }
};


// Distance function from the paper
// Lee & Funkhouser - Sketch-Based Search and Composition of 3D Models
// computes <a,dt(b)> + <b,dt(a)>
// Result range: [0,inf]
// 0 means that two descriptors are equal
template <class T, class R = float>
struct dist_df
{
    // stl
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef R result_type;

    // a and b are two descriptors we want to compare,
    // First half of the vectors is expected to contain the
    // distance transform, second half the image/sketch itself
    R operator() (const T& a, const T& b) const
    {
        assert(a.size() % 2 == 0);
        assert(a.size() == b.size());

        result_type dist = 0;
        size_t offset = a.size() / 2;
        for (size_t i = 0; i < offset; i++) {
            dist += a[i]*b[i+offset];
            dist += b[i]*a[i+offset];
        }
        return dist;
    }
};


template <class T, class R = float>
struct distance_functions
{
    typedef boost::function<R (const T&, const T&)> distfn_t;

    distfn_t make(const std::string& name)
    {
        return distfn_t();
    }
};

template <class X, class R>
struct distance_functions<std::vector<X>, R>
{
    typedef std::vector<X> T;
    typedef boost::function<R (const T&, const T&)> distfn_t;

    distfn_t make(const std::string& name)
    {
        if (name == "l1norm") return l1norm<T>();
        if (name == "l2norm") return l2norm<T>();
        if (name == "l2norm_squared") return l2norm_squared<T>();
        if (name == "jsd") return jsd<T>();
        if (name == "chi2") return chi2<T>();
        //if (name == "dotproduct") return dotproduct<T>();
        if (name == "one_minus_dot") return one_minus_dot<T>();
        if (name == "df") return dist_df<T>();
        if (name == "dist_tmp") return dist_tmp<T>();

        return distfn_t();
    }
};


} // namespace imdb

#endif // DISTANCE_HPP
