#pragma once
#include "Common.h"
#include "Core/CApplication.h"
#include "FileManager.h"
#include "BBS/CScene.h"
#include "Commands.h"

class MapViewer : public CApplication
{
public:
	MapViewer(int argc, char** argv);
	~MapViewer();

	FileManager* GetFileManager();
	BBS::CScene* GetScene();

	void QueueCommand(ICommand* cmd);
protected:
	virtual bool UserPreInit();
	virtual bool UserInit();
	virtual bool UserPostInit();
	virtual void UserPreUpdate();
	virtual void UserUpdate();
	virtual void UserDraw();
	virtual void UserPostPresent();
	virtual void UserDestroy();
};