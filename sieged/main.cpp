#include "Sieged.h"
#include <blib/util/FileSystem.h>
#include <blib/Util.h>

#pragma comment(lib, "blib.lib")

int main(int argc, char* argv[])
{
	blib::util::fixConsole();
	blib::util::FileSystem::registerHandler(new blib::util::PhysicalFileSystemHandler(""));
	blib::util::FileSystem::registerHandler(new blib::util::PhysicalFileSystemHandler("../"));
	blib::util::FileSystem::registerHandler(new blib::util::PhysicalFileSystemHandler("../blib/"));
	blib::util::Thread::setMainThread();
	blib::App* app = new Sieged();
	app->start();
	delete app;

	return -1;	
}