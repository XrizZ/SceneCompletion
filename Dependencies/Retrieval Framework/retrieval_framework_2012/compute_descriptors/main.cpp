#include <QCoreApplication>
#include <QtCore>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <queue>
#include <stdexcept>
#include <cctype>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>

#include <types.hpp>
#include <property.hpp>
#include <generator.hpp>
#include <cmdline.hpp>
#include <imagefiles.h>

#include <opencv2/highgui/highgui.hpp>

using namespace imdb;

class ordered_push_back
{
public:

    ordered_push_back(shared_ptr<PropertyWriter> writer)
        : _writer(writer)
        , _numWrittenElements(0)
    {}

    bool push_back(size_t index, const boost::any& element)
    {
        boost::lock_guard<boost::mutex> locked(_mutex);

        // since the things we have written so far is just a linear
        // index of features, this condition must hold, i.e. all elements
        // we get must be at the end or behind of what has been written so far
        assert(index >= _numWrittenElements);

        _queue.push(queue_element(index, make_shared<boost::any>(element)));

        // *linearly* write stuff into the output vector
        while (!_queue.empty() && _queue.top().first == _numWrittenElements)
        {
            _writer->push_back(*_queue.top().second);
            _queue.pop();
            _numWrittenElements++;
        }

        return true;
    }

    bool empty_buffer() const
    {
        return _queue.empty();
    }

private:

    boost::shared_ptr<PropertyWriter> _writer;
    std::size_t _numWrittenElements;
    boost::mutex _mutex;

    typedef std::pair<size_t, boost::shared_ptr<boost::any> > queue_element;
    typedef std::greater<queue_element>                       queue_compare;
    typedef std::priority_queue<queue_element, std::vector<queue_element>, queue_compare> queue_t;

    queue_t _queue;
};


struct data_loader
{
    virtual ~data_loader() {}
    virtual void get(index_t index, anymap_t& data) const=0;
    virtual size_t size() const = 0;
};

class image_loader : public data_loader
{

public:

    image_loader(const ImageFiles& imageFiles) : _imageFiles(imageFiles) {}

    void get(index_t index, anymap_t& data) const
    {
        string filename = _imageFiles.get_filename(index);
        mat_8uc3_t image;

        try
        {
            // second parmeter: flags >0 means that the loaded image is forced to be a 3-channel color image
            //
            // Although this is not explicitly stated in the OpenCV docs, the channel
            // order is BGR, this has been tested by Mathias 08.June.2011 for both png
            // and jpg images. I.e. the following code
            // cv::Mat img = cv::imread("/Users/admin/tmp/blue.jpg");
            // cv::Vec3b v = img.at<cv::Vec3b>(0,0);
            // will result in the blue information at v[0], green at v[1] and red at v[2]
            image = cv::imread(filename, 1);
        }
        catch(cv::Exception& e)
        {
            // the original opencv exception message is very poor -- make it more clear
            // and more importantly give the problematic filename
            throw std::runtime_error("compute_descriptors: cv::imread failed for file: " + filename);
        }


        data["image"]          = image;
        data["image_filename"] = filename;
    }

    size_t size() const { return _imageFiles.size(); }

private:

    const ImageFiles& _imageFiles;
};


//class multi_image_loader : public data_loader
//{

//public:

//    // the vector for now needs to contain absolute filenames, i.e. not relative to a root dir
//    multi_image_loader(const vector<vector<string> >& filenames) : _filenames(filenames) {}

//    void get(index_t index, anymap_t& data) const
//    {
//        vector<string> files   = _filenames[index];
//        QVector<QImage> images;
//        for (size_t i = 0; i < files.size(); i++)
//        {
//            QImage image(files[i].c_str());
//            images.push_back(image);
//        }

//        data["images"]          = images;
//        data["image_filenames"] = files;
//    }

//    size_t size() const { return _filenames.size(); }

//private:

//    const vector<vector<string> >& _filenames;
//};



class compute_descriptors : boost::noncopyable
{
    typedef std::pair<std::string, boost::shared_ptr<ordered_push_back> > string_writer_pair;

public:

    compute_descriptors(boost::shared_ptr<Generator> generator, const shared_ptr<data_loader> loader)
        : _generator(generator)
        , _data_loader(loader)
        , _index(0)
        , _error(false)
        , _started(false)
        , _finished(false)
    {}


    void add_writer(const std::string& name, boost::shared_ptr<PropertyWriter> writer)
    {
        _writers.push_back(std::make_pair(name, boost::make_shared<ordered_push_back>(writer)));
    }

    bool start(int num_threads)
    {
        assert(num_threads > 0);
        using namespace boost;

        if (_started) return false;

        _started = true;
        _datetime = QDateTime::currentDateTime();

        thread_group pool;
        for (int i = 0; i < num_threads; i++)
        {
            pool.add_thread(new thread(std::mem_fun(&compute_descriptors::_thread), this, _generator->clone()));
        }

        pool.join_all();

        _finished = true;
        _seconds = _datetime.secsTo(QDateTime::currentDateTime());

        for (size_t i = 0; i < _writers.size(); i++)
        {
            _error |= !_writers[i].second->empty_buffer();
        }

        return (!_error);
    }

    size_t current() const
    {
        boost::lock_guard<boost::mutex> lock(_mutex);
        return _index;
    }

    size_t num_images() const
    {
        return _data_loader->size();
    }

    bool finished() const
    {
        return _finished;
    }

    int computation_time() const
    {
        // precondition: finished() == true
        return _seconds;
    }
private:

    void _thread(boost::shared_ptr<Generator> gen)
    {
        while (!_error)
        {
            size_t current;
            anymap_t data;

            {
                boost::lock_guard<boost::mutex> lock(_mutex);
                if (_index == _data_loader->size()) break;
                current = _index;
                _index++;

            }

            try
            {
                _data_loader->get(current, data);
                gen->compute(data);
            }

            catch (std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                _error = true;
                return;
            }            

            for (std::vector<string_writer_pair>::const_iterator wi = _writers.begin(); wi != _writers.end(); ++wi)
            {
                anymap_t::const_iterator ri = data.find(wi->first);
                if (ri != data.end()) wi->second->push_back(current, ri->second);
            }
        }
    }

private:

    boost::shared_ptr<Generator>    _generator;
    const shared_ptr<data_loader>   _data_loader;
    std::vector<string_writer_pair> _writers;

    volatile size_t _index;
    volatile bool _error;
    volatile bool _started;
    volatile bool _finished;

    QDateTime _datetime;
    int       _seconds;

    mutable boost::mutex _mutex;
};

void progress_observer(const compute_descriptors& cd)
{
    size_t lastindex = 0;
    int running_sum_time = 0;
    int running_sum_processed = 0;

    QTime lasttime;
    std::queue<pair<int,int> > queue;

    // make the number of running samples we
    // compute the average from reasonably large
    // such that outliers do not strongly influence
    // the average (better use the median)
    std::size_t q_maxsize = 100;

    bool firstrun = true;
    while (!cd.finished())
    {
        size_t index = cd.current();
        int d_processed = index - lastindex;

        if (!firstrun && d_processed > 0)
        {
            int d_time = lasttime.elapsed();

            queue.push(std::make_pair(d_time, d_processed));
            running_sum_time += d_time;
            running_sum_processed += d_processed;

            if (queue.size() > q_maxsize)
            {
                running_sum_time -= queue.front().first;
                running_sum_processed -= queue.front().second;
                queue.pop();
            }

            assert(queue.size() > 0);
            assert(running_sum_processed > 0);
        }

        std::cout << "compute: " << index << "/" << cd.num_images();
        if (queue.size() > 0)
        {
            int msecdescr = running_sum_time / static_cast<float>(running_sum_processed);
            int etaseconds = (cd.num_images() - index) * msecdescr / 1000;
            int fmth = etaseconds / 3600;
            int fmtm = etaseconds / 60 % 60;
            int fmts = etaseconds % 60;
            std::cout << ", ms/descriptor: " << msecdescr << ", eta: " << fmth << ":" << fmtm << ":" << fmts;
        }

        std::cout << "            \r" << std::flush;

        lasttime.start();
        firstrun = false;
        lastindex = index;

        boost::this_thread::sleep(boost::posix_time::seconds(3));
    }

    std::cout << std::endl;
}

void progress_lookup_dir(size_t index)
{
    static const int ival = 1000;
    static const char whirl[4] = { '-', '\\', '|', '/' };
    if (index % ival == 0) std::cout << whirl[(index / ival) % 4] << " " << index << '\r' << std::flush;
}

void print_available_generators()
{
    std::cout << "available generators:" << std::endl;

    Generator::generators_t::const_iterator it;
    for (it = Generator::generators().begin(); it != Generator::generators().end(); ++it)
    {
        std::cout << "* " << it->first << std::endl;
    }
}

class command_compute : public Command
{
public:

    command_compute()
        : Command("compute <generator> [options]")
        , _co_rootdir   ("rootdir"          , "r", "root directory of data descriptors are computed from [required]")
        , _co_filelist  ("filelist"         , "f", "file that contains filenames of data (images/models) [required]")
        , _co_output    ("output"           , "o", "output prefix [required]")
        , _co_params    ("parameters"       , "p", "parameters for generator construction [optional] (default: params defined in generator)")
        , _co_numthreads("numthreads"       , "t", "number of threads for parallel computation [optional] (default: number of processors)")

    {
        add(_co_rootdir);
        add(_co_filelist);
        add(_co_output);
        add(_co_params);
        add(_co_numthreads);
    }


    bool run(const std::vector<std::string>& args)
    {
        if (args.size() == 0)
        {
            print();
            return false;
        }

        warn_for_unknown_option(args);

        std::string in_generator(args[0]);

        if (!Generator::generators().count(in_generator))
        {
            std::cerr << "no generator named " << in_generator << std::endl;
            print_available_generators();
            return false;
        }


        std::string in_rootdir;
        std::string in_filelist;
        std::string in_output;
        std::vector<std::string> in_params;

        ptree params;

        // ----------------------------------------------------------------------------------
        // number of threads to be used: by default we use as many
        // as we have processors on the machine, otherwise use
        // user-provided parameter.
        int in_numthreads = boost::thread::hardware_concurrency();
        if (_co_numthreads.parse_single<int>(args, in_numthreads)) {
            if (in_numthreads < 1) {
                std::cout << "compute_descriptors: number of threads should be > 0, using default" << std::endl;
                in_numthreads = boost::thread::hardware_concurrency();
            }
        }
        std::cout << "compute_descriptors: using " << in_numthreads << " threads" << std::endl;
        // ------------------------------------------------------------------------------------

        if (!_co_rootdir.parse_single<std::string>(args, in_rootdir)
                || !_co_output.parse_single<std::string>(args, in_output))
        {
            print();
            return false;
        }

        _co_params.parse_multiple<std::string>(args, in_params);


        for (size_t i = 0; i < in_params.size(); i++)
        {
            std::vector<std::string> pv;
            boost::algorithm::split(pv, in_params[i], boost::algorithm::is_any_of("="));

            if (pv.size() != 1 && pv.size() != 2)
            {
                std::cerr << "cannot parse parameter: " << in_params[i] << std::endl;
                return false;
            }
            params.put(pv[0], (pv.size() == 2) ? pv[1] : "");
        }

        ImageFiles files(in_rootdir);
        if (!_co_filelist.parse_single<std::string>(args, in_filelist))
        {
            print();
            return false;
        }
        else
        {
            files.load(in_filelist);
        }

        // create generator
        boost::shared_ptr<Generator> generator(Generator::generators().at(in_generator)(params));

        // instantiate data_loader
        shared_ptr<data_loader> loader = make_shared<image_loader>(files);

        // initialize a computing object
        compute_descriptors cd(generator, loader);

        // add writers for properties offered by the generator
        std::vector<boost::shared_ptr<Property> > properties = generator->properties();
        for (size_t i = 0; i < properties.size(); i++)
        {
            const std::string n = properties[i]->name();
            cd.add_writer(n, properties[i]->create_writer(in_output + n));
        }

        // start computing descriptors
        QDateTime time = QDateTime::currentDateTime();

        boost::thread obs(progress_observer, boost::ref(cd));

        bool okay = cd.start(in_numthreads);

        int seconds = time.secsTo(QDateTime::currentDateTime());
        obs.join();

        int fmth = seconds / 3600;
        int fmtm = seconds / 60 % 60;
        int fmts = seconds % 60;
        std::cout << "finished." << std::endl;
        std::cout << "duration: " << fmth << "h " << fmtm << "m " << fmts << "s" << " (" << seconds << " s)" << std::endl;

        string filename = in_output + "parameters";
        boost::property_tree::write_json(filename, generator->parameters());


        if (!okay)
        {
            std::cerr << "error while computing descriptors occured" << std::endl;
        }

        return okay;
    }

private:

    CmdOption _co_rootdir;
    CmdOption _co_filelist;
    CmdOption _co_output;
    CmdOption _co_params;
    CmdOption _co_numthreads;
};

class command_info : public Command
{
public:

    command_info()
        : Command("info <generator>")
    {}

    bool run(const std::vector<std::string>& args)
    {
        if (args.size() == 0)
        {
            print();
            return false;
        }

        warn_for_unknown_option(args);

        std::string in_generator(args[0]);

        if (!Generator::generators().count(in_generator))
        {
            std::cerr << "no generator named " << in_generator << std::endl;
            print_available_generators();

            return false;
        }

        // create generator
        shared_ptr<Generator> generator(Generator::generators().at(in_generator)(ptree()));

        // print properties, parameters

        // descriptions unfortunately no more available with boost::property_tree
        // const strmap_t& gen_descs = generator->descriptions();

        // since we have just created a "new" generator (passing an empty parameters object
        // the parameters contained in it describe its default values
        // const strmap_t& gen_dvals = generator->defaultvalues();

        const ptree& params = generator->parameters();

        const int c0 = 20;
        std::cout << " parameter          | default" << std::endl;
        std::cout << "                    +        " << std::endl;

        // TODO: would be nice to have a function that displays a complete subtree

        // we assume that all parameter related entries we want
        // to print here are stored in the subtree "params".
        // if this subtree does not exist, we print nothing
        const boost::optional<const ptree&> subtree(params.get_child_optional("params"));
        if (subtree)
        {
            for (ptree::const_iterator it = subtree->begin(); it != subtree->end(); ++it)
            {
                const std::string& param_name = it->first;
                const std::string& param_default = it->second.data();
                std::cout << param_name;
                for (int i = 0; i < c0 - int(param_name.length()); i++) std::cout << ' ';
                std::cout << param_default;
                std::cout << std::endl;
            }
        }

        return true;
    }
};

class command_list : public Command
{
public:

    bool run(const std::vector<std::string>& /*args*/)
    {
        print_available_generators();
        return true;
    }
};


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    typedef std::map<std::string, std::pair<boost::shared_ptr<Command>, std::string> > cmd_map_t;
    cmd_map_t cmd_desc;
    cmd_desc["compute"]    = std::make_pair(boost::make_shared<command_compute>()   , "compute descriptors");
    cmd_desc["info"]       = std::make_pair(boost::make_shared<command_info>()      , "print informations of specific generator");
    cmd_desc["list"]       = std::make_pair(boost::make_shared<command_list>()      , "print list of available generators");
    //cmd_desc["convert"]    = std::make_pair(boost::make_shared<command_convert>()   , "convert old property file to new one");

    if (argc <= 1 || !cmd_desc.count(argv[1]))
    {
        std::cout << "usage: " << (argc > 0 ? argv[0]:"compute_descriptors") << " <command> ..." << std::endl;
        std::cout << " commands:" << std::endl;

        const int c0 = 20;
        cmd_map_t::const_iterator it;
        for (it = cmd_desc.begin(); it != cmd_desc.end(); ++it)
        {
            std::cout << " * " << it->first;
            for (int k = 0; k < c0 - (int)it->first.length(); k++) std::cout << ' ';
            std::cout << " : " << it->second.second << std::endl;
        }

        return 1;
    }
    return cmd_desc[argv[1]].first->run(argv_to_strings(argc-2, &argv[2])) ? 0:1;
}


