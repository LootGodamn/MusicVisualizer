/*#include <iostream>
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdlib.h>

using namespace std;

int main() {
	
	system("explorer /select,C:\\Windows\\notepad.exe");
	return 0;
}*/

#include <windows.h>
#include <string>
#include <iostream>
#include <shobjidl.h>   // For IFileDialog

using namespace std;

int main() {
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
                        // Print the selected file path
                        cout << "Selected file: %s\n" << filePath;

                        Sleep(5000);

                        // Remember to free the allocated memory
                        CoTaskMemFree(filePath);
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

    return 0;
}