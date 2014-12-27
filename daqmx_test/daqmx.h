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

struct dev
{
    static int aiChannels(std::string& devname, std::vector<std::string> &ch);
    static int aoChannels(std::string& devname, std::vector<std::string> &ch);
    static int ciChannels(std::string& devname, std::vector<std::string> &ch);
    static int coChannels(std::string& devname, std::vector<std::string> &ch);
    static int diLines(std::string& devname, std::vector<std::string> &ch);
    static int doLines(std::string& devname, std::vector<std::string> &ch);
};

}

#endif // DAQMX_H
