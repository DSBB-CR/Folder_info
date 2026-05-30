#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sstream>


#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>


#include "Scaner/scan.hpp"



int main(int argc, const char * argv[]) {
    std::string startPath = "";
    if (argc > 1) {
        startPath = argv[1];
    }

    if (startPath.empty()) {
        startPath = fs::current_path().string();
    }

    if (!fs::exists(startPath)) {
        std::cerr << "❌ Ошибка: путь '" << startPath << "' не существует!" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::vector<FolderInfo> folders;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(
            startPath,
            fs::directory_options::skip_permission_denied
        )) {
            if (fs::is_directory(entry.status())) {
                std::cout << "Обработка: " << entry.path().string() << std::endl;
                folders.push_back(getFolderInfo(entry.path()));
            }
        }

        
        std::string outputFile = "folder_analysis_" + 
                                 std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + 
                                 ".csv";
        saveToCSV(folders, outputFile);
             
    } catch (const fs::filesystem_error& e) {
        std::cerr << "❌ Ошибка файловой системы: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}