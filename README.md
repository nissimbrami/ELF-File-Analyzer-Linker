# ELF File Analyzer & Linker

A C-based tool for analyzing and linking ELF (Executable and Linkable Format) files. Features include ELF header examination, section name display, symbol table analysis, and basic file merging operations. Built with memory mapping (mmap) for efficient file handling, providing an interactive interface for 32-bit ELF files.

## Author
- **Nissim Brami**

## Table of Contents
- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Features](#features)

## Overview
This tool provides capabilities for:
- ELF file header analysis
- Section name examination
- Symbol table inspection
- File merge compatibility checking
- Basic ELF file merging operations

## Prerequisites
- Linux operating system
- GCC compiler
- Make utility
- 32-bit system libraries

## Installation

1. Clone the repository:
```bash
git clone https://github.com/nissimbrami/elf-analyzer.git
cd elf-analyzer
```

2. Build the project:
```bash
make
```

## Usage

Launch the program:
```bash
./myELF
```

The program presents an interactive menu with the following options:

```
Choose action:
0-Toggle Debug Mode    (Enables/disables detailed output)
1-Examine ELF File    (Load and analyze an ELF file)
2-Print Section Names (Display all section information)
3-Print Symbols       (Show symbol table contents)
4-Check Files for Merge (Verify merge compatibility)
5-Merge ELF Files     (Combine compatible files)
6-Quit               (Exit program)
```

### File Analysis Example
```bash
# Select option 1 to examine an ELF file
1
Enter file name: ./example.o

# The program will display:
# - ELF magic number verification
# - Data encoding information
# - File offsets and sizes
# - Section and program header details
```

### Section Analysis Example
```bash
# Select option 2 to view sections
2

# Output format for each section:
# [index] section_name section_address section_offset section_size section_type
```

### Symbol Table Example
```bash
# Select option 3 to examine symbols
3

# Output format for each symbol:
# [index] value section_index section_name symbol_name
```

## Features in Detail

### 1. ELF File Examination
- Validates ELF file format
- Displays file structure information
- Shows entry points and header locations

### 2. Section Analysis
- Lists all section names
- Shows section sizes and offsets
- Displays section types and addresses

### 3. Symbol Table Analysis
- Shows all defined symbols
- Displays symbol locations
- Indicates symbol definitions

### 4. File Merging
- Checks symbol compatibility
- Verifies section alignment
- Validates merge operations

## License
MIT License

Copyright (c) 2024 Nissim Brami

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
