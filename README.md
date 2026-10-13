# Workplace Shell for Windows — Native Port

IBM's Workplace Shell, rescued from Windows 3.1 and rebuilt as a native
32-bit and 64-bit Windows application.

This project ports the 1995 employee-written **Workplace Shell for Windows
1.51** to modern Windows while preserving its unusual OS/2-inspired desktop,
object model, folders, templates, shadows, settings notebooks, drag-and-drop
behavior, animations and original artwork.

It is not an emulator and it does not contain a Win16 compatibility layer.
The application itself has been made pointer-safe, rebuilt with MinGW-w64 and
adapted to current Win32 APIs. Both x86 and x64 binaries are included.

> **Current release: R6b**
>
> The standalone edition has been exercised successfully on Windows 10,
> including folders, drive labels, program launching, settings, animations,
> utilities and desktop background images. The alternate-shell edition builds
> from the same source and is provided first as a safe preview; it should still
> be treated as experimental.

## What is this?

Workplace Shell for Windows was an IBM Employee Written Software project that
brought a surprisingly large part of the OS/2 Workplace Shell experience to
Windows 3.1. Instead of reducing the desktop to groups of icons, it models
folders, programs, data files, drives, shadows, templates and the shredder as
objects with their own menus and settings.

The released source was deeply tied to Win16:

- 16-bit handles, pointers and callback thunks
- binary-only IBM notebook and spin-button controls
- Windows 3.1 shell, DDE and Program Manager assumptions
- DOS interrupts and old drive-enumeration APIs
- tiny fixed path and filename buffers
- WinHelp, CTL3D and Win16 resource conventions
- structure layouts that are unsafe on both Win32 and Win64

This port retains the original application design rather than recreating the
interface from scratch. The old object and folder code is still doing the
work; the incompatible operating-system edges have been replaced around it.

## What users can expect

On first launch, the standalone build opens a resizable classic desktop with:

- **Programs**, containing ready-made Notepad and Command Prompt objects
- **Drives**, showing drive letters and volume labels
- **Templates**, for creating new folders and program objects
- the **Shredder**

Double-click objects to open them. Right-click an object or the desktop for its
menu. Settings opens the familiar multi-page notebook. The inherited
Workplace Shell drag operation uses the **right mouse button**; drag templates
onto the desktop to create objects.

The experience intentionally remains old-fashioned. It looks and behaves much
more like OS/2 and Windows 3.1 than Explorer, but programs and filesystem
operations are real native Windows operations.

### Working native features

- x86 and x64 standalone builds
- persistent object desktop and folder hierarchy
- physical drive and directory browsing
- program, folder, data-file, shadow, template and shredder objects
- right-button object dragging and the original animations
- Explorer-to-WPS file and directory drops
- program launching with arguments, working directories and environment
  variables
- recent Run-command history
- settings notebooks using native replacement controls
- remembered window placement and desktop state
- BMP, PNG and JPEG folder/desktop backgrounds
- centered, tiled and scaled background modes
- built-in Help/F1 guide replacing WinHelp
- native Window List, LaunchPad and Windows Shutdown utilities
- crash/startup diagnostics and linker maps

## Download and run

Extract the complete archive into a writable directory. Do not run it from
inside the ZIP.

For normal 64-bit Windows:

~~~text
bin\x64\wpshell.exe
~~~

For the 32-bit build:

~~~text
bin\x86\wpshell.exe
~~~

Keep <code>WPS_ENUS.DLL</code> beside the matching executable. The utility
executables should remain in the same directory as well.

The standalone edition is an ordinary application. It does **not** replace
Explorer, change the registry shell, modify Winlogon, install startup entries
or reserve the Windows work area.

Windows may display an unrecognized-app warning because these experimental
binaries are not code-signed. Build from source if you prefer.

## Standalone and shell editions

R6 supplies two separately built editions:

| Edition | Executable | Behavior |
| --- | --- | --- |
| Standalone | <code>bin/x64/wpshell.exe</code> | Resizable Workplace Shell window; safest place to start |
| Shell preview | <code>bin/x64/shell/wpshell-shell.exe</code> | Borderless desktop in the monitor work area while Explorer remains active |
| Shell session | <code>bin/x64/shell/wpshell-shell.exe --shell</code> | Full-monitor desktop intended for an explicitly configured test account/session |

Equivalent executables are included under <code>bin/x86</code>.

Double-click the shell executable or run <code>Preview.cmd</code> to try
preview mode. Neither preview nor <code>--shell</code> changes the registry or
installs itself as the Windows shell. Selecting <code>--shell</code> only
changes the application's behavior; making it the logon shell is deliberately
left to the tester.

The shell edition provides bottom controls for **Run**, **Window List**,
**LaunchPad**, **Explorer** and **Exit**. It also registers these shortcuts
while running:

| Shortcut | Action |
| --- | --- |
| Ctrl+Alt+R | Run |
| Ctrl+Alt+W | Window List |
| Ctrl+Alt+F12 | Exit Workplace Shell |

If another application already owns a shortcut, use the on-screen controls or
Alt+F4. In <code>--shell</code> mode, Exit starts Explorer before closing; if
Explorer cannot be started, Workplace Shell remains open.

The shell edition is **not** an Explorer clone. There is no Start menu,
notification area, taskbar/appbar reservation, automatic installation or
recovery service. Test preview mode first and use a disposable Windows account
or VM before experimenting with logon-shell configuration.

Standalone and shell builds keep separate profiles. To clone a desktop, close
both editions and copy <code>wpshell32.dat</code> and
<code>wpsettings.ini</code> into the shell subdirectory.

## Desktop backgrounds

Right-click the desktop, choose **Settings**, then open **Background**.
Browse accepts BMP, PNG and JPEG images.

- **Normal** centers the image at its original size.
- **Tiled** repeats it across the folder.
- **Scaled** stretches it to the current window.

Changes appear immediately. Save the desktop to retain the image path and
mode. The image is referenced rather than embedded, so it must remain at that
path. This changes the Workplace Shell background, not the host Windows
wallpaper.

Transparent source images are flattened onto white. Images above 64 million
pixels are rejected, and paths are still subject to the application's
ANSI/MAX_PATH design.

## Included utilities

The desktop **Utilities** menu launches three native replacements:

### Window List — WNDLIST.EXE

Lists visible top-level windows. Selecting a minimized window restores it and
brings it forward.

### LaunchPad — LAUNCHPD.EXE

Maintains a persistent list of up to 32 programs, shortcuts, documents and
folders. Add entries with Browse or drop them from Explorer. The list is stored
in <code>launchpad.ini</code> beside the utility.

For arguments or a custom working directory, create a normal Windows shortcut
with those properties and add the shortcut to LaunchPad.

### Windows Shutdown — WINSHUT.EXE

Provides lock, sign-out, restart and power-off commands. It defaults to the
non-destructive Lock action and asks for confirmation. It does not force-close
applications.

These operations affect Windows itself, not only Workplace Shell.

## Profiles, upgrades and reset

Runtime state is stored beside each executable:

| File | Purpose |
| --- | --- |
| <code>wpshell32.dat</code> | Native desktop object profile |
| <code>wpshell32.dat.bak</code> | Immediately preceding successfully saved profile |
| <code>wpsettings.ini</code> | Preferences, Run history and window placement |
| <code>wpshell-port.log</code> | Most recent startup/crash diagnostic log |

R4 through R6b use the current profile format. R4 can migrate native R1–R3
profiles, but once saved in the expanded format those older builds cannot read
them. Keep a separate backup before upgrading.

The native profile is not compatible with the original Win16
<code>WPSHELL.INI</code>. Converting a thirty-year-old installation was
intentionally left out of scope.

To reset the desktop, exit the program and rename or remove
<code>wpshell32.dat</code> and <code>wpsettings.ini</code>. A fresh default
desktop will be created on the next launch.

Desktop saves are transactional: a temporary file is written and flushed, the
previous profile is retained as <code>.bak</code>, and only then is the active
profile replaced.

## Building from source

Requirements:

- GNU make
- a POSIX shell under Linux, WSL or MSYS2
- MinGW-w64 C++ compiler and <code>windres</code> for the selected architecture

Python is not required by the build or by the application.

Build all four configurations:

~~~sh
make ARCH=x64 CROSS=x86_64-w64-mingw32 -j4
make ARCH=x64 CROSS=x86_64-w64-mingw32 VARIANT=shell -j4
make ARCH=x86 CROSS=i686-w64-mingw32 -j4
make ARCH=x86 CROSS=i686-w64-mingw32 VARIANT=shell -j4
~~~

Outputs are written beneath <code>bin/x64</code> and <code>bin/x86</code>. The
C++ runtime is linked statically; the executables depend only on Windows system
DLLs. Run <code>make clean</code> to remove generated binaries and intermediate
objects.

The source deliberately stays close to the original late-1990s C++ style.
The build therefore uses GNU++98 rather than attempting an unrelated wholesale
rewrite.

## Porting work

The native port required considerably more than changing the compiler target:

- converted stored pointers, window data, object lists, handles and notebook
  parameters to pointer-sized types
- removed Win16 callback-instance thunks and corrected callback/dialog return
  types
- replaced constructor-time whole-object memory clearing that destroyed modern
  C++ virtual tables
- corrected <code>WM_COMMAND</code>, scrollbar and packed-message handling
- replaced DOS enumeration and interrupt-based drive probing with Win32 APIs
- expanded internal filename/path storage and removed several 8.3 assumptions
- implemented the subset of IBM CUA notebook and spin controls used by the
  application with native controls
- removed the CTL3D and binary-only Win16 DLL requirements
- rebuilt icons, cursors, bitmaps, dialogs, menus and English resources
- replaced Win16 launch paths with <code>ShellExecuteEx</code>
- implemented Explorer file drops without the old Win16 drag negotiation
- added GDI+ image loading for BMP, PNG and JPEG backgrounds
- added local INI settings, safe profile replacement, diagnostics and maps
- corrected inherited allocation, lifetime, string-search, uninitialized-data
  and pointer-truncation bugs exposed by modern compilers
- matched hit testing to the original combined minimize/maximize bitmap instead
  of Windows 10/11's much wider invisible caption-button regions

The R6b caption arrangement is intentionally faithful to the artwork: the
left decorated glyph minimizes/hides, the right glyph maximizes or restores,
and the unused modern caption region remains draggable.

## Known limitations

- English resources only
- ANSI paths and MAX_PATH-era limits; no full Unicode/long-path support
- legacy DPI behavior; no complete per-monitor DPI modernization
- replacement notebook controls implement the message subset this program
  needs, not the complete IBM control libraries
- built-in help is a practical usage guide, not a topic-for-topic conversion
  of the historical WinHelp book
- no automatic Explorer-shell registration or system modification
- no Program Manager migration utility or original installer
- no legacy global minimized-window manager or WinOldApp integration
- original Program Manager/SHELL DDE compatibility services remain disabled
- applications that immediately hand off to another process may not cooperate
  with “display existing instance” tracking

<code>PMTOWP.EXE</code> and <code>CONVINI.EXE</code> are not ported. Their
purpose and binary layouts belong to Windows 3.1 and do not apply safely to
the native profile.

The shredder and file-management commands operate on real files. Begin with
disposable data when testing copy, move or delete behavior.

## Troubleshooting and useful feedback

If startup or an operation fails, include:

1. whether you used x86 or x64, standalone or shell;
2. the Windows version;
3. the steps that reproduced the problem;
4. <code>wpshell-port.log</code>; and
5. a screenshot when the issue is visual.

The matching <code>.map</code> files and debug information are included to
make old-code failures diagnosable rather than mysterious.

Useful first checks are:

- open Programs and launch Notepad;
- change a folder title in Settings and reopen it;
- move an icon, resize or maximize the desktop, save and restart;
- browse a small directory from Drives;
- select a PNG or JPEG background and try all three display modes;
- add and remove a LaunchPad entry; and
- try shell preview without closing Explorer.

## License and provenance

The original program is IBM Employee Written Software, not an official IBM
product, and was distributed without IBM technical support. Original copyright
notices have been retained.

The project is distributed under the **Workplace Shell for Windows Public
License v1.0** in <code>LICENSE.TXT</code>. The historical README and license
material are preserved under <code>docs/</code>.

This port is an independent modernization of the published source. IBM has not
endorsed or supported it.

See <code>docs/BUILD-VERIFICATION.txt</code> for the current compiler and
binary checks.
