// Windows.h is included above Common.h so we can be sure we get the full Windows.h and not a reduced version
// that may be indirectly included by any library headers
#include <Windows.h>

#include "Common.h"
#include "FileManager.h"

#pragma comment(lib, "Comdlg32") // Needed for GetOpenFileName

namespace fs = std::filesystem;

bool dirContains(const fs::path& directory, const std::string& name)
{
	if (!fs::is_directory(directory)) return false;

	for (auto dirItr = fs::directory_iterator(directory); dirItr != fs::directory_iterator(); dirItr++)
	{
		if (dirItr->is_directory() && dirItr->path().filename() == name)
		{
			return true;
		}
	}

	return false;
}

FileManager::FileManager()
{
	fs::path currentPath = std::filesystem::current_path();
	std::string pathString = currentPath.string();
	std::cout << "[FS] Starting from path " << pathString << std::endl;

	bool foundResouces = dirContains(currentPath, "resources");
	if (!foundResouces)
	{
		if (currentPath.filename() == "bin")
		{
			currentPath = currentPath.parent_path();
			std::filesystem::current_path(currentPath);
			foundResouces = dirContains(currentPath, "resources");
		}

		if (!foundResouces)
		{
			// TODO: multiplatform.
			// Finding the exe path is a non-trival problem
			wchar_t pathBuf[MAX_PATH];
			GetModuleFileName(NULL, pathBuf, MAX_PATH);
			fs::path trialPath = fs::path(pathBuf).parent_path();
			if (trialPath.filename() == "bin")
			{
				trialPath = trialPath.parent_path();
			}
			if (dirContains(trialPath, "resources"))
			{
				currentPath = trialPath;
				fs::current_path(trialPath);
				foundResouces = true;
			}
			
		}
	}

	if (foundResouces)
	{
		rootPath = fs::canonical(currentPath);
		resourcesPath = fs::canonical(currentPath.append("resources"));

		std::cout << "[FS] Root found at " << rootPath << std::endl;
		std::cout << "[FS] Resources found at " << resourcesPath << std::endl;
	}
	else
	{
		MessageBox(NULL, TEXT("FATAL ERROR: Failed to find resources path\nPlease check working directory."), TEXT("FATAL ERORR"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
		std::exit(-1);
	}

	// TODO: VFS ???

}

std::string FileManager::GetFontPath(const std::string& fontName)
{
	fs::path filePath = resourcesPath;
	filePath.append("fonts").append(fontName);
	if (fs::exists(filePath))
		return filePath.string();

	std::cerr << "[FS] FAILED TO FIND FONT " << fontName << std::endl;
	return std::string("");
}

// TODO: dedupe this logic
std::string FileManager::GetShaderPath(const std::string& shaderName)
{
	fs::path filePath = resourcesPath;
	filePath.append("shaders").append(shaderName);
	if (fs::exists(filePath))
		return filePath.string();

	std::cerr << "[FS] FAILED TO FIND SHADER " << shaderName << std::endl;
	return std::string("");
}

void FileManager::GetFontCPath(const std::string& fontname, char* buffer, size_t buffer_len)
{
	std::string path = GetFontPath(fontname);
	memcpy_s(buffer, buffer_len, path.c_str(), path.length());
	buffer[buffer_len - 1] = '\0';
}

void FileManager::GetShaderCPath(const std::string& shaderName, char* buffer, size_t buffer_len)
{
	std::string path = GetShaderPath(shaderName);
	memcpy_s(buffer, buffer_len, path.c_str(), path.length());
	buffer[buffer_len - 1] = '\0';
}

bool FileManager::OpenFileWindow(std::string& out_filePath)
{
	fs::path orignalPath = fs::current_path();
	
	TCHAR filename[MAX_PATH];
	OPENFILENAME ofn;

	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = TEXT("All Files\0*.*\0\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = resourcesPath.c_str();
	ofn.lpstrTitle = TEXT("Select a File");
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("pmp");
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn))
	{
		//out_filePath = std::string(filename);
		fs::path openPath = fs::path(filename);
		openPath = fs::absolute(openPath);
		out_filePath = openPath.string();
		fs::current_path(orignalPath);
		std::cout << "[FS] Open file: " << openPath << std::endl;
		return true;
	}
	else
	{
		switch (CommDlgExtendedError())
		{
			case CDERR_DIALOGFAILURE: std::cout << "[FS] CDERR_DIALOGFAILURE\n"; break;
			case CDERR_FINDRESFAILURE: std::cout << "[FS] CDERR_FINDRESFAILURE\n"; break;
			case CDERR_INITIALIZATION: std::cout << "[FS] CDERR_INITIALIZATION\n"; break;
			case CDERR_LOADRESFAILURE: std::cout << "[FS] CDERR_LOADRESFAILURE\n"; break;
			case CDERR_LOADSTRFAILURE: std::cout << "[FS] CDERR_LOADSTRFAILURE\n"; break;
			case CDERR_LOCKRESFAILURE: std::cout << "[FS] CDERR_LOCKRESFAILURE\n"; break;
			case CDERR_MEMALLOCFAILURE: std::cout << "[FS] CDERR_MEMALLOCFAILURE\n"; break;
			case CDERR_MEMLOCKFAILURE: std::cout << "[FS] CDERR_MEMLOCKFAILURE\n"; break;
			case CDERR_NOHINSTANCE: std::cout << "[FS] CDERR_NOHINSTANCE\n"; break;
			case CDERR_NOHOOK: std::cout << "[FS] CDERR_NOHOOK\n"; break;
			case CDERR_NOTEMPLATE: std::cout << "[FS] CDERR_NOTEMPLATE\n"; break;
			case CDERR_STRUCTSIZE: std::cout << "[FS] CDERR_STRUCTSIZE\n"; break;
			case FNERR_BUFFERTOOSMALL: std::cout << "[FS] FNERR_BUFFERTOOSMALL\n"; break;
			case FNERR_INVALIDFILENAME: std::cout << "[FS] FNERR_INVALIDFILENAME\n"; break;
			case FNERR_SUBCLASSFAILURE: std::cout << "[FS] FNERR_SUBCLASSFAILURE\n"; break;
			default: std::cout << "[FS] File open cancelled.\n";
		}

		fs::current_path(orignalPath);
		return false;
	}

}