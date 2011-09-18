#ifndef _SIMPLE_APPLICATION
#define _SIMPLE_APPLICATION

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment (lib,"Gdiplus.lib")

#include <string>
#include <vector>
#include <algorithm>
#include <commctrl.h>
#include <Uxtheme.h>
#include <Shellapi.h>
#include <Shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <gdiplus.h>

#include "MessageMap.h"

namespace SimpleApplication
{

using namespace std;
using namespace Gdiplus;

#ifdef _UNICODE
#define String wstring
#else
#define String string
#endif

class Exception
{
protected:
	String _errorString;
public:
	Exception(const String &errorString);
	String toString();
};



#define MAX_LOAD_STRING 200

//==== BITMAP ====
class BitmapImage
{
protected:
	HBITMAP _handle;
public:
	BitmapImage(HBITMAP handle);
	HBITMAP getHandle();
	~BitmapImage();
};

//==== IMAGE LIST ====
class ImageList
{
protected:
	HIMAGELIST _handle;
	bool _canDestroy;
public:
	ImageList(HIMAGELIST handle);
	ImageList(int width, int height, int num, int grow, bool canDestroy = true);
	void add(BitmapImage *image);
	HIMAGELIST getHandle();
	~ImageList();
};

//==== FILEINFO ====
class FileInfo
{
protected:
	static HIMAGELIST _largeImageList;
	static HIMAGELIST _smallImageList;

	String _fileName;
	String _path;
	String _file;
	LONGLONG _fileSize;

	// Addition
	String _extensionName;
	String _fileNameWithoutExtension;
	String _fileSizeString;
	String _attributesString;

	int _iconIndex;
	bool _isDir;
	bool _isRoot;
public:
	static const TCHAR PATH_SEPARATOR;
	static ImageList *getSystemSmallImageList();
	static ImageList *getSystemLargeImageList();
	static vector<FileInfo> getDrivesList();
	static bool isValid(const String &path);

	FileInfo();
	FileInfo(const String &file);
	void setFile(const String &file);
	FileInfo getPathInfo() const;
	const String &getFileName() const;
	const String &getPath() const;
	const String &getFile() const;
	
	// Addition
	const String &getExtensionName() const;
	const String &getFileNameWithoutExtension() const;
	const String &getFileSizeString() const;
	const String &getAttributesString() const;

	String appendPath(const String &path) const;

	int getIconIndex();
	bool isDir() const;
	bool isRoot() const;
	vector<FileInfo> getChildren(const String &keyword = _T("*")) const;
	virtual ~FileInfo();
};

//==== APPLICATION ====
class Application
{
protected:
	/* Application handle to instance, API */
	HINSTANCE _handle;

	/* GDI+ token */
	ULONG_PTR _gdiplusToken;

	/* Command show, init applicaiton */
	int _commandShow;
public:
	/* Winmain proxy */
	void init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
	
	/* API compatible */
	HINSTANCE getHandle();
	int getCommandShow();

	/* Resource */
	String loadString(UINT id);

	BitmapImage *loadBitmapImage(UINT id);

	/* GDI+ utility */
	Image *loadImage(UINT id, const String &type);

	Image *loadPNG(UINT id);

	/* Run application */
	virtual int run()=0;

	/* Destructor */
	virtual ~Application();
};

class Window
{
protected:
	/* Handle to window, API */
	HWND _handle;

	/* Application belong to */
	Application *_application;

	/* Register window classname if necessary */
	virtual void _registerWindowClassName();

	/* Post init window */
	virtual void _postInit();

	/* Window belong to */
	Window *_parent;

	/* Handle to menu, API */
	HMENU _menuId;

	bool _canDestroy;
public:
	/* Constructor */
	Window(Application *application);
	Window(Window *parent);

	/* Create window */
	void init(DWORD extendedStyes, DWORD styles, const String &windowName, int x, int y, int width, int height, HMENU menu);
	virtual String getWindowClassName()=0;

	/* Utility */
	Application *getApplication();
	HWND getHandle();
	bool show(int commandShow);
	bool update();
	void focus();
	bool isFocus();
	Window *getParent();
	DWORD getWindowStyles();
	void setWindowStyles(DWORD styles);
	void addWindowStyles(DWORD styles);
	void removeWindowStyles(DWORD styles);
	String getWindowText();
	void setWindowText(const String &s);
	void move(int x, int y, int width, int height, bool repaint = true);
	RECT getRect();
	RECT getClientRect();
	static LRESULT sendMessage(Window *w, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam);

	/* Destructor */
	virtual ~Window();
};

class WrapperWindow : public Window
{
public:
	WrapperWindow(HWND handle);
	String getWindowClassName();
	virtual ~WrapperWindow();
};

class CustomWindow : public Window
{
protected:
	virtual void _registerWindowClassName()=0;
public:
	CustomWindow(Application *application) : Window(application) {}
	CustomWindow(Window *parent) : Window(parent) {}
	static LRESULT CALLBACK commonWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK preCreateWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)=0;
	virtual String getWindowClassName()=0;
	virtual ~CustomWindow();
};

class Label : public Window
{
public:
	void init(String text, int x, int y, int width, int height);
	Label(Window *parent) : Window(parent) {}
	String getWindowClassName();
	virtual ~Label();
};

class Button : public Window
{
public:
	void init(String text, int x, int y, int width, int height, int command);
	Button(Window *parent) : Window(parent) {}
	String getWindowClassName();
	virtual ~Button();
};

class CheckBox : public Window
{
public:
	void init(String text, int x, int y, int width, int height);
	CheckBox(Window *parent) : Window(parent) {}
	String getWindowClassName();
	bool isChecked();
	void check();
	void uncheck();
	virtual ~CheckBox();
};

class ComboBox : public Window
{
public:
	void init(int x, int y, int width, int height, DWORD additionType);
	ComboBox(Window *parent) : Window(parent) {}
	String getWindowClassName();
	void addString(const String &s);
	int getSelectedIndex();
	void setSelectedIndex(int i);
	virtual ~ComboBox();
};

class EditBox : public Window
{
public:
	EditBox(Window *parent) : Window(parent) {}
	void init(String text, int x, int y, int width, int height);
	String getWindowClassName();
	void setReadOnly(bool isReadOnly);
	virtual ~EditBox();
};

//==== STATUS BAR ====
class StatusBar : public Window
{
protected:
	typedef struct
	{
		int width;
		String text;
	} Part;
	vector<Part> _parts;
	void _updateParts();
public:
	/* CONST */
	static const int EXTEND_PART = -1;
	StatusBar(Window *parent);
	String getWindowClassName();
	void setMinHeight(int minHeight);
	void addPart(int width, const String &text);
	void removePart(int index);
	void resizePart(int index, int width);
	void editPartText(int index, const String &text);
	virtual ~StatusBar();
};

//==== TOOLBAR ====
class ToolBar : public Window
{
protected:
	ImageList *_imageList;
public:
	ToolBar(Window *parent);
	String getWindowClassName();
	void addButton(int images, int command, const String &text, DWORD styles);
	void setImageList(ImageList *il);
	void loadStandardButton(int type);
	void setHeight(int height);
	void enableButton(int command);
	void disableButton(int command);
	virtual ~ToolBar();
};

//==== LISTVIEW ====
class ListView : public Window
{
protected:
	ImageList *_smallImageList;
	ImageList *_largeImageList;
public:
	ListView(Window *parent);
	String getWindowClassName();
	void setSmallImageList(ImageList *il);
	void setLargeImageList(ImageList *il);
	void addColumn(int index, const String &title, int width);
	void setItemCount(int count);
	virtual ~ListView();
};

class Log
{
protected:
	FILE *_logFile;
public:
	Log(const String &fileName);
	void write(const String &text);
	~Log();
};

class RegistryKey
{
protected:
	HKEY _handle;
	bool _isPredefined;
	String _path;
	void _writeToFile(FILE *f, DWORD &countKey);
	RegistryKey();
	static HKEY _getPredefinedKey(const String &keyName);
	
	// Info
	DWORD _countSubKey;
	DWORD _countValue;
	DWORD _maxSubKeySize;
	DWORD _maxValueSize;
	DWORD _maxValueDataSize;
public:
	class Value;
	RegistryKey(const String &path, DWORD accessLevel = KEY_ALL_ACCESS);
	HKEY getHandle();
	RegistryKey *loadSubKey(int index, DWORD accessLevel = KEY_ALL_ACCESS);
	RegistryKey *createSubKey(const String &name);
	Value *loadValue(int index);
	Value *loadValue(const String &name = _T(""));
	Value *createValue(const String &name, DWORD type);
	const String &getName();
	int countSubKey();
	int countValue();
	void refresh();
	void deleteTree();
	void saveToFile(const String &fileName);
	static void restoreFromFile(const String &fileName);
	~RegistryKey();
};

class RegistryKey::Value
{
protected:
	RegistryKey *_key;
	LPBYTE _data;
	DWORD _size;
	DWORD _type;
	String _name;
	void _save();
public:
	Value(RegistryKey *key, const String &name, DWORD type);
	void setType(DWORD type);
	String getName();
	bool isString();
	String getString();
	void setString(const String &data);
	bool isDword();
	DWORD getDword();
	void setDword(DWORD data);
	bool isBinary();
	DWORD getBinarySize();
	LPBYTE getBinary();
	void setBinary(LPBYTE data, DWORD size);
	void loadBinary(LPBYTE data, DWORD size);
	void deleteValue();
	DWORD getType();
	~Value();
};

}

#endif