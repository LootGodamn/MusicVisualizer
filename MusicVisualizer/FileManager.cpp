#include "FileManager.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <shobjidl.h>   // For IFileDialog
#include <fstream>

using namespace std;

const char* convertPWSTRtoConstChar(PWSTR WideString);

const char* FileManager::OpenFileExplorer()
{
        // CoInitialize is needed to initialize the COM library
        CoInitialize(NULL);

        // Create an instance of the File Open dialog
        IFileDialog* FileDialog;
        HRESULT HResult = CoCreateInstance(
            CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&FileDialog)
        );

        if (SUCCEEDED(HResult)) {
            // Set file type filters
            COMDLG_FILTERSPEC FileTypes[] = {
                { L"MP3 Files", L"*.mp3" },
                { L"WAV Files", L"*.wav" },
                { L"OGG Files", L"*.ogg" },
                { L"FLAC Files", L"*.flac" },
                { L"All Files", L"*.*" }
            };
            HResult = FileDialog->SetFileTypes(_countof(FileTypes), FileTypes);

            if (SUCCEEDED(HResult)) {
                // Show the File Open dialog
                HResult = FileDialog->Show(NULL);

                if (SUCCEEDED(HResult)) {
                    // Get the selected file(s)
                    IShellItem* Item;
                    HResult = FileDialog->GetResult(&Item);

                    if (SUCCEEDED(HResult)) {
                        PWSTR FilePath;
                        HResult = Item->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

                        if (SUCCEEDED(HResult)) {

                            // Convert PWSTR to const char*
                            const char* NarrowString = convertPWSTRtoConstChar(FilePath);

                            // Use the converted narrow string
                            std::cout << "Narrow String: " << NarrowString << std::endl;

                            return NarrowString;
                        }

                        Item->Release();
                    }
                }
            }

            // Release the File Open dialog
            FileDialog->Release();
        }

        // CoUninitialize to clean up the COM library
        CoUninitialize();
}

const char* convertPWSTRtoConstChar(PWSTR WideString) {
    // Get the length of the wide string
    int WideStringLength = lstrlenW(WideString);

    // Calculate the required buffer size for the narrow string
    int BufferSize = WideCharToMultiByte(CP_ACP, 0, WideString, WideStringLength, NULL, 0, NULL, NULL);

    // Allocate memory for the narrow string
    char* NarrowString = new char[BufferSize + 1];  // +1 for null terminator

    // Convert the wide string to narrow string
    WideCharToMultiByte(CP_ACP, 0, WideString, WideStringLength, NarrowString, BufferSize, NULL, NULL);

    // Null-terminate the narrow string
    NarrowString[BufferSize] = '\0';

    return NarrowString;
}

int FileManager::file_size(const char* FilePath) {
    // Open the file in binary mode
    std::ifstream file(FilePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << FilePath << std::endl;
        return -1;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    int fileSize = file.tellg();

    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Close the file
    file.close();

    return fileSize;
}