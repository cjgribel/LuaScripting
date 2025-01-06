#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <string>
#include <vector>

/**
 * @struct PathComponents
 * @brief Represents the components of a file path.
 */
struct PathComponents {
    std::string directory;              ///< The directory of the file
    std::string filename;               ///< The filename including extension
    std::string name_without_extension; ///< The filename without the extension
    std::string extension;              ///< The file extension (without the dot)
};

/**
 * @class FileManager
 * @brief Provides utilities for file and directory operations.
 */
class FileManager {
public:
    /**
     * @brief Saves a string to a file.
     * @param filePath The file path where the content will be saved.
     * @param content The string content to save.
     * @return True if the operation succeeded, false otherwise.
     */
    static bool SaveToFile(const std::string& filePath, const std::string& content);

    /**
     * @brief Loads the content of a file into a string.
     * @param filePath The file path to read.
     * @param content Output parameter to hold the file's content.
     * @return True if the operation succeeded, false otherwise.
     */
    static bool LoadFromFile(const std::string& filePath, std::string& content);

    /**
     * @brief Checks if a file exists.
     * @param filePath The file path to check.
     * @return True if the file exists, false otherwise.
     */
    static bool FileExists(const std::string& filePath);

    /**
     * @brief Deletes a file.
     * @param filePath The file path to delete.
     * @return True if the file was successfully deleted, false otherwise.
     */
    static bool DeleteFile(const std::string& filePath);

    /**
     * @brief Appends content to an existing file or creates it if it doesn't exist.
     * @param filePath The file path to append to.
     * @param content The string content to append.
     * @return True if the operation succeeded, false otherwise.
     */
    static bool AppendToFile(const std::string& filePath, const std::string& content);

    /**
     * @brief Gets the size of a file.
     * @param filePath The file path to check.
     * @return The file size in bytes, or -1 if the file doesn't exist.
     */
    static std::int64_t GetFileSize(const std::string& filePath);

    /**
     * @brief Retrieves the last modified time of a file.
     * @param filePath The file path to check.
     * @return The last modified time as a string, or an empty string on error.
     */
    static std::string GetLastModifiedTime(const std::string& filePath);

    /**
     * @brief Splits a file path into its components.
     * @param path The file path to split.
     * @return A PathComponents structure containing the path components.
     */
    static PathComponents SplitPath(const std::string& path);

    /**
     * @brief Lists all files in a given folder.
     * @param folderPath The folder path to list files from.
     * @return A vector of file paths in the folder.
     */
    static std::vector<std::string> GetFilesInFolder(const std::string& folderPath);
};

#endif // FILE_MANAGER_HPP
