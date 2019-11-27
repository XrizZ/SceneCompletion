#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <vector>
#include <map>
#include <string>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <opencv2/core/core.hpp>

#include "types.hpp"
#include "property.hpp"
#include "registry.hpp"


namespace imdb {

class Generator
{

    public:

    typedef vector<shared_ptr<Property> > properties_t;
    typedef map<string, function<shared_ptr<Generator> (const ptree&)> > generators_t;


    // static stuff
    template <class generator_t>
    static inline bool register_generator(const string& name)
    {
        generators_t& generators_map = registry().get<generators_t>("generators");
        generators_map[name] = boost::bind(create<generator_t>, name, boost::arg<1>());
        return true;
    }

    // creates a generator from a parameters file
    // the created generator has exactly the same
    // parameters as stored in the file (i.e. descriptor type,
    // and its corresponding parameters)
    static shared_ptr<Generator> from_parameters_file(const string& filename)
    {
        ptree params;
        boost::property_tree::read_json(filename, params);

        // required parameter: name of the generator
        // if not available, we cannot lookup the generator instance
        string key = params.get<string>("name");

        generators_t::const_iterator cit = generators().find(key);

        if (cit == generators().end())
        {
            std::cerr << "generator <" << key << "> not registered -- probably need to include the corresponding cpp file." << std::endl;
            // will crash after that, but at least we have a hint on the console about what has happened
        }
        return generators().at(key)(params);
    }


    // returns a generator initialized with its default parameters
    static shared_ptr<Generator> from_default_parameters(const string& name)
    {
        ptree emptyPtree;
        generators_t::const_iterator cit = generators().find(name);

        if (cit == generators().end())
        {
            std::cerr << "generator <" << name << "> not registered -- probably need to include the corresponding cpp file." << std::endl;
            // will crash after that, but at least we have a hint on the console about what has happened
        }
        return generators().at(name)(emptyPtree);
    }

    // creates a generator from a parameters file given
    // the name of the generator (which comes from a session)
    // the created generator has exactly the same
    // parameters as stored in the file (i.e. descriptor type,
    // and its corresponding parameters)
    /*
      Mathias 03.05.11: it feels a bit weird that we still need to provide the generator name
      shouldn't that be completely specified in its parameters file?!
    */


    // TODO: check if we need the extra generator name parameter?!
    // we currently expect that it is enough to have the paramters file
    // which always also defines the generator name
//    static shared_ptr<Generator> from_parameters_file_ptree(const string& filename, const string& generator_name)
//    {
//        ptree params;
//        boost::property_tree::read_json(filename, params);

//        generators_t::const_iterator cit = generators().find(generator_name);

//        // Mathias 03.05.11: maybe better: throw exception in case no generator found!
//        // rather than just printing an error message
//        if (cit == generators().end())
//        {
//            std::cerr << "generator <" << generator_name << "> not registered -- probably need to include the corresponding cpp file." << std::endl;
//            // will crash after that, but at least we have a hint on the console about what has happened
//        }
//        return (cit->second)(params);
//    }

    static inline const generators_t& generators() { return registry().get<generators_t>("generators");}




    // class members
    Generator(const ptree& parameters, const Properties& properties)
     : _parameters(parameters)
     , _properties(properties)
    {}

    virtual ~Generator() {}
    virtual shared_ptr<Generator> clone() const = 0;

    virtual void compute(anymap_t& data) = 0;

    const properties_t& properties() const
    {
        return _properties.get();
    }

    const ptree& parameters() const
    {
        return _parameters;
    }


    protected:

    // must be protected instead of private since specific generators
    // make use of the parse() function which must be able to
    // replace/add entries to the property tree
    ptree        _parameters;
    Properties   _properties;
    
    private:

    template <class generator_t>
    static shared_ptr<Generator> create(const string& name, const ptree& parameters)
    {
        ptree extendedParams(parameters);
        extendedParams.put("name", name);
        return boost::make_shared<generator_t>(extendedParams);
    }
};

template <class T>
class GeneratorWithCopyClone : public Generator
{
    public:

    GeneratorWithCopyClone(const ptree& parameters, const Properties& properties) : Generator(parameters, properties) {}

    shared_ptr<Generator> clone() const
    {
        return shared_ptr<Generator>(new T(dynamic_cast<const T&>(*this)));
    }
};

class DetectExtractInterface
{
    public:

    virtual ~DetectExtractInterface() {}

    virtual double scale(const cv::Mat& image, cv::Mat& scaled) = 0;

    virtual void detect(const cv::Mat& image, std::vector<cv::Point2f>& keypoints) = 0;

    virtual void extract(const cv::Mat& image, const std::vector<cv::Point2f>& keypoints, vec_vec_f32_t& features) = 0;

};

} // namespace imdb

#endif // GENERATOR_HPP
