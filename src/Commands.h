#pragma once
#include "Common.h"
#include "MapViewer.h"
#include "FileTypes/BbsPmp.h"

class ICommand
{
public:
	ICommand();
	virtual ~ICommand();
	virtual void Execute(MapViewer* app) = 0;
};

class StartOpenMapCommand : public ICommand
{
public:
	StartOpenMapCommand();
	virtual ~StartOpenMapCommand();
	virtual void Execute(MapViewer* app) override;
};

class OpenMapCommand : public ICommand
{
public:
	OpenMapCommand(std::string path);
	virtual ~OpenMapCommand();
	virtual void Execute(MapViewer* app) override;
private:
	std::string mPath;
};