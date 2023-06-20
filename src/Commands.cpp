#include "Commands.h"

ICommand::ICommand() {}

ICommand::~ICommand() {}

StartOpenMapCommand::StartOpenMapCommand() {}

StartOpenMapCommand::~StartOpenMapCommand() {}

void StartOpenMapCommand::Execute(MapViewer* app)
{
	std::string newFile;
	if (app->GetFileManager()->OpenFileWindow(newFile))
		app->QueueCommand(new OpenMapCommand(newFile));
}

OpenMapCommand::OpenMapCommand(std::string path) : mPath(path) {}

OpenMapCommand::~OpenMapCommand() {}

void OpenMapCommand::Execute(MapViewer* app)
{
	app->GetScene()->theMap->Clear();
	app->GetScene()->theMap->LoadMapFile(mPath);
}