#include "FileManager.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <shobjidl.h>   // For IFileDialog

using namespace std;

const char* convertPWSTRtoConstChar(PWSTR wideString);

const char* FileManager::OpenFileExplorer()
{
        // CoInitialize is needed to initialize the COM library
        CoInitialize(NULL);

        // Create an instance of the File Open dialog
        IFileDialog* pFileDialog;
        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pFileDialog)
        );

        if (SUCCEEDED(hr)) {
            // Set file type filters
            COMDLG_FILTERSPEC fileTypes[] = {
                { L"MP3 Files", L"*.mp3" },
                { L"WAV Files", L"*.wav" },
                { L"OGG Files", L"*.ogg" },
                { L"FLAC Files", L"*.flac" },
                { L"All Files", L"*.*" }
            };
            hr = pFileDialog->SetFileTypes(_countof(fileTypes), fileTypes);

            if (SUCCEEDED(hr)) {
                // Show the File Open dialog
                hr = pFileDialog->Show(NULL);

                if (SUCCEEDED(hr)) {
                    // Get the selected file(s)
                    IShellItem* pItem;
                    hr = pFileDialog->GetResult(&pItem);

                    if (SUCCEEDED(hr)) {
                        PWSTR filePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

                        if (SUCCEEDED(hr)) {

                            // Convert PWSTR to const char*
                            const char* narrowString = convertPWSTRtoConstChar(filePath);

                            // Use the converted narrow string
                            std::cout << "Narrow String: " << narrowString << std::endl;

                            return narrowString;
                        }

                        pItem->Release();
                    }
                }
            }

            // Release the File Open dialog
            pFileDialog->Release();
        }

        // CoUninitialize to clean up the COM library
        CoUninitialize();
}

const char* convertPWSTRtoConstChar(PWSTR wideString) {
    // Get the length of the wide string
    int wideStringLength = lstrlenW(wideString);

    // Calculate the required buffer size for the narrow string
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wideString, wideStringLength, NULL, 0, NULL, NULL);

    // Allocate memory for the narrow string
    char* narrowString = new char[bufferSize + 1];  // +1 for null terminator

    // Convert the wide string to narrow string
    WideCharToMultiByte(CP_ACP, 0, wideString, wideStringLength, narrowString, bufferSize, NULL, NULL);

    // Null-terminate the narrow string
    narrowString[bufferSize] = '\0';

    return narrowString;
}
