#include <curl/curl.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <math.h>

#include <switch.h>
#include <json.hpp>

#include "download.hpp"

const std::string API_AGENT = "PoloNX";
#define _1MiB 0x100000

typedef struct{
    char* memory;
    size_t size;
} MemoryStruct_t;

typedef struct
{
    u_int8_t *data;
    size_t data_size;
    u_int64_t offset;
    FILE *out;
} ntwrk_struct_t;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t num_files, void *userp)
{
    ntwrk_struct_t *data_struct = (ntwrk_struct_t *)userp;
    size_t realsize = size * num_files;

    if (realsize + data_struct->offset >= data_struct->data_size)
    {
        fwrite(data_struct->data, data_struct->offset, 1, data_struct->out);
        data_struct->offset = 0;
    }

    memcpy(&data_struct->data[data_struct->offset], contents, realsize);
    data_struct->offset += realsize;
    data_struct->data[data_struct->offset] = 0;
    return realsize;
}

int download_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal <= 0.0) return 0;

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    int counter = round(tv.tv_usec / 100000);

    if (counter == 0 || counter == 2 || counter == 4 || counter == 6 || counter == 8)
    {
        //printf("* Telechargement: %.2fMB sur %.2fMB *\r", dlnow / _1MiB, dltotal / _1MiB);
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << "\r* Telechargement: " << dlnow / _1MiB << "MB sur " << dltotal / _1MiB << "MB *";
        consoleUpdate(NULL);
    }

    return 0;
}

struct MemoryStruct
{
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback2(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    char* ptr = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
    if (ptr == NULL) {
        /* out of memory! */
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

long downloadPage(const std::string& url, std::string& res, const std::vector<std::string>& headers, const std::string& body)
{
    CURL* curl_handle;
    struct MemoryStruct chunk;
    struct curl_slist* list = NULL;
    long status_code;

    chunk.memory = static_cast<char*>(malloc(1)); /* will be grown as needed by the realloc above */
    chunk.size = 0;                               /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    if (!headers.empty()) {
        for (auto& h : headers) {
            list = curl_slist_append(list, h.c_str());
        }
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
    }
    if (body != "") {
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body.c_str());
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback2);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, API_AGENT);

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_perform(curl_handle);
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &status_code);
    curl_easy_cleanup(curl_handle);
    res = std::string(chunk.memory);
    free(chunk.memory);

    curl_global_cleanup();
    return status_code;
}



namespace net{
   bool downloadFile(const std::string &url, const std::string &output, const bool api){
        CURL *curl = curl_easy_init();
        if (curl)
        {
            FILE *fp = fopen(output.c_str(), "wb");
            if (fp)
            {
                std::cout << std::endl;

                ntwrk_struct_t chunk = {0};
                chunk.data = static_cast<u_int8_t*>(malloc(_1MiB));
                chunk.data_size = _1MiB;
                chunk.out = fp;


                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_USERAGENT, API_AGENT.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

                // write calls
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

                if (api == false)
                {
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
                }

                // execute curl, save result
                CURLcode res = curl_easy_perform(curl);

                // write from mem to file
                if (chunk.offset)
                fwrite(chunk.data, 1, chunk.offset, fp);

                // clean
                curl_easy_cleanup(curl);
                free(chunk.data);
                fclose(chunk.out);

                if (res == CURLE_OK)
                {
                    std::cout << "\n\nTelechargement complete\n\n";
                    consoleUpdate(NULL);
                    return true;
                }
            }
        }
        
        std::cout << "\n\nErreur de telechargement\n\n";
        consoleUpdate(NULL);
        return false;
    } 

    long getRequest(const std::string& url, nlohmann::ordered_json& res, const std::vector<std::string>& headers, const std::string& body){
        std::string request;
        long status_code = downloadPage(url, request, headers, body);
        if(nlohmann::json::accept(request)){
            res = nlohmann::ordered_json::parse(request);
        }
        else{
            res = nlohmann::ordered_json::object();
        }

        return status_code;
    }
}