#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <vector>
#include <chrono>
#include "imgui\imgui.h"

struct Notification {
    std::string icon;
    std::string message;
    std::string information;
    float duration; // Duration in seconds
    std::chrono::steady_clock::time_point start_time;
    ImU32 color;
};

#endif // NOTIFICATION_H
