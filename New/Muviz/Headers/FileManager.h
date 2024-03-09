#pragma once
class FileManager{
public:
    const char* OpenFileExplorer(int FileType);
    int file_size(const char* FilePath);
};