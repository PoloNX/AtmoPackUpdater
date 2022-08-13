#include <filesystem>
#include <iostream>
#include <string>

#include <switch.h>

#define PATH				"/switch/AtmoPackUpdater/"
#define FULL_PATH   		"/switch/AtmoPackUpdater/AtmoPackUpdater.nro"
#define BAD_PATH   			"/switch/AtmoPackUpdater.nro"
#define FORWARDER_PATH	  	"/config/AtmoPackUpdater/amssu-forwarder.nro"
#define APP_DOWNLOAD_PATH 	"/config/AtmoPackUpdater/app.nro"

int main()
{
	std::cout<< "CREATE DIRECTORY PATH" << std::endl;
	consoleUpdate(NULL);
	std::filesystem::create_directory(PATH);
	if(std::filesystem::exists(BAD_PATH) && std::filesystem::exists(FULL_PATH)){
		std::cout<< "REMOVE BAD PATH" << std::endl;
		consoleUpdate(NULL);
		std::filesystem::remove(BAD_PATH);
	}

	if(std::filesystem::exists(BAD_PATH) && !std::filesystem::exists(FULL_PATH)){
		std::cout<< "REMOVE BAD PATH" << std::endl;
		consoleUpdate(NULL);
		std::filesystem::rename(BAD_PATH, FULL_PATH);
	}

	if(std::filesystem::exists(APP_DOWNLOAD_PATH)) {
		std::cout<< "RENAME DOWNLOAD PATH" << std::endl;
		consoleUpdate(NULL);
		std::filesystem::remove(FULL_PATH);
		std::filesystem::rename(APP_DOWNLOAD_PATH, FULL_PATH);
		std::filesystem::remove(APP_DOWNLOAD_PATH);
	}

	std::cout << "REMOVES" << std::endl;
	consoleUpdate(NULL);

	std::filesystem::remove(FORWARDER_PATH);

	std::cout << "ENV" << std::endl;
	consoleUpdate(NULL);
	envSetNextLoad("switch/AtmoPackUpdater/AtmoPackUpdater.nro", "\"/switch/AtmoPackUpdater/AtmoPackUpdater.nro\"");
	std::cout << "RETURN" << std::endl;
	consoleUpdate(NULL);
	return 0;
}
