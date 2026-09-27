# ALAT

ALAT translates configured source-language tokens into a target accent. The command-line program stores settings and the latest restore backup under `%APPDATA%\ALAT`.

## Install

Run the default build task (`Ctrl+Shift+B`) to build both ALAT and its self-contained installer. Then double-click `alat-installer.exe`:

If ALAT is not installed, the installer asks whether to install it under `%LOCALAPPDATA%\Programs\ALAT` and add it to the current user's PATH. If ALAT is already installed, launching the same installer asks whether to uninstall it. Settings and restore data under `%APPDATA%\ALAT` are kept when uninstalling. Open a new command prompt after installation.

## Commands

```text
alat lang <filename>  Set the language syntax file
alat acc <filename>   Set the target accent file
alat tran <filename>  Translate a source file in place
alat rest             Restore the latest translated file
alat stat             Show configured paths
alat lang -o          Open the configured language file
alat acc -o           Open the configured accent file
alat rest -o          Open the latest restore backup
alat -h               Show help
```

Set the language and accent once, then run translations from any directory:

```powershell
alat lang "C:\languages\python.alat"
alat acc "C:\ALAT\accents\my-accent.alat"
alat tran "C:\projects\script.py"
alat rest
```

`tran` replaces its input file. Its original contents are saved as the latest restore backup; `rest` restores the source file from that backup. Each new translation replaces the previous restore backup.

`lang -o`, `acc -o`, and `rest -o` use Windows file associations and do nothing if the requested file does not exist.

## File formats

Language syntax files map token names to source spellings, for example:

```text
IF = if
FUNC_DECL = def
LIBRARY_IMPORT = import
COMMENT = #
```

Accent files map those same token names to replacement spellings, for example:

```text
IF = whenever
FUNC_DECL = define
LIBRARY_IMPORT = bring
COMMENT = #
```

Language profiles are in `languages/`. The token names supported by this build are documented in [languages/README.md](languages/README.md).