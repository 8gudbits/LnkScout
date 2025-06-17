# LnkScout

LnkScout is a command-line tool for scanning `.lnk` (shortcut) files in a specified directory. It identifies **valid** and **invalid** shortcuts, optionally removes broken ones, and ensures system-critical shortcuts remain untouched.

## Usage

```bash
LnkScout <directory path> [--valid/-v | --invalid/-i [--remove/-r]] [--sort/-s] [--nobanner/-n] [--help/-h]
```

## Options

- `-v, --valid` Display only valid shortcuts.
- `-i, --invalid` Display only invalid shortcuts.
- `-r, --remove` Remove invalid shortcuts (except protected system files).
- `-s, --sort` Sort output: Skipped first, Valid second, Invalid last.
- `-n, --nobanner` Suppress the banner output.
- `-h, --help` Display this help message.

## Download exe for Windows

This tool is part of the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) project. To download the executable for Windows, visit the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) repository.

## For the Tech People

- Uses **Windows COM API** (`IShellLinkW`, `IPersistFile`) to resolve `.lnk` files.
- Checks shortcut validity by verifying the **target path** exists.
- Uses **recursive directory scanning** to process all subdirectories.
- Supports **sorting** results for better readability.
- Handles **CLI arguments** using `std::vector` for input parsing.
- Ensures **safe deletion** of invalid shortcuts while preserving system stability.

Note: This tool is designed for Windows environments and may not work on other platforms.
