# peTool

## Overview
peTool is a graphical user interface (GUI) tool designed to extract various information from Portable Executable (PE) files. It includes functionalities to extract import tables, export tables, resources, section information, and parse headers.

## Description
The `peTool` project comprises a single C++ source file named `peTool.cpp`, which implements the functionality of the peTool GUI. This file includes the necessary header files: `PEHeader.h`, `FileIO.h`, `Entropy.h`, and `Utils.h`, along with standard C++ library headers.



https://github.com/mendax0110/peTool/assets/52537419/5f0a372d-3976-4319-a759-f15b637551c3



## Usage (GUI)
./peTool

## Usage (CLI)
./peTool <option> <file_path>

## Options

- **1. Extract Import Table:** Extracts the import table from the specified PE file.
- **2. Extract Export Table:** Extracts the export table from the specified PE file.
- **3. Extract Resources:** Extracts resources from the specified PE file.
- **4. Extract Section Info:** Extracts section information from the specified PE file.
- **5. Parse Headers:** Parses headers of the specified PE file.
- **6. Get Process ID:** Retrieves the process ID for injection.
- **7. Inject DLL:** Injects a DLL into a process.

## Options (GUI)
- **8. Histogram:** Create a Histogram.

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

## Run (CLI)
To run the peTool project, navigate to the build directory and run the following command:
```
./peTool <option> <file_path>
```

## Run Injection (CLI)
```
./peTool <option> <procName>
```

```
./peTool <option> <procId> <dllPath>
```

## Example (CLI)
```
./peTool 1 /path/to/pe/file
```
