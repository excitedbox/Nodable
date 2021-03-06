#include "File.h"
#include "History.h"
#include "FileView.h"
#include "ContainerView.h"
#include "Container.h"
#include "Parser.h"
#include "View.h"
#include "Variable.h"
#include "Log.h"

#include "IconFontCppHeaders/IconsFontAwesome5.h"

#include <Windows.h>
#include <SDL.h>
#include <SDL2\include\SDL_syswm.h>
#include <fstream>

using namespace Nodable;

Nodable::File::File(
	const char* _path,
	const char* _content,
	const char* _name)
{	
	path = _path;
	name = _name;

	/*
		Creates the FileView
	*/
	auto fileView = new FileView();
	addComponent("view", fileView);
	fileView->init();
	fileView->setText(_content);
	auto textEditor = fileView->getTextEditor();

	/*
		Creates an history for UNDO/REDO
	*/
	auto history = new History();
	addComponent("history", history);
    auto undoBuffer = history->createTextEditorUndoBuffer(textEditor);
	fileView->setUndoBuffer(undoBuffer);
	
	/*
		Creates a node container
	*/
	auto container = new Container;
	addComponent("container", container);
	container->addComponent("view", new ContainerView);
	container->setOwner(this);

}

void File::save()
{
	if (modified) {
		std::ofstream fileStream(this->path.c_str());
		auto view = getComponent("view")->getAs<FileView*>();
		auto content = view->getText();
		fileStream.write( content.c_str(), content.size());
		modified = false;
		LOG_DBG("File %s saved\n", name.c_str());
	}
	else {
		LOG_DBG("File %s saving ignored because not modified\n", name.c_str());
	}
	
}

File* File::CreateFileWithPath(const char* _filePath)
{
	/*
		Creates the File
	*/

	// Sanitize path to get only slashes, and no antislashes
	std::string cleanedFilePath(_filePath);
	std::replace(cleanedFilePath.begin(), cleanedFilePath.end(), '/', '\\');

	// Extract file name from filePath
	std::string name = cleanedFilePath;
	auto firstSlashPosition = cleanedFilePath.find_last_of('\\');
	if (firstSlashPosition != std::string::npos)
		name = cleanedFilePath.substr(firstSlashPosition + 1, cleanedFilePath.size() - firstSlashPosition - 1);

	std::ifstream fileStream(cleanedFilePath.c_str());

	if (!fileStream.is_open())
	{
		LOG_MSG("Unable to load \"%s\"\n", cleanedFilePath.c_str());
		return nullptr;
	}

	LOG_MSG("Loading \"%s\"\n", cleanedFilePath.c_str());
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(cleanedFilePath.c_str(), content.c_str(), name.c_str());


	return file;
}

std::string File::BrowseForFileAndReturnItsAbsolutePath(SDL_Window* currentWindow)
{
	OPENFILENAME ofn;                             // common dialog box structure
	char szFile[512];                             // buffer for file name            
	HANDLE hf;                                    // file handle

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(currentWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;           // owner sdlWindow

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn))
		return szFile;

	return "";
}

bool File::evaluateExpression(std::string& _expression)
{
	LOG_MSG("Application::eval() - create a variable.\n");

	auto var = getContainer()->createNodeVariable(ICON_FA_CODE);
	reinterpret_cast<View*>( var->getComponent("view"))->setVisible(false);

	LOG_DBG("Parser::eval() - assign the expression string to that variable\n");
	var->setValue(_expression);

	LOG_DBG("Parser::eval() - check if expression is not empty\n");
	if (var->isSet())
	{
		/* Create a Parser node. The Parser will cut expression string into tokens
		(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
		LOG_DBG("Parser::eval() - create a Parser with the expression string\n");
		auto Parser = getContainer()->createNodeParser(var);
		return Parser->eval();
		//container->destroyNode(Parser);
	}

	return false;
}

bool File::update() {

	auto hasChanged = getContainer()->update();
	
	if (!hasChanged)
		return false;

	auto result		= getContainer()->getResultVariable();

	if (!result) {
		LOG_DBG("Container has no result variable, unable to update text portion.");
		return false;
	}

	auto member		= result->getValueMember();
	auto expression = member->getSourceExpression();
	auto view		= getComponent("view")->getAs<FileView*>();

	view->replaceSelectedText(expression);
	
	return true;
}

bool File::evaluateSelectedExpression()
{
	bool success;

	getContainer()->clear();

	auto view = getComponent("view")->getAs<FileView*>();

	auto expression = view->getSelectedText();
	success = evaluateExpression(expression);

	return success;
}