#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<string>
#include<SDL.h>
#include"cJSON.h"
#include"imgui.h"
struct note_count
{
    std::string song_name;
    long long RCs = 0;
    long long LNs = 0;
};
enum class Screen {
    Main,
    SubA,
    SubB
};

// 添加一个结构体来存储计算所需的数据
struct AccCalcData {
    std::vector<double> input_accs;  // 存储用户输入的ACC
    bool ready_to_calc = false;      // 是否已经准备好计算
};
extern AccCalcData acc_calc_data;
const int song_count = 4;

namespace MyApp {

    double score_v1(long long s_max, long long s_300, long long s_200, long long s_100, long long s_50, long long s_miss);
    double score_v2(long long s_max, long long s_300, long long s_200, long long s_100, long long s_50, long long s_miss);

}

namespace OsuMania {
    // 从ImGui demo复制的日志类
    struct ExampleAppLog
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool                AutoScroll;  // Keep scrolling if already at the bottom.

        ExampleAppLog()
        {
            AutoScroll = true;
            Clear();
        }

        void Clear()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n')
                    LineOffsets.push_back(old_size + 1);
        }

        void Draw(const char* title, bool* p_open = NULL)
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }

            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (clear)
                    Clear();
                if (copy)
                    ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                const char* buf = Buf.begin();
                const char* buf_end = Buf.end();
                if (Filter.IsActive())
                {
                    for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        if (Filter.PassFilter(line_start, line_end))
                            ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                else
                {
                    ImGuiListClipper clipper;
                    clipper.Begin(LineOffsets.Size);
                    while (clipper.Step())
                    {
                        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                        {
                            const char* line_start = buf + LineOffsets[line_no];
                            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                            ImGui::TextUnformatted(line_start, line_end);
                        }
                    }
                    clipper.End();
                }
                ImGui::PopStyleVar();

                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
            ImGui::End();
        }
    };

    extern bool show_demo_window;
    extern Screen currentScreen;
    void RenderUI();
    bool load_dan_pack(const std::string& path);

    // 添加段位选择相关的结构
    struct DanSelection {
        std::string current_dan_pack;
        std::string current_dan;
        std::vector<std::string> dan_packs;
        std::vector<std::string> dans;
        std::string json_content;  // 添加一个字段保存JSON内容
    };
    
    extern DanSelection dan_selection;
    void UpdateDanList(const std::string& dan_pack);  // 更新段位列表

    // 声明为extern
    extern ExampleAppLog log_debug;      // Debug Log for dan course calculations
    extern ExampleAppLog log_show;       // Song Information log
    extern ExampleAppLog log_convert;    // Convert Result log
}

