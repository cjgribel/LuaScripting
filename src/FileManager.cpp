#include "FileManager.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

bool FileManager::SaveToFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << filePath << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}

bool FileManager::LoadFromFile(const std::string& filePath, std::string& content) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for reading: " << filePath << std::endl;
        return false;
    }
    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return true;
}

bool FileManager::FileExists(const std::string& filePath) {
    return std::filesystem::exists(filePath);
}

bool FileManager::DeleteFile(const std::string& filePath) {
    try {
        return std::filesystem::remove(filePath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: Unable to delete file: " << e.what() << std::endl;
        return false;
    }
}

bool FileManager::AppendToFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for appending: " << filePath << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}

std::int64_t FileManager::GetFileSize(const std::string& filePath) {
    try {
        return std::filesystem::file_size(filePath);
    } catch (const std::filesystem::filesystem_error&) {
        return -1;
    }
}

std::string FileManager::GetLastModifiedTime(const std::string& filePath) {
    try {
        auto ftime = std::filesystem::last_write_time(filePath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    } catch (const std::filesystem::filesystem_error&) {
        return "";
    }
}

PathComponents FileManager::SplitPath(const std::string& path) {
    PathComponents components;
    std::filesystem::path fs_path(path);

    components.directory = fs_path.parent_path().string() + "/";
    components.filename = fs_path.filename().string();
    components.name_without_extension = fs_path.stem().string();
    components.extension = fs_path.extension().string();

    // Remove leading dot from the extension
    if (!components.extension.empty() && components.extension[0] == '.') {
        components.extension.erase(0, 1);
    }

    return components;
}

std::vector<std::string> FileManager::GetFilesInFolder(const std::string& folderPath) {
    std::vector<std::string> fileNames;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (std::filesystem::is_regular_file(entry.status())) {
                fileNames.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }
    return fileNames;
}
