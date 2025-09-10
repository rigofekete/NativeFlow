#include "task_storage.h"
#include <fstream>
#include <shlobj.h> // For SHGetFolderPath
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace fs = std::filesystem;


// Define static members
const std::wstring TaskStorage::APP_FOLDER_NAME = L"BoarTaskHybrid";
fs::path TaskStorage::storagePath;

fs::path TaskStorage::getStoragePath()
{
    if (storagePath.empty())
    {
        // // Try to find Dropbox folder in the user's directory
        // wchar_t userPath[MAX_PATH];
        // if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, userPath)))
        // {
        //     // Direct Dropbox path
        //      // The / operator in std::filesystem isn't actually division - it's an operator that has been overloaded to mean "append to path". It's used to combine parts of a file path in a platform-independent way
        //     fs::path directPath = fs::path(userPath) / "Dropbox" / APP_FOLDER_NAME;

        //     // Is equivalent to writing something like:
        //     // On Windows:
        //     // storagePath = fs::path(appDataPath) + "\\BoarTaskHybrid";

        //     // The advantage of using the / operator is that:

        //     // It automatically uses the correct path separator for the operating system
        //     // It handles cases where paths might or might not end with a separator
        //     // It's more readable and clearly shows we're working with paths

        //     // For instance, if appDataPath is C:\Users\YourName\AppData\Roaming, then:
        //     // cppCopystoragePath = fs::path(appDataPath) / APP_FOLDER_NAME;
        //     // Results in: "C:\Users\YourName\AppData\Roaming\BoarTaskHybrid"

        //     char debug[512];
        //     sprintf_s(debug, "Checking Dropbox path: %s\n", directPath.string().c_str());
        //     OutputDebugStringA(debug);
            
        //     if (fs::exists(directPath)) 
        //     {
        //         OutputDebugStringA("Found tasks in Dropbox folder!\n");

        //         storagePath = directPath;
        //         // return found and confirmed path to DropBox directory 
        //         return storagePath;
        //     }
        // }

        // If Dropbox path not found, fall back to AppData
        OutputDebugStringA("No Dropbox path found, falling back to AppData\n");
        wchar_t appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath)))
        {
            storagePath = fs::path(appDataPath) / APP_FOLDER_NAME;

            char debug[512];
            sprintf_s(debug, "Using AppData fallback path: %s\n", storagePath.string().c_str());
            OutputDebugStringA(debug);
        }
        else
        {
            storagePath = fs::current_path() / APP_FOLDER_NAME;
        }
    }
    return storagePath;
}


// This Lock implemenation is buggy and freezes the start up many times, messing up the task data
////////////////////////////////////////////////////////////////////////////////////////////////////
// // File lock implementation for concurrent access
// // TODO: Check this class implementation 
// class FileLock 
// {
//     fs::path lockPath;
//     bool locked = false;

//     public:
//         FileLock(const fs::path& path) : lockPath(path.string() + ".lock") 
//         {
//             int attempts = 0;
//             const int maxAttempts = 50; // 5 seconds total
            
//             while (attempts < maxAttempts) {
//                 try {
//                     if (!fs::exists(lockPath)) {
//                         std::ofstream lock(lockPath);
//                         if (lock) {
//                             locked = true;
//                             break;
//                         }
//                     }
//                 }
//                 catch (...) {}
                
//                 Sleep(100); // Wait 100ms between attempts
//                 attempts++;
//             }
            
//             if (!locked) {
//                 throw std::runtime_error("Could not acquire file lock");
//             }
//         }
    
//     ~FileLock() 
//     {
//         if (locked) {
//             try {
//                 fs::remove(lockPath);
//             }
//             catch (...) {}
//         }
//     }
// };


// class FileLock 
// {
//     fs::path lockPath;
//     bool locked = false;

//     public:
//         FileLock(const fs::path& path) : lockPath(path.string() + ".lock") 
//         {
//             // First, try to clean up any stale lock files
//             try {
//                 if (fs::exists(lockPath)) {
//                     // Check if the lock file is older than 30 seconds
//                     auto lockTime = fs::last_write_time(lockPath);
//                     auto now = fs::file_time_type::clock::now();
//                     if (now - lockTime > std::chrono::seconds(30)) {
//                         fs::remove(lockPath);
//                     }
//                 }
//             } catch (...) {}

//             int attempts = 0;
//             const int maxAttempts = 100; // 10 seconds total
            
//             while (attempts < maxAttempts) {
//                 try {
//                     if (!fs::exists(lockPath)) {
//                         std::ofstream lock(lockPath);
//                         if (lock) {
//                             lock.close();
//                             locked = true;
//                             break;
//                         }
//                     }
//                 }
//                 catch (const std::exception& e) {
//                     char debug[512];
//                     sprintf_s(debug, "Lock attempt failed: %s\n", e.what());
//                     OutputDebugStringA(debug);
//                 }
                
//                 Sleep(100); // Wait 100ms between attempts
//                 attempts++;
//             }
            
//             if (!locked) {
//                 // If we couldn't acquire the lock, try to force it
//                 try {
//                     if (fs::exists(lockPath)) {
//                         fs::remove(lockPath);
//                     }
//                     std::ofstream lock(lockPath);
//                     if (lock) {
//                         lock.close();
//                         locked = true;
//                     }
//                 } catch (const std::exception& e) {
//                     char debug[512];
//                     sprintf_s(debug, "Force lock failed: %s\n", e.what());
//                     OutputDebugStringA(debug);
//                     throw std::runtime_error("Could not acquire file lock after cleanup attempt");
//                 }
//             }
//         }
    
//     ~FileLock() 
//     {
//         if (locked) {
//             try {
//                 fs::remove(lockPath);
//                 locked = false;
//             }
//             catch (const std::exception& e) {
//                 char debug[512];
//                 sprintf_s(debug, "Lock cleanup failed: %s\n", e.what());
//                 OutputDebugStringA(debug);
//             }
//         }
//     }
// };



void TaskStorage::ensureDirectoryExists()
{
    fs::path path = getStoragePath();
    if (!fs::exists(path))
    {
        try
        {
            fs::create_directories(path);

            char debug[512];
            sprintf_s(debug, "Created directory: %s\n", path.string().c_str());
            OutputDebugStringA(debug);
        }
        catch (const fs::filesystem_error& e)
        {
            std::cerr << "Failed to create directory: " << e.what() << std::endl;
            MessageBoxA(NULL, e.what(), "Error Creating Directory", MB_OK | MB_ICONERROR);
        }
    }
}


void TaskStorage::saveTasksToFile(const std::vector<TaskRect> &tasks)
{
    try
    {
        ensureDirectoryExists();
        fs::path filePath = getStoragePath() / "tasks.dat";
        // second path for the backupfile
        fs::path fileBackupPath = getStoragePath() / "tasks_backup.dat";

        // // Create lock to prevent concurrent access
        // FileLock lock(filePath);

        std::ofstream file(filePath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Could not open file for writing: " + filePath.string());
        }

        // Second error check fot he backup file opening 
        std::ofstream fileBackup(fileBackupPath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Could not open backup file for writing: " + filePath.string());
        }

        boost::archive::text_oarchive oa(file);
        // second archive boost stream for the backup file  
        boost::archive::text_oarchive oa2(fileBackup);

        oa  << tasks;
        // data insetion for the backup file 
        oa2 << tasks;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving tasks: " << e.what() << std::endl;
        MessageBoxA(NULL, e.what(), "Error Saving Tasks", MB_OK | MB_ICONERROR);
    }
}

std::vector<TaskRect> TaskStorage::loadTasksFromFile()
{
    std::vector<TaskRect> tasks;
    try
    {
        fs::path filePath = getStoragePath() / "tasks.dat";

        if (!fs::exists(filePath))
        {
            return tasks; // Return empty vector if file doesn't exist yet
        }

        // // Create lock for reading
        // FileLock lock(filePath);

        std::ifstream file(filePath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Could not open file for reading: " + filePath.string());
        }

        boost::archive::text_iarchive ia(file);
        ia >> tasks;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading tasks: " << e.what() << std::endl;
        MessageBoxA(NULL, e.what(), "Error Loading Tasks", MB_OK | MB_ICONERROR);
    }
    return tasks;
}
