#include "Functions.h"
#include "Interface.h"
#include "Styling.h"
#include "DX Functions.h"
#include <imgui_impl_dx12.h>
#include <algorithm>
#include <filesystem>

std::string ToNarrow(const wchar_t* s, char dfault = '?',
    const std::locale& loc = std::locale())
{
    std::ostringstream stm;

    while (*s != L'\0') {
        stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
    }
    return stm.str();
}

// Get system font file path
std::string GetSystemFontFile(const std::wstring& wsFaceName) {

    HKEY hKey;
    LONG result;

    // Open Windows font registry key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return "";
    }

    DWORD maxValueNameSize, maxValueDataSize;
    result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
    if (result != ERROR_SUCCESS) {
        return "";
    }

    DWORD valueIndex = 0;
    LPWSTR valueName = new WCHAR[maxValueNameSize];
    LPBYTE valueData = new BYTE[maxValueDataSize];
    DWORD valueNameSize, valueDataSize, valueType;
    std::wstring wsFontFile;

    // Look for a matching font name
    do {

        wsFontFile.clear();
        valueDataSize = maxValueDataSize;
        valueNameSize = maxValueNameSize;

        result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

        valueIndex++;

        if (result != ERROR_SUCCESS || valueType != REG_SZ) {
            continue;
        }

        std::wstring wsValueName(valueName, valueNameSize);

        // Found a match
        if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0) {

            wsFontFile.assign((LPWSTR)valueData, valueDataSize);
            break;
        }
    } while (result != ERROR_NO_MORE_ITEMS);

    delete[] valueName;
    delete[] valueData;

    RegCloseKey(hKey);

    if (wsFontFile.empty()) {
        return "";
    }

    // Build full font file path
    WCHAR winDir[MAX_PATH];
    if (GetWindowsDirectory(winDir, MAX_PATH) == 0)
        return "";

    std::wstringstream ss;
    ss << winDir << "\\Fonts\\" << wsFontFile;
    wsFontFile = ss.str();

    return ToNarrow(wsFontFile.c_str());
}

int CALLBACK EnumFontFamExProc(
    ENUMLOGFONTEX* lpelfe,
    ENUMTEXTMETRIC*,
    DWORD FontType,
    LPARAM
)
{
    wchar_t* end = lpelfe->elfFullName + sizeof(lpelfe->elfFullName) / sizeof(lpelfe->elfFullName[0]);
    wchar_t* pos = std::find(lpelfe->elfFullName, end, L'@');

    if (FontType == TRUETYPE_FONTTYPE && std::wcscmp(lpelfe->elfScript, L"Western") == 0 && pos == end)
    {
        auto font_path = GetSystemFontFile(lpelfe->elfFullName);
        if (!font_path.empty())
            g_Options.LyricVars.AvailableFonts.push_back(std::pair<std::string, std::string>(ToNarrow(lpelfe->elfFullName), font_path));
    }
    return 1;
}

void getFonts()
{
    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    HDC hDC = GetDC(NULL);
    EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)(EnumFontFamExProc), NULL, 0);
}

void UpdateFonts()
{
    if (!g_Options.LyricVars.ShouldUpdateFonts)
        return;
    ImGuiIO& io = ImGui::GetIO();

    for (auto& font : g_Options.LyricVars.LoadedFonts)
        font->~ImFont();
    g_Options.LyricVars.LoadedFonts.clear();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16.f * 2.5f);
    for (const auto& to_load : g_Options.LyricVars.SelectedFonts)
    {
        ImFont* font = io.Fonts->AddFontFromFileTTF(to_load.FilePath.c_str(), (float)to_load.FontSize);
        g_Options.LyricVars.LoadedFonts.emplace_back(std::move(font));
    }

    ImGui_ImplDX12_CreateDeviceObjects();

    g_Options.LyricVars.UpdateFonts = false;
    g_Options.LyricVars.ShouldUpdateFonts = false;
}
