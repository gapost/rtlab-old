#ifndef DAQMX_H
#define DAQMX_H

#include <string>
#include <list>
#include <vector>

namespace daqmx {

struct sys
{
    static int deviceNames(std::vector<std::string> &devs);
};

class dev
{
    std::string devName_;

    dev();
public:
    explicit dev(const std::string& name) : devName_(name)
    {}
    dev(const dev& other) : devName_(other.devName_)
    {}

    const std::string& name() const
    { return devName_; }

    int aiChannels(std::vector<std::string> &ch) const;
    int aoChannels(std::vector<std::string> &ch) const;
    int ciChannels(std::vector<std::string> &ch) const;
    int coChannels(std::vector<std::string> &ch) const;
    int diLines(std::vector<std::string> &ch) const;
    int doLines(std::vector<std::string> &ch) const;
};

}

#endif // DAQMX_H
