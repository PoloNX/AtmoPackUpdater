#include <minizip/unzip.h>
#include <string>

#include <switch.h>

namespace extract {
    int unzip(const std::string &file, const std::string &output, const bool overwrite_inis);
}