#include <minizip/unzip.h>
#include <string>
#include <string.h>
#include <dirent.h>
#include <iostream>

#include <switch.h>

#include "unzip.hpp"

constexpr size_t WRITEBUFFERSIZE = 0x100000;

namespace extract{
    int unzip(const std::string &file, const std::string &output){
        chdir(output.c_str());
        unzFile zfile = unzOpen(file.c_str());
        unz_global_info gi = {0};
        unzGetGlobalInfo(zfile, &gi);

        for (int i = 0; i < gi.number_entry; ++i){
            char filename_inzip[0x301]= {0};
            unz_file_info file_info = {0};
            unzOpenCurrentFile(zfile);
            unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
            std::string filename_inzip_s = filename_inzip;

            if ((filename_inzip[strlen(filename_inzip) - 1]) == '/'){
                DIR *dir = opendir(filename_inzip);
                if(dir) closedir(dir);
                else{
                    std::cout << "Creation du repertoir : " << filename_inzip << std::endl;
                    mkdir(filename_inzip, 0777);
                }
            }
            else{
                FILE *outfile;
                void *buf = malloc(WRITEBUFFERSIZE);

                if ((filename_inzip_s == "atmosphere/package3") || (filename_inzip_s == "switch/AtmoPackUpdater.nro") || (filename_inzip_s == "atmosphere/stratosphere.romfs")){
                    outfile = fopen((filename_inzip_s + ".temp").c_str(), "wb");
                }

                else
                    outfile = fopen(filename_inzip, "wb");

                std::cout << "Extraction de: " << filename_inzip_s << std::endl;
                consoleUpdate(NULL);

                for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE)){
                    fwrite(buf, 1, j, outfile);
                }

                fclose(outfile);
                free(buf);
            }


            unzCloseCurrentFile(zfile);
            unzGoToNextFile(zfile);
            consoleUpdate(NULL);
        }

        unzClose(zfile);
        //remove(output.c_str());

        return 0;
    }

}