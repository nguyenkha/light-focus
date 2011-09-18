#include "stdafx.h"
#include "Application.h"

using namespace SimpleApplication;

Exception::Exception(const String &errorString)
{
	_errorString = errorString;
}

String Exception::toString()
{
	return _errorString;
}

const TCHAR FileInfo::PATH_SEPARATOR = '\\';

HIMAGELIST FileInfo::_largeImageList = 0;

HIMAGELIST FileInfo::_smallImageList = 0;

ImageList *FileInfo::getSystemLargeImageList()
{
	if (_largeImageList == 0) {
		SHFILEINFO info;
		_largeImageList = (HIMAGELIST)SHGetFileInfo(_T("C:\\"), 0, &info, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	}
	return new ImageList(_largeImageList);
}

ImageList *FileInfo::getSystemSmallImageList()
{
	if (_smallImageList == 0) {
		SHFILEINFO info;
		_smallImageList = (HIMAGELIST)SHGetFileInfo(_T("C:\\"), 0, &info, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	}
	return new ImageList(_smallImageList);
}

vector<FileInfo> FileInfo::getDrivesList()
{
	vector<FileInfo> drivesList;
	TCHAR buffer[100];
	LPTSTR pt = buffer;
	int length = GetLogicalDriveStrings(100, buffer);
	int nameLength;
	FileInfo temp;

	while (length > 0) {
		nameLength = _tcslen(pt);
		temp.setFile(pt);
		drivesList.push_back(temp);
		length = length - (nameLength + 1);
		pt = pt + (nameLength + 1);
	}
	return drivesList;
}

FileInfo::FileInfo(const String &file)
{
	setFile(file);
}

FileInfo::FileInfo()
{}

void FileInfo::setFile(const String &file)
{
	_file = file;
	_iconIndex = -1;
	size_t found = file.rfind(PATH_SEPARATOR);
	if (found != String::npos) {
		_fileName = _file.substr(found + 1);
		_path = _file.substr(0, found);
		_isRoot = false;
		// Root
		if (_path.find(PATH_SEPARATOR) == String::npos) {
			_path += PATH_SEPARATOR;
		}
		if (_fileName.length() == 0) {
			_fileName = _path;
			_path.clear();
			_isRoot = true;
		}

		WIN32_FILE_ATTRIBUTE_DATA fileInfo;
		GetFileAttributesEx(_file.c_str(), GetFileExInfoStandard, & fileInfo);
		_isDir = fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		// Addition
		if (isDir()) {
			_extensionName = _T("");
			_fileNameWithoutExtension = _fileName;
			_fileSizeString = _T("<DIR>");
		} else if (isRoot()){
			_fileNameWithoutExtension = _fileName;
			_fileSizeString = _T("");
			_extensionName = _T("");
		} else {
			found = _fileName.rfind(_T("."));
			_extensionName = _fileName.substr(found + 1);
			_fileNameWithoutExtension = _fileName.substr(0, found);
			LARGE_INTEGER largeSize;
			largeSize.LowPart = fileInfo.nFileSizeLow;
			largeSize.HighPart = fileInfo.nFileSizeHigh;
			_fileSize = largeSize.QuadPart;
			
			// Calculate file size
			LONGLONG size = _fileSize/1024;
			String unit = _T(" KB");
			
			TCHAR buffer[50];
			_ui64tot(size, buffer, 10);
			_fileSizeString = buffer;
			_fileSizeString = _fileSizeString + unit;
		}

		_attributesString = _T("----");
		if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			_attributesString[0] = 'r';
		}
		if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			_attributesString[1] = 'a';
		}
		if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			_attributesString[2] = 'h';
		}
		if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			_attributesString[3] = 's';
		}
	}
}

FileInfo FileInfo::getPathInfo() const
{
	FileInfo pathInfo(_path);
	return pathInfo;
}

const String &FileInfo::getFileName() const
{
	return _fileName;
}

const String &FileInfo::getPath() const
{
	return _path;
}

const String &FileInfo::getFile() const
{
	return _file;
}

bool FileInfo::isValid(const String &path)
{
	return true;
}

const String &FileInfo::getExtensionName() const
{
	return _extensionName;
}

const String &FileInfo::getFileNameWithoutExtension() const
{
	return _fileNameWithoutExtension;
}

const String &FileInfo::getFileSizeString() const
{
	return _fileSizeString;
}

const String &FileInfo::getAttributesString() const
{
	return _attributesString;
}

String FileInfo::appendPath(const String &path) const
{
	String pathSeparator;
	if (isRoot()) {
		pathSeparator = _T("");
	} else {
		pathSeparator = PATH_SEPARATOR;
	}
	return _file + pathSeparator + path;
}

int FileInfo::getIconIndex()
{
	// Lazy-load because it's very slow T_T
	if (_iconIndex == -1) {
		SHFILEINFO info;
		if (SHGetFileInfo(_file.c_str(), 0, &info, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES | SHGFI_ICON) == 0) {
			// MessageBox(NULL, _T("Can't open file"), _T("Error!"), MB_OK);
		} else {
			_iconIndex = info.iIcon;
			DestroyIcon(info.hIcon);
		}
	}
	return _iconIndex;
}

bool FileInfo::isDir() const
{
	return _isDir;
}

bool FileInfo::isRoot() const
{
	return _isRoot;
}

vector<FileInfo> FileInfo::getChildren(const String &keyword) const
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	vector<FileInfo> children;
	FileInfo temp;
	String child;
	String search = appendPath(keyword);

	hFind = FindFirstFile(search.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return children;
	}
	do {
		child = appendPath(findFileData.cFileName);
		temp.setFile(child);
		children.push_back(temp);
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);

	if (children.begin()->getFileName() == _T(".")) {
		children.erase(children.begin());
	}

	return children;
}

FileInfo::~FileInfo()
{}

void Application::init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	_handle = hInstance;
	_commandShow = nCmdShow;

	// Init common control
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES |
		ICC_USEREX_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx(&icex);

	// Init GDI plus
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
}

HINSTANCE Application::getHandle()
{
	return _handle;
}

int Application::getCommandShow()
{
	return _commandShow;
}

String Application::loadString(UINT id)
{
	TCHAR buffer[MAX_LOAD_STRING];
	LoadString(_handle, id, buffer, MAX_LOAD_STRING);
	String result(buffer);
	return result;
}

BitmapImage *Application::loadBitmapImage(UINT id)
{
	HBITMAP handle;
	handle = LoadBitmap(_handle, MAKEINTRESOURCE(id));
	return new BitmapImage(handle);
}

Image *Application::loadImage(UINT id, const String &type)
{
	HRSRC resourceHandle = FindResource(_handle, MAKEINTRESOURCE(id), type.c_str());
	DWORD resourceSize = SizeofResource(_handle, resourceHandle);
	const void* resourceData = LockResource(LoadResource(_handle, resourceHandle));
	HGLOBAL buffer = GlobalAlloc(GMEM_MOVEABLE, resourceSize);
	void *lockedBuffer = GlobalLock(buffer);
	CopyMemory(lockedBuffer, resourceData, resourceSize);
	IStream* stream = NULL;
	CreateStreamOnHGlobal(buffer, FALSE, &stream);
	Image *image = new Image(stream);
	stream->Release();
	GlobalUnlock(buffer);
	GlobalFree(buffer);
	return image;
}

Image *Application::loadPNG(UINT id)
{
	return loadImage(id, _T("PNG"));
}

Application::~Application()
{
	// Shutdown GDI+
	GdiplusShutdown(_gdiplusToken);
}

Window::Window(SimpleApplication::Application *application)
{
	_parent = NULL;
	_application = application;
	_canDestroy = true;
}

Window::Window(SimpleApplication::Window *parent)
{
	_parent = parent;
	_application = _parent->_application;
	_canDestroy = true;
}

Application *Window::getApplication()
{
	return _application;
}

HWND Window::getHandle()
{
	return _handle;
}

DWORD Window::getWindowStyles()
{
	return (DWORD)GetWindowLong(_handle, GWL_STYLE);
}

void Window::setWindowStyles(DWORD styles)
{
	SetWindowLong(_handle, GWL_STYLE, (LONG)styles);
}

void Window::addWindowStyles(DWORD styles)
{
	setWindowStyles(getWindowStyles() | styles);
}

void Window::removeWindowStyles(DWORD styles)
{
	setWindowStyles(getWindowStyles() & (~styles));
}

String Window::getWindowText()
{
	int length = GetWindowTextLength(_handle) + 1;
	LPTSTR buffer = new TCHAR[length];
	GetWindowText(_handle, buffer, length);
	String s(buffer);
	delete[] buffer;
	return s;
}

void Window::setWindowText(const String &s)
{
	SetWindowText(_handle, s.c_str());
}

void Window::move(int x, int y, int width, int height, bool repaint)
{
	MoveWindow(_handle, x, y, width, height, repaint);
}

RECT Window::getRect()
{
	RECT info;
	GetWindowRect(_handle, &info);
	return info;
}

RECT Window::getClientRect()
{
	RECT info;
	GetClientRect(_handle, &info);
	return info;
}

Window::~Window()
{
	//if (_canDestroy) {
	//	DestroyWindow(_handle);
	//}
}

void Window::init(DWORD extentedStyles, DWORD styles, const String &windowName, int x, int y, int width, int height, HMENU menu)
{
	HWND parentHandle = NULL;
	String className;
	WNDCLASSEX w;
	className = getWindowClassName();
	if (_parent != NULL) {
		parentHandle = _parent->_handle;
	}
	// Ensure class was registered
	if (!GetClassInfoEx(_application->getHandle(), className.c_str(), &w)) {
		_registerWindowClassName();
	}
	_handle = CreateWindowEx(extentedStyles, className.c_str(), windowName.c_str(), styles, 
		x, y, width, height, parentHandle, menu, _application->getHandle(), this);

	if (!_handle) {
		MessageBox(NULL, _T("Can't create window"), _T("Error!"), MB_OK);
		// Halt
		exit(1);
	}

	// Post init window
	_postInit();
}

bool Window::show(int commandShow)
{
	return ShowWindow(_handle, commandShow);
}

bool Window::update()
{
	return UpdateWindow(_handle);
}

void Window::focus()
{
	SetFocus(_handle);
}

bool Window::isFocus()
{
	return GetFocus() == _handle;
}

Window *Window::getParent()
{
	return _parent;
}

// Do nothing
void Window::_registerWindowClassName()
{}

// Post init window
void Window::_postInit()
{
	/*NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));*/
}

LRESULT Window::sendMessage(Window *w, UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(w->_handle, message, wParam, lParam);
}

LRESULT Window::sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(_handle, message, wParam, lParam);
}

LRESULT CALLBACK CustomWindow::preCreateWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT) lParam;
		// Set window object belong to hWnd and set handle for window object
		CustomWindow *w = (CustomWindow *) cs->lpCreateParams;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG) w);
		w->_handle = hWnd;
		// Set new winproc
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG) CustomWindow::commonWindowProcedure);
		// Dispatch WM_NCCREATE as normal
		return CustomWindow::commonWindowProcedure(hWnd, message, wParam, lParam);
	} else {
		// Use default handling; the window is not associated yet
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT CALLBACK CustomWindow::commonWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Redirect messages to the window procedure of the associated window
	return ((CustomWindow *) GetWindowLong(hWnd, GWL_USERDATA))->windowProcedure(hWnd, message, wParam, lParam);
}

CustomWindow::~CustomWindow()
{}

// const String windowClassName(_T("BUTTON"));

String Button::getWindowClassName()
{
	return WC_BUTTON;
}

void Button::init(String text, int x, int y, int width, int height, int command)
{
	Window::init(NULL, WS_VISIBLE | WS_CHILD, text.c_str(), x, y, width, height, (HMENU)command);
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

Button::~Button()
{}

String CheckBox::getWindowClassName()
{
	return WC_BUTTON;
}

void CheckBox::init(String text, int x, int y, int width, int height)
{
	Window::init(NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, text.c_str(), x, y, width, height, (HMENU)NULL);
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

bool CheckBox::isChecked()
{
	return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void CheckBox::check()
{
	sendMessage(BM_SETCHECK, BST_CHECKED, 0);
}

void CheckBox::uncheck()
{
	sendMessage(BM_SETCHECK, BST_UNCHECKED, 0);
}

CheckBox::~CheckBox()
{}

void ComboBox::init(int x, int y, int width, int height, DWORD additionType)
{
	Window::init(NULL, WS_VISIBLE | WS_CHILD | additionType, _T(""), x, y, width, height, (HMENU)NULL);
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

String ComboBox::getWindowClassName()
{
	return _T("COMBOBOX");
}

void ComboBox::addString(const String &s)
{
	sendMessage(CB_ADDSTRING, 0, (LPARAM)(s.c_str()));
}

int ComboBox::getSelectedIndex()
{
	return (int)sendMessage(CB_GETCURSEL, 0, 0);
}

void ComboBox::setSelectedIndex(int i)
{
	sendMessage(CB_SETCURSEL, (WPARAM)i, 0);
}

ComboBox::~ComboBox()
{}

String EditBox::getWindowClassName()
{
	return _T("EDIT");
}

void EditBox::init(String text, int x, int y, int width, int height)
{
	Window::init(WS_EX_CLIENTEDGE, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, text, x, y, width, height, (HMENU)0);
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

void EditBox::setReadOnly(bool isReadOnly)
{
	if (isReadOnly) {
		sendMessage(EM_SETREADONLY, TRUE, 0);
	} else {
		sendMessage(EM_SETREADONLY, FALSE, 0);
	}
}

EditBox::~EditBox()
{}

StatusBar::StatusBar(Window *parent) : Window(parent) 
{}

String StatusBar::getWindowClassName()
{
	return STATUSCLASSNAME;
}

void StatusBar::setMinHeight(int minHeight)
{
	sendMessage(SB_SETMINHEIGHT, (WPARAM)minHeight, 0);
}

void StatusBar::_updateParts()
{
	int i;
	int numParts = (int)_parts.size();
	int *parts = new int[numParts];
	for (i = 0; i < numParts; i++) {
		parts[i] = _parts[i].width;
	}
	sendMessage(SB_SETPARTS, (WPARAM)numParts, (LPARAM)parts);
	for (i = 0; i < numParts; i++) {
		sendMessage(SB_SETTEXT, MAKEWPARAM(i, 0), (LPARAM)_parts[i].text.c_str());
	}
	delete parts;
}

void StatusBar::addPart(int width, const String &text)
{
	Part p;
	p.width = width;
	p.text = text;
	_parts.push_back(p);
	_updateParts();
}

void StatusBar::removePart(int index)
{
	_parts.erase(_parts.begin() + index);
	_updateParts();
}

void StatusBar::resizePart(int index, int width)
{
	(_parts.begin() + index)->width = width;
	_updateParts();
}

void StatusBar::editPartText(int index, const String &text)
{
	(_parts.begin() + index)->text = text;
	_updateParts();
}

StatusBar::~StatusBar()
{}

BitmapImage::BitmapImage(HBITMAP handle)
{
	_handle = handle;
}

BitmapImage::~BitmapImage()
{
	DeleteObject(_handle);
}

HBITMAP BitmapImage::getHandle()
{
	return _handle;
}

ImageList::ImageList(int width, int height, int num, int grow, bool canDestroy)
{
	_handle = ImageList_Create(width, height, ILC_COLOR32 | ILC_MASK, num, grow);
	_canDestroy = canDestroy;
}

ImageList::ImageList(HIMAGELIST handle)
{
	_handle = handle;
	_canDestroy = false;
}

void ImageList::add(BitmapImage *image)
{
	ImageList_Add(_handle, image->getHandle(), NULL);
}

HIMAGELIST ImageList::getHandle()
{
	return _handle;
}

ImageList::~ImageList()
{
	if (_canDestroy) {
		ImageList_Destroy(_handle);
	}
}

ToolBar::ToolBar(Window *parent) : Window(parent)
{
	_imageList = NULL;
}

void ToolBar::setHeight(int height)
{
	sendMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(0, height));
}

void ToolBar::enableButton(int command)
{
	sendMessage(TB_ENABLEBUTTON, command, MAKELPARAM(TRUE, 0));
}

void ToolBar::disableButton(int command)
{
	sendMessage(TB_ENABLEBUTTON, command, MAKELPARAM(FALSE, 0));
}

String ToolBar::getWindowClassName()
{
	return TOOLBARCLASSNAME;
}

void ToolBar::addButton(int images, int command, const String &text, DWORD styles)
{
	TBBUTTON b = {MAKELONG(images, 0), command, TBSTATE_ENABLED, styles, {0}, 0, (INT_PTR)text.c_str()};
	sendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	sendMessage(TB_INSERTBUTTON, (WPARAM)-1, (LPARAM)&b);
	sendMessage(TB_AUTOSIZE, 0, 0);
}

void ToolBar::setImageList(ImageList *il)
{
	if (_imageList != NULL) {
		delete _imageList;
	}
	_imageList = il;
	sendMessage(TB_SETIMAGELIST, (WPARAM)0, (LPARAM)_imageList->getHandle());
}

void ToolBar::loadStandardButton(int type)
{
	sendMessage(TB_LOADIMAGES, (WPARAM)type, (LPARAM)HINST_COMMCTRL);
}

ToolBar::~ToolBar()
{
	if (_imageList != NULL) {
		delete _imageList;
	}
}

ListView::ListView(Window *parent) : Window(parent)
{
	_smallImageList = NULL;
	_largeImageList = NULL;
}

String ListView::getWindowClassName()
{
	return WC_LISTVIEW;
}

void ListView::addColumn(int index, const String &title, int width)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	lvc.iSubItem = index;
	lvc.cx = width;
    lvc.pszText = (LPTSTR)title.c_str();
	ListView_InsertColumn(_handle, index, &lvc);
}

void ListView::setItemCount(int count)
{
	ListView_SetItemCountEx(_handle, count, 0);
}

void ListView::setSmallImageList(ImageList *il)
{
	if (_smallImageList != NULL) {
		delete _smallImageList;
	}
	_smallImageList = il;
	ListView_SetImageList(_handle, _smallImageList->getHandle(), LVSIL_SMALL);
}

void ListView::setLargeImageList(ImageList *il)
{
	if (_largeImageList != NULL) {
		delete _largeImageList;
	}
	_largeImageList = il;
	ListView_SetImageList(_handle, _largeImageList->getHandle(), LVSIL_NORMAL);
}

ListView::~ListView()
{
	if (_smallImageList != NULL) {
		delete _smallImageList;
	}
	if (_largeImageList != NULL) {
		delete _largeImageList;
	} 
}

void Label::init(String text, int x, int y, int width, int height)
{
	Window::init(NULL, WS_CHILD | WS_VISIBLE, text, x, y, width, height, (HMENU)0);
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, (PVOID)&info, NULL);
	HFONT font = CreateFontIndirect(&(info.lfMessageFont));
	SendMessage(_handle, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

String Label::getWindowClassName()
{
	return WC_STATIC;
}

Label::~Label()
{}

Log::Log(const String &fileName)
{
	_logFile = _tfopen(fileName.c_str(), _T("a"));
}

void Log::write(const String &text)
{
	_fputts(text.c_str(), _logFile);
}

Log::~Log()
{
	fclose(_logFile);
}

WrapperWindow::WrapperWindow(HWND handle) : Window((Application *)NULL)
{
	_handle = handle;
	_canDestroy = false;
}

String WrapperWindow::getWindowClassName()
{
	return _T("");
}

WrapperWindow::~WrapperWindow()
{}

RegistryKey::RegistryKey()
{}

HKEY RegistryKey::_getPredefinedKey(const String &keyName)
{
	HKEY key;
	if (keyName == _T("HKEY_CLASSES_ROOT")) {
		key = HKEY_CLASSES_ROOT;
	} else if (keyName == _T("HKEY_CURRENT_USER")) {
		key = HKEY_CURRENT_USER;
	} else if (keyName == _T("HKEY_LOCAL_MACHINE")) {
		key = HKEY_LOCAL_MACHINE;
	} else if (keyName == _T("HKEY_USERS")) {
		key = HKEY_USERS;
	} else if (keyName == _T("HKEY_CURRENT_CONFIG")) {
		key = HKEY_CURRENT_CONFIG;
	} else {
		key = NULL;
	}
	return key;
}

void RegistryKey::_writeToFile(FILE *f, DWORD &countKey)
{
	// Write key path
	DWORD length = _path.length();
	fwrite(&length, sizeof(DWORD), 1, f);
	fwrite(_path.c_str(), sizeof(TCHAR)*length, 1, f);

	// Write value
	DWORD posCountValue, posCurrent, realCountValue, i, valueNameSize, valueDataSize, type;
	String valueName;
	Value *v;
	realCountValue = 0;
	posCountValue = ftell(f);
	fwrite(&realCountValue, sizeof(DWORD), 1, f);
	for (i = 0; i < _countValue; i++) {
		try {
			// Some value can't read
			v = loadValue(i);
		} catch (Exception &e) {
			// Go to next value
			continue;
		}
		valueName = v->getName();
		valueNameSize = valueName.size();
		valueDataSize = v->getBinarySize();
		fwrite(&valueNameSize, sizeof(DWORD), 1, f);
		if (valueNameSize > 0) {
			fwrite(valueName.c_str(), sizeof(TCHAR)*valueNameSize, 1, f);
		}
		type = v->getType();
		fwrite(&type, sizeof(DWORD), 1, f);
		fwrite(&valueDataSize, sizeof(DWORD), 1, f);
		if (valueDataSize > 0) {
			fwrite(v->getBinary(), sizeof(BYTE)*valueDataSize, 1, f);
		}
		delete v;
		realCountValue++;
	}
	posCurrent = ftell(f);
	fseek(f, posCountValue, SEEK_SET);
	fwrite(&realCountValue, sizeof(DWORD), 1, f);
	fseek(f, posCurrent, SEEK_SET);

	// Write sub key
	RegistryKey *key;
	for (i = 0; i < _countSubKey; i++) {
		try {
			// Some key can't read
			key = loadSubKey(i);
		} catch (Exception &e) {
			// Go to next value
			continue;
		}
		key->_writeToFile(f, countKey);
		delete key;
	}
	countKey++;
}

RegistryKey::RegistryKey(const String &path, DWORD accessLevel)
{
	_path = path;
	// Find path
	size_t found;
	String rootPath, relativePath;
	HKEY rootKey;
	found = _path.find(_T("\\"));
	if (found != String::npos) {
		rootPath = _path.substr(0, found);
		relativePath = _path.substr(found + 1);
	} else {
		rootPath = _path;
		relativePath = _T("");
	}
	// Detect root key
	rootKey = _getPredefinedKey(rootPath);
	// Open key
	DWORD returnCode;
	if (relativePath.length() == 0) {
		_isPredefined = true;
		_handle = rootKey;
	} else {
		_isPredefined = false;
		returnCode = RegOpenKeyEx(rootKey, relativePath.c_str(), 0, accessLevel, &_handle);
		if (returnCode != ERROR_SUCCESS) {
			throw Exception(_T("Can\'t open key"));
		}
	}
	refresh();
}

const String &RegistryKey::getName()
{
	return _T("");
}

HKEY RegistryKey::getHandle()
{
	return _handle;
}

void RegistryKey::refresh()
{
	// Don't use :|
	DWORD maxClassSize, descriptor;
	RegQueryInfoKey(_handle, NULL, NULL, NULL, &_countSubKey, &_maxSubKeySize, &maxClassSize, &
		_countValue, &_maxValueSize, &_maxValueDataSize, &descriptor, NULL);
}

RegistryKey *RegistryKey::loadSubKey(int index, DWORD accessLevel)
{
	HKEY handle;
	RegistryKey *key;
	DWORD keyNameSize = _maxSubKeySize + 1;
	LPTSTR keyName = new TCHAR[keyNameSize];
	String keyPath = _path;
	DWORD returnCode;
	returnCode = RegEnumKeyEx(_handle, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL);
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can't load key"));
	}
	keyPath += _T("\\");
	keyPath += keyName;
	returnCode = RegOpenKeyEx(_handle, keyName, 0, accessLevel, &handle);
	delete[] keyName;
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can't load key"));
	}
	key = new RegistryKey();
	key->_handle = handle;
	key->_path = keyPath;
	key->_isPredefined = false;
	key->refresh();
	return key;
}

RegistryKey *RegistryKey::createSubKey(const String &name)
{
	DWORD returnCode;
	RegistryKey *key = new RegistryKey();
	returnCode = RegCreateKeyEx(_handle, name.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key->_handle, NULL);
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can\'t create sub key"));
	}
	key->_path = _path + _T("\\");
	key->_path += name;
	key->_isPredefined = false;
	key->refresh();
	refresh();
	return key;
}

RegistryKey::Value *RegistryKey::loadValue(int index)
{
	DWORD returnCode, type;
	DWORD valueNameSize = _maxValueSize + 1;
	LPTSTR valueName = new TCHAR[valueNameSize];
	DWORD dataSize = _maxValueDataSize;
	LPBYTE data = new BYTE[dataSize];
	returnCode = RegEnumValue(_handle, index, valueName, &valueNameSize, NULL, &type, data, &dataSize);
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can\'t load value"));
	}
	Value *v = new Value(this, valueName, type);
	v->loadBinary(data, dataSize);
	delete data;
	delete[] valueName;
	return v;
}

RegistryKey::Value *RegistryKey::loadValue(const String &name)
{
	DWORD returnCode, type;
	DWORD dataSize = _maxValueDataSize;
	LPBYTE data = new BYTE[dataSize];
	returnCode = SHGetValue(_handle, _T(""), name.c_str(), &type, data, &dataSize);
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can\'t load value"));
	}
	Value *v = new Value(this, name, type);
	v->loadBinary(data, dataSize);
	delete[] data;
	return v;
}

RegistryKey::Value *RegistryKey::createValue(const String &name, DWORD type)
{
	return new Value(this, name, type);
}

int RegistryKey::countSubKey()
{
	return _countSubKey;
}

int RegistryKey::countValue()
{
	return _countValue;
}

void RegistryKey::deleteTree()
{
	if (!_isPredefined) {
		DWORD returnCode;
		returnCode = SHDeleteKey(_handle, _T(""));
		if (returnCode != ERROR_SUCCESS) {
			throw Exception(_T("Can\'t delete key"));
		}
	}
}

void RegistryKey::saveToFile(const String &fileName)
{
	FILE *f = _tfopen(fileName.c_str(), _T("wb"));
	DWORD countKey = 0;
	fwrite(&countKey, sizeof(DWORD), 1, f);
	_writeToFile(f, countKey);
	rewind(f);
	fwrite(&countKey, sizeof(DWORD), 1, f);
	fclose(f);
}

void RegistryKey::restoreFromFile(const String &fileName)
{
	FILE *f = _tfopen(fileName.c_str(), _T("rb"));
	DWORD keyCount, i, j, valueCount, keySize, valueSize, dataSize, type;
	LPTSTR keyName, valueName;
	LPBYTE data;
	RegistryKey *r;
	RegistryKey *rootKey;
	RegistryKey::Value *v;
	size_t found;
	String rootPath, relativePath, path;
	fread(&keyCount, sizeof(DWORD), 1, f);
	for (i = 0; i < keyCount; i++) {
		fread(&keySize, sizeof(DWORD), 1, f);
		keyName = new TCHAR[keySize + 1];
		if (keySize > 0) {
			fread(keyName, sizeof(TCHAR)*keySize, 1, f);
		}		
		keyName[keySize] = '\0';
	
		// Find path
		path = keyName;
		found = path.find(_T("\\"));
		if (found != String::npos) {
			rootPath = path.substr(0, found);
			relativePath = path.substr(found + 1);
		} else {
			rootPath = path;
			relativePath = _T("");
		}
		// Detect root key
		rootKey = new RegistryKey(rootPath);
		if (relativePath.length() > 0) {
			try {
				r = rootKey->createSubKey(relativePath);
			} catch (Exception &e) {
				try {
					r = new RegistryKey(path);
				} catch (Exception &e1) {
					continue;
				}
			}
		} else {
			r = rootKey;
		}

		delete[] keyName;
		fread(&valueCount, sizeof(DWORD), 1, f);
		// Value max size name :D
		for (j = 0; j < valueCount; j++) {
			fread(&valueSize, sizeof(DWORD), 1, f);
			valueName = new TCHAR[valueSize + 1];
			if (valueSize > 0) {
				fread(valueName, sizeof(TCHAR)*valueSize, 1, f);
			}		
			valueName[valueSize] = '\0';
			fread(&type, sizeof(DWORD), 1, f);
			fread(&dataSize, sizeof(DWORD), 1, f);
			data = new BYTE[dataSize];
			fread(data, sizeof(BYTE)*dataSize, 1, f);
			
			try {
				v = r->createValue(valueName, type);
				v->setBinary(data, dataSize);
			} catch (Exception &e) {
				MessageBox(NULL, _T("Unknown error"), _T("Unknown error"), MB_OK);
				continue;
			}

			delete v;
			delete[] valueName;
			delete[] data;
		}
		delete r;
	}
	fclose(f);
}

RegistryKey::~RegistryKey()
{
	if (!_isPredefined) {
		RegCloseKey(_handle);
	}
}

RegistryKey::Value::Value(RegistryKey *key, const String &name, DWORD type)
{
	_key = key;
	_size = 0;
	_name = name;
	_type = type;
}

String RegistryKey::Value::getName()
{
	return _name;
}

bool RegistryKey::Value::isString()
{
	return _type == REG_SZ;
}

String RegistryKey::Value::getString()
{
	return (LPTSTR)_data;
}

void RegistryKey::Value::setString(const String &data)
{
	setBinary((LPBYTE)data.c_str(), sizeof(TCHAR)*(data.length() + 1));
}

bool RegistryKey::Value::isDword()
{
	return _type == REG_DWORD;
}

DWORD RegistryKey::Value::getDword()
{
	return *((LPDWORD)_data);
}

void RegistryKey::Value::setDword(DWORD data)
{
	setBinary((LPBYTE)&data, sizeof(DWORD));
}

DWORD RegistryKey::Value::getBinarySize()
{
	return _size;
}

bool RegistryKey::Value::isBinary()
{
	return _type == REG_BINARY;
}

LPBYTE RegistryKey::Value::getBinary()
{
	return _data;
}

void RegistryKey::Value::loadBinary(LPBYTE data, DWORD size)
{
	if (_size > 0) {
		delete[] _data;
	}
	_size = size;
	_data = new BYTE[_size];
	for (int i = 0; i < _size; i++) {
		_data[i] = data[i];
	}
}

void RegistryKey::Value::setBinary(LPBYTE data, DWORD size)
{
	loadBinary(data, size);
	_save();
}

void RegistryKey::Value::_save()
{
	DWORD returnCode;
	returnCode = SHSetValue(_key->getHandle(), _T(""), _name.c_str(), _type, _data, _size);
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can\'t save value"));
	}
}

void RegistryKey::Value::deleteValue()
{
	DWORD returnCode;
	returnCode = SHDeleteValue(_key->getHandle(), _T(""), _name.c_str());
	if (returnCode != ERROR_SUCCESS) {
		throw Exception(_T("Can\'t delete key"));
	}
}

DWORD RegistryKey::Value::getType()
{
	return _type;
}

RegistryKey::Value::~Value()
{
	if (_size > 0) {
		delete _data;
	}
}
