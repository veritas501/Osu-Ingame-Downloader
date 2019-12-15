#include "ingame_overlay.h"
#include "game_hook.h"
#include "UbuntuMono_R_ttf.h"
#include "downloader.h"
#include "map_db.h"

#define VERSION "Version: Beta 0.6"

OV* OV::inst() {
	static OV _ov;
	return &_ov;
}

void OV::InitOverlay(HDC hdc) {
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,     // Color bits ignored
		0,                    // No alpha buffer
		0,                    // Shift bit ignored
		0,                    // No accumulation buff
		0, 0, 0, 0,           // Accum bits ignored
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,       // Main layer
		0,                    // Reserved
		0, 0, 0               // Layer masks ignored
	};
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);
	HGLRC glContext = wglCreateContext(hdc);
	gl3wInit();

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryTTF(UbuntuMono_R_ttf, UbuntuMono_R_ttf_len, 18.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	auto* style = &ImGui::GetStyle();
	style->WindowRounding = 5.3f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;
	style->Colors[ImGuiCol_TextDisabled] = { 0.34f, 0.34f, 0.34f, 1.00f };
	style->Colors[ImGuiCol_WindowBg] = { 0.23f, 0.24f, 0.25f, 0.94f };
	style->Colors[ImGuiCol_ChildBg] = { 0.23f, 0.24f, 0.25f, 0.00f };
	style->Colors[ImGuiCol_PopupBg] = { 0.23f, 0.24f, 0.25f, 0.94f };
	style->Colors[ImGuiCol_Border] = { 0.33f, 0.33f, 0.33f, 0.50f };
	style->Colors[ImGuiCol_BorderShadow] = { 0.15f, 0.15f, 0.15f, 0.00f };
	style->Colors[ImGuiCol_FrameBg] = { 0.16f, 0.16f, 0.16f, 0.54f };
	style->Colors[ImGuiCol_FrameBgHovered] = { 0.45f, 0.67f, 0.99f, 0.67f };
	style->Colors[ImGuiCol_FrameBgActive] = { 0.47f, 0.47f, 0.47f, 0.67f };
	style->Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
	style->Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
	style->Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.80f };
	style->Colors[ImGuiCol_MenuBarBg] = { 0.27f, 0.28f, 0.29f, 0.80f };
	style->Colors[ImGuiCol_ScrollbarBg] = { 0.27f, 0.28f, 0.29f, 0.60f };
	style->Colors[ImGuiCol_ScrollbarGrab] = { 0.21f, 0.30f, 0.41f, 0.51f };
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21f, 0.30f, 0.41f, 1.00f };
	style->Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13f, 0.19f, 0.26f, 0.91f };
	style->Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
	style->Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
	style->Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
	style->Colors[ImGuiCol_Button] = { 0.33f, 0.35f, 0.36f, 0.49f };
	style->Colors[ImGuiCol_ButtonHovered] = { 0.21f, 0.30f, 0.41f, 1.00f };
	style->Colors[ImGuiCol_ButtonActive] = { 0.13f, 0.19f, 0.26f, 1.00f };
	style->Colors[ImGuiCol_Header] = { 0.33f, 0.35f, 0.36f, 0.53f };
	style->Colors[ImGuiCol_HeaderHovered] = { 0.45f, 0.67f, 0.99f, 0.67f };
	style->Colors[ImGuiCol_HeaderActive] = { 0.47f, 0.47f, 0.47f, 0.67f };
	style->Colors[ImGuiCol_Separator] = { 0.57f, 0.54f, 0.54f, 1.00f };
	style->Colors[ImGuiCol_SeparatorHovered] = { 0.31f, 0.31f, 0.31f, 1.00f };
	style->Colors[ImGuiCol_SeparatorActive] = { 0.31f, 0.31f, 0.31f, 1.00f };
	style->Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
	style->Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
	style->Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
	style->Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
	style->Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
	style->Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
	style->Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
	style->Colors[ImGuiCol_TextSelectedBg] = { 0.18f, 0.39f, 0.79f, 0.90f };
	ImGui_ImplWin32_Init(WindowFromDC(hdc));
	ImGui_ImplOpenGL3_Init();
}

bool OV::isShowingSettings() {
	return showSetting;
}

void OV::ReverseShowSettings() {
	showSetting = !showSetting;
	if (showSetting) {
		HK::inst()->DisablRawInputDevices();
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = true;
		statusPinned = 0;
	}
	else {
		HK::inst()->RestoreRawInputDevices();
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = false;
		statusPinned = 1;
	}
}

bool OV::isShowingStatus() {
	return showStatus;
}

void OV::ShowStatus() {
	showStatus = true;
}

void OV::HideStatus() {
	showStatus = false;
}

void OV::RenderOverlay(HDC hdc) {
	if (!showStatus && !showSetting) {
		return;
	}
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//===================== MY UI START =====================

	if (showStatus || showSetting) {
		ImGuiWindowFlags statusWindowFlag = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		if (!statusPinned) {
			statusWindowFlag &= ~ImGuiWindowFlags_NoMove;
		}
		ImGui::Begin(STATUS_WINDOW_NAME, nullptr, ImVec2(0, 0), 0.8f, statusWindowFlag);
		DL::inst()->SetTaskReadLock();
		auto tasks = &DL::inst()->tasks;
		auto keyIter = tasks->begin();
		if (keyIter == tasks->end()) {
			ImGui::Text("  Status: Idle");
		}
		else {
			while (keyIter != tasks->end()) {
				switch (keyIter->second.dlStatus) {
				case PARSE:
					ImGui::Text("  Status: Parsing %c", "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
					ImGui::Text("    Link: %s", keyIter->second.songName);
					break;
				case DOWNLOAD:
					ImGui::Text("  Status: Downloading %c", "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
					ImGui::Text("MapsetID: %llu", keyIter->second.sid);
					ImGui::Text("SongName: %s", keyIter->second.songName.c_str());
					switch (keyIter->second.category) {
					case GRAVEYARD:
						ImGui::Text("Category: Graveyard");
						break;
					case WIP:
						ImGui::Text("Category: WIP");
						break;
					case PENDING:
						ImGui::Text("Category: Pending");
						break;
					case RANKED:
						ImGui::Text("Category: Ranked");
						break;
					case APPROVED:
						ImGui::Text("Category: Approved");
						break;
					case QUALIFIED:
						ImGui::Text("Category: Qualified");
						break;
					case LOVED:
						ImGui::Text("Category: Loved");
						break;
					default:
						ImGui::Text("Category: Unkown");
						break;
					}
					ImGui::Text("FileSize: %.2fMB / %.2fMB", keyIter->second.fileSize / 0x100000, keyIter->second.downloaded / 0x100000);
					ImGui::ProgressBar(keyIter->second.percent, ImVec2(-1, 5));
					break;
				default:
					ImGui::Text("  Status: Idle");
					break;
				}

				keyIter++;
				if (keyIter != tasks->end()) {
					ImGui::Separator();
				}
			}
		}
		ImGui::End();
		DL::inst()->UnsetTaskLock();
	}

	if (showSetting) {
		ImGui::Begin(SETTING_WINDOW_NAME, nullptr, ImVec2(0, 0), -1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
		ImGui::Text("=============[ InGame Downloader ]=============");
		ImGui::Text(VERSION);
		ImGui::Separator();
		ImGui::Text("Helps: ");
		ImGui::Text("1. Use Alt+M to show/hide this window.");
		ImGui::Text("2. You can move status window now.");
		ImGui::Text("3. Status window will auto show when download");
		ImGui::Text("   started, and will auto hide when finished.");
		ImGui::Separator();
		ImGui::Checkbox("Stop using ingame downloader", &DL::inst()->dontUseDownloader);

		ImGui::Text("OSZ Version: ");
		ImGui::SameLine();
		ImGui::Combo("", &DL::inst()->downloadType, DlTypeName, IM_ARRAYSIZE(DlTypeName));
		ImGui::SameLine();
		HelpMarker("Help:\n1. <Full Version> is full version.\n2. <No Video> doesn't contain video.\n3. <Mini> doesn't contain video and keysound.");
		static char str0[128] = "Hello, world!";
		ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
		ImGui::End();
	}

	//===================== MY UI END =====================
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OV::HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
