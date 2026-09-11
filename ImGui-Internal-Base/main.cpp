#include <thread>
#include <stdexcept>
#include "Core/globals.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "includes.h"
#include "elements.h"
#include "byte.h"
#include "../ImGui-Internal-Base/imgui/imgui_internal.h"
#include <cmath>
#include <cstdlib> // For rand()
#include <ctime>   // For seeding random numbers
#include <thread>
#include "AntiCheat.h"
#include <future>
#include <atomic>
#include <string>
#include "Notification.h"
#include "Contact.h"
#include <regex>
#include <unordered_set>
#include <set>
#include <Windows.h>
#include <vector>
#include <array>
#include <thread>
#include <chrono>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <wininet.h>
#include <string>
#include <algorithm>
#include <curl/curl.h>
#include "Function.h"
#pragma comment(lib, "wininet.lib")
extern void Form1();
extern void Form2();
extern void Form3();
extern void Form4();
extern bool Form1Show;
extern bool Form2Show;
extern bool Form3Show;
extern bool Form4Show;
extern bool show_overlay;
bool Form1Show = false;
bool Form2Show = false;
bool Form3Show = false;
bool Form4Show = true;

using namespace std;
enum heads
{
	HEAD_1,
	HEAD_2,
	HEAD_3,
	HEAD_4
};
namespace fonts
{
	ImFont* medium = nullptr;
	ImFont* semibold = nullptr;
	ImFont* customFont = nullptr;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK WNDProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
bool Menu::setup_wnd_class(const char* class_name) noexcept
{
	wnd_class.cbSize = sizeof(WNDCLASSEX);
	wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	wnd_class.lpfnWndProc = DefWindowProc;
	wnd_class.cbClsExtra = 0;
	wnd_class.cbWndExtra = 0;
	wnd_class.hInstance = GetModuleHandle(NULL);
	wnd_class.hIcon = 0;
	wnd_class.hCursor = 0;
	wnd_class.hbrBackground = 0;
	wnd_class.lpszMenuName = 0;
	wnd_class.lpszClassName = class_name;
	wnd_class.hIconSm = 0;

	if (!RegisterClassEx(&wnd_class))
		return false;

	return true;
}
void Menu::destroy_wnd_class() noexcept
{
	UnregisterClass( wnd_class.lpszClassName, wnd_class.hInstance);
}
bool Menu::setup_hwnd(const char* name) noexcept
{
	hwnd = CreateWindow( wnd_class.lpszClassName, name, WS_OVERLAPPEDWINDOW, 0, 0, 10, 10, 0, 0, wnd_class.hInstance, 0);

	if (!hwnd)
		return false;

	return true;
}
void Menu::destroy_hwnd() noexcept
{
	if (hwnd)
		DestroyWindow(hwnd);
}
bool Menu::SetupDX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return false;

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(handle, "Direct3DCreate9"));
	if (!create)
		return false;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return false;

	D3DPRESENT_PARAMETERS params = {  };
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = 0;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = hwnd;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = 0;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &device) < 0) 
		return false;

	return true;
}
void Menu::DestroyDX() noexcept
{
	if (device)
	{
		device->Release();
		device = NULL;
	}

	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}
void Menu::Core()
{
	if (!setup_wnd_class("class1"))
		throw std::runtime_error("Failed to create window class!");

	if (!setup_hwnd("ImGuiBaseProB1"))
		throw std::exception("failed to create window / SF");

	if (!SetupDX())
		throw std::runtime_error("failed to create device");

	destroy_hwnd();
	destroy_wnd_class();
}


void Menu::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	auto params = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&params);

	hwnd = params.hFocusWindow;
	org_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WNDProc)));

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(device);

	ImGuiIO& io = ImGui::GetIO();
	fonts::customFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 13.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
	io.FontDefault = fonts::customFont;


	setup = true;
}
void Menu::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(org_wndproc));

	DestroyDX();
}

//=====CUSTOMIZE SLIDER=====//
void SliderTest(const char* label, float* value, float min, float max, ImVec4 fillColor)
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 size(340, 20);
	ImVec2 padding(0, 0);
	float rounding = style.FrameRounding;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(label);
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 bb_max = ImVec2(pos.x + size.x, pos.y + size.y);
	ImRect bb(pos, bb_max);
	ImGui::InvisibleButton(label, size);

	static std::map<ImGuiID, float> startValueMap;
	static std::map<ImGuiID, float> targetValueMap;
	static std::map<ImGuiID, float> startTimeMap;
	static std::map<ImGuiID, float> currentAnimationValueMap;
	static std::map<ImGuiID, bool> isDraggingMap;

	float& animationStartValue = startValueMap[id];
	float& animationTargetValue = targetValueMap[id];
	float& animationStartTime = startTimeMap[id];
	float& currentAnimationValue = currentAnimationValueMap[id];
	bool& isDragging = isDraggingMap[id];

	const float animationDuration = 0.8f;
	bool value_changed = false;

	if (ImGui::IsItemActive()) {
		if (ImGui::IsMouseDragging(0)) {
			float mouse_fraction = (ImGui::GetIO().MousePos.x - pos.x - padding.x) / (size.x - 2 * padding.x);
			mouse_fraction = ImClamp(mouse_fraction, 0.0f, 1.0f);
			animationTargetValue = mouse_fraction * (max - min) + min;
			animationStartValue = currentAnimationValue;
			animationStartTime = ImGui::GetTime();
			value_changed = true;
			isDragging = true;
		}
		else if (ImGui::IsMouseClicked(0)) {
			float click_fraction = (ImGui::GetIO().MousePos.x - pos.x - padding.x) / (size.x - 2 * padding.x);
			click_fraction = ImClamp(click_fraction, 0.0f, 1.0f);
			animationTargetValue = click_fraction * (max - min) + min;
			animationStartValue = currentAnimationValue;
			animationStartTime = ImGui::GetTime();
			value_changed = true;
			isDragging = false;
		}
	}
	else {
		if (isDragging) {
			isDragging = false;
		}
	}

	float timeElapsed = ImGui::GetTime() - animationStartTime;
	float t = ImSaturate(timeElapsed / animationDuration);
	if (!isDragging) {
		currentAnimationValue = ImLerp(animationStartValue, animationTargetValue, t);
	}
	else {
		currentAnimationValue = animationTargetValue;
	}
	*value = currentAnimationValue;

	// Draw the background frame
	ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, rounding);

	// Calculate the fill area
	float value_fraction = (currentAnimationValue - min) / (max - min);
	ImVec2 fill_start = ImVec2(pos.x + padding.x, pos.y + padding.y);
	ImVec2 fill_end = ImVec2(pos.x + (size.x - 2 * padding.x) * value_fraction + padding.x, pos.y + size.y - padding.y);

	// Apply rounding correctly based on fill percentage
	ImDrawFlags fill_rounding_flags = ImDrawFlags_RoundCornersNone;

	if (value_fraction > 0.0f) {
		if (value_fraction < 1.0f) {
			fill_rounding_flags = ImDrawFlags_RoundCornersLeft | ImDrawFlags_RoundCornersRight;
		}
		else {
			fill_rounding_flags = ImDrawFlags_RoundCornersAll;
		}
	}

	// Draw the solid fill color with the correct rounding
	window->DrawList->AddRectFilled(fill_start, fill_end, ImGui::GetColorU32(fillColor), rounding, fill_rounding_flags);

	// Draw the slider border
	ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), true, rounding);
}
//=====CUSTOMIZE SLIDER=====//
ImColor RainbowColor(float time)
{
	// Adjust these parameters to control the rainbow effect
	float frequency = 0.3f; // Controls how fast the colors cycle
	float amplitude = 127.0f; // Controls the color intensity

	const float pi = std::acos(-1); // Get pi from cmath

	// Calculate RGB values using sine function
	int r = static_cast<int>(sin(frequency * time + 0) * amplitude + 128);
	int g = static_cast<int>(sin(frequency * time + 2 * pi / 3) * amplitude + 128);
	int b = static_cast<int>(sin(frequency * time + 4 * pi / 3) * amplitude + 128);

	return ImColor(r, g, b);
}
//=====BACKGROUND=====//
struct Particle {
	ImVec2 position;
	ImVec2 velocity;
	float size;
};
class ParticleNetwork {
public:
	ParticleNetwork(int numParticles, ImVec2 windowSize) : numParticles(numParticles), windowSize(windowSize) {
		std::srand(static_cast<unsigned>(std::time(0)));
		initParticles();
	}

	void draw(ImDrawList* draw_list) {
		updateParticles();
		drawParticles(draw_list);
		connectParticles(draw_list);
	}
private:
	int numParticles;
	ImVec2 windowSize;
	std::vector<Particle> particles;

	void initParticles() {
		for (int i = 0; i < numParticles; ++i) {
			float x = static_cast<float>(std::rand()) / RAND_MAX * windowSize.x;
			float y = static_cast<float>(std::rand()) / RAND_MAX * windowSize.y;
			float vx = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 1.0f;
			float vy = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 1.0f;
			float size = 2.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f;

			particles.push_back({ ImVec2(x, y), ImVec2(vx, vy), size });
		}
	}

	void updateParticles() {
		float speedFactor = 0.2f; // Reduce speed to 50%

		for (auto& particle : particles) {
			particle.position.x += particle.velocity.x * speedFactor;
			particle.position.y += particle.velocity.y * speedFactor;

			// Bounce off edges
			if (particle.position.x <= 0.0f || particle.position.x >= windowSize.x)
				particle.velocity.x *= -1.0f;
			if (particle.position.y <= 0.0f || particle.position.y >= windowSize.y)
				particle.velocity.y *= -1.0f;
		}
	}

	void drawParticles(ImDrawList* draw_list) {
		for (const auto& particle : particles) {
			draw_list->AddCircleFilled(particle.position, particle.size, IM_COL32(0, 255, 255, 255));
		}
	}

	void connectParticles(ImDrawList* draw_list) {
		float maxDistance = 50.0f;

		for (size_t i = 0; i < particles.size(); ++i) {
			for (size_t j = i + 1; j < particles.size(); ++j) {
				float dx = particles[i].position.x - particles[j].position.x;
				float dy = particles[i].position.y - particles[j].position.y;
				float distance = sqrtf(dx * dx + dy * dy);

				if (distance < maxDistance) {
					float opacity = 1.0f - (distance / maxDistance);
					ImU32 color = IM_COL32(0, 255, 255, static_cast<int>(opacity * 255));
					draw_list->AddLine(particles[i].position, particles[j].position, color);
				}
			}
		}
	}
};
void ShowParticleNetworkBackground() {
	static ParticleNetwork network(150, ImVec2(800, 600)); // You may want to adjust the size

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImDrawList* draw_list = window->DrawList;

	network.draw(draw_list);
}

std::vector<Notification> notifications;
void ShowNotification(const std::string& icon, const std::string& information, const std::string& message, int duration_ms, ImU32 color) {
	Notification notification;
	notification.icon = icon;
	notification.information = information;
	notification.message = message;
	notification.duration = duration_ms / 1000.0f; // Convert milliseconds to seconds
	notification.start_time = std::chrono::steady_clock::now();
	notification.color = color;
	notifications.push_back(notification);
}

void RenderNotifications() {
	auto now = std::chrono::steady_clock::now();

	for (int i = 0; i < notifications.size();) {
		Notification& notification = notifications[i];
		float elapsed = std::chrono::duration<float>(now - notification.start_time).count();

		if (elapsed > notification.duration) {
			notifications.erase(notifications.begin() + i);
		}
		else {
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 350, ImGui::GetIO().DisplaySize.y - (i + 1) * 80), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(300, 70), ImGuiCond_Always);

			ImGui::Begin(("Notification##" + std::to_string(i)).c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("%s", notification.information.c_str());
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::TextWrapped("%s", notification.message.c_str());
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::End();

			++i;
		}
	}
}



void Sliders()
{}

std::atomic<bool> isHWIDValidationCompleted(false);
std::atomic<bool> isHWIDValid(false);
#pragma region FORM 3 SECURITY
bool HasTooManyUniqueChars(const std::string& str, size_t maxUniqueChars) {
	std::set<char> uniqueChars(str.begin(), str.end());
	return uniqueChars.size() > maxUniqueChars;
}

// Function to validate the username
bool IsValidUsername(const std::string& username) {
	// Username must be between 3 and 20 characters long
	if (username.length() < 3 || username.length() > 20) return false;

	// Username must contain at least one letter and should not be just numbers or gibberish
	std::regex validUsernameRegex("^(?!\\d+$)[a-zA-Z0-9]+$");

	// Check if the username is valid based on the regex
	if (!std::regex_match(username, validUsernameRegex)) return false;

	// Check for commonly used gibberish patterns
	std::unordered_set<std::string> gibberishWords = { "qweqwe", "123123", "asdfghjkkl", "qwertyuiop" };

	// Convert the username to lowercase for case-insensitive comparison
	std::string lowerUsername = username;
	std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);

	// Check if the username is in the gibberish list
	if (gibberishWords.find(lowerUsername) != gibberishWords.end()) return false;

	// Check for repeating sequences of two or more characters (e.g., "ababab" or "qwertyqwerty")
	std::regex repeatingPatternRegex(R"((\w{2,})\1+)");
	if (std::regex_search(lowerUsername, repeatingPatternRegex)) return false;

	// Check for too many unique characters (to block highly random strings)
	if (HasTooManyUniqueChars(lowerUsername, 10)) return false;

	return true;
}

bool IsValidPassword(const std::string& password) {
	// Password must be between 6 and 20 characters long
	if (password.length() < 6 || password.length() > 20) return false;

	// Ensure the password follows the "Name Ran Ep#" format
	// Name: starts with letters only, followed by " Ran Ep" and a number between 3 and 11

	// Check for too many unique characters (to block highly random strings)
	if (HasTooManyUniqueChars(password, 15)) return false;

	return true;
}

void SaveHWID(const std::string& hwid) {
	// Change the path to C:\Users\Public\Documents
	std::ofstream hwidFile("C:\\Users\\Public\\Documents\\sent_hwids.txt", std::ios::app);
	hwidFile << hwid << std::endl;
}

#pragma endregion
void ValidateHWID() {
	std::string urlPaste = "https://www.dropbox.com/scl/fo/oxz2xipi2cjiyrvol72o9/ACutQ2_r9KfJgBnQAWNSSWY?rlkey=7fbf80lxx2d42xrxn42o28rsk&st=4r5xzt7d&dl=1";
	std::string HWIDTarget = GetComputerSid();
	isHWIDValid = IsHWIDValid(std::wstring(urlPaste.begin(), urlPaste.end()), HWIDTarget);
	isHWIDValidationCompleted = true;
}
void ValidateHWIDPeriodically() {
	while (!isHWIDValid) {  // Continue checking only if HWID is not valid
		ValidateHWID(); // Call the HWID validation
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for 1 seconds
	}
}
#include <Windows.h> // For GetAsyncKeyState

void Form1() {
	ImVec2 windowSize = ImGui::GetWindowSize();

	// --- ADD CLOSE BUTTON "X" TOP RIGHT ---
	float closeButtonSize = 25.0f; // maliit lang na square button
	ImGui::SetCursorPos(ImVec2(windowSize.x - closeButtonSize - 10.0f, 10.0f)); // 10 px padding mula sa kanan at taas
	if (ImGui::Button("X", ImVec2(closeButtonSize, closeButtonSize))) {
		// Ito ang fix: itigil ang buong overlay
		Menu::show_overlay = false; // ← itigil ang buong GUI rendering
	}

	// --- EXISTING CODE MO SIMULA DITO ---

	float statusTextWidth = ImGui::CalcTextSize("Status: Validated").x;
	float statusPosX = windowSize.x - statusTextWidth - 20.0f;
	ImGui::SetCursorPos(ImVec2(statusPosX, 10.0f));
	for (int i = 0; i < 10; ++i) {
		ImGui::Spacing();
	}
	float welcomeTextWidth = ImGui::CalcTextSize("KOYAWELL").x;
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - welcomeTextWidth) * 0.5f);
	ImGui::Text("KOYAWELL");
	for (int i = 0; i < 3; ++i) {
		ImGui::Spacing();
	}
	float pressTextWidth = ImGui::CalcTextSize("PRESS SUCCESS TO PROCEED.").x;
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - pressTextWidth) * 0.5f);
	ImGui::Text("PRESS SUCCESS TO PROCEED.");


	static std::future<void> validationThread = std::async(std::launch::async, ValidateHWIDPeriodically);
	float buttonWidth = 300.0f;
	float buttonHeight = 30.0f;
	float buttonPosX = (ImGui::GetWindowSize().x - buttonWidth) * 0.5f;
	float buttonPosY = ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y * 2;
	ImGui::SetCursorPos(ImVec2(buttonPosX, buttonPosY));
	RenderNotifications();
	if (ImGui::Button("   SUCCESS   ", ImVec2(buttonWidth, buttonHeight)))
	{
		if (isHWIDValidationCompleted && isHWIDValid)
		{
			SuspendNtdllThreadsSkipTrainer();
			Sleep(20);
			ScriptCaving();
			Form2Show = true;
		}
		else
		{
			ShowNotification("A", "Invalid User", "Copy your HWID and send it to the developer!", 3000, IM_COL32(255, 255, 255, 255)); // Display for 3 seconds
		}
	};
}

std::string DownloadPasswordFromDropbox(const char* url);
void Form2() {
	static heads head_selected = HEAD_4;
	ShowParticleNetworkBackground();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImVec4 bgColor = ImVec4(19 / 255.f, 23 / 255.f, 22 / 255.f, 0.95f);
	auto draw = ImGui::GetWindowDrawList();
	auto pos = ImGui::GetWindowPos();
	auto size = ImGui::GetWindowSize();
	float currentTime = ImGui::GetTime();
	ImColor rainbowColor = RainbowColor(currentTime);

	// 🔍 Read 2 bytes raw from memory
	uint8_t bytes[2] = {};
	SIZE_T bytesRead = 0;
	BOOL ok = ReadProcessMemory(
		GetCurrentProcess(),            // or target process handle
		(LPCVOID)0x029343f4, 
		bytes,
		sizeof(bytes),
		&bytesRead
	);

	// 🧩 Format as "FF AA"
	char buf[16];
	if (ok && bytesRead == sizeof(bytes)) {
		// "%02X" produces uppercase hex with leading zero
		snprintf(buf, sizeof(buf), "%02X %02X", bytes[0], bytes[1]);
	}
	else {
		strcpy_s(buf, sizeof(buf), "-- --");
	}
	// hex formatting like this is standard practice in C/C++ :contentReference[oaicite:1]{index=1}

	// Rotate header texts every 7 seconds
	static float lastChangeTime = 0.0f;
	static int textIndex = 0;
	const char* textOptions[] = { "KOYAWELL", "BABYRAN", "ALL IN ONE" };
	if (currentTime - lastChangeTime >= 7.0f) {
		textIndex = (textIndex + 1) % IM_ARRAYSIZE(textOptions);
		lastChangeTime = currentTime;
	}
	const char* displayText = textOptions[textIndex];

	draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 40),
		ImColor(23 / 255.f, 23 / 255.f, 23 / 255.f), 9.0f, ImDrawFlags_RoundCornersTop);

	draw->AddText(ImVec2(pos.x + 20, pos.y + 13), ImColor(rainbowColor), displayText);

	// Move the "FF AA" display to the right of the last tab (e.g., after "ADMIN")
	float hexDisplayX = pos.x + 304;  // Adjust as needed
	draw->AddText(ImVec2(hexDisplayX, pos.y + 13),
		ImColor(1.0f, 1.0f, 0.5f, 1.0f), buf);

	float lineX = pos.x + 110;
	draw->AddLine(ImVec2(lineX, pos.y + 3), ImVec2(lineX, pos.y + 35),
		ImColor(255, 255, 255));

	ImGui::PopStyleVar();

	ImGui::SetCursorPos({ 125, 13 });
	ImGui::BeginGroup();
	{
		if (elements::tab("MAIN", head_selected == HEAD_1)) head_selected = HEAD_1;
		ImGui::SameLine();
		if (elements::tab("INFO", head_selected == HEAD_4)) head_selected = HEAD_4;
		ImGui::SameLine();
		
	}
	ImGui::EndGroup();

	switch (head_selected) {
	case HEAD_1:
		// Call BooleanFunct to check antidog state and enable/disable monitoring
		BooleanFunct();
		RenderNotifications();
		ImGui::Dummy(ImVec2(0, 1));  // Space for tab

		ImVec2 avail = ImGui::GetContentRegionAvail();  // NOW get available space AFTER dummy

		ImGui::BeginChild("GAGO KA BA?", avail, true, ImGuiWindowFlags_HorizontalScrollbar);



		// ------------------------
		// ✅ HEAD_1 CONTENT BELOW
		// ------------------------
		ImGui::Columns(2, nullptr, false); // 2-column layout
		// LEFT COLUMN - Checkbox setup
		ImGui::Checkbox("GODSEYEEXTND", &mata);
		ImGui::Checkbox("WALLNEYM", &wallneym);
		ImGui::Checkbox("AREAOFEFFECT", &lawakpepe);
		ImGui::Checkbox("30% FAST SCROLL", &fscroll);
		ImGui::Checkbox("REMOVE FOG (NEAR)", &fog);
		ImGui::Checkbox("AUTO RED NAME", &autored);  // This will be toggled via F8
		ImGui::Checkbox("ANTI TAGO", &tago);
		ImGui::Checkbox("BILIS PET", &bilispet);
		ImGui::Checkbox("BILIS BANGON", &bohay);
		ImGui::Checkbox("THRU WALL", &tagoswall);

		ImGui::NextColumn(); // Switch to right column

		// RIGHT COLUMN - More checkboxes
		ImGui::Checkbox("PY SP BP TAXI", &pyheck);
		ImGui::Checkbox("SPACE HECK", &space);  // This checkbox will be toggled via Right Alt
		ImGui::Checkbox("LR BUS", &buslayo);
		ImGui::Checkbox("ANTI ASO", &antingongo);
		ImGui::Checkbox("Anti move", &vend);    // This will be toggled via F9
		
	

		ImGui::Columns(1); // Reset columns

		ImGui::Columns(1); // Reset columns

		// Static variable to track the last state of the Right Alt key
		static bool lastAltState = false;  // To track the last state of Right Alt
		static bool lastF8State = false;   // To track the last state of F8 (Auto PK)
		static bool lastF9State = false;   // To track the last state of F9 (Vend Mode

		bool rightAltPressed = (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
		bool f8Pressed = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;  // Check for F8 key (Auto PK)
		bool f9Pressed = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;  // Check for F9 key (Vend Mode)

		// Handle Right Alt key press for "AUTO SPACE" checkbox (same as before)
		if (rightAltPressed && !lastAltState) {
			if (!space) {
				// Check the checkbox if it's not already checked
				space = true;
				// Show the notification for "Auto PK is ON"
				ShowNotification("Auto Space", "System", "Auto Space is ON", 2000, 0xFF00FF00);  // Example color: Green
			}
			else {
				// Uncheck the checkbox if it's already checked
				space = false;
				// Show the notification for "Auto PK is OFF"
				ShowNotification("Auto Space", "System", "Auto Space is OFF", 2000, 0xFFFF0000);  // Example color: Red
			}
		}

		// Handle F8 key press for "AUTO PK" checkbox
		if (f8Pressed && !lastF8State) {
			if (!autored) {
				// Enable Auto PK
				autored = true;
				// Show the notification for "Auto PK is ON"
				ShowNotification("Red Name", "System", "Red Name is ON", 2000, 0xFF00FF00);  // Green color
			}
			else {
				// Disable Auto PK
				autored = false;
				// Show the notification for "Auto PK is OFF"
				ShowNotification("Red Name", "System", "Red Name is OFF", 2000, 0xFFFF0000);  // Red color
			}
		}

		// Handle F9 key press for "Vend Mode" checkbox
		if (f9Pressed && !lastF9State) {
			if (!vend) {
				// Enable Vend Mode
				vend = true;
				// Show the notification for "Vend Mode is ON"
				ShowNotification("Anti Move", "System", "Anti move is ON", 2000, 0xFF00FF00);  // Green color
			}
			else {
				// Disable Vend Mode
				vend = false;
				// Show the notification for "Vend Mode is OFF"
				ShowNotification("Anti Move", "System", "Anti move is OFF", 2000, 0xFFFF0000);  // Red color
			}
		}

		// Store the last state of the keys to detect changes in the next frame
		lastAltState = rightAltPressed;
		lastF8State = f8Pressed;
		lastF9State = f9Pressed;


		ImGui::Separator();  // This adds a border line after HEAD_1

		// ------------------------
		   // ✅ HEAD_2 CONTENT BELOW
		   // ------------------------

		ImGui::Spacing();
		ImGui::Text("LAYO");
		ImGui::SameLine(320);
		ImGui::Text("%.3f", lrval);  // Display current value of lrval

		ImGui::Spacing();
		ImGui::SameLine(20);

		// Input box for lrval using InputText
		char lrvalStr[64];
		// Use sprintf_s to safely format the float into the string buffer
		sprintf_s(lrvalStr, IM_ARRAYSIZE(lrvalStr), "%.3f", lrval);  // Convert float to string to show in text box

		if (ImGui::InputText("###LR_Input", lrvalStr, IM_ARRAYSIZE(lrvalStr))) {
			// As soon as the user types, update the value directly
			float newValue = atof(lrvalStr);
			if (newValue < 0.0f) newValue = 0.0f;
			if (newValue > 999.0f) newValue = 999.0f;  // Clamp to max 999
			lrval = newValue;  // Update value immediately
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20);
		ImGui::Text("BILIS");
		ImGui::SameLine(320);
		ImGui::Text("%.3f", asval1);  // Display current value of asval

		ImGui::Spacing();
		ImGui::SameLine(20);

		// Input box for asval using InputText
		char asval1Str[64];
		sprintf_s(asval1Str, IM_ARRAYSIZE(asval1Str), "%.3f", asval1);

		if (ImGui::InputText("###AS_Input", asval1Str, IM_ARRAYSIZE(asval1Str)))
		{
			float newValue = atof(asval1Str);

			// 👉 If 0 or empty/invalid, default to 1.00
			if (newValue == 0.0f)
				newValue = 1.0f;

			if (newValue < 0.0f) newValue = 0.0f;
			if (newValue > 999.0f) newValue = 999.0f;

			asval1 = newValue;
		}

		//ImGui::Spacing();
		//ImGui::Spacing();
		//ImGui::Spacing();
		//ImGui::SameLine(20);
		//ImGui::Text("ATTACK SPEED VER 2");
		//ImGui::SameLine(320);
		//ImGui::Text("%.3f", asval2);  // Display current value of asval2

		//ImGui::Spacing();
		//ImGui::SameLine(20);

		// Input box for asval using InputText
		//char asval2Str[64];
		// Use sprintf_s to safely format the float into the string buffer
		//sprintf_s(asval2Str, IM_ARRAYSIZE(asval2Str), "%.3f", asval2);  // Convert float to string to show in text box

		//if (ImGui::InputText("###AS2_Input", asval2Str, IM_ARRAYSIZE(asval2Str))) {
		//	 As soon as the user types, update the value directly
		//	float newValue = atof(asval2Str);
		//	if (newValue < 0.0f) newValue = 0.0f;
		//	if (newValue > 999.0f) newValue = 999.0f;  // Clamp to max 999
		//	asval2 = newValue;  // Update value immediately
		//}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20);
		ImGui::Text("Cut");
		ImGui::SameLine(320);
		ImGui::Text("%.3f", csval);  // Display current value of csval

		ImGui::Spacing();
		ImGui::SameLine(20);

		// Input box for csval using InputText
		char csvalStr[64];
		// Use sprintf_s to safely format the float into the string buffer
		sprintf_s(csvalStr, IM_ARRAYSIZE(csvalStr), "%.3f", csval);  // Convert float to string to show in text box

		if (ImGui::InputText("###CS_Input", csvalStr, IM_ARRAYSIZE(csvalStr))) {
			// As soon as the user types, update the value directly
			float newValue = atof(csvalStr);
			if (newValue < 0.0f) newValue = 0.0f;
			if (newValue > 999.0f) newValue = 999.0f;  // Clamp to max 999
			csval = newValue;  // Update value immediately
		}

		ImGui::Separator();  // This adds a border line after HEAD_2


		// ------------------------
	// ✅ HEAD_3 CONTENT BELOW
	// ------------------------

		ImGui::Spacing();

		// Center Combo Box
		ImVec2 windowSize = ImGui::GetWindowSize();
		float centerX = (windowSize.x - 300) / 2;
		ImGui::SetCursorPosX(centerX);

		const char* comboItems[57] = {
			"SUPER LIGHTNING SLAUGHER", "SUPER CLOUD BLAST", "SUPER CHICKEN GOD BURNING",
			"SUPER CLASS DEFORMATION CUTTING", "SUPER VIOLENT FLACTUATION", "SUPER MONKEY GOD SUMMONS",
			"SUPER SPIDER BLAZE", "SUPER DRAGON GOD SUMMONS", "SUPER FUTURE CONTAINER",
			"SUPER BLOOD RAIN", "SUPER WILL O WISP", "SUPER SD", "SUPER SEISMIC CYCLONE",
			"SUPER COSMIC DANCE", "SUPER DIMENSION ILLUSION", "SUPER SNOW FLAKE CYCLOTON",
			"SUPER GRIM REAPER BREAKING", "SUPER RED RESTRICTED", "SUPER DRAGON RISING",
			"SUPER DANCE OF THE WARRIOR", "SUPER BLUE SPIRIT RED SHADOW", "SUPER RENDEZVOUS",
			"EXTREME DANCE OF WARRIOR", "EXTREME VIOLENT FLACTUATION", "EXTREME RED RESTRICTED",
			"EXTREME BLOOD RAIN", "EXTREME SOUL REAVER", "EXTREME GRIM REAPER BREAKING",
			"EXTREME SNOWFLAKE CYCLOTRON", "EXTREME DIMENSION ILLUSION", "EXTREME BLUE DRAGON RISER",
			"EXTREME COSMIC DANCE", "EXTREME SEISMIC CYCLONE", "EXTREME WILL O WISP",
			"EXTREME DEMON SPEAR DANCE", "EXTREME DEFORMATION CUTTING", "EXTREME MONKEY GOD SUMMONS",
			"SHAMAN WILL O WISP", "SHAMAN DEMON SPEAR DANCE", "SHAMAN DEFORMATION CUTTING",
			"SHAMAN MONKEY GOD SUMMONS", "GUNNER LIGHTNING SLAUGTHER NO DELAY",
			"GUNNER SPIDER BLAZE NO DELAY","GUNNER CLOUD BLAST NO DELAY","GUNNER CHICKEN GOD BURNING NO DELAY",
			"ASSASSIN FUTURE CONTAINER NO DELAY","ASSASSIN BLUE SPIRIT RED SHADOW NO DELAY",
			"ASSASSIN DRAGON GOD SUMMONS NO DELAY","ASSASSIN DEMON HUNTER NO DELAY","ASSASSIN GENOCIDE NO DELAY",
			"ASSASSIN RENDEZVOUS NO DELAY","MAGICIAN JOKER PRANK NO DELAY","MAGICIAN CHEER FOR VICTORY NO DELAY",
			"MAGICIAN TIGER GOD STRIKES NO DELAY","MAGICIAN BLUE SHADOW WHIP NO DELAY","MAGICIAN BLAST DISINTERGRATION NO DELAY",
			"OFF AUTO KILL / FARM",
		};

		static int selectedCombo = 0;
		static std::vector<BYTE> skillBytes;

		if (ImGui::Combo("Select Skill", &selectedCombo, comboItems, IM_ARRAYSIZE(comboItems))) {
			std::string value;
			switch (selectedCombo) {
			case 0: value = "38 00 3f 00"; break;
			case 1: value = "38 00 43 00"; break;
			case 2: value = "38 00 44 00"; break;
			case 3: value = "37 00 5b 00"; break;
			case 4: value = "34 00 5A 00"; break;
			case 5: value = "37 00 5C 00"; break;
			case 6: value = "38 00 3E 00"; break;
			case 7: value = "39 00 4C 00"; break;
			case 8: value = "39 00 4E 00"; break;
			case 9: value = "35 00 4B 00"; break;
			case 10: value = "37 00 54 00"; break;
			case 11: value = "34 00 5C 00"; break;
			case 12: value = "36 00 5a 00"; break;
			case 13: value = "36 00 59 00"; break;
			case 14: value = "36 00 52 00"; break;
			case 15: value = "35 00 55 00"; break;
			case 16: value = "35 00 54 00"; break;
			case 17: value = "34 00 5b 00"; break;
			case 18: value = "34 00 58 00"; break;
			case 19: value = "34 00 4f 00"; break;
			case 20: value = "39 00 4d 00"; break;
			case 21: value = "39 00 46 00"; break;
			case 22: value = "1e 00 37 00"; break;
			case 23: value = "1e 00 4b 00"; break;
			case 24: value = "1e 00 4c 00"; break;
			case 25: value = "1F 00 38 00"; break;
			case 26: value = "1F 00 37 00"; break;
			case 27: value = "1F 00 4B 00"; break;
			case 28: value = "1F 00 4C 00"; break;
			case 29: value = "20 00 38 00"; break;
			case 30: value = "20 00 37 00"; break;
			case 31: value = "20 00 4C 00"; break;
			case 32: value = "20 00 4D 00"; break;
			case 33: value = "21 00 38 00"; break;
			case 34: value = "21 00 37 00"; break;
			case 35: value = "21 00 4D 00"; break;
			case 36: value = "21 00 4E 00"; break;
			case 37: value = "0F 00 01 00"; break;
			case 38: value = "0F 00 00 00"; break;
			case 39: value = "0F 00 08 00"; break;
			case 40: value = "0F 00 09 00"; break;
			case 41: value = "27 00 08 00"; break;
			case 42: value = "27 00 07 00"; break;
			case 43: value = "27 00 0c 00"; break;
			case 44: value = "27 00 0d 00"; break;
			case 45: value = "2b 00 16 00"; break;
			case 46: value = "2b 00 15 00"; break;
			case 47: value = "2b 00 14 00"; break;
			case 48: value = "2b 00 11 00"; break;
			case 49: value = "2b 00 06 00"; break;
			case 50: value = "2b 00 0e 00"; break;
			case 51: value = "2f 00 0c 00"; break;
			case 52: value = "2f 00 0a 00"; break;
			case 53: value = "2f 00 13 00"; break;
			case 54: value = "2f 00 14 00"; break;
			case 55: value = "2f 00 15 00"; break;
			case 56: value = "ff ff ff ff"; break;
			default: value = "FF FF FF FF"; break;
			}

			// Convert to byte array
			skillBytes.clear();
			std::stringstream ss(value);
			std::string byteStr;
			while (ss >> byteStr) {
				skillBytes.push_back(static_cast<BYTE>(std::stoi(byteStr, nullptr, 16)));
			}

			// ✅ Auto write to address 0x029269FC upon skill selection
			if (!skillBytes.empty()) {
				WriteBytes(0x0267ABD4, skillBytes.data(), skillBytes.size());
			}
		}

		// Variables
		static bool isToggled = false;
		static int value5 = 100;
		static char value5Input[16];
		sprintf_s(value5Input, "%d", value5);


		static bool wasPressed = false;
		static bool lastToggleState = isToggled;  // Track previous state

		if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState('Z') & 0x8000)) {
			if (!wasPressed) {
				isToggled = !isToggled;
				wasPressed = true;

				if (isToggled) {
					ShowNotification("Auto Skill", "System", "Auto Skill is ON", 2000, 0xFF00FF00);  // Green
				}
				else {
					ShowNotification("Auto Skill", "System", "Auto Skill is OFF", 2000, 0xFFFF4444); // Red
				}

				lastToggleState = isToggled;
			}
		}
		else {
			wasPressed = false;
		}

		// Align to left
		ImGui::SetCursorPosX(10); // Left alignment (adjust as needed)

		// Bigger Checkbox
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
		ImGui::Checkbox("##FreezeSkill", &isToggled); // Hidden label
		ImGui::PopStyleVar();
		ImGui::SameLine();

		// Label + ON/OFF status
		ImGui::Text("Auto Pilot: ");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, isToggled ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		ImGui::Text(isToggled ? "ON" : "OFF");
		ImGui::PopStyleColor();
		ImGui::SameLine();

		// Input box for value5
		ImGui::Text("Value:");
		ImGui::SameLine();
		if (ImGui::InputText("##Value5", value5Input, IM_ARRAYSIZE(value5Input), ImGuiInputTextFlags_CharsDecimal)) {
			int newValue = atoi(value5Input);
			if (newValue >= 0 && newValue <= 9999) {
				value5 = newValue;
			}
		}

		// Constant loop check (every 100ms)
		static DWORD lastUpdate = GetTickCount();
		if (GetTickCount() - lastUpdate > 100) {
			lastUpdate = GetTickCount();

			if (isToggled) {

				// Skill bypass NOPs


				if (!skillBytes.empty()) {
					WriteBytes(SKILLHEX, skillBytes.data(), skillBytes.size());
				}

				BYTE value1 = 1;
				WriteBytes(0x0267aba8, &value1, 1);

				BYTE value2 = 1;
				WriteBytes(0x0267abac, &value2, 1);

				int value3 = 100;
				WriteBytes(0x0267abb0, (BYTE*)&value3, 4);

				int value4 = 999;
				WriteBytes(0x0267abb8, (BYTE*)&value4, 4);

				WriteBytes(0x0267abbc, (BYTE*)&value5, 4);
				
			}
			else {
				BYTE offVal1 = 0;
				WriteBytes(0x0267aba8, &offVal1, 1);

				DWORD offValRest = 0xFFFFFFFF;
				WriteBytes(0x0267abac, (BYTE*)&offValRest, 4);
				WriteBytes(0x0267abb0, (BYTE*)&offValRest, 4);
				WriteBytes(0x0267abb8, (BYTE*)&offValRest, 4);
				WriteBytes(0x0267abbc, (BYTE*)&offValRest, 4);
				
			}
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::EndChild(); // End of scrollable area
		break;

	}




	switch (head_selected) {
	case HEAD_4:
		ImGui::SetCursorPos({ 20, 55 });
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20);
		ImGui::Text("WELCOME!");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20);
		ImGui::Text("F8 RED NAME ON & OFF");  // Display Auto PK status
		ImGui::SameLine(320);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20);  // Space between elements
		ImGui::Text("F9 VEND MODE ON & OFF");  // Display Vend Mode status
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20); // Space between elements
		ImGui::Text("LEFT-ALT AUTO SPACE ON & OFF");  // Display Auto Space status
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20); // Space between elements
		ImGui::Text("ALT + Z ON & OFF");  // Display Auto Space status
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine(20); // Space between elements
		ImGui::Text("HOVER,PET,BANK ZERO CD");  // Display Auto Space status

		ImGui::Spacing(); // Add space after this row




		// Set the position of the first button to the bottom-left with the same space from the left
		ImGui::SetCursorPos(ImVec2(20, 300 - 40));  // Same left margin (20), but position it at the bottom (300 is the height, 40 is for spacing)

		// Push only the button text color to rainbow for the first button
		ImVec4 rainbowColor1 = RainbowColor(currentTime); // Get the rainbow color for the first button

		// Push the custom text color for the button text
		ImGui::PushStyleColor(ImGuiCol_Text, rainbowColor1);

		// Draw the first button (without changing the background color)
		if (ImGui::Button("CLIENT FOR Bossran", ImVec2(160, 30))) {
			// Run minia.exe with a timeout to close the CMD window after 1 second
			system("start /min cmd /c \"minia.exe fpp123nb_run && timeout /t 1 && exit\"");
		}

		// Pop the style color after rendering the first button
		ImGui::PopStyleColor(1); // Only pop the text color style change for the first button

		// Add same spacing between the buttons (adjust x position for the second button)
		ImGui::SameLine(20 + 160 + 10); // Move 160 pixels right for the first button and add extra spacing of 10

		// Set the position of the second button to the right
		ImGui::SetCursorPos(ImVec2(20 + 160 + 10, 300 - 40));  // Right next to the first button

		// Push rainbow color for the second button
		ImVec4 rainbowColor2 = RainbowColor(currentTime); // Get the rainbow color for the second button

		// Push the custom text color for the second button text
		ImGui::PushStyleColor(ImGuiCol_Text, rainbowColor2);

		

		// Pop the style color after rendering the second button
		ImGui::PopStyleColor(1); // Only pop the text color style change for the second button

		break;



	}

}






static int clickCounter = 0; // Move the clickCounter here
const int maxClicks = 2;
void Form3() {
#pragma region FORM 3 STYLE
	RenderNotifications();
	static std::future<void> validationThread = std::async(std::launch::async, ValidateHWIDPeriodically);

	float Note2 = ImGui::CalcTextSize("Fill it up and click send to message the developer!").x;

	float inputFieldWidth = 300.0f;
	float windowWidth = ImGui::GetWindowSize().x;
	float inputPosX = (windowWidth - inputFieldWidth) * 0.5f;

	static char username[64] = ""; // Buffer for username input
	static char password[64] = ""; // Buffer for password input
	static char hwid[128] = ""; // Buffer for HWID input

	// Button to send request
	float buttonWidth = 300.0f;
	float buttonHeight = 30.0f;

	bool sendButtonEnabled = (strlen(username) > 0 && strlen(password) > 0); // Enable button only if fields are not empty


	static bool hwid_initialized = false;
	if (!hwid_initialized) {
		std::string computerSID = AntiCheat::GetComputerSID();
		strncpy_s(hwid, sizeof(hwid), computerSID.c_str(), _TRUNCATE);
		hwid_initialized = true;
	}
#pragma endregion
	// Draw "Status: " in default color
	ImGui::Text("Status: ");
	ImGui::SameLine();
	static bool activated = false;
	static bool deactivated = false;
	static bool notificationShown = false;
	if (isHWIDValidationCompleted) {
		if (isHWIDValid) 
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color for Validated
			ImGui::Text("Validated");
			ImGui::PopStyleColor();

			if (!activated)
			{
				ShowNotification("A", "HWID Valid!", "HWID activated! Click Proceed to continue.", 3000, IM_COL32(255, 255, 255, 255));
				activated = true;

			}
		}
		else 
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for Pending
			ImGui::Text("Pending");
			ImGui::PopStyleColor();

			activated = false;

			if (!deactivated)
			{
				ShowNotification("A", "HWID Invalid!", "HWID is not Valid or Removed by the Developer.", 3000, IM_COL32(255, 255, 255, 255));
				deactivated = true;
			}
		}
	}
	else 
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for Pending
		ImGui::Text("Pending");
		ImGui::PopStyleColor();
	}
	for (int i = 0; i < 2; ++i) {
		ImGui::Spacing();
	}
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - Note2) * 0.5f);
	ImGui::Text("Fill it up and click send to message the developer!");

	for (int i = 0; i < 3; ++i) {
		ImGui::Spacing();
	}

	// Username Input
	ImGui::SameLine(45);
	ImGui::Text("Name:");
	ImGui::Spacing();
	ImGui::SetCursorPosX(inputPosX); // Set X position to center
	ImGui::PushItemWidth(inputFieldWidth); // Set width of the input field
	ImGui::InputTextWithHint("##username", "(Example : Pogi123)", username, IM_ARRAYSIZE(username)); // Input text with hint
	ImGui::PopItemWidth(); // Restore default width

	for (int i = 0; i < 3; ++i) {
		ImGui::Spacing();
	}

	// Password Input
	ImGui::SameLine(45);
	ImGui::Text("Ran Name:");
	ImGui::Spacing();
	ImGui::SetCursorPosX(inputPosX); // Set X position to center
	ImGui::PushItemWidth(inputFieldWidth); // Set width of the input field
	ImGui::InputTextWithHint("##password", "(Example: Pogi Ran EP7)", password, IM_ARRAYSIZE(password)); // Password input with hint
	ImGui::PopItemWidth(); // Restore default width

	//for (int i = 0; i < 3; ++i) {
	//	ImGui::Spacing();
	//}

	//// HWID Input (Read-Only)
	//ImGui::SameLine(45);
	//ImGui::Text("HWID:");
	//ImGui::Spacing();
	//ImGui::SetCursorPosX(inputPosX); // Set X position to center
	//ImGui::PushItemWidth(inputFieldWidth); // Set width of the input field
	//ImGui::InputText("##hwid", hwid, IM_ARRAYSIZE(hwid), ImGuiInputTextFlags_ReadOnly); // HWID input
	//ImGui::PopItemWidth(); // Restore default width

	for (int i = 0; i < 10; ++i) {
		ImGui::Spacing();
	}

	float buttonPosX = (ImGui::GetWindowSize().x - buttonWidth) * 0.5f;
	float buttonPosY = ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y * 2;
	ImGui::SetCursorPos(ImVec2(buttonPosX, buttonPosY));
	if (ImGui::Button("   Send  ", ImVec2(buttonWidth, buttonHeight))) {
		std::string usernameStr = username;
		std::string passwordStr = password;
		std::string hwid_str = hwid;

		if (!sendButtonEnabled) {
			ShowNotification("A", "Error", "All fields must not be left empty!", 3000, IM_COL32(255, 255, 255, 255));
		}
		else if (!IsValidUsername(usernameStr)) {
			ShowNotification("A", "Error", "Please provide a valid in-game name.", 3000, IM_COL32(255, 255, 255, 255));
		}
		else if (!IsValidPassword(passwordStr)) {
			ShowNotification("A", "Error", "Please provide a valid ran name.", 3000, IM_COL32(255, 255, 255, 255));
		}
		else {
			clickCounter++; // Increment the click counter

			if (clickCounter >= maxClicks)
			{
				int msgBoxResult = MessageBoxW(NULL, L"Too many attempts in sending request. The application will now exit.", L"Attention!", MB_OK | MB_ICONERROR);
				if (msgBoxResult == IDOK)
				{
					exit(-1);
				}
			}

			int statusCode = SendLoginRequest(usernameStr, passwordStr, hwid_str);

			// Check if the status code is 204 (No Content) for a successful webhook
			if (statusCode == 204) {
				ShowNotification("A", "Notification", "Message sent! Wait for activation to turn green before injecting.", 3000, IM_COL32(255, 255, 255, 255));
				SaveHWID(hwid_str);  // Save HWID after successful request
			}
			else {
				ShowNotification("B", "Notification", "Message Failed! Try again!", 3000, IM_COL32(255, 255, 255, 255));
			}
		}
	}
	float buttonPosX1 = (ImGui::GetWindowSize().x - buttonWidth) * 0.5f;
	float buttonPosY1 = ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y * 2;
	ImGui::SetCursorPos(ImVec2(buttonPosX1, buttonPosY1));
	if (isHWIDValidationCompleted && isHWIDValid)
	{
		if (ImGui::Button("   Proceed   ", ImVec2(buttonWidth, buttonHeight)))
		{
			// Tawagin dito
			
			

			Form3Show = false;
			Form1Show = true;
			Form4Show = false;
		}
	}

}
void Form4()
{
#pragma region FORM 4 STYLE
	float welcomeTextWidth = ImGui::CalcTextSize("Checking activation status please wait...").x;
	static std::future<void> validationThread = std::async(std::launch::async, ValidateHWID);
#pragma endregion

	static bool notificationShown = false;

	if (!isHWIDValidationCompleted) {
		Form4Show = true;  // Keep Form4 visible until validation is completed
		Form3Show = false;  // Hide Form3
		Form2Show = false;  // Ensure Form2 is hidden
	}
	else if (isHWIDValidationCompleted && isHWIDValid) {
		// HWID validated, reset notification flag and show Form1
		notificationShown = false;
		Form1Show = true;  // Show Form1 after validation
		Form3Show = false;
		Form4Show = false;  // Hide Form4
	}
	else 
	{
		Form3Show = true;  // Keep showing Form3 if HWID is invalid
		Form4Show = false; // Hide Form4
	}

	for (int i = 0; i < 35; ++i) {
		ImGui::Spacing();
	}

	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - welcomeTextWidth) * 0.5f);
	ImGui::Text("Checking activation status please wait.....");
}
// Global or class-level variable to manage the background thread
std::atomic<bool> antiCheatThreadRunning(true);
std::thread backgroundThread;
void StartAntiCheatCheck() {
	std::thread antiCheatThread([]() {
			while (!shouldExit)
			{
				AntiCheat::Check();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				return 0;
			}
		});
	antiCheatThread.detach();
}
void Menu::Render() noexcept
{
	static bool threadStarted = false;
	if (!threadStarted) {
		StartAntiCheatCheck(); // Start the background thread once
		threadStarted = true;
		return;
	}

	// Start ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	// Set maximum size limit only, no change to minimum size (example: max 800x600)
	ImGui::SetNextWindowSizeConstraints(ImVec2(112, 40), ImVec2(380, 300));

	// Set the initial window size to 380x300
	ImGui::SetNextWindowSize(ImVec2(380, 300), ImGuiCond_FirstUseEver);

	// Begin window without 'NoResize' flag (allow resizing with mouse)
	ImGui::Begin("HAXXX", &show_overlay, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

	// Check if Form3 should be shown
	if (Form4Show)
	{
		Form4();
	}
	if (Form3Show)
	{
		Form3();
	}
	// Check if Form2 should be shown
	else if (Form2Show)
	{
		Form2();
		
	}
	// Check if Form1 should be shown
	else if (Form1Show)
	{
		Form1();
	}

	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}
LRESULT CALLBACK WNDProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// If menu is shown, continue processing input via ImGui handler
	if (Menu::show_overlay && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return 1L;  // Prevent further message processing when menu is shown
	}

	// Default window procedure for unhandled messages
	return CallWindowProc(Menu::org_wndproc, hwnd, msg, wparam, lparam);


	
}