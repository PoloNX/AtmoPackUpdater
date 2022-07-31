#include <filesystem>
#include <iostream>
#include <string>

#include <switch.h>

#define PATH		"/switch/AtmoPackUpdater/"
#define FULL_PATH   "/switch/AtmoPackUpdater/AtmoPackUpdater.nro"
#define BAD_PATH   "/switch/AtmoPackUpdater.nro"
#define CONFIG_PATH "/switch/temp.zip"
#define FORWARDER_PATH	  "/switch/AtmoPackUpdater/amssu-forwarder.nro"

int removeDir(const char* path)
{
	Result ret = 0;
	FsFileSystem *fs = fsdevGetDeviceFileSystem("sdmc");
	if (R_FAILED(ret = fsFsDeleteDirectoryRecursively(fs, path))) {
		return ret;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	std::filesystem::create_directory(PATH);
	if(std::filesystem::exists(BAD_PATH)){
		std::filesystem::remove(BAD_PATH);
	}
	if(std::filesystem::exists(CONFIG_PATH)){
		std::filesystem::remove(FULL_PATH);
		std::filesystem::rename(CONFIG_PATH, FULL_PATH);
	}

	std::filesystem::remove(FORWARDER_PATH);

	envSetNextLoad("switch/AtmoPackUpdater/AtmoPackUpdater.nro", "\"/switch/AtmoPackUpdater/AtmoPackUpdater.nro\"");
	return 0;
}
