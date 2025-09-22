# NativeFlow

**Prototype task management program built with C/C++ and the Win32 API**

---

## 📌 Description

Prototype program written in C/C++, using the Win32 API and custom rendering libraries.  
This was built to provide a native task management solution with **deadline tracking**, **cross-device synchronization**, and **dynamic visual feedback** through a custom frame processing system.

---

## 🖥️ Native UI & Behavior

This program creates every UI element (windows, buttons, text fields, menus, etc.) directly through **Win32 API calls**, providing complete control over the interface behavior and appearance.  

Users can create and manage tasks with associated deadlines, while the internal logic continuously monitors and tracks these time-sensitive items in real-time.  

All task data is automatically synchronized to a designated **Dropbox path**, enabling seamless access and collaboration across multiple machines and users.  

The application also features a **custom frame processing and rendering system** that dynamically renders visual elements to the screen based on current task states, creating an engaging and interactive user experience with visual feedback.

---

## ✨ Implemented Features

- 🖼️ **Native Win32 UI architecture** with all interface elements built from scratch using direct API calls  
- ⏰ **Comprehensive deadline management system** with internal logic for monitoring and tracking time-sensitive tasks  
- ☁️ **Automatic data persistence and synchronization** to Dropbox storage path for multi-device and multi-user access  
- 🎨 **Custom frame processing and rendering system** that provides dynamic visual feedback based on task states and user interactions  

---


## 📺 Demo
![Application Demo](https://github.com/user-attachments/assets/81c8df7f-c4fd-4003-a497-ac24348d6cb1)
*NativeFlow workflow demonstration*


## 🚀 Quickstart (Windows PowerShell / Command Prompt)

### Prerequisites
- **Visual Studio Community** (with C++ build tools)
- **Git**
- **VS Code** with C/C++ and CMake Tools extensions

### 1. Set up vcpkg
```powershell
# Clone vcpkg to C:\Dev (or your preferred location)
git clone https://github.com/Microsoft/vcpkg.git C:\Dev\vcpkg
cd C:\Dev\vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Install required Boost library
.\vcpkg install boost-serialization:x64-windows-static
```

### 2. Clone and Build the Project
```powershell
# Clone the repository
git clone https://github.com/rigofekete/NativeFlow
cd NativeFlow

# Open in VS Code
code .
```

### 3. Configure VS Code
1. **Select CMake Kit**: Press `Ctrl+Shift+P` → Type "CMake: Select a Kit" → Choose **"Visual Studio Community 2022 Release - amd64"**
2. **Configure Project**: Press `Ctrl+Shift+P` → Type "CMake: Configure"
3. **Build Project**: Press `Ctrl+Shift+P` → Type "CMake: Build"

### 4. Set up Debugging
1. **Open Run and Debug Panel**: Press `Ctrl+Shift+D`
2. **Create launch.json**: Click "create a launch.json file" → Select "C++ (Windows)"
3. **Update configuration**: Replace the `"program"` line with:
   ```json
   "program": "${workspaceFolder}/build/Debug/NativeFlow.exe"
   ```
4. **Start Debugging**: Press `F5`

### 5. Run the Application
- **Debug Mode**: Press `F5`
- **Run Without Debugging**: Press `Ctrl+F5`
- **Direct Execution**: `.\build\Debug\NativeFlow.exe`

## 📝 Notes
- Make sure vcpkg path in `CMakeLists.txt` matches your installation: `C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake`
- If you installed vcpkg elsewhere, update the `CMAKE_TOOLCHAIN_FILE` path accordingly
- The project uses static Boost libraries for easier deployment

