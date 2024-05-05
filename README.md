# peTool

## Overview
peTool is a command-line tool designed to extract various information from Portable Executable (PE) files. It includes functionalities to extract import tables, export tables, resources, section information, and parse headers.

## Description
The `peTool` project comprises a single C++ source file named `peTool.cpp`, which implements the functionality of the peTool command-line interface. This file includes the necessary header files: `PEHeader.h`, `FileIO.h`, and `Utils.h`, along with standard C++ library headers.

## Usage
./pe_tool <option> <file_path>

## Options

- **1. Extract Import Table:** Extracts the import table from the specified PE file.
- **2. Extract Export Table:** Extracts the export table from the specified PE file.
- **3. Extract Resources:** Extracts resources from the specified PE file.
- **4. Extract Section Info:** Extracts section information from the specified PE file.
- **5. Parse Headers:** Parses headers of the specified PE file.
- **6. Get Process ID:** Retrieves the process ID for injection.
- **7. Inject DLL:** Injects a DLL into a process.

### help
- **-help**
Displays the help message.


## Build
To build the peTool project, navigate to the project directory and run the following command:
```
mkdir build

cd build

cmake --build .
```

## Run
To run the peTool project, navigate to the build directory and run the following command:
```
./peTool <option> <file_path>
```

## Run Injection
```
./peTool <option> <procName>
```

```
./peTool <option> <procId> <dllPath>
```

## Example
```
./peTool 1 /path/to/pe/file
```
