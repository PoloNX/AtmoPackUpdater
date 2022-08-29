#include <minizip/unzip.h>
#include <string>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "fs.hpp"
#include "utils.hpp"
#include "extract.hpp"
#include "progress_event.hpp"

constexpr size_t WRITE_BUFFER_SIZE = 0x100000;

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void extractEntry(std::string filename, unzFile& zfile)
{
    if (filename.back() == '/') {
        fs::createTree(filename);
        return;
    }

    if (!std::filesystem::exists(filename)){
        fs::createTree(filename);
    }

    void* buf = malloc(WRITE_BUFFER_SIZE);
    FILE* outfile;
    outfile = fopen(filename.c_str(), "wb");
    std::cout << "extracted :" << filename << std::endl;
    for (int j = unzReadCurrentFile(zfile, buf, WRITE_BUFFER_SIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITE_BUFFER_SIZE)) {
        fwrite(buf, 1, j, outfile);
    }
    free(buf);
    fclose(outfile);
}

namespace extract {
    int unzip(const std::string &file, const std::string &output, const int overwrite_inis) {
        if(!std::filesystem::exists(output)) {
            fs::removeDir(output);
            fs::createTree(output);
        }
        unzFile zfile = unzOpen(file.c_str());
        unz_global_info gi = {0};
        unzGetGlobalInfo(zfile, &gi);

        ProgressEvent::instance().setTotalSteps(gi.number_entry);
        ProgressEvent::instance().setStep(0);

        std::string appPath = util::getAppPath();
        std::cout << appPath << std::endl;

        for (uLong i = 0; i < gi.number_entry; ++i) {
            char filename_inzip[0x301] = {0};
            unz_file_info file_info = {0};
            unzOpenCurrentFile(zfile);
            unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
            std::string filename_inzip_s = filename_inzip;

            if (ProgressEvent::instance().getInterupt()) {
                unzCloseCurrentFile(zfile);
                break;
            }

            if (GOOD_APP_PATH != output + filename_inzip_s && BAD_APP_PATH != output + filename_inzip_s) {
                if (overwrite_inis == 1){
                    if (ends_with(filename_inzip_s, ".ini")) {
                        ProgressEvent::instance().incrementStep(1);
                        unzCloseCurrentFile(zfile);
                        unzGoToNextFile(zfile);
                        continue;
                    }
                }
                if (filename_inzip_s.find("Firmware") != std::string::npos){
                    std::string temp = filename_inzip_s;
                    while((temp[0] != '/')) {
                        temp.erase(0, 1);
                    }
                    temp.erase(0, 1);
                    std::cout << "temp =" <<temp << std::endl;
                    filename_inzip_s = temp;
                }
                if ((filename_inzip_s == "atmosphere/package3") || (filename_inzip_s == "atmosphere/stratosphere.romfs")) {
                    filename_inzip_s.insert(0, output);
                    extractEntry(filename_inzip_s + ".temp", zfile);
                }
                else {
                    filename_inzip_s.insert(0, output);
                    extractEntry(filename_inzip_s, zfile);
                }
            }
   
            ProgressEvent::instance().incrementStep(1);
            unzCloseCurrentFile(zfile);
            unzGoToNextFile(zfile);
        }
        
        unzClose(zfile);
        remove(file.c_str());
        ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());

        return 0;
    }
}