// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Interface.h"
#include "Styling.h"
#include "DX Functions.h"
#include "Functions.h"
#include "imgui_internal.h"

#ifdef ENABLE_CONFIG
#include "ConfigSystem.h"
#endif

#include <algorithm>
#include <filesystem>

bool window_open = true;
static auto start_time_font = std::chrono::high_resolution_clock::now();
static auto start_time_text = std::chrono::high_resolution_clock::now();
static auto start_time_timecode = std::chrono::high_resolution_clock::now();

enum FORCEMODE
{
    OFF = 0,
    PREFER,
    FORCE
};

bool UpdateFontToUse(FORCEMODE force_mode = OFF)
{
    bool retval = false;

    if (g_Options.LyricVars.SelectedFonts.empty())
        return retval;

    auto end_time = std::chrono::high_resolution_clock::now(); //-V656
    std::chrono::duration<float, std::milli> delta_time = end_time - start_time_font;
    std::chrono::duration<float, std::milli> delta_time_text = end_time - start_time_text;
    auto delta_time_ms = delta_time.count();
    auto delta_time_text_ms = delta_time_text.count();

    const auto& dur = g_Options.LyricVars.SelectedFonts.at(g_Options.LyricVars.FontToUse).DurationMS;
    const auto& dur_text = g_Options.LyricVars.SelectedTexts.at(g_Options.LyricVars.TextToUse).DurationMS;
    //if text updated, and this font has been shown for over 50% of its time - update font
    bool force_update = force_mode == FORCE || (force_mode == PREFER && delta_time_ms >= (dur * 0.5f));
    bool timed_update = delta_time_ms >= dur;
    //if font has been shown for over 50% of its time and less than 10% of text time left, don't update font
    //unless updating would left it show for more than 50%
    bool skip_update = delta_time_ms >= (dur * 0.5f) && delta_time_text_ms >= (dur_text * 0.9f) &&
        (dur_text - delta_time_text_ms) < (dur * 0.5f);

    if (!skip_update && (force_update || timed_update))
    {
        g_Options.LyricVars.FontToUse++;
        start_time_font = end_time;
        retval = true;
    }

    if (g_Options.LyricVars.FontToUse >= g_Options.LyricVars.LoadedFonts.size())
    {
        g_Options.LyricVars.FontToUse = 0;
        retval = true;
    }
    return retval;
}

bool UpdateTextToUse(FORCEMODE force_mode = OFF)
{
    bool retval = false;

    if (g_Options.LyricVars.SelectedTexts.empty())
        return retval;

    auto end_time = std::chrono::high_resolution_clock::now(); //-V656

    switch (g_Options.LyricVars.DurType)
    {
    case DUR_LENGTH:
    {
        std::chrono::duration<float, std::milli> delta_time = end_time - start_time_text;
        auto delta_time_ms = delta_time.count();

        const auto& dur = g_Options.LyricVars.SelectedTexts.at(g_Options.LyricVars.TextToUse).DurationMS;
        bool force_update = force_mode == FORCE;
        bool timed_update = delta_time_ms >= dur;

        if (force_update || timed_update)
        {
            g_Options.LyricVars.TextToUse++;
            start_time_text = end_time;
            retval = true;
        }

        if (g_Options.LyricVars.TextToUse >= g_Options.LyricVars.SelectedTexts.size())
        {
            g_Options.LyricVars.TextToUse = 0;
            retval = true;
        }
    }
    break;
    case DUR_TIMECODE:
    {
        std::chrono::duration<float, std::milli> lifetime = end_time - start_time_timecode;
        auto lifetime_ms = lifetime.count();

        const auto& el_size = g_Options.LyricVars.SelectedTexts.size();

        int32_t max_tc = 0;
        for (size_t idx = 0; idx < el_size; idx++)
        {
            const auto& el = g_Options.LyricVars.SelectedTexts[idx];
        
            if (el.TimecodeMS > max_tc && el.TimecodeMS <= lifetime_ms)
            {
                max_tc = el.TimecodeMS;
                g_Options.LyricVars.TextToUse = (uint32_t)idx;
                retval = true;
            }
        }
    }
    break;
    }

    return retval;
}

//here goes the main window drawing
void DrawUI()
{
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

    if (g_Options.Menu.ChangedWindowSize)
    {
        RECT screen_rect;
        GetWindowRect(GetDesktopWindow(), &screen_rect);

        ImGui::SetNextWindowSize(g_Options.Menu.BorderlessWindowed ? ImVec2((float)screen_rect.right, (float)screen_rect.bottom) : ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImGuiCond_Always);

        g_Options.Menu.ChangedWindowSize = false;
    }

    if (CheckPopups())
        return;

    if (g_Options.Menu.MenuEnabled)
    {
        g_Options.LyricVars.ResetDrawing = true;

        ImGui::Begin(NAME_APP, &window_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::Columns(2, "tabs", false);
        auto col_width = ImGui::GetColumnWidth();

        if (ImGui::Button("Main", ImVec2(col_width - 20, 0)))
            g_Options.Menu.MenuTab = MENU_MAIN;

#ifdef ENABLE_CONFIG
        ImGui::NextColumn();
        ImGui::Spacing();
        ImGui::SameLine(0, 6);

        if (ImGui::Button("Config", ImVec2(col_width - 20, 0)))
            g_Options.Menu.MenuTab = MENU_CONFIG;
#endif

        ImGui::Columns(1, "menu", false);

        switch (g_Options.Menu.MenuTab)
        {
        case MENU_MAIN:
            ImGui::Text("Defaults:");
            ImGui::SliderInt("Font Size", &g_Options.LyricVars.DefaultFontSize, 1, 512);
            ImGui::SliderInt("Font Duration", &g_Options.LyricVars.DefaultFontDuration, 1, 5000);
            ImGui::SliderInt("Text Duration", &g_Options.LyricVars.DefaultTextDuration, 1, 5000, "%dms");

            ImGui::Separator();

            ImGui::Combo("Selected Fonts", (int32_t*)&g_Options.LyricVars.FontSelected,
                [](void* data, int32_t idx, char const** out_text) {
                    *out_text = ((const std::vector<FontVariable>*)data)->at(idx).PrettyName.c_str(); return true;
                }, (void*)&g_Options.LyricVars.SelectedFonts, (int32_t)g_Options.LyricVars.SelectedFonts.size());

            if (g_Options.LyricVars.SelectedFonts.size() > g_Options.LyricVars.FontSelected)
            {
                auto& sel_font = g_Options.LyricVars.SelectedFonts.at(g_Options.LyricVars.FontSelected);
                if (ImGui::SliderInt("Size", &sel_font.FontSize, 1, 512))
                    g_Options.LyricVars.UpdateFonts = true;

                ImGui::SliderInt("Duration##f", &sel_font.DurationMS, 1, 5000, "%dms");
            }

            if (ImGui::Selectable("Remove##f", false, (g_Options.LyricVars.SelectedFonts.size() > g_Options.LyricVars.FontSelected ? 0 : ImGuiSelectableFlags_Disabled)))
            {
                g_Options.LyricVars.SelectedFonts.erase(g_Options.LyricVars.SelectedFonts.begin() + g_Options.LyricVars.FontSelected);
                g_Options.LyricVars.FontSelected = std::clamp(g_Options.LyricVars.FontSelected, 0u, (uint32_t)g_Options.LyricVars.SelectedFonts.size() - 1);
                g_Options.LyricVars.UpdateFonts = true;
            }

            ImGui::Separator();

            ImGui::Combo("Available Fonts", (int32_t*)&g_Options.LyricVars.FontAvailSelected,
                [](void* data, int32_t idx, char const** out_text) {
                    *out_text = ((const std::vector<std::pair<std::string, std::string>>*)data)->at(idx).first.c_str(); return true;
                }, (void*)&g_Options.LyricVars.AvailableFonts, (int32_t)g_Options.LyricVars.AvailableFonts.size());
            if (ImGui::Selectable("Add##f", false, (g_Options.LyricVars.SelectedFonts.size() < 32 ? 0 : ImGuiSelectableFlags_Disabled)))
            {
                g_Options.LyricVars.SelectedFonts.insert(g_Options.LyricVars.FontSelected < g_Options.LyricVars.SelectedFonts.size() ? g_Options.LyricVars.SelectedFonts.begin() + g_Options.LyricVars.FontSelected + 1 : g_Options.LyricVars.SelectedFonts.end(), { g_Options.LyricVars.AvailableFonts.at(g_Options.LyricVars.FontAvailSelected), g_Options.LyricVars.DefaultFontSize, g_Options.LyricVars.DefaultFontDuration });
                g_Options.LyricVars.FontSelected = std::clamp(g_Options.LyricVars.FontSelected + 1, 0u, (uint32_t)g_Options.LyricVars.SelectedFonts.size() - 1);
                g_Options.LyricVars.UpdateFonts = true;
            }
            ImGui::Separator();

            ImGui::Combo("Duration Type", (int*)&g_Options.LyricVars.DurType, DurTypeNames, ARRAYSIZE(DurTypeNames));

            ImGui::Combo("Selected Texts", (int32_t*)&g_Options.LyricVars.TextSelected,
                [](void* data, int32_t idx, char const** out_text) {
                    *out_text = ((const std::vector<TextVariable>*)data)->at(idx).Text.c_str(); return true;
                }, (void*)&g_Options.LyricVars.SelectedTexts, (int32_t)g_Options.LyricVars.SelectedTexts.size());
            if (g_Options.LyricVars.SelectedTexts.size() > g_Options.LyricVars.TextSelected)
            {
                auto& sel_font = g_Options.LyricVars.SelectedTexts.at(g_Options.LyricVars.TextSelected);
                switch (g_Options.LyricVars.DurType)
                {
                case DUR_LENGTH:
                    ImGui::SliderInt("Duration##t", &sel_font.DurationMS, 1, 5000, "%dms");
                    break;
                case DUR_TIMECODE:
                {
                    sel_font.TimecodeMS = (int32_t)round(sel_font.TimecodeMS / 10) * 10;
                    int32_t min = sel_font.TimecodeMS / (1000 * 60);
                    int32_t sec = (sel_font.TimecodeMS - 1000 * 60 * min) / (1000);
                    int32_t ms  = (sel_font.TimecodeMS - 1000 * 60 * min - 1000 * sec) / 10;
                    char converted[16];
                    sprintf_s(converted, "%.2d:%.2d:%.2d", min, sec, ms);
                    ImGui::SliderInt("Timecode##t", &sel_font.TimecodeMS, 0, 1000 * 60, converted);
                }
                break;
                }
                ImGui::ColorEdit4("Color##t", (float*)(&sel_font.Color), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
            }

            if (ImGui::InputText("Custom Text", g_Options.LyricVars.CustomText, sizeof(g_Options.LyricVars.CustomText), ImGuiInputTextFlags_EnterReturnsTrue) && g_Options.LyricVars.SelectedFonts.size() < 32)
            {
                int32_t max_tc = 0;
                for (const auto& el : g_Options.LyricVars.SelectedTexts)
                    if (el.TimecodeMS >= max_tc)
                        max_tc = el.TimecodeMS + g_Options.LyricVars.DefaultTextDuration;

                g_Options.LyricVars.SelectedTexts.insert(
                    g_Options.LyricVars.TextSelected < g_Options.LyricVars.SelectedTexts.size() ? g_Options.LyricVars.SelectedTexts.begin() + g_Options.LyricVars.TextSelected + 1 : g_Options.LyricVars.SelectedTexts.end(),
                    { g_Options.LyricVars.CustomText, g_Options.LyricVars.DefaultTextDuration, max_tc, col_text });
                memset(&g_Options.LyricVars.CustomText[0], 0, sizeof(g_Options.LyricVars.CustomText));
                g_Options.LyricVars.TextSelected = std::clamp(g_Options.LyricVars.TextSelected + 1, 0u, (uint32_t)g_Options.LyricVars.SelectedTexts.size() - 1);
                ImGui::SetKeyboardFocusHere(-1);
            }
            if (ImGui::Selectable("Add##t", false, (g_Options.LyricVars.SelectedFonts.size() < 32 ? 0 : ImGuiSelectableFlags_Disabled)))
            {
                int32_t max_tc = 0;
                for (const auto& el : g_Options.LyricVars.SelectedTexts)
                    if (el.TimecodeMS >= max_tc)
                        max_tc = el.TimecodeMS + g_Options.LyricVars.DefaultTextDuration;

                g_Options.LyricVars.SelectedTexts.insert(
                    g_Options.LyricVars.TextSelected < g_Options.LyricVars.SelectedTexts.size() ? g_Options.LyricVars.SelectedTexts.begin() + g_Options.LyricVars.TextSelected + 1 : g_Options.LyricVars.SelectedTexts.end(),
                    {g_Options.LyricVars.CustomText, g_Options.LyricVars.DefaultTextDuration, max_tc, col_text });
                memset(&g_Options.LyricVars.CustomText[0], 0, sizeof(g_Options.LyricVars.CustomText));
                g_Options.LyricVars.TextSelected = std::clamp(g_Options.LyricVars.TextSelected + 1, 0u, (uint32_t)g_Options.LyricVars.SelectedTexts.size() - 1);
            }

            if (ImGui::Selectable("Remove##t", false, (g_Options.LyricVars.SelectedTexts.size() > g_Options.LyricVars.TextSelected ? 0 : ImGuiSelectableFlags_Disabled)))
            {
                g_Options.LyricVars.SelectedTexts.erase(g_Options.LyricVars.SelectedTexts.begin() + g_Options.LyricVars.TextSelected);
                g_Options.LyricVars.TextSelected = std::clamp(g_Options.LyricVars.TextSelected, 0u, (uint32_t)g_Options.LyricVars.SelectedTexts.size() - 1);
            }

            break;
#ifdef ENABLE_CONFIG
        case MENU_CONFIG:
            DrawConfigPanel();

            break;
#endif
        }
    }
    else
    {
        if (g_Options.LyricVars.ResetDrawing)
        {
            g_Options.LyricVars.FontToUse = 0;
            g_Options.LyricVars.TextToUse = 0;
            start_time_font = start_time_text = start_time_timecode = std::chrono::high_resolution_clock::now();

            int32_t max_tc = INT_MAX;
            for (const auto& el : g_Options.LyricVars.SelectedTexts)
                if (el.TimecodeMS < max_tc)
                    max_tc = el.TimecodeMS;
            start_time_timecode -= std::chrono::milliseconds(max_tc);

            g_Options.LyricVars.ResetDrawing = false;
        }

        static constexpr ImVec4 black(0, 0, 0, 1);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, black);
        ImGui::PushStyleColor(ImGuiCol_Border, black);
        ImGui::Begin(NAME_APP, &window_open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleColor(2);

        if (g_Options.LyricVars.UpdateFonts)
            g_Options.LyricVars.ShouldUpdateFonts = true;

        if (!g_Options.LyricVars.UpdateFonts && !g_Options.LyricVars.ShouldUpdateFonts && !g_Options.LyricVars.LoadedFonts.empty())
        {
            bool updated = UpdateTextToUse();
            UpdateFontToUse((FORCEMODE)updated);

            const auto& text = g_Options.LyricVars.SelectedTexts.at(g_Options.LyricVars.TextToUse);

            ImGui::PushFont(g_Options.LyricVars.LoadedFonts.at(g_Options.LyricVars.FontToUse));
            ImGui::PushStyleColor(ImGuiCol_Text, text.Color);

            ImGuiWindow* window = ImGui::GetCurrentWindow();
            auto text_size = ImGui::CalcTextSize(text.Text.c_str());
            ImGui::SetCursorPosX(roundf((window->Size.x - text_size.x) * 0.25f) * 2);
            ImGui::SetCursorPosY(roundf((window->Size.y - text_size.y) * 0.25f) * 2);
            ImGui::TextUnformatted(text.Text.c_str());

            ImGui::PopFont();
            ImGui::PopStyleColor();
        }
    }

    ImGui::End();
}

#ifdef ENABLE_CONFIG
void DrawConfigPanel()
{
    ImGui::PushItemWidth(-1);
    ImGui::Combo("##File", &g_Options.Menu.ConfigFile,
        [](void* data, int idx, char const** out_text) {
            *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str(); return true;
        }, (void*)&configFiles, (int)configFiles.size());
    if (ImGui::Selectable("Load", false)) {
        g_Options.Menu.LoadBox = true;
        g_Options.Menu.SaveBox = g_Options.Menu.DeleteBox = g_Options.Menu.NewConfigBox = false;
    }
    if (ImGui::Selectable("Save", false))
        if (!g_Options.Menu.SaveBox && (int)configFiles.size() > g_Options.Menu.ConfigFile && configFiles.at(g_Options.Menu.ConfigFile).length() > 0)
        {
            g_Options.Menu.SaveBox = true;
            g_Options.Menu.LoadBox = g_Options.Menu.DeleteBox = g_Options.Menu.NewConfigBox = false;
        }
    if (ImGui::Selectable("Delete", false))
        if (!g_Options.Menu.DeleteBox && (int)configFiles.size() > g_Options.Menu.ConfigFile && configFiles.at(g_Options.Menu.ConfigFile).length() > 0)
        {
            g_Options.Menu.DeleteBox = true;
            g_Options.Menu.LoadBox = g_Options.Menu.SaveBox = g_Options.Menu.NewConfigBox = false;
        }
    if (ImGui::Selectable("Create New", false))
        if (!g_Options.Menu.NewConfigBox)
        {
            g_Options.Menu.NewConfigBox = true;
            g_Options.Menu.LoadBox = g_Options.Menu.SaveBox = g_Options.Menu.DeleteBox = false;
        }
    if (ImGui::Selectable("Open Config Folder", false))
        ShellExecuteA(NULL, "open", pathToConfigs.c_str(), NULL, NULL, SW_NORMAL);

    ImGui::Separator();
    ImGui::NewLine();
    ImGui::Text("Toggle UI with [INS]");
}
#endif

//detect mouse dragging on the topbar and reposition the window appropriately
void WindowMoving()
{
    if (g_Options.Menu.BorderlessWindowed)
        return;

    ImGuiIO& io = ImGui::GetIO();

    static bool can_move_window = false, has_unclicked = true;
    static POINT oldMousePos, mousePos;
    bool window_focused = GetActiveWindow() == hwnd;
    GetCursorPos(&mousePos);
    if (window_focused)
    {
        bool shouldUpdate = false;
        if (io.MouseDown[0])
        {
            auto TitleBarHeight = ImGui::GetCurrentWindow()->TitleBarHeight();
            ImGuiStyle& style = ImGui::GetStyle();
            float button_sz = ImGui::GetFontSize() + style.FramePadding.x + style.ItemInnerSpacing.x;

            if (has_unclicked && io.MousePos.y < TitleBarHeight && io.MousePos.x < WINDOW_WIDTH - button_sz)
                can_move_window = true;
            has_unclicked = false;
        }
        else
        {
            has_unclicked = true;
            can_move_window = false;
        }
        if (can_move_window)
        {
            mouse_x -= int(oldMousePos.x - mousePos.x);
            mouse_y -= int(oldMousePos.y - mousePos.y);
            if (mousePos.x != oldMousePos.x || mousePos.y != oldMousePos.y)
                shouldUpdate = true;
        }
        if (shouldUpdate)
            MoveWindow(hwnd, mouse_x, mouse_y, WINDOW_WIDTH, WINDOW_HEIGHT, false);
    }
    else
    {
        has_unclicked = true;
        can_move_window = false;
    }

    oldMousePos = mousePos;
}

//popup box asking for user to accept or deny a request
DialogReturn AcceptBox(const std::string& String)
{
    ImGuiIO& io = ImGui::GetIO();
    DialogReturn ret = RETURN_NONE;
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

    if (ImGui::Begin(NAME_APP, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::BeginChild("first child accept", ImVec2(-1, -1));
        {
            ImGui::Text(String.c_str());
            ImGui::Text("");
            ImGui::Text("");
            ImGui::Text("");

            ImGui::Columns(2, "YES/NO", false);

            if (ImGui::Button("Yes", ImVec2((WINDOW_WIDTH * 0.95f - 2 * 35) * 0.5f, 0)) || !io.WantCaptureKeyboard && (ImGui::IsKeyReleased(ImGuiKey_Enter) || ImGui::IsKeyReleased(ImGuiKey_KeypadEnter))) //-V648
                ret = RETURN_OK;
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::SameLine();

            if (ImGui::Button("No", ImVec2((WINDOW_WIDTH * 0.95f - 2 * 35) * 0.5f, 0)))
                ret = RETURN_NO;
        }ImGui::EndChild();
    }ImGui::End();

    return ret;
}

//popup box asking user for input
DialogReturn InputBox(const std::string& String, bool new_box, char* input_var, int str_len)
{
    if (!input_var)
        return RETURN_NO;

    DialogReturn ret = RETURN_NONE;
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    if (new_box)
        ImGui::SetNextWindowFocus();
    if (ImGui::Begin(NAME_APP, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::BeginChild("first child input", ImVec2(-1, -1));
        {
            ImGui::Text(String.c_str());
            ImGui::Text("");
            ImGui::PushItemWidth(-1);
            if (new_box)
                ImGui::SetKeyboardFocusHere();
            ImGui::InputText("##input_var", input_var, str_len);
            ImGui::Text("");

            ImGui::Columns(2, "GOOD/CANCEL", false);

            if (ImGui::Button("Create", ImVec2((WINDOW_WIDTH * 0.95f - 2 * 35) * 0.5f, 0)) || ImGui::IsKeyReleased(ImGuiKey_Enter) || ImGui::IsKeyReleased(ImGuiKey_KeypadEnter))
                ret = RETURN_OK;
            ImGui::NextColumn();
            ImGui::Spacing();
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2((WINDOW_WIDTH * 0.95f - 2 * 35) * 0.5f, 0)))
                ret = RETURN_NO;
        }ImGui::EndChild();
    }ImGui::End();

    return ret;
}

//checks if a popup should be drawn, and deals with their logic
bool CheckPopups()
{
    bool priority = false;

#ifdef ENABLE_CONFIG
    static bool new_input_box = true;
    if (g_Options.Menu.NewConfigBox) {
        priority = true;

        auto var = &g_Options.Menu.ConfigName;
        auto ret = InputBox("Input Config Name:", new_input_box, *var, 32);
        switch (ret)
        {
        case(RETURN_OK):
            ConfigSys->Save(true);
            [[fallthrough]];
        case(RETURN_NO):
            memset(&var[0], 0, sizeof(*var));
            g_Options.Menu.NewConfigBox = false;
        }

        new_input_box = false;
    }
    else
    {
        auto var = &g_Options.Menu.ConfigName;
        memset(&var[0], 0, sizeof(*var));
        new_input_box = true;
    }

    if (g_Options.Menu.DeleteBox && configFiles.size()) {
        priority = true;
        std::string deleteString = "Delete '" + (configFiles.at(g_Options.Menu.ConfigFile)) + "'?";
        auto ret = AcceptBox(deleteString);
        switch (ret)
        {
        case(RETURN_OK):
            ConfigSys->Delete();
            [[fallthrough]];
        case(RETURN_NO):
            g_Options.Menu.DeleteBox = false;
        }
    }

    if (g_Options.Menu.LoadBox && configFiles.size()) {
        priority = true;
        std::string loadString = "Load '" + configFiles.at(g_Options.Menu.ConfigFile) + "'?";
        auto ret = configFiles.at(g_Options.Menu.ConfigFile).compare(g_Options.Menu.AssignedConfig) ? AcceptBox(loadString) : RETURN_OK;
        switch (ret)
        {
        case(RETURN_OK):
            if ((int)configFiles.size() > g_Options.Menu.ConfigFile && configFiles.at(g_Options.Menu.ConfigFile).length() > 0)
                ConfigSys->Load();
            [[fallthrough]];
        case(RETURN_NO):
            g_Options.Menu.LoadBox = false;
        }
    }

    if (g_Options.Menu.SaveBox) {
        priority = true;
        std::string saveString;
        saveString = "Overwrite '" + configFiles.at(g_Options.Menu.ConfigFile) + "'?";
        auto ret = AcceptBox(saveString);
        switch (ret)
        {
        case(RETURN_OK):
            ConfigSys->Save();
            [[fallthrough]];
        case(RETURN_NO):
            g_Options.Menu.SaveBox = false;
        }
    }
#endif

    return priority;
}

Variables g_Options;