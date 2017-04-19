May, 2011:
- fixed problem with api hooking
- changed handling of the window right click menu
- fixed problems with "Move to" menu with IE9 and firefox 4
- set version to 1.02

Apr, 25, 2011:
- fixed reported crash during desktop change with hotkeys
- no mini windows for sticky windows
- adjust height for the all toolbars (quicklaunch etc), reported bug
- check if running the right version of wp (32, 64bit)
- changed detection of 64bit windows (os)
- more info in the dump file names
- 32/64 Bit info in the about dialog
- set version to 1.01

Mar, 16, 2011:
- fixed bug if window menu is not visible after wp restart - after 3,5 years of development i finally made 1.00 release
Development of windowspager was started after i switched from Linux to Windows to
learn C++. Ater using Linux for years i missed a good pager.
So making my own pager was good project to start using C++.
I know a lot of feature are still missing but the time to work on this project
is limited and one man show. So... time for 3,5 beers tonight ;)
- set version to 1.00

Feb, 27, 2011:
- fixed problems with FF 4.0 and IE 9:
* changed hooking of the window menu
* started using of WS_EX_TOOLWINDOW again to avoid of 
poping up the taskbarbutton of FF 4.0 (only FF, only first hide/show)
- set version to 1.00 RC1

Feb, 12, 2011:
- use of API Hooking with the famous MinHook library:
http://www.codeproject.com/KB/winsdk/LibMinHook.aspx:
Hook some Apis like GetWindowRect() to avoid problems during shutdown
- removed the Shutdown/reboot .. menus, now it is safe to just shutdow Windows
even if windowspager is still running
- removed the shutdown blocking
- merged hook32/64 projects
- merged hookstarter32/64 projects
- added DLL/EXE infos
- restore all windows on WM_QUERYENDSESSION
- set version to 0.99almostOneF_devSnap

Jan, 15, 2011
- fixed problems with steam
- fixed reported bug, first crash report with the new crash reporting tool thanks !
- set version to 0.99almostOneD

Nov, 07, 2010:
- included automatic crash reporting with MiniDumpWriteDump, thanks to Author of the
codeproject article: http://www.codeproject.com/KB/debug/postmortemdebug_standalone1.aspx
- some try/catch here and there (reported crash)
- use of SetUnhandledExceptionFilter()
- added to the windowspager.ini the option crash_reporting=full,silent, none
- set version to 0.99almostOneC (I know its time for 1.0 ;) )

Oct, 17, 2010:
- MoveAndSwitch feature: move a window to an other destkop and 
automatically switch the desktop afterwards
- no hung up info window for process currently debugging using Visual Studio
- set version to 0.99almostOneB

Sep, 17, 2010:
- support for vertical panel (ufff finally i made it)
- non case sensitive ignore window list
- set version to 0.99almostOne

Aug, 17, 2010
- removed EnableWindow() code, seems to make problems with MS Office
- adjust windows move/size changed events to fix problems with some win7 resize features
- remember_active_window is now default on
- set version 0.99andsomewhat

Aug, 01, 2010
- fixed (some) problems with taskbar under Windows 7
- use of EnableWindow() instead of WM_DISABLED
- reset windows menu if add to ignore list
- set version to 0.99

Jun, 20, 2010:
- ignore chrome windows in the menu hook, this fixes a
bug with chrome applications (reported bug)
- do not subclass MediaMonkey to avoid crashing of MediaMonkey (reported bug)
- reset the window menu in the destroy/hide event to avoid problems with
reused windows (reported bug)
- delete PAGER_IGNORE window prop during windowspager start
- set version to 0.98

Jun, 13, 2010:
- added menu option to hibernate and sleep system
- experimental option to remember active window on each desktop enable it with:
remember_active_window=yes
- set version to 0.97

May 30, 2010:
- do not return FALSE for WM_ENDSESSION and WM_QUERYENDSESSION in the hook
- set empty ShutdownBlockReasonCreate string in the hoostarters
- get all path to exes with GetModuleFileName() to avoid problems (reported bug)
- save windows position in UpdateMiniWindow() instead of Hide() to avoid problems with fast desktop change
- set correct window position during mini window dragging
- updated window class für Chrome 5
- set versio to 0.96devsnap

May, 15, 2010:
- set menu hook for all windows (as before), fixes problem with Foobar2000 (reported bug)
- use subclass menu stuff only for Chrome browser
- Post WM_CLOSE before System shutdown/log off/reboot, to avoid problems with Windows which
save there current position on screen (Firefox for example)
- fixed reported translation error in the hang up info window 
- set version to 0.95(not released)

May, 2, 2010:
- ignore class 'WorkerW', this avoid problems under Seven (reported bug)
- added menu entries: Shutdown System and Log off
- reject system shutdown/Log off to avoid problems
- create a message window to get WM_QUERYENDSESSION message, set shutdown string
- removed WM_QUERYENDSESSION posting from hooks, return just false (no effect but...)
- make a real hookstarter window and return false for WM_QUERYENDSESSION
- Win Seven: adjust taskbutton window too
- do not hide windows on second monitor during CheckForVisibleWindows() (reported bug)
- hide windows in CheckForVisibleWindows()
- Window move between primary and secondary monitor: do not remove/set tool window style if 
the window is born with this flag
- set version to 0.93

Apr, 04, 2010:
- switched back to move windows out of visible area instead of
making them transparent. its faster on XP and works with more programs (even Photoshop)
- huge refactoring and code clean up
- added "start_delay" option to delay wp startup
- removed DOS reparent shit
- Release COM objects
- subclassing for non explorer windows, now even the right click menu in chrome browser works
- menu hook only for explorer windows
- avoid hidden window popup or move with intercepting the messages using subclassing
- fixed bug with Enumvisiblewindows()
- do more menu stuff inside subclass
- start manager.exe inside a thread in windowspager.exe
- set version to 0.92almoststable

Jan, 15, 2010:
- added possiblity to select the way command windows should be treated:
command_window_mode=full, half, none
- dont ignore pinned windows under Win7 to avoid problem with the taskbar preview (reported bug)
- no result in GetMiniWindowByPoint() if WM_EX_TRANSPARENT or console window
with mode=half
- WinVersion cleanup
- updated year in the about window
- removed prob "WAS_PAGER_WINDOW"
- remove WS_EX_TRANSPARENT in windowspager.cpp if pager was crashing
- remove "PAGER_WINDOW" on window destroy
- set version to 0.90

Jan, 12, 2010:
- disabled subclassing till it is more stable
- use of WM_WINDOWPOSCHANGED instead of WM_SHOWWINDOW to get more events
- no mini-window if window has flag WM_EX_TRANSPARENT
- improvements in EnumToplevelWindows()
- rewrite of MaybeAddWindow()
- added some classes to the bad window list
- remove trans/disable shit on window hide only if on foreign desktop
- dont set/remove WS_EX_TRANSPARENT if window use it
- do not check for WM_CHILD in the window-selector
- fixed problems with sidebar apps under Win 7
- added some app to the ignore list
- set version to 0.89devsnap2

Jan, 04,2010:
- fixed problem with Win2000: load console function from dll so WindowsPager
works with 2000 again, reported bug
- set version to 0.89devsnap

Jan, 02,2010: 
- support for WPF applications by subclassing the windows and eating
the WM_STYLECHANGING message, yesssss
- subclass other windows too just for fun
- set version to 0.88devsnap

Dec, 26, 2009:
- added 'Window-Selector', a graphical tool to add windows to the ignore list
- removed ignore_window_classes, ignore_window_titles from windowspager.ini
- added [IgnoreWindows] section
- added menu option to open the Window-Selector
- added menu option to open the download page (requested feature)
- more internal unicode support
- set version to 0.86devsnap

Dec, 16, 2009:
- use AttachConsole() function to get font/size of the cmd window,
this trick was a suggestion from a friendly developer, thanks dude
now windowspager passed the test on http://www.virustotal.com/
- set version to 0.84

Dec, 15, 2009:
- fixed handle leak with the help of process explorer
- use of shared_ptr here and there
- set version to 0.83

Dec, 13, 2009:
- manage cmd.exe watchdogs in a list
- get cmd.exe font settings by injecting the cmd.exe process
- do a lot of ugly things to improve cmd window stuff
- incrementel cmd wimdow resize
- reparent cmd.exe again if font or size has changed
- do not manage cmd window under Win 2000 for the moment
- do not set the default wallpaper on exit (reported bug)
- get window border and caption hide by with API
- remove trans and disabled on window hide
- set version to 0.82stillexperimental

Nov, 22, 2009:
- new method to hide currently not visible windows: make them completely transparent
this fix problems with Visual Studio and Photoshop
- little refactoring
- do not hide/show taskbar buttons if pinned (win 7)
- update hang up info window correctly
- move owner only if it has "ignore" flag set
- get file name in utf
- fixed problems with aim2
- removed the "info circle" near the mouse cursor, we will see if somebody is missing it
- set version to 0.79experimental

Nov, 12, 2009:
- fixed bug with Gimp 2.6 and the sticky notes program from Windows 7
- moved smart fixme code into a separate file
- cleaned up changelog html site
- set version to 0.77

Nov, 5, 2009:
- possibility to disable command/dos window management (requested feature)
- fixed bug with resulting crash if the mini-window get destroyed/hide during dragging
- set version to 0.76

Oct, 18, 2009:
- initialize all data in .SHARDATA
- do not export CallWndProc() and MenuProc() in hook32/64.dll
- to avoid false alarm with anti virus programs i used some workarounds:
* compiled hookstarter32/64.exe with no optimization
* load ShellExecute() with GetProcAddress()
* find "ReBarWindow32" window avoid using the string "ReBarWindow32"
- tested all programs/DLLs with http://www.virustotal.com
- set version to 0.75

Oct, 10, 2009:
- use of DisableThreadLibraryCalls() in hook
- hook handle inside shared data
- modify window system menu inside hook to avoid loosing theme after quiting windowspager,
removed menu timer
- new Init code to avoid problems if started inside autostart (reported bug)
- set version to 0.73

Sep, 27, 2009:
- support for Vista Sidebar
- set version to 0.72

Sep, 27, 2009:
- fixed bug introduced in 0.70 (with CMD window)
- check on start if manager.exe exist
- added some information shit to the windowspager.exe
- set vesion to 0.71

Sep, 19, 2009:
- use of GDI+ for drawing with following pros:
* Windowspager is now opaque under Vista/Seven !
* setting the taskbar toobaque is not necessary anymore
* better appearance
- use of double buffering: flicker free mini-window dragging even with Win2000
- removed opaque panel hack code
- Win2000: get running windows during Windowspager startup like in Windows Seven
- Win2000: search the right window class for the tasklist window
- other icon for explorer windows
- set version to 0.70

Aug, 30, 2009:
- posibility to disable the window which inform about non responding windows (requested feature)
- add SideBar_HTMLHostWindow class to the ignore list (Win Seven mini desktop apps)
- set version to 0.66

Aug, 16, 2009:
- Win7: get running programs during Windowspager startup
- Win7: search the right window class for the tasklist window
- set version to 0.65

Aug, 9. 2009:
- fixed reported bug with mini-window dragging if you use 2 rows of desktops
- use of SetCapture() during dragging
- abort dragging on WM_CANCELMODE (new window popup etc.)
- code clean up
- in Win7 getting the tasklist windows on startup do not work, no fix ATM
- check return value of strol() better
- allow mini-window dragging inside VMware and with Win2000
- do not allow dragging of windows with WS_EX_TOPMOST do avoid bugs
- set version to 0.64

Aug, 2, 2009:
- accept 0 as a return value from GetWindowLong() if GetLastError() return success
this fix a problem with JDownloader during desktop switch
- set version to 0.63

Jul, 19, 2009:
- more robust mini-window dragging
- disable mini-window dragging inside VMware (ClipCursor() not working inside VMware)
- fixed reported bug with trayicon mode
- subtract taskbar height from desktop height in single monitor mode
- set version to 0.62

Jul, 12, 2009:
- fixed frame of mini-window if real is window maximized
- mini-window drag now only with mouse (will begin after 300 ms, On Top windows don not work atm)
- set version to 0.61experimental

Jun, 27, 2009:
- added the possibility to move the active window to the left/right (or by number) desktop with a keyboard shortcut (requested feature)
- ignore transparent windows
- windowspager.ini file clean up: removed 'usevirtualkeys', its possiblenow to always use virtual keys
- set version to 0.60

Jun, 15, 2009:
- update mini-window size during panel size change
- skip the hidden VLC window
- readd panel blur on exit
- start the hooks in separate process to support Win32 programs unter 64 bit OS
- get the application icons from the application EXE
- support for Windows 7 RC
- set version to 0.59

Jun, 07, 2009:
- buf fix release: 0.57 leaks memory with windows which do not respond
- removed information of non responding windows on the windows title cause of memory consumption
- unregister 'PagerDummyWindow' and 'PagerMessageWindow' on exit
- clear some lists on exit
- remove WS_EX_COMPOSITED on dummywindows cause with this flag AnimateWindow() do not work
- use DestroyIcon() instead of DeleteObject() in ~DummyWindow()
- no 'slow die' if dummywindows was not visible
- Animate dummywindow on show too
- use of binary_search() for bad window classes (sounds faster)
- do not use TerminateThread(), cause it leaks memory
- set version to 0.58

Jun, 05, 2009:
- test on startup if panel window and co. exist, usefull for autostart
- removed FORCE_MINIMIZE code
- inform in the window title if a window is not responding (Vista with Aero)
- show "dummywindow" on hang up during show after 1 sec, during hide after 10 sec, with no aero/XP after 4 sec
- removed timer in dummywindow class
- possibility to specify classes and windows titles you want to ignore in windowspager.ini
- fixed Windows-logout code
- fixed prob with hang up windows and its owner windows
- do not observe hang up windows on seondary screen
- do not observe windows with flag "ignore"
- set version to 0.57

May, 30, 2009:
- show windows minimized before Windows lock out
- fixed bugs with mini-window z-order
- use of WS_EX_NOACTIVATE instead of WS_EX_TOOLWINDOW: less flicker
- use of WS_DISABLED for all windows
- refuse poup of iconic windows (Adobe Reader for example)
- set version to 0.56experimental

May, 23, 2009:
- 64bit version
- new default modifier keys are now: control-win, because win-arrowkeys are used by Windows 7
- set iconic windows WS_DISABLED if hidden to avoid pop up
- fixed probs with mini-window z-order and owner windows
- set version to 0.55

May, 19, 2009:
- fixed probs with owner windows and its taskbuttons (workspace change, toggle sticky, etc)
- grayed windows-menu for owner windows
- improvements on mini-window movement
- add confiigure option to disable tasklist buttons for sticky windows: sticky_window_button=yes/no
- uninitialize sticky list on exit
- set version to 0.54

May 17, 2009:
- important bug fix release! old version could hide windows for ever!
- ignore flashing windows on secondary monitor with 'PagerOnlyOnPrimaryScreen' mode
- fixed prob with CMD parent window
- restore CMD windows on exit
- remove taskbarbutton if window is on secondary monitor on statup
- restore windows on seondary screen on exit
- do not add/remove minimize box on secondary screen, it is already a toolwindow
- don not update window during show/hide or if in hidden rectangle
- set version to 0.53

May 16, 2009:
- fixed bug with title in hang-notification window
- move windows only horizontal
- set version to 0.52

May, 14, 2009:
- removed "non_documented_hide_method" stuff, too bugy
- trayicon support
- Desktop scrolling support: go to left, right, up, down desktop
- modified windowspager.ini
- Show Online Help instead Dialog Window
- same 64bit changes
- set version to 0.51experimental

May, 4, 2009:
- Multi monitor support
- possibility to disable mini window for sticky windows
- set version to 0.50experimental

Apr, 11, 2009:
- fixed typo in windowspager.ini
- more suitable icons in the dialog windows
- set version to 0.49

Feb, 28, 2009:
- not yet public method to hide hung up windows
- improvements in the hung-up notification window
- use of RegisterShellHookWindow direct without loading it from dll
- use of DeregisterShellHookWindow on exit
- set version to 0.48experimental

Feb, 19, 2009:
- set WINVER again, this fix the cmd window problem and more....
- set version to 0.47experimental

Feb, 16, 2009:
- remove/add ToolStyle for button windows always (fix a problem with Avant browser)
- increase settoolstyle timeout
- show info window about hang up window only if the window is hung up for 4 sec or more
- set version to 0.46experimental

Feb, 13, 2009:
- support for different wallpaper on each Desktop
- Window which inform about hangup windows is now smaller
and on top of the screen, no hide/show
- use of a nonmodal dialog windows
- make Vista panel obaque to see something
- add hidden window to get the WM_DWMCOMPOSITIONCHANGED event
- do not obsorve DOS window for hangup
- set version to 0.45experimental

Feb, 7, 2009:
- set/remove Toolstyle in separate thread to avoid blocking if a windows is hunging
- check for hanging windows during desktop change
- obsorve hanging windows
- show a message window to inform the user about a hanging window
- detect Vista Aero
- fixed 32 bit only function calls
- set version to 0.44experimental

Jan, 26. 2009:
- use SWP_FRAMECHANGED in SetWindowPos() to avoid problems with Window-Border for example with Putty
- removed old code
- readded death window check during show/hide
- check more WinApi return values
- if a new Window is shown on currently hidden Desktop flash pager (fix problems with WinSCP)
- fixed updating the z-order of the mini-windows
- only remove mIRC custum menu before adding my menu
- set version to 0.43

Jan, 19, 2009:
- added windowspager.ini
- removed command line parameter
- added free configurable shortcuts/hotkeys for each desktop
- restore Window Menu on exit and before adding costum stuff
- exit Pager and restore Windows if Explorer.exe is crashing
- reject events during Init()
- use WS_EX_COMPOSITED on WinXP Classic
- set version to 0.42

Jan, 12, 2009:
- support for Windows XP Classic Theme
- fixed: call EnumVisibleWindows() even if zlist ist empty
- ignore Windows with WS_EX_TOOLWINDOW and no owner (some desktop gadgets)
- set version to 0.41

Jan, 08, 2009:
- new flash code, only flash if window is real flashing (like xchat, mirc)
- new timer code
- fixed DOS window support
- ignore Skype windows (Skype will work but without Desktop support)
- fixed Window menu after toogle 'always visible'
- removed some critical code
- set version to 0.40

Oct, 26, 2008:
- removed 0.38, 0.39, 0.39a
- do not restore windows on current desktop
- set version to 0.39experimental 'use with care'

Oct, 25, 2008:
- restore iconic windows on exit or crash too
- restore windows after crash on the old position
- removed process watchdog for now
- removed "Fensterklasse" from bad window classes
- added "pagermain" to bad window classes
- fixed ITaskbar code
- set version to 0.39a

Oct, 22, 2008:
- fixed memory leak problem with drag 'n drop code
- added automatic memory leak detection code
- set version to 0.39

Oct, 20, 2008:
- support for more patterns
- drag and drop between desktops
- remove Toolstyle property on exit
- restore windows outside 15000 after crash
- set version to 0.38

Aug, 11, 2008:
- same GUI for Vista and XP
- support for silver and green XP panel
- reject windows not owned by the user (admin windows)
- reject notify events if not a windows with a panel button
- more space between pager and tray area
- set version to 0.37

Jul, 19, 2008:
- set WS_EX_TOOLWINDOW for all windows, to avoid wrong notify events
- flash the preview desktop and display a red circle near to the mouse pointer
if a window want to notify the user (new chat message etc)
- removed old flash code
- removed old OSD code
- set version to 0.36experimental

Jun, 2, 2008:
- set WS_EX_TOOLWINDOW for iconic windows too, and on mini-window move
- set fixed bug not displaying window menu for dos windows
- fixed dos window shit for Vista
- set version to 0.35

May, 15, 2008:
- set WS_EX_TOOLWINDOW during hiding to avoid window activating
- display OSD during forced desk change again
- set version to 0.34

May, 10, 2008:
- new design for Vista
- no OSD on normal desktop change
- set version to 0.33

Apr, 08, 2008:
- fixed problems with mini-windows if real window is out off screen
- set version 0.31

Apr, 08, 2008:
- increased space between preview windows for Windows XP
- increased height for 2 row panel
- readded death window checking between desktop switching
- fixed problem with windows visible before windowspager start
- removed unnecessary code
- set version 0.30 'stable?'

Apr, 03, 2008:
- switched from reparent stuff to move the windows JWD
- set version to 0.30-test3

Mar, 17, 2008:
- reactivated mini window move
- reactivated auto desk change
- ehh what ever
- set version to 0.30-test2 'fed up for a while'

Mar, 4, 2008:
- support for Windows Vista (black panel with AERO disabled)
- reparenting instead of hiding it if not visible (Vista)
- Who added the mutex shit ?
- get icon from exe as last chance
- lot of changes
- move miniwindows disabled atm
- automatic deskchange disabled atm
- set version to 0.30-test1 'Nir Halowani is a jackass'

Feb, 2, 2008:
- maybe found the bug now :)
- set version to 0.29b

Feb, 2, 2008:
- maybe found the bug (crash)
- set a maximum pager height even if the panel is larger (suggested byStalkR)
- changed code indention
- set version to 0.29a

Jan, 31, 2008:
- support for Windows 2000 (excluding moving of the preview windows)
- added Keyboard Shortcuts for desktop switching
- support for different panel sizes
- using XWinVer class to retrieve Windows version (http://www.codeproject.com/KB/system/XWinVer.aspx)
- reject windows in MaybeAddWindow() if it could be a tasklistwindow
- move more code to System class
- found crash, cannot reproduce it ;(
- set version to 0.29

Jan, 17, 2008:
- switched to Visual Studio Express
- fixed crash occurred after switch
- removed console build
- added trace function
- added System class
- use of save sprintf_s stuff
- changed code for DOS (cmd.exe) window support
- fixed bug avoid adding a new thread on every move to another workspace
- decreased OSD size
- reset tasklist window size on exit (bug reported by Daniel)
- set version to 0.28

Jan, 02, 2008:
- removed 0.26 version (bugs)
- fixed bugs
- set version to 0.27

Dec, 29, 2007:
- hide all windows before a desk change (MS Word Search window for example)
- set version to 0.26

Dec, 22, 2007:
- fixed problem with windows not visible after desk change
- improved icon quality by using better resizing algo and using the big window icons
- changed preview window background and shadow
- code improvements
- set version to 0.25

Dec, 16, 2007:
- Added "Sticky" windows feature (set window always visible)
- support for windows running from a different user account (e.g. administrator)
- completely remove shell hook
- make use of singleton pattern for Pager class
- fixed bug with "Always on top"
- renamed "Always on top" to "Keep on top"
- change window menu in a loop
- use of flags instead of window properties
- use of ShowOwnedPopups function
- use a mutex to synchronise events
- set version to 0.24

*Nov, 28, 2007:
- support for "flashing-windows", show the button in taskbar if a windows is flashing
- use of RegisterShellWindow, trick found in the "Virtual Dimension" pager
- start "manager.exe" with high priority
- hook window menu little bit later
- set version to 0.21

*Nov, 22, 2007:
- free some memory
- different approach for automatic desk change
- set version to 0.20

*Nov, 21, 2007:
- removed version 0.13 from sourceforge, too bugy
- rescued from 0.13:
- split into 2 different processes for huge safety improvement
- decreased OSD font size
- destroy window object on window hide
- added About and Help Dialog
- added a trick to avoid hourglass cursor on startup - added a watchdog to watch for the end of processes
- set version to 0.15

*Nov, 16, 2007:
- added "spy window" to all windows to detect window death better
- split into 2 different processes for huge safety improvement
- decreased OSD font size
- destroy window object on window hide
- added About and Help Dialog
- fixed some bugs
- set version to 0.13

*Nov, 8, 2007:
- improved some stuff
- fixed problems with Thunderbird mail notifier
- set version to 0.12

*Nov, 5, 2007:
- added "Move to Desktop x" in the windows menu (right click on title, or taskbutton)
- added "Always on top" in the windows menu
- added On Screen Display (displays the Desktop number)
- added problems with Thunderbird
- fixed some bugs
- set version to 0.11

*Oct, 18, 2007:
- huge internal reorganizations for more generic program support
- mouse hook not longer needed
- clicking on a web-link, new email dialog etc. will move to the desktop with the
desired application
- keeping the tasklist.order for programs already running befor windowspager was started
- get icon from the window "owner" (tasklist window) instead of the owned window
- improved "cmd" window suppport
- set version to 0.10

*Oct 6, 2007:
- huge internal improvements to support more programs
- recognize of minimize/maximize of windows
- special "DOS window" (cmd.exe) support (alpha)
- better icon quality inside mini windows
- set version to 0.08

*Aug 26, 2007:
- reject screensaver window
- display miniwindows with an icon
- fixed bug with moving
- added clean program exit
- added "Quit" popup menu
- added "Release" configuration
- fixed small bugs
- set version to 0.05

*Aug, 18, 2007:
- fixed small bug
- set version to 0.02

*Aug, 18 2007:
- initial relases
- set version to 0.01