#include "axpch.h"

#include "Utils/FileDialogs.h"

#include <Windows.h>
#include <filesystem>
#include <optional>
#include <shobjidl.h>
#include <string>

namespace Axiom {
    static std::wstring uft8ToUtf16(const std::string& str) {
        if (str.empty()) {
            return std::wstring();
        }
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0);
        std::wstring wstrTo(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], sizeNeeded);
        return wstrTo;
    }

    std::optional<std::filesystem::path> FileDialogs::openFolder(const std::string& title) {
        std::optional<std::filesystem::path> result = std::nullopt;

        HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        IFileOpenDialog* pDialog = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pDialog));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            if (SUCCEEDED(pDialog->GetOptions(&dwOptions))) {
                pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR);
            }

            std::wstring wTitle = Utf8ToUtf16(title);
            if (!wTitle.empty()) {
                pDialog->SetTitle(wTitle.c_str());
            }

            if (SUCCEEDED(pDialog->Show(NULL))) {
                IShellItem* pItem;
                if (SUCCEEDED(pDialog->GetResult(&pItem))) {
                    PWSTR pszFilePath;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                        result = std::filesystem::path(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pDialog->Release();
        }

        if (SUCCEEDED(hrInit)) {
            CoUninitialize();
        }

        return result;
    }

    std::optional<std::filesystem::path> FileDialogs::openFile(const std::string& title, const std::string& filter) {
        std::optional<std::filesystem::path> result = std::nullopt;

        HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        IFileOpenDialog* pDialog = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pDialog));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            if (SUCCEEDED(pDialog->GetOptions(&dwOptions))) {
                pDialog->SetOptions(dwOptions | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR);
            }

            std::wstring wTitle = Utf8ToUtf16(title);
            if (!wTitle.empty()) {
                pDialog->SetTitle(wTitle.c_str());
            }

            std::wstring wFilterExt;
            COMDLG_FILTERSPEC filterSpec[] = {{L"Allowed Files", L"*.*"}};

            if (!filter.empty()) {
                std::wstring wFilter = Utf8ToUtf16(filter);

                if (wFilter.find(L'*') == std::wstring::npos) {
                    wFilterExt = L"*." + wFilter;
                } else {
                    wFilterExt = wFilter;
                }

                filterSpec[0].pszName = wFilterExt.c_str();
                filterSpec[0].pszSpec = wFilterExt.c_str();
                pDialog->SetFileTypes(1, filterSpec);
            }

            if (SUCCEEDED(pDialog->Show(NULL))) {
                IShellItem* pItem;
                if (SUCCEEDED(pDialog->GetResult(&pItem))) {
                    PWSTR pszFilePath;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                        result = std::filesystem::path(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pDialog->Release();
        }

        if (SUCCEEDED(hrInit)) {
            CoUninitialize();
        }

        return result;
    }
} // namespace Axiom
