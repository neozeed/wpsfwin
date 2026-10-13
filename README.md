# Workplace Shell for Windows — native port R4

An experimental x86/x64 source port of the supplied IBM employee-written
Workplace Shell for Windows 3.1 source. It retains the original C++ object,
folder, drawing, drag/drop, template and settings code.

**Status: R3 is confirmed working by the user. R4 builds for x86/x64;
its new Windows interactions and profile migration need live testing.**

## R4 features

- Explorer file drops now work without Win16 drag negotiation. Drop onto the
  desktop or a virtual folder to create references. Directory drops create
  actual directory objects, rather than empty virtual folders. Imported names
  retain case and extension. Physical directory windows reject this import.
- Program objects use ShellExecuteEx and process handles for the existing
  'display existing instance' setting, with environment-variable expansion,
  working directories, startup state and native error messages. Applications
  that hand off to another process may not be reusable through this tracking.
- Desktop **Window list** lists visible windows and switches to the selected
  one, restoring minimized windows. Refresh rebuilds the list.
- **System setup** opens Windows Control Panel.
- Help menus and F1 open a built-in, scrollable usage guide. It replaces the
  WinHelp dependency; it is not a full conversion of the original help book.
- Data objects support 255-character filenames and 259-character directory
  fields. Full paths are still limited by the application's ANSI/MAX_PATH
  design. Program detection checks the actual extension and includes `.cmd`.
- Desktop saves use a temporary file, flush it, retain the previous profile
  as `wpshell32.dat.bak`, then replace the active profile. Failed exit saves
  keep the application open. Explicit Save Desktop also records placement.

### Upgrade and rollback

Close R3 and keep a separate copy of its EXE/DLL and `wpshell32.dat` before
upgrading. Replace the EXE and DLL together. R4 reads R1-R3 profiles, but writes
an expanded R4 profile format. Older builds cannot read the new format.
The `.bak` file holds only the immediately preceding save, so retain your
separate R3 copy for rollback. Keep `wpsettings.ini` as before.

### Suggested R4 checks

1. Drop a disposable directory and a text file with a filename longer than
   15 characters from Explorer onto the desktop. Open both, save, and reopen.
2. Open a Program object twice with 'display existing instance' enabled,
   close the program, then open it again. Also try a spaced path and `.cmd`.
3. Open Window list, switch to a minimized window, then refresh after closing
   another application. Try System setup and Help/F1.
4. Verify your previous folders, shadows and preferences survive migration.

## R3 features

- Run Program now supports quoted executable paths, arguments, environment
  variables such as `%SystemRoot%`, documents, folders and URLs.
- The command box remembers the last ten successful launches. Browse quotes
  selected paths correctly, and failures leave the dialog open with a Windows
  error message. Help explains the new syntax without requiring WinHelp.
- The desktop remembers its normal position/size and maximized state when
  exiting with desktop saving enabled. Minimized windows reopen normally (or
  maximized if appropriate), and disconnected-monitor positions are clamped
  to an available monitor's work area.

Right-click the desktop and select **Run**. Try `%SystemRoot%\notepad.exe`,
then browse to an executable under Program Files and add any arguments after
its closing quote. Reopen Run to check the recent-command list. Resize or
maximize the desktop, exit with saving enabled, and relaunch.

Run history and window placement use new sections in `wpsettings.ini`.
Delete `[RunHistory]` to clear remembered commands or `[NativeDesktop]` to
reset window placement while retaining other settings.

## R2 corrections

- Drive icons show the drive letter and volume label when available, with a
  drive-letter fallback for unlabelled or unavailable media.
- The top-left system-menu icon is square. Its hit area matches its painting,
  so double-clicking the icon uses the system-menu close action.
- The standalone desktop uses the folder background brush and image painting
  instead of the old transparent Win16 desktop override. Scaled bitmap
  backgrounds are also enabled.

Drive titles refresh when the profile loads. See the R4 upgrade instructions above.

## Try it

1. Extract the whole archive into a writable folder.
2. On 64-bit Windows, run `bin/x64/wpshell.exe`.
   For the 32-bit build, run `bin/x86/wpshell.exe`.
3. Keep the matching `WPS_ENUS.DLL` beside the executable.

The intended first-run result is a resizable 960 x 640 teal desktop window,
with Programs, Drives, Templates and Shredder objects. Programs contains
Notepad and Command Prompt. It runs as an ordinary application.

Double-click objects to open them. Right-click an object or the background
for its menu. Settings opens the notebook. The inherited OS/2-style drag
operation uses the right mouse button. Folder and program templates can be
dragged out to create new objects. These interactions still need live testing.

The desktop's Exit Workplace Shell menu closes this application. Global shell
hooks, host-desktop subclassing, system shutdown and the legacy DDE services
are disabled in this build.

## First test and feedback

Please start with the following small smoke test:

- Does the desktop appear, with four icons and readable labels?
- Open Programs, then launch Notepad.
- Open Programs' Settings, switch notebook tabs, change its title and close it.
- Move an icon, resize the desktop, then exit and reopen it.
- Open Drives and browse a small directory.

`wpshell-port.log` is written beside the EXE and reset at each startup. It
records initialization stages and unhandled exception addresses. Send that
log and a screenshot or short description if something fails. The matching
`wpshell.map` and debug information are included for diagnosis.

## Settings and reset

Each architecture has its own files beside its executable:

- `wpshell32.dat`: native desktop object profile.
- `wpsettings.ini`: preferences formerly stored in SYSTEM.INI.
- `wpshell-port.log`: most recent startup/crash log.

The native profile has a new magic value. It is not compatible with the
original Win16 WPSHELL.INI layout. To reset a test desktop, exit the application
and rename `wpshell32.dat` and `wpsettings.ini`, then launch again.

## Build

Requirements: GNU make, a POSIX shell (Linux, WSL or MSYS2), and MinGW-w64 C++
and windres for the selected architecture. No Python is used by the build or
the application.

```sh
make ARCH=x64 CROSS=x86_64-w64-mingw32 -j4
make ARCH=x86 CROSS=i686-w64-mingw32 -j4
```

Outputs are in `bin/x64` and `bin/x86`. C++ runtime support is linked statically;
the EXE imports Windows system DLLs, without libstdc++/libgcc/libwinpthread DLL
dependencies. Run `make clean` to remove binaries and intermediate objects.

## Changes

- Replaced Win16 callback instance thunks and updated dialog return types.
- Converted object lists, stored object pointers, window extra data, class
  handles and notebook page parameters to pointer-sized representations.
- Corrected WM_COMMAND and scrollbar parameter packing.
- Replaced whole-object constructor memset calls with member initialization;
  the old calls overwrite modern C++ virtual-function tables.
- Replaced DOS enumeration calls with FindFirstFile/FindNextFile wrappers.
- Replaced interrupt-based CD-ROM/RAM-drive probing with native drive queries.
- Raised the internal ANSI path capacity from 63 to 259 characters and fixed
  directory-entry allocations that assumed 8.3 names.
- Replaced the binary-only IBM notebook and spin-button DLL dependencies with
  native controls in `port/controls.cpp`. Notebook tabs retain a simple classic
  appearance. This implements the messages used by this shell, not the entire
  IBM control library.
- Removed CTL3D loading; retained stock Windows controls and original GDI art.
- Rebuilt the original icon, bitmap, cursor, menu, dialog and English resources.
- Added first-run objects, local settings, startup/exception logging and maps.
- Fixed several inherited allocation/deallocation, uninitialized-value,
  pointer-truncation and string-search lifetime problems encountered in porting.

## Boundaries

- English resources only. The remaining languages and support utilities
  (PMTOWP, CONVINI, WNDLIST, LAUNCHPD, WINSHUT and the old installer) are not ported.
- This is not a registered Explorer replacement. Legacy global minimized-window management,
  Program Manager migration and WinOldApp DDE integration remain disabled.
- A native usage guide replaces WinHelp. Complete historical help-topic conversion is not included.
- The replacement notebook implements the shell's required message subset;
  visual fidelity, keyboard navigation and layout need Windows testing.
- ANSI/MAX_PATH-era paths remain. Full Unicode, long-path support and high-DPI
  modernization are not implemented. Very long directory entries are skipped.
- The old file-management/shredder functionality still acts on real files.
  Use disposable files for initial copy/move/delete testing.
- Compiler/PE checks do not establish that folder drawing, launching, drag/drop,
  notebooks or profile persistence work at runtime.

See `docs/BUILD-VERIFICATION.txt` for the checks performed. Original source
copyright notices are retained. The supplied top-level license is in
`LICENSE.TXT`; historical notices and README are under `docs/`.
