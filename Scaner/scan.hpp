#pragma once

namespace fs = std::filesystem;

struct FolderInfo {
    std::string path;           // Полный путь
    std::string name;           // Имя папки
    std::string owner;          // Владелец
    uintmax_t size_bytes;       // Размер в байтах
    std::string size_formatted; // Размер в удобном формате
    std::string created;        // Дата создания
    std::string modified;       // Дата последнего изменения
    std::string accessed;       // Дата последнего доступа
};

FolderInfo getFolderInfo(const fs::path& folderPath);
void saveToCSV(const std::vector<FolderInfo>& folders, const std::string& filename);
std::string formatSize(uintmax_t bytes);