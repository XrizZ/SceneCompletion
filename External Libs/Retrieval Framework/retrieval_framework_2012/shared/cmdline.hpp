#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include <string>
#include <vector>
#include <iostream>

#include <boost/lexical_cast.hpp>

namespace imdb
{

inline bool is_short_option(const std::string& s)
{
    return (s.length() > 1 && s[0] == '-' && std::isalpha(s[1]));
}

inline bool is_long_option(const std::string& s)
{
    return (s.length() > 2 && s[0] == '-' && s[1] == '-' && std::isalpha(s[2]));
}

std::vector<std::string> argv_to_strings(int argc, char* argv[])
{
    std::vector<std::string> strings;
    for (int i = 0; i < argc; i++) strings.push_back(argv[i]);
    return strings;
}

class CmdOption
{
    public:

    CmdOption(const std::string& long_option, const std::string& short_option, const std::string& description)
        : _long_option(long_option)
        , _short_option(short_option)
        , _description(description)
    {}

    template <class T>
    bool parse_single(const std::vector<std::string>& args, T& value)
    {
        bool found = false;

        for (int i = 0; i < (int)args.size() - 1; i++)
        {
            if (match(args[i]))
            {
                if (is_short_option(args[i + 1]) || is_long_option(args[i + 1])) continue;

                try
                {
                    value = boost::lexical_cast<T>(args[i + 1]);
                    found = true;
                }
                catch (boost::bad_lexical_cast&)
                {
                    std::cerr << "bad parameter value: " << args[i + 1] << std::endl;
                }
            }
        }

        return found;
    }

    template <class T>
    bool parse_multiple(const std::vector<std::string>& args, std::vector<T>& values)
    {
        bool found = false;

        for (int i = 0; i < (int)args.size() - 1; i++)
        {
            if (match(args[i]))
            {
                for (size_t k = i+1; k < args.size(); k++ )
                {
                    if (is_short_option(args[k]) || is_long_option(args[k])) break;

                    try
                    {
                        values.push_back(boost::lexical_cast<T>(args[k]));
                        found = true;
                    }
                    catch (boost::bad_lexical_cast&)
                    {
                        std::cerr << "bad parameter value: " << args[k] << std::endl;
                    }
                }
            }
        }

        return found;
    }

    bool match(const std::string& arg)
    {
        return ((is_short_option(arg) && arg.compare(1, arg.length()-1, _short_option) == 0) ||
                (is_long_option(arg) && arg.compare(2, arg.length()-2, _long_option) == 0));
    }

    const std::string& long_option() const { return _long_option; }
    const std::string& short_option() const { return _short_option; }
    const std::string& description() const { return _description; }

    private:

    std::string _long_option;
    std::string _short_option;
    std::string _description;
};

class Command
{
    public:

    Command(const std::string& usage = "") : _usage(usage)
    {}

    void add(const CmdOption& option)
    {
        _options.push_back(option);
    }

    std::vector<std::string> check_for_unknown_option(const std::vector<std::string>& args)
    {
        std::vector<std::string> unknown;

        for (size_t i = 0; i < args.size(); i++)
        {
            if (!is_short_option(args[i]) && !is_long_option(args[i])) continue;

            bool found = false;

            for (size_t k = 0; k < _options.size() && !found; k++)
            {
                if (_options[k].match(args[i])) found = true;
            }

            if (!found) unknown.push_back(args[i]);
        }

        return unknown;
    }

    void warn_for_unknown_option(const std::vector<std::string>& args)
    {
        std::vector<std::string> unknown = check_for_unknown_option(args);
        for (size_t i = 0; i < unknown.size(); i++)
        {
            std::cerr << "WARNING: unknown option: " << unknown[i] << std::endl;
        }
    }

    void print() const
    {
        static const int c0 = 30;

        std::cout << _usage << std::endl;

        if (_options.size() > 0) std::cout << "options:" << std::endl;

        for (size_t i = 0; i < _options.size(); i++)
        {
            std::string line = "  --" + _options[i].long_option() + ", -" + _options[i].short_option();
            for (int k = line.length(); k < c0; k++) line += ' ';
            std::cout << line << _options[i].description() << std::endl;
        }
    }

    virtual bool run(const std::vector<std::string>& /*args*/)
    {
        return false;
    }

    private:

    std::vector<CmdOption> _options;
    std::string            _usage;
};

} // namespace imdb

#endif // CMDLINE_HPP
