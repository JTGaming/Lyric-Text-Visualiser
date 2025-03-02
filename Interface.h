#pragma once
#include "Variables.h"

DialogReturn AcceptBox(const std::string& String);
DialogReturn InputBox(const std::string& String, bool new_box, char* input_var, int str_len);
bool CheckPopups();
void DrawUI();
void DrawConfigPanel();
void WindowMoving();

extern bool window_open;