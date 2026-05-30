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

#include "scan.hpp"


static std::string formatTime(const fs::file_time_type& timePoint) {
    auto sysTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        timePoint - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    
    auto timeT = std::chrono::system_clock::to_time_t(sysTime);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static std::string getOwnerLinux(const std::string& path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        struct passwd* pw = getpwuid(fileStat.st_uid);
        if (pw != nullptr) {
            return std::string(pw->pw_name);
        }
    }
    return "Unknown";
}

static uintmax_t calculateFolderSize(const fs::path& folderPath) {
    uintmax_t totalSize = 0;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(
            folderPath, 
            fs::directory_options::skip_permission_denied
        )) {
            if (fs::is_regular_file(entry.status())) {
                try {
                    totalSize += fs::file_size(entry);
                } catch (const fs::filesystem_error&) {
                    // Пропускаем файлы, к которым нет доступа
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        // Пропускаем папки, к которым нет доступа
    }
    
    return totalSize;
}


std::string formatSize(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024 && unitIndex < 4) {
        size /= 1024;
        unitIndex++;
    }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return ss.str();
}

FolderInfo getFolderInfo(const fs::path& folderPath) {
    FolderInfo info;
    
    info.path = folderPath.string();
    info.name = folderPath.filename().string();
    

    info.owner = getOwnerLinux(info.path);

    info.size_bytes = calculateFolderSize(folderPath);
    info.size_formatted = formatSize(info.size_bytes);
    
    try {
        auto ftime = fs::last_write_time(folderPath);
        info.modified = formatTime(ftime);
        

        info.created = "N/A";
        info.accessed = "N/A";
        

        struct stat fileStat;
        if (stat(info.path.c_str(), &fileStat) == 0) {
            char buffer[20];
            struct tm* timeinfo;
            
            timeinfo = localtime(&fileStat.st_ctime);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            info.created = buffer;
            
            timeinfo = localtime(&fileStat.st_atime);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            info.accessed = buffer;
        }

    } catch (const fs::filesystem_error&) {
        info.modified = "Unknown";
    }
    
    return info;
}


void saveToCSV(const std::vector<FolderInfo>& folders, const std::string& filename) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка: не могу создать файл " << filename << std::endl;
        return;
    }
    
    file << "\xEF\xBB\xBF"; // UTF-8 BOM для корректного открытия в Excel
    file << "Путь;Имя папки;Владелец;Размер(байт);Размер(формат);Создана;Изменена;Последний доступ\n";
    
    // Данные
    for (const auto& folder : folders) {
        file << "\"" << folder.path << "\";"
             << "\"" << folder.name << "\";"
             << "\"" << folder.owner << "\";"
             << folder.size_bytes << ";"
             << "\"" << folder.size_formatted << "\";"
             << "\"" << folder.created << "\";"
             << "\"" << folder.modified << "\";"
             << "\"" << folder.accessed << "\"\n";
    }
    
    file.close();
    std::cout << "✅ Данные сохранены в файл: " << filename << std::endl;
    std::cout << "📊 Всего обработано папок: " << folders.size() << std::endl;
}