<p align="center">
  <img src="https://cdn-icons-png.flaticon.com/512/6295/6295417.png" width="100" />
</p>
<p align="center">
    <h1 align="center">PETOOL</h1>
</p>
<p align="center">
	<img src="https://img.shields.io/github/license/mendax0110/peTool?style=flat&color=0080ff" alt="license">
	<img src="https://img.shields.io/github/last-commit/mendax0110/peTool?style=flat&logo=git&logoColor=white&color=0080ff" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/mendax0110/peTool?style=flat&color=0080ff" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/mendax0110/peTool?style=flat&color=0080ff" alt="repo-language-count">
<p>
<p align="center">
		<em>Developed with the software and tools below.</em>
</p>
<p align="center">
	<img src="https://img.shields.io/badge/C-A8B9CC.svg?style=flat&logo=C&logoColor=black" alt="C">
	<img src="https://img.shields.io/badge/GitHub%20Actions-2088FF.svg?style=flat&logo=GitHub-Actions&logoColor=white" alt="GitHub%20Actions">
</p>
<hr>

## Demo Video

https://github.com/mendax0110/peTool/assets/52537419/10e026ed-bbae-46ee-8f58-e464a75fc9f9

---

##  Quick Links

> - [ Repository Structure](#-repository-structure)
> - [ Modules](#-modules)
> - [ Getting Started](#-getting-started)
    >   - [ Installation](#-installation)
>   - [ Running peTool](#-running-peTool)
> - [ Contributing](#-contributing)
> - [ License](#-license)
> - [ Acknowledgments](#-acknowledgments)

##  Repository Structure

```sh
└── peTool/
    ├── .github
    │   └── workflows
    │       ├── cmake-macos-platform.yml
    │       ├── cmake-windows-platform.yml
    │       ├── macos-release.yml
    │       └── windows-release.yml
    ├── CMakeLists.txt
    ├── Doxyfile
    ├── Doxyfile.in
    ├── License.txt
    ├── README.md
    ├── docs
    │   └── Doxygen
    │       └── html
    │           ├── _c_l_i_8cpp.html
    │           ├── _c_l_i_8cpp_8o_8d.html
    │           ├── _c_l_i_8h.html
    │           ├── _c_l_i_8h_source.html
    │           ├── _c_make_c_compiler_id_8c.html
    │           ├── _c_make_c_x_x_compiler_id_8cpp.html
    │           ├── _console_8cpp.html
    │           ├── _console_8cpp_8o_8d.html
    │           ├── _console_8h.html
    │           ├── _console_8h_source.html
    │           ├── _database_8cpp.html
    │           ├── _database_8cpp_8o_8d.html
    │           ├── _database_8h.html
    │           ├── _database_8h_source.html
    │           ├── _debugger_8cpp.html
    │           ├── _debugger_8cpp_8o_8d.html
    │           ├── _debugger_8h.html
    │           ├── _debugger_8h_source.html
    │           ├── _detector_8cpp.html
    │           ├── _detector_8cpp_8o_8d.html
    │           ├── _detector_8h.html
    │           ├── _detector_8h_source.html
    │           ├── _disassembler_8cpp.html
    │           ├── _disassembler_8cpp_8o_8d.html
    │           ├── _disassembler_8h.html
    │           ├── _disassembler_8h_source.html
    │           ├── _entropy_8cpp.html
    │           ├── _entropy_8cpp_8o_8d.html
    │           ├── _entropy_8h.html
    │           ├── _entropy_8h_source.html
    │           ├── _file_editor_8cpp.html
    │           ├── _file_editor_8cpp_8o_8d.html
    │           ├── _file_editor_8h.html
    │           ├── _file_editor_8h_source.html
    │           ├── _file_i_o_8cpp.html
    │           ├── _file_i_o_8cpp_8o_8d.html
    │           ├── _file_i_o_8h.html
    │           ├── _file_i_o_8h_source.html
    │           ├── _graph_view_8cpp.html
    │           ├── _graph_view_8cpp_8o_8d.html
    │           ├── _graph_view_8h.html
    │           ├── _graph_view_8h_source.html
    │           ├── _injector_8cpp.html
    │           ├── _injector_8cpp_8o_8d.html
    │           ├── _injector_8h.html
    │           ├── _injector_8h_source.html
    │           ├── _logger_8cpp.html
    │           ├── _logger_8cpp_8o_8d.html
    │           ├── _logger_8h.html
    │           ├── _logger_8h_source.html
    │           ├── _mem_profiler_8cpp.html
    │           ├── _mem_profiler_8cpp_8o_8d.html
    │           ├── _mem_profiler_8h.html
    │           ├── _mem_profiler_8h_source.html
    │           ├── _memory_manager_8cpp.html
    │           ├── _memory_manager_8cpp_8o_8d.html
    │           ├── _memory_manager_8h.html
    │           ├── _memory_manager_8h_source.html
    │           ├── _p_e_8cpp.html
    │           ├── _p_e_8cpp_8o_8d.html
    │           ├── _p_e_8h.html
    │           ├── _p_e_8h_source.html
    │           ├── _perf_mon_8cpp.html
    │           ├── _perf_mon_8cpp_8o_8d.html
    │           ├── _perf_mon_8h.html
    │           ├── _perf_mon_8h_source.html
    │           ├── _process_monitor_8cpp.html
    │           ├── _process_monitor_8cpp_8o_8d.html
    │           ├── _process_monitor_8h.html
    │           ├── _process_monitor_8h_source.html
    │           ├── _rebuilder_8cpp.html
    │           ├── _rebuilder_8cpp_8o_8d.html
    │           ├── _rebuilder_8h.html
    │           ├── _rebuilder_8h_source.html
    │           ├── _threading_base_8cpp.html
    │           ├── _threading_base_8cpp_8o_8d.html
    │           ├── _threading_base_8h.html
    │           ├── _threading_base_8h_source.html
    │           ├── _untils_8cpp.html
    │           ├── _untils_8cpp_8o_8d.html
    │           ├── _utils_8h.html
    │           ├── _utils_8h_source.html
    │           ├── annotated.html
    │           ├── bc_s.png
    │           ├── bc_sd.png
    │           ├── class_base_mem_profiler-members.html
    │           ├── class_base_mem_profiler.html
    │           ├── class_base_mem_profiler.png
    │           ├── class_cli_interface_1_1_c_l_i-members.html
    │           ├── class_cli_interface_1_1_c_l_i.html
    │           ├── class_console_internals_1_1_console-members.html
    │           ├── class_console_internals_1_1_console.html
    │           ├── class_database-members.html
    │           ├── class_database.html
    │           ├── class_debugger-members.html
    │           ├── class_debugger.html
    │           ├── class_detector_internals_1_1_anti_debug_detection-members.html
    │           ├── class_detector_internals_1_1_anti_debug_detection.html
    │           ├── class_detector_internals_1_1_packer_detection-members.html
    │           ├── class_detector_internals_1_1_packer_detection.html
    │           ├── class_dissassembler_internals_1_1_disassembler-members.html
    │           ├── class_dissassembler_internals_1_1_disassembler.html
    │           ├── class_dll_injector_1_1_injector_platform-members.html
    │           ├── class_dll_injector_1_1_injector_platform.html
    │           ├── class_entropy_internals_1_1_entropy-members.html
    │           ├── class_entropy_internals_1_1_entropy.html
    │           ├── class_file_editor_internals_1_1_file_editor-members.html
    │           ├── class_file_editor_internals_1_1_file_editor.html
    │           ├── class_file_io_internals_1_1_file_i_o-members.html
    │           ├── class_file_io_internals_1_1_file_i_o.html
    │           ├── class_graph_view-members.html
    │           ├── class_graph_view.html
    │           ├── class_logger-members.html
    │           ├── class_logger.html
    │           ├── class_mac_mem_profiler-members.html
    │           ├── class_mac_mem_profiler.html
    │           ├── class_mac_mem_profiler.png
    │           ├── class_mem_profiler-members.html
    │           ├── class_mem_profiler.html
    │           ├── class_memory_manager-members.html
    │           ├── class_memory_manager.html
    │           ├── class_pe_internals_1_1_p_e-members.html
    │           ├── class_pe_internals_1_1_p_e.html
    │           ├── class_pe_internals_1_1_resource_directory-members.html
    │           ├── class_pe_internals_1_1_resource_directory.html
    │           ├── class_pe_internals_1_1_resource_directory_traverser-members.html
    │           ├── class_pe_internals_1_1_resource_directory_traverser.html
    │           ├── class_pe_internals_1_1_resource_directory_traverser_factory-members.html
    │           ├── class_pe_internals_1_1_resource_directory_traverser_factory.html
    │           ├── class_performance_monitor-members.html
    │           ├── class_performance_monitor.html
    │           ├── class_process_monitor-members.html
    │           ├── class_process_monitor.html
    │           ├── class_rebuilder-members.html
    │           ├── class_rebuilder.html
    │           ├── class_threading_base-members.html
    │           ├── class_threading_base.html
    │           ├── class_utils_internals_1_1_utils-members.html
    │           ├── class_utils_internals_1_1_utils.html
    │           ├── class_win_mem_profiler-members.html
    │           ├── class_win_mem_profiler.html
    │           ├── class_win_mem_profiler.png
    │           ├── classes.html
    │           ├── closed.png
    │           ├── dir_017ad5e908c41501f6a06b5119b37293.html
    │           ├── dir_1172bb5c332d8aaafdf275e832708c31.html
    │           ├── dir_2369ca6a9db3fb08286493d49901ef6e.html
    │           ├── dir_3573a29eb2bcb9c145a8a4e604b93f65.html
    │           ├── dir_4581fb84a8bf7da1ab9e0691301abbd4.html
    │           ├── dir_63189e78386bdf55ec00d64f4a5092fb.html
    │           ├── dir_6782cefcabc378658964ed864e935d2d.html
    │           ├── dir_68267d1309a1af8e8297ef4c3efbcdba.html
    │           ├── dir_6f85998d687e06b4b13364dea3046761.html
    │           ├── dir_7d419bccfa827d7fbeff703abd74ab96.html
    │           ├── dir_80413701ebb2905e45ccebf8c9ab3532.html
    │           ├── dir_84a7603056c012692a626a8a53c537f2.html
    │           ├── dir_88f45123b231e2cd697a666f8850ff1b.html
    │           ├── dir_8ddf2797e2f76e55e538b7ce4720fc2b.html
    │           ├── dir_9af8047499592558425159c869a308bb.html
    │           ├── dir_a18918b93668b435612395bbc2e8b82b.html
    │           ├── dir_bae135b08326cef4e17362485455e9f4.html
    │           ├── dir_d44c64559bbebec7f509842c48db8b23.html
    │           ├── dir_d6dcfb7f7658843f68ddeda5b429b6b7.html
    │           ├── dir_edec38e5d743c07e5c2e29043a4d94a8.html
    │           ├── dir_f584182df4c69fab0b14563b4d535158.html
    │           ├── dir_fecb511e6ef370f316cfc3c5cc95d0a0.html
    │           ├── doc.svg
    │           ├── docd.svg
    │           ├── doxygen.css
    │           ├── doxygen.svg
    │           ├── dynsections.js
    │           ├── files.html
    │           ├── folderclosed.svg
    │           ├── folderclosedd.svg
    │           ├── folderopen.svg
    │           ├── folderopend.svg
    │           ├── functions.html
    │           ├── functions_b.html
    │           ├── functions_c.html
    │           ├── functions_d.html
    │           ├── functions_e.html
    │           ├── functions_enum.html
    │           ├── functions_f.html
    │           ├── functions_func.html
    │           ├── functions_func_b.html
    │           ├── functions_func_c.html
    │           ├── functions_func_d.html
    │           ├── functions_func_e.html
    │           ├── functions_func_f.html
    │           ├── functions_func_g.html
    │           ├── functions_func_h.html
    │           ├── functions_func_i.html
    │           ├── functions_func_j.html
    │           ├── functions_func_l.html
    │           ├── functions_func_m.html
    │           ├── functions_func_o.html
    │           ├── functions_func_p.html
    │           ├── functions_func_r.html
    │           ├── functions_func_s.html
    │           ├── functions_func_t.html
    │           ├── functions_func_u.html
    │           ├── functions_func_v.html
    │           ├── functions_func_w.html
    │           ├── functions_func_~.html
    │           ├── functions_g.html
    │           ├── functions_h.html
    │           ├── functions_i.html
    │           ├── functions_j.html
    │           ├── functions_l.html
    │           ├── functions_m.html
    │           ├── functions_n.html
    │           ├── functions_o.html
    │           ├── functions_p.html
    │           ├── functions_q.html
    │           ├── functions_r.html
    │           ├── functions_s.html
    │           ├── functions_t.html
    │           ├── functions_type.html
    │           ├── functions_u.html
    │           ├── functions_v.html
    │           ├── functions_vars.html
    │           ├── functions_w.html
    │           ├── functions_~.html
    │           ├── globals.html
    │           ├── globals_defs.html
    │           ├── globals_func.html
    │           ├── globals_vars.html
    │           ├── hierarchy.html
    │           ├── imgui_8cpp_8o_8d.html
    │           ├── imgui__demo_8cpp_8o_8d.html
    │           ├── imgui__draw_8cpp_8o_8d.html
    │           ├── imgui__impl__metal_8mm_8o_8d.html
    │           ├── imgui__impl__sdl2_8cpp_8o_8d.html
    │           ├── imgui__tables_8cpp_8o_8d.html
    │           ├── imgui__widgets_8cpp_8o_8d.html
    │           ├── index.html
    │           ├── jquery.js
    │           ├── linux_2linux_8cpp_8o_8d.html
    │           ├── linux_8cpp.html
    │           ├── linux_8cpp_8o_8d.html
    │           ├── macos_2macos_8mm_8o_8d.html
    │           ├── macos_8mm.html
    │           ├── macos_8mm_8o_8d.html
    │           ├── menu.js
    │           ├── menudata.js
    │           ├── minus.svg
    │           ├── minusd.svg
    │           ├── namespace_cli_interface.html
    │           ├── namespace_console_internals.html
    │           ├── namespace_detector_internals.html
    │           ├── namespace_dissassembler_internals.html
    │           ├── namespace_dll_injector.html
    │           ├── namespace_entropy_internals.html
    │           ├── namespace_file_editor_internals.html
    │           ├── namespace_file_io_internals.html
    │           ├── namespace_injector_platform.html
    │           ├── namespace_pe_internals.html
    │           ├── namespace_utils_internals.html
    │           ├── namespaces.html
    │           ├── nav_f.png
    │           ├── nav_fd.png
    │           ├── nav_g.png
    │           ├── nav_h.png
    │           ├── nav_hd.png
    │           ├── open.png
    │           ├── plus.svg
    │           ├── plusd.svg
    │           ├── search
    │           │   ├── all_0.js
    │           │   ├── all_1.js
    │           │   ├── all_10.js
    │           │   ├── all_11.js
    │           │   ├── all_12.js
    │           │   ├── all_13.js
    │           │   ├── all_14.js
    │           │   ├── all_15.js
    │           │   ├── all_16.js
    │           │   ├── all_2.js
    │           │   ├── all_3.js
    │           │   ├── all_4.js
    │           │   ├── all_5.js
    │           │   ├── all_6.js
    │           │   ├── all_7.js
    │           │   ├── all_8.js
    │           │   ├── all_9.js
    │           │   ├── all_a.js
    │           │   ├── all_b.js
    │           │   ├── all_c.js
    │           │   ├── all_d.js
    │           │   ├── all_e.js
    │           │   ├── all_f.js
    │           │   ├── classes_0.js
    │           │   ├── classes_1.js
    │           │   ├── classes_2.js
    │           │   ├── classes_3.js
    │           │   ├── classes_4.js
    │           │   ├── classes_5.js
    │           │   ├── classes_6.js
    │           │   ├── classes_7.js
    │           │   ├── classes_8.js
    │           │   ├── classes_9.js
    │           │   ├── classes_a.js
    │           │   ├── classes_b.js
    │           │   ├── classes_c.js
    │           │   ├── classes_d.js
    │           │   ├── classes_e.js
    │           │   ├── classes_f.js
    │           │   ├── close.svg
    │           │   ├── defines_0.js
    │           │   ├── defines_1.js
    │           │   ├── defines_2.js
    │           │   ├── defines_3.js
    │           │   ├── defines_4.js
    │           │   ├── defines_5.js
    │           │   ├── defines_6.js
    │           │   ├── enums_0.js
    │           │   ├── enums_1.js
    │           │   ├── enumvalues_0.js
    │           │   ├── enumvalues_1.js
    │           │   ├── enumvalues_2.js
    │           │   ├── enumvalues_3.js
    │           │   ├── enumvalues_4.js
    │           │   ├── files_0.js
    │           │   ├── files_1.js
    │           │   ├── files_2.js
    │           │   ├── files_3.js
    │           │   ├── files_4.js
    │           │   ├── files_5.js
    │           │   ├── files_6.js
    │           │   ├── files_7.js
    │           │   ├── files_8.js
    │           │   ├── files_9.js
    │           │   ├── files_a.js
    │           │   ├── files_b.js
    │           │   ├── files_c.js
    │           │   ├── functions_0.js
    │           │   ├── functions_1.js
    │           │   ├── functions_10.js
    │           │   ├── functions_11.js
    │           │   ├── functions_12.js
    │           │   ├── functions_13.js
    │           │   ├── functions_14.js
    │           │   ├── functions_2.js
    │           │   ├── functions_3.js
    │           │   ├── functions_4.js
    │           │   ├── functions_5.js
    │           │   ├── functions_6.js
    │           │   ├── functions_7.js
    │           │   ├── functions_8.js
    │           │   ├── functions_9.js
    │           │   ├── functions_a.js
    │           │   ├── functions_b.js
    │           │   ├── functions_c.js
    │           │   ├── functions_d.js
    │           │   ├── functions_e.js
    │           │   ├── functions_f.js
    │           │   ├── mag.svg
    │           │   ├── mag_d.svg
    │           │   ├── mag_sel.svg
    │           │   ├── mag_seld.svg
    │           │   ├── namespaces_0.js
    │           │   ├── namespaces_1.js
    │           │   ├── namespaces_2.js
    │           │   ├── namespaces_3.js
    │           │   ├── namespaces_4.js
    │           │   ├── namespaces_5.js
    │           │   ├── namespaces_6.js
    │           │   ├── search.css
    │           │   ├── search.js
    │           │   ├── searchdata.js
    │           │   ├── typedefs_0.js
    │           │   ├── variables_0.js
    │           │   ├── variables_1.js
    │           │   ├── variables_10.js
    │           │   ├── variables_2.js
    │           │   ├── variables_3.js
    │           │   ├── variables_4.js
    │           │   ├── variables_5.js
    │           │   ├── variables_6.js
    │           │   ├── variables_7.js
    │           │   ├── variables_8.js
    │           │   ├── variables_9.js
    │           │   ├── variables_a.js
    │           │   ├── variables_b.js
    │           │   ├── variables_c.js
    │           │   ├── variables_d.js
    │           │   ├── variables_e.js
    │           │   └── variables_f.js
    │           ├── splitbar.png
    │           ├── splitbard.png
    │           ├── struct_connection-members.html
    │           ├── struct_connection.html
    │           ├── struct_file_editor_internals_1_1_file_editor_1_1_edit_action-members.html
    │           ├── struct_file_editor_internals_1_1_file_editor_1_1_edit_action.html
    │           ├── struct_file_editor_internals_1_1_file_editor_1_1_position-members.html
    │           ├── struct_file_editor_internals_1_1_file_editor_1_1_position.html
    │           ├── struct_logger_1_1_log_message-members.html
    │           ├── struct_logger_1_1_log_message.html
    │           ├── struct_memory_manager_1_1_allocation_info-members.html
    │           ├── struct_memory_manager_1_1_allocation_info.html
    │           ├── struct_node-members.html
    │           ├── struct_node.html
    │           ├── struct_performance_monitor_1_1_performance_data-members.html
    │           ├── struct_performance_monitor_1_1_performance_data.html
    │           ├── sync_off.png
    │           ├── sync_on.png
    │           ├── tab_a.png
    │           ├── tab_ad.png
    │           ├── tab_b.png
    │           ├── tab_bd.png
    │           ├── tab_h.png
    │           ├── tab_hd.png
    │           ├── tab_s.png
    │           ├── tab_sd.png
    │           ├── tabs.css
    │           ├── windows_2windows_8cpp_8o_8d.html
    │           ├── windows_8cpp.html
    │           └── windows_8cpp_8o_8d.html
    ├── external
    ├── icon
    │   └── cog.icns
    ├── include
    │   ├── CLI
    │   │   ├── CLI.h
    │   │   └── Console.h
    │   ├── CORE
    │   │   ├── Debugger.h
    │   │   ├── Detector.h
    │   │   ├── Disassembler.h
    │   │   ├── Injector.h
    │   │   ├── Logger.h
    │   │   ├── PE.h
    │   │   ├── ProcessMonitor.h
    │   │   ├── Rebuilder.h
    │   │   └── ThreadingBase.h
    │   ├── FILEIO
    │   │   ├── Database.h
    │   │   ├── FileEditor.h
    │   │   ├── FileIO.h
    │   │   └── Utils.h
    │   ├── MANMON
    │   │   ├── MemProfiler.h
    │   │   ├── MemoryManager.h
    │   │   └── PerfMon.h
    │   └── VIEW
    │       ├── Entropy.h
    │       └── GraphView.h
    └── src
        ├── CLI.cpp
        ├── Console.cpp
        ├── Database.cpp
        ├── Debugger.cpp
        ├── Detector.cpp
        ├── Disassembler.cpp
        ├── Entropy.cpp
        ├── FileEditor.cpp
        ├── FileIO.cpp
        ├── GraphView.cpp
        ├── Injector.cpp
        ├── Logger.cpp
        ├── MemProfiler.cpp
        ├── MemoryManager.cpp
        ├── PE.cpp
        ├── PerfMon.cpp
        ├── ProcessMonitor.cpp
        ├── Rebuilder.cpp
        ├── ThreadingBase.cpp
        ├── Untils.cpp
        ├── linux
        │   └── linux.cpp
        ├── macos
        │   └── macos.mm
        └── windows
            └── windows.cpp
```
---

##  Modules

<details closed><summary>.</summary>

| File                                                                              | Summary          |
| ---                                                                               |------------------|
| [License.txt](https://github.com/mendax0110/peTool/blob/master/License.txt)       | `License.txt`    |
| [Doxyfile.in](https://github.com/mendax0110/peTool/blob/master/Doxyfile.in)       | `Doxyfile.in`    |
| [Doxyfile](https://github.com/mendax0110/peTool/blob/master/Doxyfile)             | `Doxyfile`       |
| [CMakeLists.txt](https://github.com/mendax0110/peTool/blob/master/CMakeLists.txt) | `CMakeLists.txt` |

</details>

<details closed><summary>include.VIEW</summary>

| File                                                                                     | Summary                    |
| ---                                                                                      | ---                                                  |
| [Entropy.h](https://github.com/mendax0110/peTool/blob/master/include/VIEW/Entropy.h)     | `include/VIEW/Entropy.h`   |
| [GraphView.h](https://github.com/mendax0110/peTool/blob/master/include/VIEW/GraphView.h) | `include/VIEW/GraphView.h` |

</details>

<details closed><summary>include.CORE</summary>

| File                                                                                               | Summary                         |
| ---                                                                                                |---------------------------------|
| [Logger.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Logger.h)                 | `include/CORE/Logger.h`         |
| [ProcessMonitor.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/ProcessMonitor.h) | `include/CORE/ProcessMonitor.h` |
| [PE.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/PE.h)                         | `include/CORE/PE.h`             |
| [Injector.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Injector.h)             | `include/CORE/Injector.h`       |
| [Rebuilder.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Rebuilder.h)           | `include/CORE/Rebuilder.h`      |
| [Debugger.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Debugger.h)             | `include/CORE/Debugger.h`       |
| [Detector.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Detector.h)             | `include/CORE/Detector.h`       |
| [ThreadingBase.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/ThreadingBase.h)   | `include/CORE/ThreadingBase.h`  |
| [Disassembler.h](https://github.com/mendax0110/peTool/blob/master/include/CORE/Disassembler.h)     | `include/CORE/Disassembler.h`   |

</details>

<details closed><summary>include.MANMON</summary>

| File                                                                                               | Summary                          |
| ---                                                                                                |----------------------------------|
| [PerfMon.h](https://github.com/mendax0110/peTool/blob/master/include/MANMON/PerfMon.h)             | `include/MANMON/PerfMon.h`       |
| [MemoryManager.h](https://github.com/mendax0110/peTool/blob/master/include/MANMON/MemoryManager.h) | `include/MANMON/MemoryManager.h` |
| [MemProfiler.h](https://github.com/mendax0110/peTool/blob/master/include/MANMON/MemProfiler.h)     | `include/MANMON/MemProfiler.h`   |

</details>

<details closed><summary>include.CLI</summary>

| File                                                                                | Summary                  |
| ---                                                                                 |--------------------------|
| [Console.h](https://github.com/mendax0110/peTool/blob/master/include/CLI/Console.h) | `include/CLI/Console.h`  |
| [CLI.h](https://github.com/mendax0110/peTool/blob/master/include/CLI/CLI.h)         | `include/CLI/CLI.h`      |

</details>

<details closed><summary>include.FILEIO</summary>

| File                                                                                         | Summary                         |
| ---                                                                                          |---------------------------------|
| [Utils.h](https://github.com/mendax0110/peTool/blob/master/include/FILEIO/Utils.h)           | `include/FILEIO/Utils.h`        |
| [FileEditor.h](https://github.com/mendax0110/peTool/blob/master/include/FILEIO/FileEditor.h) | `include/FILEIO/FileEditor.h`   |
| [Database.h](https://github.com/mendax0110/peTool/blob/master/include/FILEIO/Database.h)     | `include/FILEIO/Database.h`     |
| [FileIO.h](https://github.com/mendax0110/peTool/blob/master/include/FILEIO/FileIO.h)         | `include/FILEIO/FileIO.h`       |

</details>

<details closed><summary>.github.workflows</summary>

| File                                                                                                                        | Summary                                        |
| ---                                                                                                                         |------------------------------------------------|
| [cmake-macos-platform.yml](https://github.com/mendax0110/peTool/blob/master/.github/workflows/cmake-macos-platform.yml)     | `.github/workflows/cmake-macos-platform.yml`   |
| [macos-release.yml](https://github.com/mendax0110/peTool/blob/master/.github/workflows/macos-release.yml)                   | `.github/workflows/macos-release.yml`          |
| [cmake-windows-platform.yml](https://github.com/mendax0110/peTool/blob/master/.github/workflows/cmake-windows-platform.yml) | `.github/workflows/cmake-windows-platform.yml` |
| [windows-release.yml](https://github.com/mendax0110/peTool/blob/master/.github/workflows/windows-release.yml)               | `.github/workflows/windows-release.yml`        |

</details>

<details closed><summary>src</summary>

| File                                                                                          | Summary                    |
| ---                                                                                           |----------------------------|
| [Entropy.cpp](https://github.com/mendax0110/peTool/blob/master/src/Entropy.cpp)               | `src/Entropy.cpp`          |
| [MemoryManager.cpp](https://github.com/mendax0110/peTool/blob/master/src/MemoryManager.cpp)   | `src/MemoryManager.cpp`    |
| [FileIO.cpp](https://github.com/mendax0110/peTool/blob/master/src/FileIO.cpp)                 | `src/FileIO.cpp`           |
| [PE.cpp](https://github.com/mendax0110/peTool/blob/master/src/PE.cpp)                         | `src/PE.cpp`               |
| [FileEditor.cpp](https://github.com/mendax0110/peTool/blob/master/src/FileEditor.cpp)         | `src/FileEditor.cpp`       |
| [Untils.cpp](https://github.com/mendax0110/peTool/blob/master/src/Untils.cpp)                 | `src/Untils.cpp`           |
| [PerfMon.cpp](https://github.com/mendax0110/peTool/blob/master/src/PerfMon.cpp)               | `src/PerfMon.cpp`          |
| [Injector.cpp](https://github.com/mendax0110/peTool/blob/master/src/Injector.cpp)             | `src/Injector.cpp`         |
| [Logger.cpp](https://github.com/mendax0110/peTool/blob/master/src/Logger.cpp)                 | `src/Logger.cpp`           |
| [Database.cpp](https://github.com/mendax0110/peTool/blob/master/src/Database.cpp)             | `src/Database.cpp`         |
| [ThreadingBase.cpp](https://github.com/mendax0110/peTool/blob/master/src/ThreadingBase.cpp)   | `src/ThreadingBase.cpp`    |
| [Debugger.cpp](https://github.com/mendax0110/peTool/blob/master/src/Debugger.cpp)             | `src/Debugger.cpp`         |
| [GraphView.cpp](https://github.com/mendax0110/peTool/blob/master/src/GraphView.cpp)           | `src/GraphView.cpp`        |
| [Disassembler.cpp](https://github.com/mendax0110/peTool/blob/master/src/Disassembler.cpp)     | `src/Disassembler.cpp`     |
| [MemProfiler.cpp](https://github.com/mendax0110/peTool/blob/master/src/MemProfiler.cpp)       | `src/MemProfiler.cpp`      |
| [Console.cpp](https://github.com/mendax0110/peTool/blob/master/src/Console.cpp)               | `src/Console.cpp`          |
| [ProcessMonitor.cpp](https://github.com/mendax0110/peTool/blob/master/src/ProcessMonitor.cpp) | `src/ProcessMonitor.cpp`   |
| [CLI.cpp](https://github.com/mendax0110/peTool/blob/master/src/CLI.cpp)                       | `src/CLI.cpp`              |
| [Detector.cpp](https://github.com/mendax0110/peTool/blob/master/src/Detector.cpp)             | `src/Detector.cpp`         |
| [Rebuilder.cpp](https://github.com/mendax0110/peTool/blob/master/src/Rebuilder.cpp)           | `src/Rebuilder.cpp`        |

</details>

<details closed><summary>src.macos</summary>

| File                                                                            | Summary                |
| ---                                                                             |------------------------|
| [macos.mm](https://github.com/mendax0110/peTool/blob/master/src/macos/macos.mm) | `src/macos/macos.mm`   |

</details>

<details closed><summary>src.windows</summary>

| File                                                                                    | Summary                     |
| ---                                                                                     |-----------------------------|
| [windows.cpp](https://github.com/mendax0110/peTool/blob/master/src/windows/windows.cpp) | `src/windows/windows.cpp`   |

</details>

<details closed><summary>src.linux</summary>

| File                                                                              | Summary                |
| ---                                                                               |------------------------|
| [linux.cpp](https://github.com/mendax0110/peTool/blob/master/src/linux/linux.cpp) | `src/linux/linux.cpp`  |

</details>

---

##  Getting Started

***Requirements***

Ensure you have the following dependencies installed on your system:

* **CPP**: `version x.y.z`

###  Installation

1. Clone the peTool repository:

```sh
git clone https://github.com/mendax0110/peTool
```

2. Change to the project directory:

```sh
cd peTool
```

3. Get Dependencies:

```sh
cd DependencySetup
```

4. Run the Dependency Setup Script (macos/windows)

```sh
./setup_macos_dep.sh 
```

```sh
setup_windows_dep.bat
```

5. Build the project:

```sh
cmake ..
```

4. Compile the project:

```sh
cmake --build .
```


###  Running peTool

Use the following command to run peTool in graphical mode:

```sh
./peTool --gui 
```
Use the following command to run peTool in command-line mode:

```sh
./peTool
```

##  Contributing

Contributions are welcome! Here are several ways you can contribute:

- **[Submit Pull Requests](https://github.com/mendax0110/peTool/blob/main/CONTRIBUTING.md)**: Review open PRs, and submit your own PRs.
- **[Join the Discussions](https://github.com/mendax0110/peTool/discussions)**: Share your insights, provide feedback, or ask questions.
- **[Report Issues](https://github.com/mendax0110/peTool/issues)**: Submit bugs found or log feature requests for Petool.

<details closed>
    <summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your GitHub account.
2. **Clone Locally**: Clone the forked repository to your local machine using a Git client.
   ```sh
   git clone https://github.com/mendax0110/peTool
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b new-feature-x
   ```
4. **Make Your Changes**: Develop and test your changes locally.
5. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'Implemented new feature x.'
   ```
6. **Push to GitHub**: Push the changes to your forked repository.
   ```sh
   git push origin new-feature-x
   ```
7. **Submit a Pull Request**: Create a PR against the original project repository. Clearly describe the changes and their motivations.

Once your PR is reviewed and approved, it will be merged into the main branch.

</details>

---

##  License

This project is protected under the MIT License. For more details, refer to the [LICENSE](https://github.com/mendax0110/peTool/blob/main/License.txt) file.

---

##  Acknowledgments

- List any resources, contributors, inspiration, etc. here.

[**Return**](#-quick-links)

---