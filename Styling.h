#pragma once
#include "imgui.h"

#define NAME_APP "Lyric Text Visualiser"
#define NAME_APP_WIDE L"Lyric Text Visualiser"
#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

//simple customized menu theme
inline ImVec4 Scaled(const ImVec4& val, float scale) { return ImVec4(val.x * scale, val.y * scale, val.z * scale, val.w); }
inline ImVec4 AlphaScaled(const ImVec4& val, const float& scale) { return ImVec4(val.x, val.y, val.z, val.w * scale); }
inline ImColor ShiftHSV(const ImColor& val, float h, float s, float v, float a = 1.0f) { float r, g, b, hg, sg, vg; ImGui::ColorConvertRGBtoHSV(val.Value.x, val.Value.y, val.Value.z, hg, sg, vg); ImGui::ColorConvertHSVtoRGB(h + hg, s + sg, v + vg, r, g, b); return ImColor(r, g, b, a); }

static ImColor im_col_main = ImColor(175, 25, 235);
static ImVec4 col_invisible = ImColor(0, 0, 0, 0);
static ImVec4 col_text = ImColor(235, 225, 250);
static ImVec4 col_text_disabled = Scaled(col_text, 0.75f);
static ImVec4 col_theme = ImColor(130, 73, 230); //regular
static ImVec4 col_theme_light = Scaled(col_theme, 1.1f); //clicked and hovered
static ImVec4 col_theme_hover = Scaled(col_theme, 0.9f); //hovered not clicked
static ImVec4 col_theme_active = Scaled(col_theme, 0.8f); //clicked not hovered
static ImVec4 background = ImColor(40, 40, 45);
static ImVec4 background_hover = Scaled(background, 1.1f);
static ImVec4 background_active = Scaled(background, 0.9f);

inline void SetupImGuiStyle()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = col_text;
	style.Colors[ImGuiCol_TextDisabled] = col_text_disabled;
	style.Colors[ImGuiCol_WindowBg] = background;
	style.Colors[ImGuiCol_ChildBg] = background;
	style.Colors[ImGuiCol_Border] = ShiftHSV(im_col_main, 0.02f, -0.3f, -0.625f);
	style.Colors[ImGuiCol_BorderShadow] = col_invisible;
	style.Colors[ImGuiCol_FrameBg] = Scaled(background, 0.97f);
	style.Colors[ImGuiCol_FrameBgHovered] = Scaled(background_hover, 0.97f);
	style.Colors[ImGuiCol_FrameBgActive] = Scaled(background_active, 0.97f);
	style.Colors[ImGuiCol_TitleBg] = col_theme;
	style.Colors[ImGuiCol_TitleBgCollapsed] = col_theme;
	style.Colors[ImGuiCol_TitleBgActive] = col_theme;
	style.Colors[ImGuiCol_MenuBarBg] = col_theme;
	style.Colors[ImGuiCol_ScrollbarBg] = background;
	style.Colors[ImGuiCol_ScrollbarGrab] = col_theme_hover;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = col_theme_light;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = col_theme_active;
	style.Colors[ImGuiCol_CheckMark] = col_theme;
	style.Colors[ImGuiCol_SliderGrabActive] = col_theme;
	style.Colors[ImGuiCol_Button] = col_theme;
	style.Colors[ImGuiCol_ButtonHovered] = col_theme_hover;
	style.Colors[ImGuiCol_ButtonActive] = col_theme_active;
	style.Colors[ImGuiCol_Header] = col_theme;
	style.Colors[ImGuiCol_HeaderHovered] = col_theme_hover;
	style.Colors[ImGuiCol_HeaderActive] = col_theme_active;
	style.Colors[ImGuiCol_TextSelectedBg] = AlphaScaled(col_theme, 0.8f);
	style.Colors[ImGuiCol_PopupBg] = background;

	style.Alpha = 1.f;
	style.WindowPadding = ImVec2(8, 8);
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 0.5f;
	style.WindowTitleAlign = ImVec2(0.f, 0.5f);
	style.ChildRounding = 5.f;
	style.FramePadding = ImVec2(4, 4);
	style.FrameRounding = 5.f;
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 22.0f;
	style.ColumnsMinSpacing = 0.f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 4.f;
	style.GrabMinSize = 20.f;
	style.GrabRounding = 5.f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
	style.CurveTessellationTol = 1.f;
	style.FrameBorderSize = 1.f;
	
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16.f * 2.5f);
}
