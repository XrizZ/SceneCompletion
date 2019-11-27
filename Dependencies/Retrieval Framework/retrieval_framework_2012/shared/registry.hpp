#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <types.hpp>

namespace imdb {

class Registry
{
    public:

    template <class T>
    T& get(const std::string name)
    {
        if (!_entries.count(name)) _entries.insert(std::make_pair(name, boost::make_shared<T>()));
        return *boost::any_cast<boost::shared_ptr<T> >(_entries.find(name)->second);
    }

    private:

    anymap_t _entries;

    // this gives the one and only instance of Registry
    static Registry& instance() { static Registry singleton; return singleton; }
    friend Registry& registry();

    // protect construction from outside
    Registry() {}
    Registry(const Registry&) {}
    ~Registry() {}
};

inline Registry& registry() { return Registry::instance(); }

} // namespace imdb

#endif // REGISTRY_HPP
