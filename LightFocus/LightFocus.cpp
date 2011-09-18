#include "stdafx.h"
#include "LightFocus.h"

MainWindow::MainWindow(Application *application) : CustomWindow(application)
{
	Window::init(NULL, WS_OVERLAPPEDWINDOW, _application->loadString(IDS_APP_TITLE), CW_USEDEFAULT, 0, 400, 300, NULL);

	// Load options
	loadOptions();

	// Never show this window
	show(SW_HIDE);

	// Init notify icon
	NOTIFYICONDATA createStruct;
	createStruct.cbSize = sizeof(NOTIFYICONDATA);
	createStruct.hWnd = _handle;
	createStruct.uID = 0;
	createStruct.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	createStruct.uCallbackMessage = UWM_NOTIFY_ICON;
	createStruct.hIcon = LoadIcon(_application->getHandle(), MAKEINTRESOURCE(IDI_LIGHTFOCUS));
	_tcscpy(createStruct.szTip, _T("Light Focus - A screenshot program")); // name of program for icon in system tray
	Shell_NotifyIcon(NIM_ADD, &createStruct); // NIM_ADD : flag for adding icon into system tray, createStruct is struct of icon to add.

	// Create notify icon menu
	_notifyIconMenu = CreatePopupMenu();
	InsertMenu(_notifyIconMenu, 0, MF_BYPOSITION | MF_STRING, IDM_ABOUT, _T("About"));
	InsertMenu(_notifyIconMenu, 1, MF_BYPOSITION | MF_STRING, IDM_OPEN_FODLER, _T("Open strorage Folder"));
	InsertMenu(_notifyIconMenu, 2, MF_BYPOSITION | MF_STRING, IDM_OPTIONS, _T("Options"));
	InsertMenu(_notifyIconMenu, 3, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
	InsertMenu(_notifyIconMenu, 4, MF_BYPOSITION | MF_STRING, IDM_EXIT, _T("Exit"));
	InsertMenu(_notifyIconMenu, 5, MF_BYPOSITION | MF_STRING, IDM_PRINT_SCREEN, _T("Take a screenshot"));

	// Init background window
	_backgroundWindow = new BackgroundWindow(this);

	// Init overlay window
	_overlayWindow = new OverlayWindow(this);

	_optionsWindow = NULL;
}

void MainWindow::loadOptions()
{
	RegistryKey *r;
	RegistryKey::Value *v;
	try {
		r = new RegistryKey(_T("HKEY_CURRENT_USER\\Software\\Light Focus"));
	} catch (Exception &e) {
		r = registerOptions();
	}
	v = r->loadValue(_T("SavePath"));
	_savePath = v->getString();
	delete v;
	v = r->loadValue(_T("HookPrintScreen"));
	if (v->getDword() == 1) {
		setHook(_handle);
		_isHookPrintScreen = true;
	} else {
		_isHookPrintScreen = false;
	}
	delete v;
	v = r->loadValue(_T("IncludeBackground"));
	if (v->getDword() == 1) {
		_isIncludeBackground = true;
	} else {
		_isIncludeBackground = false;
	}
	delete v;
	v = r->loadValue(_T("Delay"));
	_delay = v->getDword();
	delete v;
	v = r->loadValue(_T("SaveType"));
	_saveType = v->getDword();
	delete v;
	v = r->loadValue(_T("CaptureMode"));
	_captureMode = v->getDword();
	delete v;
	delete r;
}

void MainWindow::saveOptions()
{
	RegistryKey *r;
	RegistryKey::Value *v;
	r = new RegistryKey(_T("HKEY_CURRENT_USER\\Software\\Light Focus"));
	v = r->loadValue(_T("SavePath"));
	v->setString(_savePath);
	delete v;
	v = r->loadValue(_T("HookPrintScreen"));
	if (_isHookPrintScreen) {
		v->setDword(1);
	} else {
		v->setDword(0);
	}
	delete v;
	v = r->loadValue(_T("IncludeBackground"));
	if (_isIncludeBackground) {
		v->setDword(1);
	} else {
		v->setDword(0);
	}
	delete v;
	v = r->loadValue(_T("Delay"));
	v->setDword(_delay);
	delete v;
	v = r->loadValue(_T("SaveType"));
	v->setDword(_saveType);
	delete v;
	v = r->loadValue(_T("CaptureMode"));
	v->setDword(_captureMode);
	delete v;
	delete r;
}

RegistryKey *MainWindow::registerOptions()
{
	RegistryKey *r;
	RegistryKey *software = new RegistryKey(_T("HKEY_CURRENT_USER\\Software"));
	r = software->createSubKey(_T("Light Focus"));
	delete software;
	RegistryKey::Value *v;
	v = r->createValue(_T("SavePath"), REG_SZ);
	v->setString(_T("C:\\"));
	delete v;
	v = r->createValue(_T("HookPrintScreen"), REG_DWORD);
	v->setDword(1);
	delete v;
	v = r->createValue(_T("IncludeBackground"), REG_DWORD);
	v->setDword(0);
	delete v;
	v = r->createValue(_T("Delay"), REG_DWORD);
	v->setDword(0);
	delete v;
	v = r->createValue(_T("SaveType"), REG_DWORD);
	v->setDword(1);
	delete v;
	v = r->createValue(_T("CaptureMode"), REG_DWORD);
	v->setDword(1);
	delete v;
	r->refresh();
	return r;
}

bool MainWindow::isHookPrintScreen()
{
	return _isHookPrintScreen;
}

bool MainWindow::isIncludeBackground()
{
	return _isIncludeBackground;
}

int MainWindow::getDelay()
{
	return _delay;
}

String MainWindow::getSavePath()
{
	return _savePath;
}

int MainWindow::getSaveType()
{
	return _saveType;
}

int MainWindow::getCaptureMode()
{
	return _captureMode;
}

void MainWindow::setHookPrintScreen(bool isHookPrintScreen)
{
	if (_isHookPrintScreen != isHookPrintScreen) {
		if (_isHookPrintScreen) {
			clearHook();
		} else {
			setHook(_handle);
		}
	}
	_isHookPrintScreen = isHookPrintScreen;
}

void MainWindow::setIncludeBackground(bool isIncludeBackground)
{
	_isIncludeBackground = isIncludeBackground;
}

void MainWindow::setDelay(int delay)
{
	_delay = delay;
}

void MainWindow::setSavePath(const String &savePath)
{
	_savePath = savePath;
}

void MainWindow::setSaveType(int type)
{
	_saveType = type;
}

void MainWindow::setCaptureMode(int mode)
{
	_captureMode = mode;
}

MainWindow::~MainWindow()
{
	// Delete notify icon
	NOTIFYICONDATA createStruct;
	createStruct.cbSize = sizeof(NOTIFYICONDATA);
	createStruct.hWnd = _handle;
	createStruct.uID = 0;
	Shell_NotifyIcon(NIM_DELETE, &createStruct);

	// Delete menu
	DestroyMenu(_notifyIconMenu);

	// Delete background window
	delete _backgroundWindow;

	// Clear hook
	clearHook();
}

void MainWindow::_registerWindowClassName()
{
	WNDCLASSEX wcex;

	String windowClassName = getWindowClassName();

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CustomWindow::preCreateWindowProcedure;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _application->getHandle();
	wcex.hIcon			= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LIGHTFOCUS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= 0; //MAKEINTRESOURCE(IDC_LIGHTFOCUS)
	wcex.lpszClassName	= windowClassName.c_str();
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);
}

String MainWindow::getWindowClassName()
{
	return _T("LIGHTFOCUS_MAINCLASS");
}

LRESULT MainWindow::onPrintAndSave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (_delay > 0) {
		Sleep(_delay * 1000);
	}
	Bitmap *screenShot;
	CLSID imageClsid;
	if (_captureMode == 0) {
		RECT r;
		GetWindowRect(GetDesktopWindow(), &r);
		HDC screenDC = GetDC(NULL);
		HBITMAP bmp = CreateCompatibleBitmap(screenDC, r.right, r.bottom);
		HDC dc = CreateCompatibleDC(screenDC);
		SelectObject(dc, bmp);
		BitBlt(dc, 0, 0, r.right, r.bottom, screenDC, 0, 0, SRCCOPY);
		screenShot = new Bitmap(bmp, NULL);
		DeleteDC(dc);
		DeleteObject(bmp);
	} else if (_captureMode == 1) {
		HWND w = GetForegroundWindow(); // get handle of window on top
		screenShot = _backgroundWindow->printWindow(w, _isIncludeBackground, _saveType);// get screen
	} else {
		RECT r = _overlayWindow->getSelectionRect();
		HDC screenDC = GetDC(NULL);
		HBITMAP bmp = CreateCompatibleBitmap(screenDC, r.right - r.left, r.bottom - r.top);
		HDC dc = CreateCompatibleDC(screenDC);
		SelectObject(dc, bmp);
		BitBlt(dc, 0, 0, r.right - r.left, r.bottom - r.top, screenDC, r.left, r.top, SRCCOPY);
		screenShot = new Bitmap(bmp, NULL);
		DeleteDC(dc);
		DeleteObject(bmp);
	}
	
	String windowName = _T("Light Focus");		// and copy to windowName (string)
	FileInfo dir(_savePath); // dir store pics which are taken
	windowName = windowName + _T(" - "); // if pic has name same with older other --> change name by add subfix 
	vector<FileInfo> files;
	files = dir.getChildren(windowName + _T("*"));
	int n = 0;
	int current;
	if (files.size() > 0) {
		String fileName, num;
		for (size_t i = 0; i < files.size(); i++) {
			fileName = files[i].getFileNameWithoutExtension();
			num = fileName.substr(fileName.find(windowName) + windowName.length());
			current = _tstoi(num.c_str());
			if (current > n) {
				n = current;
			}
		}
		n++;
	}
	TCHAR buffer[20];
	_itot(n, buffer, 10);
	windowName = windowName + buffer;

	if (_saveType == 0) {
		windowName = windowName + _T(".jpg");
		LightFocus::GetEncoderClsid(_T("image/jpeg"), &imageClsid);
	} else if (_saveType == 1) {
		windowName = windowName + _T(".png");
		LightFocus::GetEncoderClsid(_T("image/png"), &imageClsid);
	} else {
		windowName = windowName + _T(".bmp");
		LightFocus::GetEncoderClsid(_T("image/bmp"), &imageClsid);
	}

	String saveFile = _savePath;
	saveFile += _T("\\");
	saveFile += windowName;
	
	screenShot->Save(saveFile.c_str(), &imageClsid, NULL);

	delete screenShot;
	return 0;
}

LRESULT MainWindow::onDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT MainWindow::onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	wmId = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	// Parse the menu selections:
	switch (wmId) {
		case IDM_EXIT:
			DestroyWindow(_handle);
			break;
		case IDM_OPTIONS:
			if (_optionsWindow == NULL) {
				_optionsWindow = new OptionsWindow(this);
			} else {
				SetForegroundWindow(_optionsWindow->getHandle());
			}
			break;
		case IDM_PRINT_SCREEN:
			PostMessage(_handle, UWM_PRINT_SCREEN, 0, 0);
			break;
		case IDM_OPEN_FODLER:
			ShellExecute(_handle, _T("open"), _savePath.c_str(), NULL, NULL, SW_SHOW);
			break;
		case IDM_ABOUT:
			MessageBox(NULL, _T("0812186 - Tran Hoang Huy\r\n0812213 - Do Nguyen Kha"), _T("Light Focus 1.0"), MB_OK);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT MainWindow::onCloseOptions(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (_optionsWindow != NULL) {
		delete _optionsWindow;
		_optionsWindow = NULL;
	}
	return 0;
}

LRESULT MainWindow::onNotifyIcon(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_RBUTTONUP) {
		POINT p;
		GetCursorPos(&p);
		BOOL n = TrackPopupMenu(_notifyIconMenu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, p.x, p.y, 0, _handle, NULL);
	} else if (lParam == WM_LBUTTONDBLCLK) {
		PostMessage(_handle, UWM_PRINT_SCREEN, 0, 0);
	}
	return 0;
}

LRESULT MainWindow::onPrintScreen(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (_captureMode == 0) {
		PostMessage(_handle, UWM_PRINT_AND_SAVE, 0, 0);
	} else {
		_overlayWindow->setSelectedWindow(NULL);
		RECT desktopRect;
		GetWindowRect(GetDesktopWindow(), &desktopRect);
		HRGN region = CreateRectRgn(0, 0, desktopRect.right, desktopRect.bottom);
		SetWindowRgn(_overlayWindow->getHandle(), region, TRUE);
		_overlayWindow->show(SW_SHOW);
	}
	return 0;
}

LightFocus::~LightFocus()
{
	delete _mainWindow;
}

int LightFocus::run()
{
	_mainWindow = new MainWindow(this);

	MSG msg;
	HACCEL hAccelTable;

	hAccelTable = LoadAccelerators(_handle, MAKEINTRESOURCE(IDC_LIGHTFOCUS));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	return (int) msg.wParam;
}

int LightFocus::GetEncoderClsid(LPCTSTR format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
	  return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
	  return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
	  if(_tcscmp(pImageCodecInfo[j].MimeType, format) == 0)
	  {
		 *pClsid = pImageCodecInfo[j].Clsid;
		 free(pImageCodecInfo);
		 return j;  // Success
	  }    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

BackgroundWindow::BackgroundWindow(Window *parent) : CustomWindow(parent)
{
	Window::init(NULL, WS_POPUP, _T("Background for capture!"), 0, 0, 0, 0, (HMENU)NULL);
}

void BackgroundWindow::_registerWindowClassName()
{
	WNDCLASSEX wcex;

	String windowClassName = getWindowClassName();

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_SAVEBITS;
	wcex.lpfnWndProc	= DefWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _application->getHandle();
	wcex.hIcon			= 0;
	wcex.hCursor		= 0;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= windowClassName.c_str();
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);
}

String BackgroundWindow::getWindowClassName()
{
	return _T("LIGHTFOCUS_BACKGROUNDCLASS");
}

Bitmap *BackgroundWindow::printWindow(HWND w, bool isIncludeBackground, int saveType)
{
	RECT r, desktopRect;
	Bitmap *b1;
	Bitmap *b2;
	HDC memoryDC1, memoryDC2;
	HBITMAP o1, o2;

	GetWindowRect(w, &r);
	GetWindowRect(GetDesktopWindow(), &desktopRect);
	HDC dc = GetDC(NULL);

	if (isIncludeBackground) {
		if (r.left < 0) {
			r.left = 0;
		}
		if (r.top < 0) {
			r.top = 0;
		}
		if (r.bottom > desktopRect.bottom) {
			r.bottom = desktopRect.bottom;
		}
		if (r.right > desktopRect.right) {
			r.right = desktopRect.right;
		}
		memoryDC2 = CreateCompatibleDC(dc);
		o2 = CreateCompatibleBitmap(dc, r.right - r.left, r.bottom - r.top);
		SelectObject(memoryDC2, o2);
		BitBlt(memoryDC2, 0, 0, r.right - r.left, r.bottom - r.top, dc, r.left, r.top, SRCCOPY);
		b2 = Bitmap::FromHBITMAP(o2, NULL);
		DeleteObject(o2);
		DeleteDC(memoryDC2);
	} else {
		// To include shadow
		r.left -= 50;
		r.top -= 50;
		r.right += 50;
		r.bottom += 50;
		
		if (r.left < 0) {
			r.left = 0;
		}
		if (r.top < 0) {
			r.top = 0;
		}
		if (r.bottom > desktopRect.bottom) {
			r.bottom = desktopRect.bottom;
		}
		if (r.right > desktopRect.right) {
			r.right = desktopRect.right;
		}

		memoryDC1 = CreateCompatibleDC(dc);
		o1 = CreateCompatibleBitmap(dc, r.right - r.left, r.bottom - r.top);
		SelectObject(memoryDC1, o1);
		memoryDC2 = CreateCompatibleDC(dc);
		o2 = CreateCompatibleBitmap(dc, r.right - r.left, r.bottom - r.top);
		SelectObject(memoryDC2, o2);
		HDC backgroundDC = GetDC(_handle);
		ShowWindow(_handle, SW_SHOWNA);
		SetWindowPos(_handle, w, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE);
		BitBlt(backgroundDC, 0, 0, r.right - r.left, r.bottom - r.top, backgroundDC, 0, 0, BLACKNESS); // create bitmap with black background
		UpdateWindow(_handle);

		BitBlt(memoryDC1, 0, 0, r.right - r.left, r.bottom - r.top, dc, r.left, r.top, SRCCOPY);// get bitmap with black background

		BitBlt(backgroundDC, 0, 0, r.right - r.left, r.bottom - r.top, backgroundDC, 0, 0, WHITENESS); // create bitmap with white background
		UpdateWindow(_handle);

		BitBlt(memoryDC2, 0, 0, r.right - r.left, r.bottom - r.top, dc, r.left, r.top, SRCCOPY);// get bitmap with white background
		
		ShowWindow(_handle, SW_HIDE);
		
		b1 = Bitmap::FromHBITMAP(o1, NULL);
		b2 = Bitmap::FromHBITMAP(o2, NULL);

		DeleteDC(memoryDC1);
		DeleteDC(memoryDC2);
		DeleteDC(backgroundDC);
		DeleteObject(o1);
		DeleteObject(o2);
	}

	// Not PNG
	if (saveType != 1 || isIncludeBackground) {
		if (!isIncludeBackground) {
			delete b1;
		}
		return b2;
	}
	Bitmap *result = new Bitmap(r.right - r.left, r.bottom - r.top);
	Color c1, c2;
	int count = 0;
	unsigned int total = result->GetHeight() * result->GetWidth();
	int ar;
	for (int i = 0; i < result->GetHeight(); i++) {
		for (int j = 0; j < result->GetWidth(); j++) {
			b1->GetPixel(j, i, &c1);
			b2->GetPixel(j, i, &c2);
			ar = c1.GetRed() - c2.GetRed() + 255;
			if (c1.GetValue() != c2.GetValue()) {
				count++;
			}
			if (ar > 0) {
				c2.SetValue(Color::MakeARGB(ar, 255 * c1.GetRed() / ar, 255 * c1.GetGreen() / ar, 255 * c1.GetBlue() / ar));
			} else {
				c2.SetValue(Color::MakeARGB(0, 0, 0, 0));
			}
			result->SetPixel(j, i, c2);
		}
	}

	RECT client;
	GetClientRect(w, &client);

	unsigned int inside = (client.right - client.left) * (client.bottom - client.top);
	unsigned int outside = total - inside;
	count = count - outside;
	if (count <= 0) {
		count = 1;
	}

	if (inside / count < 6) {
		delete result;
		result = b2;
	} else {
		delete b2;
	}
	delete b1;

	return result;
}

BackgroundWindow::~BackgroundWindow()
{}

OverlayWindow::OverlayWindow(Window *parent) : CustomWindow(parent)
{
	Window::init(WS_EX_LAYERED, WS_POPUP, _T("Light Focus overlay window"), 0, 0, 500, 500, (HMENU)NULL);
	SetLayeredWindowAttributes(_handle, RGB(255, 255, 255), 128, LWA_ALPHA | LWA_COLORKEY);
	HWND desktop = GetDesktopWindow();
	RECT desktopRect;
	GetWindowRect(desktop, &desktopRect);
	SetWindowPos(_handle, HWND_TOPMOST, 0, 0, desktopRect.right, desktopRect.bottom, SWP_NOACTIVATE);
	_isSelecting = false;
	_selection = new SelectedWindow(this);
}

String OverlayWindow::getWindowClassName()
{
	return _T("LIGHTFOCUS_OVERLAYCLASS");
}

HWND OverlayWindow::getSelectedWindow()
{
	return _selectedWindow;
}

void OverlayWindow::setSelectedWindow(HWND w)
{
	_selectedWindow = w;
}

RECT OverlayWindow::getSelectionRect()
{
	return _selection->getRect();
}

OverlayWindow::~OverlayWindow()
{}

void OverlayWindow::_registerWindowClassName()
{
	WNDCLASSEX wcex;

	String windowClassName = getWindowClassName();

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CustomWindow::preCreateWindowProcedure;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _application->getHandle();
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= windowClassName.c_str();
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);
}

LRESULT OverlayWindow::onLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Rectangle mode
	if (((MainWindow *)_parent)->getCaptureMode() == 2) {
		GetCursorPos(&_firstPoint);
		RECT r = getRect();
		HRGN overlayRgn = CreateRectRgn(0, 0, r.right, r.bottom);
		SetWindowRgn(_handle, overlayRgn, TRUE);
		_selection->show(SW_HIDE);
		_isSelecting = true;
	}
	return 0;
}

LRESULT OverlayWindow::onLeftButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	GetCursorPos(&p);
	if (((MainWindow *)_parent)->getCaptureMode() == 1) {
		ShowWindow(_handle, SW_HIDE);
		_selectedWindow = WindowFromPoint(p);
		ShowWindow(_handle, SW_SHOW);
		_selectedWindow = GetAncestor(_selectedWindow, GA_ROOT);
		if (_selectedWindow != NULL) {
			HRGN overlayrgn = CreateRectRgn(0,0,0,0);
			HRGN combine = CreateRectRgn(0,0,0,0);
			int n = GetWindowRgn(_handle, overlayrgn);
			HRGN hrgn = CreateRectRgn(0,0,0,0);
			int regionType = GetWindowRgn(_selectedWindow, hrgn);
			RECT r1, r2;
			GetWindowRect(_handle, &r1);
			GetWindowRect(_selectedWindow, &r2);
			if (regionType == 0) {
				hrgn = CreateRectRgn(r2.left, r2.top,r2.right,r2.bottom);
			} else {
				OffsetRgn(hrgn, r2.left - r1.left, r2.top - r1.top);
			}
			CombineRgn(combine, overlayrgn, hrgn, RGN_DIFF);
			SetWindowRgn(_handle, combine, TRUE);
			_selection->move(r2.left, r2.top, r2.right - r2.left, r2.bottom - r2.top);
			regionType = GetWindowRgn(_selectedWindow, hrgn);
			if (regionType == 0) {
				hrgn = CreateRectRgn(0, 0, r2.right - r2.left, r2.bottom - r2.top);
			}
			SetWindowRgn(_selection->getHandle(), hrgn, TRUE);
			_selection->show(SW_SHOW);
		}
	} else {
		int x, y, width, height;
		RECT r = getRect();
		HRGN overlayRgn = CreateRectRgn(0, 0, r.right, r.bottom);
		width = p.x - _firstPoint.x;
		height = p.y - _firstPoint.y;
		if (width < 0) {
			width = -width;
			x = p.x;
		} else {
			x = _firstPoint.x;
		}
		if (height < 0) {
			height = -height;
			y = p.y;
		} else {
			y = _firstPoint.y;
		}
		HRGN rgn = CreateRectRgn(0, 0, width, height);
		SetWindowRgn(_selection->getHandle(), rgn, TRUE);
		rgn = CreateRectRgn(0, 0, width, height);
		OffsetRgn(rgn, x, y);
		CombineRgn(overlayRgn, overlayRgn, rgn, RGN_DIFF);
		SetWindowRgn(_handle, overlayRgn, FALSE);
		_selection->move(x, y, width, height, false);
		_selection->show(SW_SHOW);
		_isSelecting = false;
	}
	
	return 0;
}

LRESULT OverlayWindow::onMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (((MainWindow *)_parent)->getCaptureMode() == 2 && _isSelecting) {
		POINT p;
		GetCursorPos(&p);
		int x, y, width, height;
		RECT r = getRect();
		HRGN overlayRgn = CreateRectRgn(0, 0, r.right, r.bottom);
		width = p.x - _firstPoint.x;
		height = p.y - _firstPoint.y;
		if (width < 0) {
			width = -width;
			x = p.x;
		} else {
			x = _firstPoint.x;
		}
		if (height < 0) {
			height = -height;
			y = p.y;
		} else {
			y = _firstPoint.y;
		}
		HRGN rgn = CreateRectRgn(0, 0, width, height);
		SetWindowRgn(_selection->getHandle(), rgn, TRUE);
		rgn = CreateRectRgn(0, 0, width, height);
		OffsetRgn(rgn, x, y);
		CombineRgn(overlayRgn, overlayRgn, rgn, RGN_DIFF);
		SetWindowRgn(_handle, overlayRgn, FALSE);
		_selection->move(x, y, width, height, false);
		_selection->show(SW_SHOW);
	}
	return 0;
}

LRESULT OverlayWindow::onPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC dc;
	dc = BeginPaint(_handle, &ps);
	Graphics g(dc);
	g.Clear(Color::Black);
	EndPaint(_handle, &ps);
	return 0;
}

void OptionsWindow::_registerWindowClassName()
{
	WNDCLASSEX wcex;

	String windowClassName = getWindowClassName();

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CustomWindow::preCreateWindowProcedure;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _application->getHandle();
	wcex.hIcon			= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LIGHTFOCUS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= windowClassName.c_str();
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);
}

OptionsWindow::OptionsWindow(Window *parent) : CustomWindow(parent)
{
	Window::init(NULL, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, _T("Light Focus Options"), CW_USEDEFAULT, 0, 300, 300, (HMENU)NULL);

	_HookPrintScreenCheckBox = new CheckBox(this);
	_HookPrintScreenCheckBox->init(_T("Use Print Screen"), 15, 10, 200, 25);

	_includeBackgroundCheckBox = new CheckBox(this);
	_includeBackgroundCheckBox->init(_T("Include background"), 15, 35, 200, 25);

	_delayLabel = new Label(this);
	_delayLabel->init(_T("Delay before capture: "), 15, 65, 200, 25);
	
	_delayEditBox = new EditBox(this);
	_delayEditBox->init(_T(""), 130, 60, 40, 20);

	_savePathLabel = new Label(this);
	_savePathLabel->init(_T("Save path: "), 15, 90, 200, 25);
	
	_savePathEditBox = new EditBox(this);
	_savePathEditBox->init(_T(""), 15, 110, 180, 20);
	_savePathEditBox->setReadOnly(true);

	_browseForFolder = new Button(this);
	_browseForFolder->init(_T("Browse"), 200, 109, 70, 22, IDM_BROWSE);

	_saveTypeLabel = new Label(this);
	_saveTypeLabel->init(_T("Save type: "), 15, 145, 200, 25);

	_saveTypeComboBox = new ComboBox(this);
	_saveTypeComboBox->init(75, 140, 90, 25, CBS_DROPDOWNLIST);
	_saveTypeComboBox->addString(_T("JPEG"));
	_saveTypeComboBox->addString(_T("PNG"));
	_saveTypeComboBox->addString(_T("BMP"));
	_saveTypeComboBox->setSelectedIndex(0);

	_captureModeLabel = new Label(this);
	_captureModeLabel->init(_T("Capture mode: "), 15, 175, 200, 25);

	_captureModeComboBox = new ComboBox(this);
	_captureModeComboBox->init(95, 170, 90, 25, CBS_DROPDOWNLIST);
	_captureModeComboBox->addString(_T("Desktop"));
	_captureModeComboBox->addString(_T("Window"));
	_captureModeComboBox->addString(_T("Rectangle"));
	_captureModeComboBox->setSelectedIndex(0);


	_okButton = new Button(this);
	_okButton->init(_T("OK"), 15, 230, 80, 25, IDM_OK);
	_cancelButton = new Button(this);
	_cancelButton->init(_T("Cancel"), 105, 230, 80, 25, IDM_CANCEL);
	_applyButton = new Button(this);
	_applyButton->init(_T("Apply"), 195, 230, 80, 25, IDM_APLLY);

	loadOptions();

	update();
	show(SW_SHOW);
}

String OptionsWindow::getWindowClassName()
{
	return _T("LIGHTFOCUS_OPTIONSCLASS");
}

LRESULT OptionsWindow::onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	wmId = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = _T("Browse for folder");
	LPITEMIDLIST pidl;

	// Parse the menu selections:
	switch (wmId) {
		case IDM_OK:
			saveOptions();
			PostMessage(_parent->getHandle(), UWM_CLOSE_OPTIONS, 0, 0);
			break;
		case IDM_CANCEL:
			PostMessage(_parent->getHandle(), UWM_CLOSE_OPTIONS, 0, 0);
			break;
		case IDM_APLLY:
			saveOptions();
			break;
		case IDM_BROWSE:
			pidl = SHBrowseForFolder(&bi);
			if (pidl != 0) {
				TCHAR path[MAX_PATH];
				if (SHGetPathFromIDList(pidl, path)) {
					_savePathEditBox->setWindowText(path);
				}

				IMalloc *imalloc = 0;
				if (SUCCEEDED(SHGetMalloc(&imalloc))) {
					imalloc->Free(pidl);
					imalloc->Release();
				}
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT OptionsWindow::onDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PostMessage(_parent->getHandle(), UWM_CLOSE_OPTIONS, 0, 0);
	return 0;
}


void OptionsWindow::loadOptions()
{
	MainWindow *w = (MainWindow *)_parent;
	_savePathEditBox->setWindowText(w->getSavePath());
	if (w->isHookPrintScreen()) {
		_HookPrintScreenCheckBox->check();
	} else {
		_HookPrintScreenCheckBox->uncheck();
	}
	if (w->isIncludeBackground()) {
		_includeBackgroundCheckBox->check();
	} else {
		_includeBackgroundCheckBox->uncheck();
	}
	TCHAR buffer[20];
	_itot(w->getDelay(), buffer, 10);
	_delayEditBox->setWindowText(buffer);
	_saveTypeComboBox->setSelectedIndex(w->getSaveType());
	_captureModeComboBox->setSelectedIndex(w->getCaptureMode());
}

void OptionsWindow::saveOptions()
{
	MainWindow *w = (MainWindow *)_parent;
	w->setSavePath(_savePathEditBox->getWindowText());
	if (_HookPrintScreenCheckBox->isChecked()) {
		w->setHookPrintScreen(true);
	} else {
		w->setHookPrintScreen(false);
	}
	if (_includeBackgroundCheckBox->isChecked()) {
		w->setIncludeBackground(true);
	} else {
		w->setIncludeBackground(false);
	}
	w->setDelay(_ttoi(_delayEditBox->getWindowText().c_str()));
	w->setSaveType(_saveTypeComboBox->getSelectedIndex());
	w->setCaptureMode(_captureModeComboBox->getSelectedIndex());
	w->saveOptions();
}

OptionsWindow::~OptionsWindow()
{
	delete _includeBackgroundCheckBox;
	delete _HookPrintScreenCheckBox;
	delete _delayEditBox;
	delete _delayLabel;

	DestroyWindow(_handle);
}

void SelectedWindow::_registerWindowClassName()
{
	WNDCLASSEX wcex;

	String windowClassName = getWindowClassName();

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= CustomWindow::preCreateWindowProcedure;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _application->getHandle();
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= windowClassName.c_str();
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);
}

SelectedWindow::SelectedWindow(Window *parent) : CustomWindow(parent)
{
	Window::init(WS_EX_LAYERED, WS_POPUP, _T("Light Focus selected window"), 0, 0, 0, 0, (HMENU)NULL);
	SetLayeredWindowAttributes(_handle, NULL, 1, LWA_ALPHA);
}

LRESULT SelectedWindow::onPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT SelectedWindow::onLeftButtonDoubleClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (((MainWindow *)_parent->getParent())->getCaptureMode() == 1) {
		SetForegroundWindow(((OverlayWindow *)_parent)->getSelectedWindow());
	}
	_parent->show(SW_HIDE);
	show(SW_HIDE);
	PostMessage(_parent->getParent()->getHandle(), UWM_PRINT_AND_SAVE, 0, 0);
	
	return 0;
}

String SelectedWindow::getWindowClassName()
{
	return _T("LIGHTFOCUS_SELECTEDCLASS");
}

SelectedWindow::~SelectedWindow()
{
	DestroyWindow(_handle);
}
