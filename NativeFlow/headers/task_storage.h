#pragma once
#include "globals.h"
#include <string>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <filesystem> // For path handling

class TaskStorage
{
    public:
        static void saveTasksToFile(const std::vector<TaskRect> &tasks);
        static std::vector<TaskRect> loadTasksFromFile();

    private:
        static std::filesystem::path getStoragePath();
        static void ensureDirectoryExists();
        static const std::wstring APP_FOLDER_NAME;
        static std::filesystem::path storagePath;
};
