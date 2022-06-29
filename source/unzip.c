#include <stdio.h>
#include <minizip/unzip.h>
#include <string.h>
#include <dirent.h>
#include <switch.h>
#include <string.h>

#include "unzip.h"

#define WRITEBUFFERSIZE 0x1000 // 4KiB
#define MAXFILENAME     0x301

int unzip(const char *output)
{
    unzFile zfile = unzOpen(output);
    unz_global_info gi = {0};
    unzGetGlobalInfo(zfile, &gi);

    for (int i = 0; i < gi.number_entry; i++)
    {
        char filename_inzip[MAXFILENAME] = {0};
        unz_file_info file_info = {0};
        unzOpenCurrentFile(zfile);
        unzGetCurrentFileInfo(zfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
       

        // check if the string ends with a /, if so, then its a directory.
        if ((filename_inzip[strlen(filename_inzip) - 1]) == '/')
        {
            // check if directory exists
            DIR *dir = opendir(filename_inzip);
            if (dir) closedir(dir);
            else
            {
                printf("Création du répertoir: %s\n", filename_inzip);
                mkdir(filename_inzip, 0777);
            }
        }    

        else if (strcmp(filename_inzip, "atmosphere/package3") == 0){
            FILE *outfile = fopen("atmosphere/package3.temp", "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf("Lecture des fichiers: %s\n", filename_inzip);
            printf ("DANS PACKAGE3! NE PAS ETEINDRE LA CONSOLE!\n");
            consoleUpdate(NULL);
            sleep(2);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        else if (strcmp(filename_inzip, "atmosphere/stratosphere.romfs") == 0){
            FILE *outfile = fopen("atmosphere/stratosphere.romfs.temp", "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf("Lecture des fichiers: %s\n", filename_inzip);
            printf ("DANS STRATOSPHERE.ROMFS! NE PAS ETEINDRE LA CONSOLE!\n");
            consoleUpdate(NULL);
            sleep(2);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        else
        {
            const char *write_filename = filename_inzip;
            FILE *outfile = fopen(write_filename, "wb");
            void *buf = malloc(WRITEBUFFERSIZE);

            printf("Lecture des fichiers: %s\n", filename_inzip);
            consoleUpdate(NULL);

            for (int j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE); j > 0; j = unzReadCurrentFile(zfile, buf, WRITEBUFFERSIZE))
                fwrite(buf, 1, j, outfile);

            fclose(outfile);
            free(buf);
        }

        unzCloseCurrentFile(zfile);
        unzGoToNextFile(zfile);
        consoleUpdate(NULL);
    }

    unzClose(zfile);
    //remove(output);
    
    printf("\nFinis!\n\nRedémarrez la switch pour que les sigpatches fonctionnent!\n");
    consoleUpdate(NULL);

    return 0;
}
