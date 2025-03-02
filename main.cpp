// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#define SKELTON_VERSION 1.0
#include "DX Functions.h"
#include "Interface.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#ifdef ENABLE_CONFIG
#include "ConfigSystem.h"
#endif
#include "Functions.h"

int APIENTRY WinMain(
    _In_ HINSTANCE,
    _In_opt_ HINSTANCE,
    _In_ PSTR,
    _In_ int
)
{
    //init directx and gui
    int ret = Setup();
    if (ret)
        return ret;

    ImGuiIO& io = ImGui::GetIO();
    //don't create a window position log since windows are not movable
    io.IniFilename = NULL;
    getFonts();

    //Main loop
    MSG msg;
    while (window_open)
    {

        UpdateFonts();

        //Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //draw the interface
        DrawUI();

        //mouse drag movement logic
        WindowMoving();

        //Poll and handle messages (inputs, window resize, etc.)
        //See the WndProc() function below for our to dispatch events to the Win32 backend.
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                window_open = false;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //Rendering
        DXRender();
    }

    //uninit directx and imgui
    Cleanup();

    return 0;
}
