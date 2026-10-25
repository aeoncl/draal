# Draal

Draal is a proxy executable that creates a suspended target process, then injects / removes libraries from the targets import table.
You can specify in a configuration file which dlls to remove and which to add.
Draal is compatible with COM invocations since it passes through the working directory of the target process and the command line arguments it receives.

### Features
- [x] exe import table add imports
- [x] exe import table remove imports
- [x] argument passthrough & workdir (for COM invocations) 
- [x] config file
- [x] App model id override (used by the shell for taskbar window grouping)


### Usage

1. Place `draal.exe` next to your target executable, make sure you match the bitness of your target process (32bits or 64bits)

2. Place a `draal.ini` configuration file next to `draal.exe`

```
[draal]
target_executable = "myprocess.exe"
dll_to_remove = "annoyingdll.dll"
dll_to_inject = "amazingdll.dll"
app_model_id_override = "MY.AMAZING.APP"

[logs]
enabled = true
```

### Debug logs
If you enable logging in the config file, logs will be located in `%appdata%/Draal/*.log`

### Wine compatibility
Draal works with wine in the `debugger` instrumentation mode (default)

## Building
You need Visual Studio 2019 with Windows XP Build Tools (v140) installed and C++ support.
