#include "daqmx.h"

#include <NIDAQmx.h>

#include <sstream>



using std::string;
using std::vector;

// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

#define BUFFLEN 4096

// parse comma separated string from NI function
void ni_tokenize(const string& str, vector<string>& tokens)
{
    std::istringstream iss(str);
    std::string token;
    tokens.clear();
    while(iss >> token) {
        // erase trailing comma
        size_t last = token.size()-1;
        if (token.at(last)==',')
            token.erase(last,1);
        tokens.push_back(token);
    }
}

int daqmx::sys::deviceNames(vector<string>& devs)
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetSysDevNames(buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, devs);
    }

    return ret;
}

int daqmx::dev::aiChannels(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevAIPhysicalChans(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}
int daqmx::dev::aoChannels(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevAOPhysicalChans(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}
int daqmx::dev::ciChannels(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevCIPhysicalChans(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}
int daqmx::dev::coChannels(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevCOPhysicalChans(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}
int daqmx::dev::diLines(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevDILines(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}
int daqmx::dev::doLines(std::vector<std::string> &ch) const
{
    const uInt32 len = BUFFLEN;
    char buff[len];

    int ret = DAQmxGetDevDOLines(devName_.c_str(),buff,len);

    if (ret==0)
    {
        ni_tokenize(buff, ch);
    }

    return ret;
}

