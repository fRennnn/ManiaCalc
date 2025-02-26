#include "osuAccCounter.h"
#include"imgui.h"

namespace OsuMania {
    Screen currentScreen = Screen::Main;
    bool show_demo_window = false;  // 在cpp文件中定义变量
    ExampleAppLog log_debug;  // 定义日志对象
    ExampleAppLog log_show;
    ExampleAppLog log_convert;
    DanSelection dan_selection;

    static AccCalcData acc_calc_data;

    // 添加一个枚举来表示计分模式
    enum class ScoreMode {
        V1,
        V2
    };
    static ScoreMode current_score_mode = ScoreMode::V1;  // 默认使用score_v1

    // 添加一个结构体来存储分数转换的数据
    struct ScoreConvertData {
        long long s_max = 0;
        long long s_300 = 0;
        long long s_200 = 0;
        long long s_100 = 0;
        long long s_50 = 0;
        long long s_miss = 0;
    };
    static ScoreConvertData score_convert_data;

    // 添加一个递归遍历JSON的辅助函数
    void traverse_json(cJSON* item, int depth = 0) {
        if (!item) return;

        // 缩进
        std::string indent(depth * 2, ' ');

        switch (item->type) {
            case cJSON_Object: {
                log_debug.AddLog("%s{\n", indent.c_str());
                cJSON* child = item->child;
                while (child) {
                    log_debug.AddLog("%s\"%s\": ", indent.c_str(), child->string);
                    traverse_json(child, depth + 1);
                    child = child->next;
                    if (child) log_debug.AddLog(",\n");
                }
                log_debug.AddLog("\n%s}", indent.c_str());
                break;
            }
            case cJSON_Array: {
                log_debug.AddLog("[\n");
                cJSON* child = item->child;
                while (child) {
                    traverse_json(child, depth + 1);
                    child = child->next;
                    if (child) log_debug.AddLog(",\n");
                }
                log_debug.AddLog("\n%s]", indent.c_str());
                break;
            }
            case cJSON_String:
                log_debug.AddLog("\"%s\"", item->valuestring);
                break;
            case cJSON_Number:
                if (item->valuedouble == (double)item->valueint)
                    log_debug.AddLog("%d", item->valueint);
                else
                    log_debug.AddLog("%f", item->valuedouble);
                break;
            case cJSON_True:
                log_debug.AddLog("true");
                break;
            case cJSON_False:
                log_debug.AddLog("false");
                break;
            case cJSON_NULL:
                log_debug.AddLog("null");
                break;
        }
    }

    void UpdateDanList(const std::string& dan_pack) {
        dan_selection.dans.clear();
        
        // 直接使用保存的JSON内容
        cJSON* root = cJSON_Parse(dan_selection.json_content.c_str());
        if (root) {
            cJSON* pack = cJSON_GetObjectItem(root, dan_pack.c_str());
            if (pack) {
                cJSON* child = pack->child;
                while (child) {
                    dan_selection.dans.push_back(child->string);
                    child = child->next;
                }
            }
            cJSON_Delete(root);
        }
    }

    bool load_dan_pack(const std::string& path) {
        std::ifstream file(path);
        if (!file.good()) {
            log_debug.AddLog("Failed to open file: %s\n", path.c_str());
            return false;
        }

        std::stringstream str_stream;
        str_stream << file.rdbuf();
        file.close();

        // 保存JSON内容
        dan_selection.json_content = str_stream.str();

        cJSON* root = cJSON_Parse(dan_selection.json_content.c_str());
        if (!root) {
            const char* error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                log_debug.AddLog("JSON parse error: %s\n", error_ptr);
            }
            return false;
        }

        // 获取所有段位包名称
        dan_selection.dan_packs.clear();
        cJSON* child = root->child;
        while (child) {
            dan_selection.dan_packs.push_back(child->string);
            child = child->next;
        }

        log_debug.AddLog("\n=== JSON Content Tree ===\n");
        traverse_json(root);
        log_debug.AddLog("\n=== End of JSON Tree ===\n\n");

        cJSON_Delete(root);
        return true;
    }

   

    // 计算单曲ACC的函数
    void calculate_song_accs(const std::vector<double>& total_accs) {
        if (!dan_selection.current_dan_pack.empty() && !dan_selection.current_dan.empty()) {
            std::vector<double> song_accs;
            double accumulated_weight = 0;
            double accumulated_value = 0;

            cJSON* root = cJSON_Parse(dan_selection.json_content.c_str());
            if (root) {
                cJSON* pack = cJSON_GetObjectItem(root, dan_selection.current_dan_pack.c_str());
                if (pack) {
                    cJSON* dan = cJSON_GetObjectItem(pack, dan_selection.current_dan.c_str());
                    if (dan) {
                        int song_index = 0;
                        cJSON* song = nullptr;
                        cJSON_ArrayForEach(song, dan) {
                            if (song_index >= total_accs.size()) break;

                            cJSON* rcs = cJSON_GetObjectItem(song, "RCs");
                            cJSON* lns = cJSON_GetObjectItem(song, "LNs");
                            
                            if (rcs && lns) {
                                // 根据当前模式计算总物量
                                const long long current_notes = rcs->valueint + 
                                    (current_score_mode == ScoreMode::V2 ? 2 * lns->valueint : lns->valueint);
                                const double current_acc = 
                                    (total_accs[song_index] * (accumulated_weight + current_notes) - accumulated_value)
                                    / current_notes;

                                song_accs.push_back(current_acc);
                                accumulated_weight += current_notes;
                                accumulated_value += current_acc * current_notes;

                                double game_acc = round(current_acc * 100) / 100.0;
                                log_debug.AddLog("Song %d ACC: %.4f%% => %.2f%% (Total Notes: %lld)\n", 
                                    song_index + 1, current_acc, game_acc, current_notes);
                            }
                            song_index++;
                        }
                    }
                }
                cJSON_Delete(root);
            }
        } else {
            log_debug.AddLog("Please select a dan course first!\n");
        }
    }
}

double MyApp::score_v1(long long s_max, long long s_300, long long s_200, long long s_100, long long s_50, long long s_miss) {
    long long total_count = s_max + s_300 + s_200 + s_100 + s_50 + s_miss;
    long long total_score = 300LL * (s_max + s_300) + 200LL * s_200 + 100LL * s_100 + 50LL * s_50;
    return (static_cast<double>(total_score) * 100.0) / (300.0 * total_count);
}

double MyApp::score_v2(long long s_max, long long s_300, long long s_200, long long s_100, long long s_50, long long s_miss) {
    long long total_count = s_max + s_300 + s_200 + s_100 + s_50 + s_miss;
    long long total_score = 305LL * s_max + 300LL * s_300 + 200LL * s_200 + 100LL * s_100 + 50LL * s_50;
    return (static_cast<double>(total_score) * 100.0) / (305.0 * total_count);
}

void OsuMania::RenderUI() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 12.0f;
    ImGui::StyleColorsLight();
    
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // 设置窗口标志
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 设置窗口样式
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Test DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiID dockspace_id = ImGui::GetID("TestDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    // 移除菜单栏中的控制按钮
    if (ImGui::BeginMenuBar())
    {
        ImGui::EndMenuBar();
    }

    // 渲染不同的界面
    if (currentScreen == Screen::Main)
    {
        ImGui::Begin("Main Screen");
        
        // 添加垂直空间到窗口中间
        ImGui::Dummy(ImVec2(0.0f, ImGui::GetWindowHeight() * 0.37f));
        
        // 设置更大的按钮尺寸
        float button_width = 300.0f;   // 增加按钮宽度
        float button_height = 40.0f;   // 设置按钮高度
        
        // 居中第一个按钮
        ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_width) * 0.5f, 0.0f));
        ImGui::SameLine();
        if (ImGui::Button("Score_v1 <--> Score_v2 Convert", ImVec2(button_width, button_height)))
        {
            currentScreen = Screen::SubA;
        }
        
        // 添加一些垂直间距
        ImGui::Dummy(ImVec2(0.0f, 30.0f));  // 增加按钮之间的间距
        
        // 居中第二个按钮
        ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - button_width) * 0.5f, 0.0f));
        ImGui::SameLine();
        if (ImGui::Button("Acc Calc in Dan Course", ImVec2(button_width, button_height)))
        {
            currentScreen = Screen::SubB;
        }
        
        ImGui::End();
    }
    else if (currentScreen == Screen::SubA)
    {
        ImGui::Begin("Score Converter");
        
        

        ImGui::Separator();

        // 输入框
        ImGui::Text("Enter judgment counts:");
        ImGui::InputScalar("MAX", ImGuiDataType_S64, &score_convert_data.s_max);
        ImGui::InputScalar("300", ImGuiDataType_S64, &score_convert_data.s_300);
        ImGui::InputScalar("200", ImGuiDataType_S64, &score_convert_data.s_200);
        ImGui::InputScalar("100", ImGuiDataType_S64, &score_convert_data.s_100);
        ImGui::InputScalar("50", ImGuiDataType_S64, &score_convert_data.s_50);
        ImGui::InputScalar("Miss", ImGuiDataType_S64, &score_convert_data.s_miss);

        ImGui::Separator();
        // 使用RadioButton选择当前分数类型
        static ScoreMode input_score_mode = ScoreMode::V1;
        
        bool is_v1 = (input_score_mode == ScoreMode::V1);
        bool is_v2 = (input_score_mode == ScoreMode::V2);

        
        if (ImGui::Button("Convert")) {
            // 计算总物量
            long long total_notes = score_convert_data.s_max + score_convert_data.s_300 + 
                score_convert_data.s_200 + score_convert_data.s_100 + 
                score_convert_data.s_50 + score_convert_data.s_miss;

            if (total_notes > 0) {
                double acc;
                if (input_score_mode == ScoreMode::V1) {
                    // 计算score_v1的ACC
                    acc = MyApp::score_v1(
                        score_convert_data.s_max,
                        score_convert_data.s_300,
                        score_convert_data.s_200,
                        score_convert_data.s_100,
                        score_convert_data.s_50,
                        score_convert_data.s_miss
                    );
                    
                    // 反推score_v2的判定数
                    double target_acc = MyApp::score_v2(
                        score_convert_data.s_max,
                        score_convert_data.s_300,
                        score_convert_data.s_200,
                        score_convert_data.s_100,
                        score_convert_data.s_50,
                        score_convert_data.s_miss
                    );

                    log_convert.AddLog("\n=== Score Conversion Result ===\n");
                    log_convert.AddLog("Score v1 ACC: %.4f%%\n", acc);
                    log_convert.AddLog("Equal to Score v2 ACC: %.4f%%\n", target_acc);
                    log_convert.AddLog("=== End of Conversion ===\n\n");
                }
                else {
                    // 计算score_v2的ACC
                    acc = MyApp::score_v2(
                        score_convert_data.s_max,
                        score_convert_data.s_300,
                        score_convert_data.s_200,
                        score_convert_data.s_100,
                        score_convert_data.s_50,
                        score_convert_data.s_miss
                    );
                    
                    // 反推score_v1的判定数
                    double target_acc = MyApp::score_v1(
                        score_convert_data.s_max,
                        score_convert_data.s_300,
                        score_convert_data.s_200,
                        score_convert_data.s_100,
                        score_convert_data.s_50,
                        score_convert_data.s_miss
                    );

                    log_convert.AddLog("\n=== Score Conversion Result ===\n");
                    log_convert.AddLog("Score v2 ACC: %.4f%%\n", acc);
                    log_convert.AddLog("Equal to Score v1 ACC: %.4f%%\n", target_acc);
                    log_convert.AddLog("=== End of Conversion ===\n\n");
                }
            }
        }
        ImGui::SameLine();
        ImGui::Text("Current Score Type:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Score v1", is_v1)) input_score_mode = ScoreMode::V1;
        ImGui::SameLine();
        if (ImGui::RadioButton("Score v2", is_v2)) input_score_mode = ScoreMode::V2;
        if (ImGui::Button("Back to Main Screen")) {
            currentScreen = Screen::Main;
        }

        ImGui::End();

        // 修改窗口标题
        ImGui::Begin("Convert Result");
        log_convert.Draw("Convert Result");
        ImGui::End();
    }
    else if (currentScreen == Screen::SubB)
    {
        ImGui::Begin("Osu!Mania");
        ImGui::Text("Enter the acc when you are in the break part of Dan");

        if (ImGui::Button("Load Dan Pack"))
        {
            if (!load_dan_pack("Dan_Course.json"))
            {
                log_debug.AddLog("Failed to load dan pack\n");
            }
        }

        if (ImGui::Button("Back to Main Screen"))
        {
            currentScreen = Screen::Main;
        }

        // 添加ACC输入界面
        if (!dan_selection.current_dan_pack.empty() && !dan_selection.current_dan.empty()) {
            ImGui::Separator();
            ImGui::Text("Enter ACC after each song (0-100%%):");
            
            // 确保vector大小为4
            while (acc_calc_data.input_accs.size() < 4) {
                acc_calc_data.input_accs.push_back(0.0);
            }

            // 输入框
            for (int i = 0; i < 4; i++) {
                char label[32];
                sprintf(label, "ACC %d", i + 1);
                ImGui::InputDouble(label, &acc_calc_data.input_accs[i], 0.0, 0.0, "%.4f");
            }

            ImGui::Separator();

            if (ImGui::Button("Calculate Song ACCs")) {
                log_debug.AddLog("\n=== Calculating Song ACCs (%s) ===\n",
                    current_score_mode == ScoreMode::V1 ? "Score v1" : "Score v2");
                calculate_song_accs(acc_calc_data.input_accs);
                log_debug.AddLog("=== Calculation Complete ===\n\n");
            }

            ImGui::SameLine();
            bool is_v1 = (current_score_mode == ScoreMode::V1);
            bool is_v2 = (current_score_mode == ScoreMode::V2);
            
            if (ImGui::RadioButton("Score v1", is_v1)) current_score_mode = ScoreMode::V1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Score v2", is_v2)) current_score_mode = ScoreMode::V2;


        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                "Please select a dan course first!");
        }

        ImGui::End();
        
        ImGui::Begin("Dan Course Pack(Support)");
        
        // 段位包选择
        if (ImGui::BeginCombo("Dan Pack", dan_selection.current_dan_pack.c_str())) {
            for (const auto& pack : dan_selection.dan_packs) {
                bool is_selected = (pack == dan_selection.current_dan_pack);
                if (ImGui::Selectable(pack.c_str(), is_selected)) {
                    dan_selection.current_dan_pack = pack;
                    UpdateDanList(pack);  // 更新段位列表
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // 具体段位选择
        if (ImGui::BeginCombo("Dan", dan_selection.current_dan.c_str())) {
            for (const auto& dan : dan_selection.dans) {
                bool is_selected = (dan == dan_selection.current_dan);
                if (ImGui::Selectable(dan.c_str(), is_selected)) {
                    dan_selection.current_dan = dan;
                    
                    // 当选择新的段位时，显示曲目信息
                    log_show.Clear();  // 清除之前的信息
                    
                    cJSON* root = cJSON_Parse(dan_selection.json_content.c_str());
                    if (root) {
                        cJSON* pack = cJSON_GetObjectItem(root, dan_selection.current_dan_pack.c_str());
                        if (pack) {
                            cJSON* dan = cJSON_GetObjectItem(pack, dan_selection.current_dan.c_str());
                            if (dan) {
                                log_show.AddLog("\n=== Songs in %s - %s ===\n\n", 
                                    dan_selection.current_dan_pack.c_str(), 
                                    dan_selection.current_dan.c_str());
                                
                                int song_index = 1;
                                cJSON* song = nullptr;
                                cJSON_ArrayForEach(song, dan) {
                                    cJSON* song_name = cJSON_GetObjectItem(song, "song_name");
                                    cJSON* rcs = cJSON_GetObjectItem(song, "RCs");
                                    cJSON* lns = cJSON_GetObjectItem(song, "LNs");
                                    
                                    if (song_name && rcs && lns) {
                                        log_show.AddLog("Song %d: %s\n", song_index++, song_name->valuestring);
                                        log_show.AddLog("  RCs: %d\n", rcs->valueint);
                                        log_show.AddLog("  LNs: %d\n", lns->valueint);
                                        log_show.AddLog("  Total Notes: %d\n\n", 
                                            rcs->valueint + lns->valueint);
                                    }
                                }
                            }
                        }
                    }
                    cJSON_Delete(root);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();

        // 显示曲目信息的log窗口
        ImGui::Begin("Song Information");
        log_show.Draw("Song Information");
        ImGui::End();

        // Debug log窗口
        ImGui::Begin("Debug Log");
        log_debug.Draw("Debug Log");
        ImGui::End();
    }

    ImGui::End(); // Test DockSpace
}
