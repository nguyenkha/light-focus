#ifndef _LIGHT_FOCUS
#define _LIGHT_FOCUS

#include "resource.h"
#include "Application.h"
#include "KeyHook.h"

#define UWM_NOTIFY_ICON WM_USER + 10
#define UWM_CLOSE_OPTIONS WM_USER + 12
#define UWM_PRINT_AND_SAVE WM_USER + 13

using namespace SimpleApplication;

class OptionsWindow : public CustomWindow
{
protected:
	void _registerWindowClassName();

	CheckBox *_HookPrintScreenCheckBox;
	CheckBox *_includeBackgroundCheckBox;
	Label *_delayLabel;
	EditBox *_delayEditBox;
	Label *_savePathLabel;
	EditBox *_savePathEditBox;
	Button *_browseForFolder;
	Label *_saveTypeLabel;
	ComboBox *_saveTypeComboBox;
	Label *_captureModeLabel;
	ComboBox *_captureModeComboBox;

	Button *_okButton;
	Button *_cancelButton;
	Button *_applyButton;

public:
	OptionsWindow(Window *parent);
	String getWindowClassName();
	LRESULT onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void saveOptions();
	void loadOptions();
	virtual ~OptionsWindow();

	beginMessageMap()
		messageMap(WM_COMMAND, onCommand)
		messageMap(WM_DESTROY, onDestroy)
	endMessageMap()
};

class SelectedWindow : public CustomWindow
{
protected:
	void _registerWindowClassName();
public:
	SelectedWindow(Window *parent);
	String getWindowClassName();
	LRESULT onLeftButtonDoubleClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual ~SelectedWindow();

	beginMessageMap()
		messageMap(WM_LBUTTONDBLCLK, onLeftButtonDoubleClick)
		messageMap(WM_PAINT, onPaint)
	endMessageMap()
};

class OverlayWindow : public CustomWindow
{
protected:
	void _registerWindowClassName();
	POINT _firstPoint;
	SelectedWindow *_selection;
	bool _isSelecting;
	HWND _selectedWindow;

public:
	OverlayWindow(Window *parent);
	String getWindowClassName();
	RECT getSelectionRect();
	void setSelectedWindow(HWND w);
	HWND getSelectedWindow();
	LRESULT onLeftButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual ~OverlayWindow();

	beginMessageMap()
		messageMap(WM_LBUTTONDOWN, onLeftButtonDown)
		messageMap(WM_LBUTTONUP, onLeftButtonUp)
		messageMap(WM_MOUSEMOVE, onMouseMove)
		messageMap(WM_PAINT, onPaint)

	endMessageMap()
};

class BackgroundWindow : public CustomWindow
{
protected:
	void _registerWindowClassName();
public:
	BackgroundWindow(Window *parent);
	Bitmap *printWindow(HWND w, bool isIncludeBackground, int saveTypes);
	String getWindowClassName();
	virtual ~BackgroundWindow();

	beginMessageMap()
	endMessageMap()
};

class MainWindow : public CustomWindow
{
protected:
	void _registerWindowClassName();
	HMENU _notifyIconMenu;
	BackgroundWindow *_backgroundWindow;
	OverlayWindow *_overlayWindow;
	OptionsWindow *_optionsWindow;
	
	bool _isHookPrintScreen;
	bool _isIncludeBackground;
	int _delay;
	String _savePath;
	int _saveType;
	int _captureMode;

public:
	MainWindow(Application *application);
	String getWindowClassName();

	LRESULT onNotifyIcon(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onCloseOptions(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onPrintScreen(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT onPrintAndSave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool isHookPrintScreen();
	bool isIncludeBackground();
	int getDelay();
	String getSavePath();
	int getSaveType();
	int getCaptureMode();

	void setHookPrintScreen(bool isHookPrintScreen);
	void setIncludeBackground(bool isIncludeBackground);
	void setDelay(int delay);
	void setSavePath(const String &savePath);
	void setSaveType(int type);
	void setCaptureMode(int mode);


	void loadOptions();
	void saveOptions();
	RegistryKey *registerOptions();
	virtual ~MainWindow();

	beginMessageMap()
		messageMap(UWM_NOTIFY_ICON, onNotifyIcon)
		messageMap(UWM_PRINT_SCREEN, onPrintScreen)
		messageMap(UWM_PRINT_AND_SAVE, onPrintAndSave)
		messageMap(UWM_CLOSE_OPTIONS, onCloseOptions)
		messageMap(WM_COMMAND, onCommand)
		messageMap(WM_DESTROY, onDestroy)
	endMessageMap()
};

class LightFocus : public Application
{
protected:
	MainWindow *_mainWindow;
public:
	int run();
	
	// From MSDN
	static int GetEncoderClsid(LPCTSTR format, CLSID* pClsid);

	virtual ~LightFocus();
};

#endif