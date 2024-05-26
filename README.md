# peTool

## Overview
peTool is a graphical user interface (GUI) tool designed to extract various information from Portable Executable (PE) files. It includes functionalities to extract import tables, export tables, resources, section information, and parse headers.

## Description Video


https://github.com/mendax0110/peTool/assets/52537419/10e026ed-bbae-46ee-8f58-e464a75fc9f9


## Usage (GUI)
./peTool --gui

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
- **8. Disassembler:** Disassembles a executable
- **9. Entropy:** Visualization of the Entropy

## Options (GUI)
- **8. Histogram:** Create a Histogram.

### help
- **--help**
Displays the help message.


## Build
To build the peTool project, navigate to the project directory and run the following command:
```
mkdir build

cd build

cmake --build .
```
