# Agent Notes

## Build

Use this command for a Debug Win32 build:

```bat
cmd /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"" x86 && set DX81_SDK=C:\DXSDK\&& msbuild src\FreeFalcon.sln /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v145 /v:m /flp:logfile=build\freefalcon-debug.log;verbosity=normal"
```

## Runtime Data

The local runtime data folder is `Falcon4/` beside `src/`. It is gitignored.

If the Falcon registry key is not present, set `FREEFALCON_BASE_DIR` to `Falcon4` before launching `FFViper.exe`.
