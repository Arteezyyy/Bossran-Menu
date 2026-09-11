#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "../ImGui-Internal-Base/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ImGui-Internal-Base/imgui/imgui_internal.h"

namespace elements {
    bool tab(const char* name, bool boolean);
}
