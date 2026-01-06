// ============================================================================
// File: game.cpp
// 
// Purpose: Implements the game module
// 
// ============================================================================

#include "game.h"

#include "game\actor\root.h"
#include "game\actor\particle.h"
#include "engine\core\engine.h"
#include "engine\sys\actor\actor.h"
#include "engine\sys\render\render.h"
#include "particle.h"
#include "game\file\file.h"
#include "game\actor\dummy.h"
#include "ogre_maths.h"

#include "imgui\imgui.h"
#include "imgui\backends\imgui_impl_glfw.h"
#include "imgui\backends\imgui_impl_opengl3.h"

#include "game\imgui_file_dialog\ImGuiFileDialog.h"

#include "game\implot\implot.h"

#include <filesystem>

static sparticle_element_param default_element ( )
{
    sparticle_element_param e {};
    e.m_type = ( int ) e_element_type::null;
    e.m_effect_type = 1;
    e.m_format_flag = 0;
    e.m_end_time_scaled = 1.0f;

    for ( int i = 0; i < 12; i++ )
        e.m_mtx [ i ] = 0.0f;

    return e;
}

static sparticle_vector_param default_vector ( )
{
    sparticle_vector_param v {};
    v.m_base = 0.0f;
    v.m_vertical_base = 0.0f;
    v.m_multiplier = 0.0f;
    v.m_angle_base = 0;
    v.m_vertical_pan = 0.0f;
    v.m_pos_pan = 0.0f;
    v.m_pos_random = 0.0f;
    v.m_unused = 0;
    return v;
}

static sparticle_vertex_param default_vertex ( )
{
    sparticle_vertex_param v {};
    v.m_color_base = { 1,1,1,1 };
    v.m_color_range = { 0,0,0,0 };

    v.m_scale_flag = 1;
    v.m_scale_base = { 0,0,0,1 };
    v.m_scale_range = { 0,0,0,0 };

    v.m_rotation_base = { 0,0,0 };
    v.m_rotation_range = { 0,0,0 };
    v.m_angular_accel_base = { 0,0,0 };
    v.m_angular_accel_range = { 0,0,0 };

    v.m_uv_flag = 0;
    v.m_uv_base = { 0,0 };
    v.m_uv_range = { 0,0 };
    v.m_columns = 1;
    v.m_rows = 1;
    v.m_start_frame = 0;
    v.m_end_frame = 0;
    v.m_width = 0.5;
    v.m_height = 0.25;
    v.m_material_flag = 0;
    return v;
}
static sparticle_emitter_param default_emitter ( )
{
    sparticle_emitter_param e {};
    e.m_element_count = 1;
    e.m_delay_min = 0;
    e.m_delay_max = 0;
    e.m_emit_min = 30;
    e.m_emit_max = 30;
    e.m_unknown3 = 0;
    e.m_life_time = 30.0f;
    e.m_inverse_speed = 0.0f;
    e.m_unknown4 = 0;
    e.m_pool_size = 1;
    e.m_vertex_type = 1;
    e.m_render_state = 0;
    e.m_model_id = 0;
    e.m_texture_id = 0;
    e.m_element_pointer = 0;
    e.m_time_scale = -1.0f;
    e.m_frame_rate = 1.0f;

    e.m_vector_param = default_vector ( );
    e.m_vertex_param = default_vertex ( );

    e.m_element_chains.clear ( );
    e.m_element_chains.push_back ( { default_element ( ) } );

    return e;
}

static sparticle_param default_particle_param ( )
{
    sparticle_param p {};
    p.m_is_enabled = 1;
    p.m_cycle_min = 1;
    p.m_cycle_max = 1;
    p.m_emit_shape = 1;
    p.m_use_surface_normals = 0;
    p.m_transform_flag = 0;
    p.m_radius_min = 0.0f;
    p.m_radius_max = 0.0f;
    p.m_angle = 0;
    p.m_rot_y = 0;
    p.m_rot_z = 0;
    p.m_scale_y_min = 1.0f;
    p.m_scale_y_max = 1.0f;
    p.m_scale_xz_min = 1.0f;
    p.m_scale_xz_max = 1.0f;
    p.m_dir_angle_min = 0;
    p.m_dir_angle_max = 0;
    p.m_special_orient_flag = 0;

    p.m_emitter_param = default_emitter ( );
    return p;
}

static sparticle* default_particle ( const std::string& name )
{
    sparticle* p = new sparticle {};
    p->m_name = name;
    p->m_id = 0;
    p->m_unk = 0;
    p->m_particle_params.push_back ( default_particle_param ( ) );
    return p;
}

static float s_left_panel_w = 260.0f;
static float s_right_panel_w = 420.0f;
static float s_top_bar_h = 0.0f;

namespace fs = std::filesystem;

cgame* cgame::instance = nullptr;
int mot_num = 32;

cgame::cgame ( ) {
	m_particle_path = "PTCL_ALL_Y1";
	m_got_folder = false;
	m_is_y2 = false;
}

cgame::~cgame ( ) {
    ImGui_ImplOpenGL3_Shutdown ( );
    ImGui_ImplGlfw_Shutdown ( );
    ImPlot::DestroyContext ( );
    ImGui::DestroyContext ( );
}

void cgame::init ( ) {
	init_ogre_maths ( );
	cengine::get ( )->act_man->init ( e_actid::num );
	cengine::get ( )->act_man->m_root_act = new cact_game ( nullptr, e_actid::root );

    cengine::get ( )->render_man->set_scr_offset ( s_left_panel_w, s_top_bar_h );
    cengine::get ( )->render_man->set_scr_offset_r ( s_right_panel_w, 0 );
    cengine::get ( )->render_man->set_window_title ( "DiPEdit", true );
    cengine::get ( )->render_man->lock_fps ( -1.0f );

    IMGUI_CHECKVERSION ( );
    ImGui::CreateContext ( );
    ImPlot::CreateContext ( );
    ImGuiIO& io = ImGui::GetIO ( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark ( );

    ImGui_ImplGlfw_InitForOpenGL ( cengine::get ( )->render_man->get_window ( ), true );
    ImGui_ImplOpenGL3_Init ( "#version 330" );

    m_looped = false;
}

void cgame::run ( )
{
    ImGui_ImplOpenGL3_NewFrame ( );
    ImGui_ImplGlfw_NewFrame ( );
    ImGui::NewFrame ( );
    ImGuiViewport* viewport = ImGui::GetMainViewport ( );
    ImGuiWindowFlags main_window_flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::SetNextWindowPos ( viewport->WorkPos );
    ImGui::SetNextWindowSize ( ImVec2 ( s_left_panel_w , viewport->WorkSize.y ) );
    ImGui::Begin ( "Main Window", nullptr, main_window_flags );

    if ( ImGui::Button ( "Open PTCL Folder" ) ) {
        ImGuiFileDialog::Instance ( )->OpenDialog ( "ChoosePTCLFolder", "Choose a folder", nullptr );
    }

    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::particle_start );

    if ( ImGui::Button ( "Play" ) ) {
        if ( p ) {
            p->create_blank ( );
        }
    };

    ImGui::Checkbox ( "Loop", &m_looped );

    ImGui::Checkbox ( "Is Yakuza 2", &m_is_y2 );

    if ( m_got_folder ) {
        ImGui::Text ( "PTCL Files" );

        draw_ptcl_tree ( );
    }

    if ( ImGuiFileDialog::Instance ( )->Display ( "ChoosePTCLFolder" ) ) {
        if ( ImGuiFileDialog::Instance ( )->IsOk ( ) ) {
            m_particle_path = ImGuiFileDialog::Instance ( )->GetCurrentPath ( );
            p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::particle_start );
            if ( p ) {
                p->set_exec_flag ( e_act_exec::pause );
                p->set_exec_flag ( e_act_exec::done );
            }
            m_got_folder = true;
			cengine::get ( )->mesh_man->clear_mesh ( );
            cengine::get ( )->tex_man->clear_tex ( );
        }
        ImGuiFileDialog::Instance ( )->Close ( );
    }

    p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::particle_start );

    if ( m_looped && p && p->m_particles.empty ( ) ) {
        p->create_blank ( );
    }

    draw_ptcl_data ( );

    if ( p && p->m_particle_data ) {
        IGFD::FileDialogConfig cfg;
        cfg.fileName = p->m_particle_data->m_name + ".ptcl";
        cfg.filePathName = ".";
        cfg.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        if ( ImGui::Button ( "Save Particle" ) )
        {
            ImGuiFileDialog::Instance ( )->OpenDialog (
                "SavePTCL",
                "Save Particle",
                ".ptcl,.bin",
                cfg
            );
        }

        if ( ImGuiFileDialog::Instance ( )->Display ( "SavePTCL" ) )
        {
            if ( ImGuiFileDialog::Instance ( )->IsOk ( ) )
            {
                std::string path =
                    ImGuiFileDialog::Instance ( )->GetFilePathName ( );

                write_particle_file ( path.c_str ( ), p->m_particle_data );
            }

            ImGuiFileDialog::Instance ( )->Close ( );
        }
    }


    /*
    ImGui::InputInt ( "Motion", &mot_num, 1, 100 );

    if ( ImGui::IsItemDeactivatedAfterEdit ( ) ) {
        cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );
        if ( d ) {
            d->set_exec_flag ( e_act_exec::pause );
            d->set_exec_flag ( e_act_exec::done );
        }
        else {
            new cact_dummy ( cengine::get ( )->act_man->get_actor ( e_actid::root ), e_actid::dummy, mot_num );
        }
    }

    */

    ImGui::End ( );

    ImGui::Render ( );
    ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData ( ) );
}

void cgame::draw_ptcl_tree ( )
{
    if ( !fs::exists ( m_particle_path ) )
        return;

    // Root folder node
    if ( ImGui::TreeNodeEx ( m_particle_path.c_str ( ), ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        for ( auto& entry : fs::directory_iterator ( m_particle_path ) )
        {
            if ( entry.path ( ).extension ( ) == ".ptcl" )
            {
                const std::string filename = entry.path ( ).filename ( ).string ( );
                // Use TreeNodeEx so we can detect clicks
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if ( m_particle_name == entry.path ( ).string ( ) )
                    flags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx ( filename.c_str ( ), flags );

                if ( ImGui::IsItemClicked ( ) )
                {
                    m_particle_name = entry.path ( ).string ( ); // store clicked file
                    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::particle_start );
                    if ( !p )
                        new cact_particle ( cengine::get ( )->act_man->get_actor ( e_actid::particle_manager ),
                            cengine::get ( )->act_man->get_free_id ( e_actid::particle_start, e_actid::particle_end ),
                            m_particle_name );
                    else {
                        p->m_ptcl_name = m_particle_name;
                        delete p->m_particle_data;
                        p->m_particle_data = load_particle_file ( p->m_ptcl_name );
                        p->create_blank ( );
                    }
                }
            }
        }

        ImGui::TreePop ( );
    }
}


// ============================================================================
// Add this to game.cpp replacing the previous draw_ptcl_data
// ============================================================================

// Helper for resizing std::string (Same as before)
struct InputTextCallback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

static int InputTextCallback ( ImGuiInputTextCallbackData* data ) {
    InputTextCallback_UserData* user_data = ( InputTextCallback_UserData* ) data->UserData;
    if ( data->EventFlag == ImGuiInputTextFlags_CallbackResize ) {
        std::string* str = user_data->Str;
        IM_ASSERT ( data->Buf == str->c_str ( ) );
        str->resize ( data->BufTextLen );
        data->Buf = ( char* ) str->c_str ( );
    }
    else if ( user_data->ChainCallback ) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback ( data );
    }
    return 0;
}

bool InputString ( const char* label, std::string* str, ImGuiInputTextFlags flags = 0 ) {
    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = NULL;
    cb_user_data.ChainCallbackUserData = NULL;
    return ImGui::InputText ( label, ( char* ) str->c_str ( ), str->capacity ( ) + 1, flags | ImGuiInputTextFlags_CallbackResize, InputTextCallback, &cb_user_data );
}

const char* element_type_names [ ] = {
    "Position", "Scale", "Color (8bit)", "Angle", "UV", "Pattern", "Color (7bit)", "Null", "Unknown"
};

const char* emit_shape_names [ ] = {
    "Circle", "Sphere"
};

const char* element_effect_names [ ] = {
    "Velocity", "Acceleration"
};

static glm::vec4 eval_param_value (
    const sparticle_element_param& p,
    const glm::vec4& base,
    float dt
) {
    glm::vec4 v (
        p.m_mtx [ 4 ],
        p.m_mtx [ 5 ],
        p.m_mtx [ 6 ],
        p.m_mtx [ 7 ]
    );
    glm::vec4 a (
        p.m_mtx [ 8 ],
        p.m_mtx [ 9 ],
        p.m_mtx [ 10 ],
        p.m_mtx [ 11 ]
    );
    return base + v * dt + 0.5f * a * dt * dt;
}

static void ensure_final_param ( std::vector<sparticle_element_param>& params ) {
    if ( params.empty ( ) ) {
        params.push_back ( default_element ( ) );
    }

    params.back ( ).m_end_time_scaled = 1.0f;
}

// Helper to get channel names based on element type
static const char** get_channel_names ( e_element_type type ) {
    static const char* chan_s [ ] = { "W", "X", "Y", "Z" };
    static const char* chan_c [ ] = { "R", "G", "B", "A" };
    static const char* chan_r [ ] = { "X", "Y", "Z", "None" };
    static const char* chan_uv [ ] = { "U", "V", "None", "None" };

    switch ( type ) {
    case e_element_type::scale: return chan_s;
    case e_element_type::color_8bit:
    case e_element_type::color_7bit: return chan_c;
    case e_element_type::angle: return chan_r;
    case e_element_type::uv: return chan_uv;
    default: return chan_s;
    }
}

void draw_improved_curve_editor ( std::vector<sparticle_element_param>& params ) {
    if ( params.empty ( ) ) return;

    static bool show_channels [ 4 ] = { true, true, true, true };
    static int active_channel = 0;
    static int mode = 0; // 0: Value, 1: Velocity, 2: Accel
    static bool enable_snapping = false;
    static int hovered_pt = -1;
    static int selected_pt = -1;

    const char** chan_names = get_channel_names ( ( e_element_type ) params [ 0 ].m_type );
    const char* mode_names [ ] = { "Value", "Velocity", "Acceleration" };

    // --- Toolbar ---
    ImGui::BeginChild ( "EditorToolbar", ImVec2 ( 0, 35 ), true );
    //ImGui::Checkbox ( "Snap", &enable_snapping ); ImGui::SameLine ( );
    ImGui::SetNextItemWidth ( 120 );
    ImGui::Combo ( "Mode", &mode, mode_names, 3 ); ImGui::SameLine ( );

    for ( int i = 0; i < 4; ++i ) {
        if ( strcmp ( chan_names [ i ], "None" ) == 0 ) continue;
        ImGui::Checkbox ( chan_names [ i ], &show_channels [ i ] ); ImGui::SameLine ( );
    }

    if ( ImGui::Button ( "Fit to View" ) ) {
        ImPlot::SetNextAxisToFit ( ImAxis_Y1 );
        ImPlot::SetNextAxisToFit ( ImAxis_X1 );
    }
    ImGui::EndChild ( );

    // --- Plot Area ---
    if ( ImPlot::BeginPlot ( "##CurvePlot", ImVec2 ( -1, 350 ), ImPlotFlags_NoMenus ) ) {
        ImPlot::SetupAxes ( "Time", "Value", ImPlotAxisFlags_None, ImPlotAxisFlags_None );
        ImPlot::SetupAxesLimits ( 0, 1, -5, 5, ImGuiCond_Once );
        ImPlot::SetupAxisLimits ( ImAxis_X1, 0.0, 1.0, ImPlotCond_Always );

        hovered_pt = -1;
        ImVec4 colors [ 4 ] = { {1,0,0,1}, {0,1,0,1}, {0,0,1,1}, {1,1,1,1} };

        for ( int ch = 0; ch < 4; ++ch ) {
            if ( !show_channels [ ch ] || strcmp ( chan_names [ ch ], "None" ) == 0 ) continue;

            std::vector<double> xs, ys;
            for ( auto& p : params ) {
                xs.push_back ( p.m_end_time_scaled );
                if ( mode == 1 ) ys.push_back ( p.m_mtx [ 4 + ch ] );
                else if ( mode == 2 ) ys.push_back ( p.m_mtx [ 8 + ch ] );
                else ys.push_back ( p.m_mtx [ ch ] ); // Simplified value mode
            }

            double startx [ 2 ] = { 0.0, xs [ 0 ] };
            double starty [ 2 ] = { 0.0, ys [ 0 ] };
            ImPlot::SetNextLineStyle ( colors [ ch ], 2.0f );
            ImPlot::PlotLine ( "##STARTSEG", startx, starty, 2 );

            // Draw line
            ImPlot::SetNextLineStyle ( colors [ ch ], 2.0f );
            ImPlot::PlotLine ( chan_names [ ch ], xs.data ( ), ys.data ( ), ( int ) xs.size ( ) );

            // Handle Points for Active Channel
            if ( ch == active_channel ) {
                for ( int i = 0; i < params.size ( ); ++i ) {
                    double px = xs [ i ];
                    double py = ys [ i ];

                    // Visual feedback for hover
                    float pt_size = ( hovered_pt == i ) ? 8.0f : 4.0f;
                    ImVec4 pt_col = ( hovered_pt == i ) ? ImVec4 ( 1, 1, 0, 1 ) : colors [ ch ];

                    if ( ImPlot::DragPoint ( i, &px, &py, pt_col, pt_size ) ) {
                        if ( enable_snapping ) px = round ( px * 20.0 ) / 20.0;
                        params [ i ].m_end_time_scaled = glm::clamp ( ( float ) px, 0.0f, 1.0f );

                        if ( mode == 1 ) params [ i ].m_mtx [ 4 + ch ] = ( float ) py;
                        else if ( mode == 2 ) params [ i ].m_mtx [ 8 + ch ] = ( float ) py;
                        else params [ i ].m_mtx [ ch ] = ( float ) py;
                    }

                    // Hover detection
                    ImPlotPoint mouse = ImPlot::GetPlotMousePos ( );
                    if ( fabs ( mouse.x - px ) < 0.02 && fabs ( mouse.y - py ) < 0.2 ) {
                        hovered_pt = i;
                        active_channel = ch; // Auto-focus channel on hover
                    }
                }
            }
        }

        // --- Input Handling ---
        if ( ImGui::IsMouseClicked ( ImGuiMouseButton_Right ) ) {
            if ( hovered_pt >= 0 ) {
                selected_pt = hovered_pt;
                ImGui::OpenPopup ( "PointCtx" );
            }
            else if ( ImPlot::IsPlotHovered ( ) ) {
                // Add new point logic...
                ImPlotPoint mp = ImPlot::GetPlotMousePos ( );
                int idx = 0;
                while ( idx < params.size ( ) && mp.x > params [ idx ].m_end_time_scaled ) idx++;

                sparticle_element_param new_p = ( idx > 0 ) ? params [ idx - 1 ] : default_element ( );
                new_p.m_end_time_scaled = ( float ) mp.x;
                params.insert ( params.begin ( ) + idx, new_p );
                selected_pt = idx;
            }
        }

        // --- Context Menu ---
        if ( ImGui::BeginPopup ( "PointCtx" ) ) {
            ImGui::Text ( "Keyframe %d", selected_pt );
            ImGui::Separator ( );
            if ( ImGui::Selectable ( "Delete" ) && params.size ( ) > 1 ) {
                params.erase ( params.begin ( ) + selected_pt );
            }
            if ( ImGui::Selectable ( "Duplicate" ) ) {
                params.insert ( params.begin ( ) + selected_pt, params [ selected_pt ] );
            }
            ImGui::EndPopup ( );
        }

        ImPlot::EndPlot ( );
    }

    // Channel Selector (Bottom)
    ImGui::Text ( "Active Channel:" ); ImGui::SameLine ( );
    ImGui::SameLine ( );
    ImGui::PushID ( "ActiveChannelSelector" );
    for ( int i = 0; i < 4; ++i ) {
        if ( strcmp ( chan_names [ i ], "None" ) == 0 ) continue;
        if ( ImGui::RadioButton ( chan_names [ i ], active_channel == i ) ) active_channel = i;
        ImGui::SameLine ( );
    }
    ImGui::PopID ( );
    ImGui::NewLine ( );

    ensure_final_param ( params );
}



static void draw_param_curve_editor (
    std::vector<sparticle_element_param>& params
) {
    if ( params.size ( ) < 1 )
        return;

    ensure_final_param ( params );

    static int channel = 0;
    static int mode = 0; // 0 = value, 1 = velocity, 2 = acceleration
    static int hovered_point = -1;
    static int selected_point = -1;

    const char* mode_names [ ] = { "Value", "Velocity", "Acceleration" };

    static const char** channel_names = get_channel_names ( ( e_element_type ) params [ 0 ].m_type );

    ImGui::Combo ( "Channel", &channel, channel_names, IM_ARRAYSIZE ( channel_names ) );

    ImGui::Combo ( "Curve", &mode, mode_names, 3 );

    static std::vector<float> times;
    static std::vector<float> values;

    times.clear ( );
    values.clear ( );

    float t0 = 0.0f;

    glm::vec4 base (
        params [ 0 ].m_mtx [ 0 ],
        params [ 0 ].m_mtx [ 1 ],
        params [ 0 ].m_mtx [ 2 ],
        params [ 0 ].m_mtx [ 3 ]
    );

    for ( int i = 0; i < params.size ( ); ++i ) {
        float t1 = params [ i ].m_end_time_scaled;
        float dt = t1 - t0;

        if ( mode == 0 ) {
            glm::vec4 v = eval_param_value ( params [ i ], base, dt );
            values.push_back ( v [ channel ] );
            base = v;
        }
        else if ( mode == 1 ) {
            values.push_back ( params [ i ].m_mtx [ 4 + channel ] );
        }
        else {
            values.push_back ( params [ i ].m_mtx [ 8 + channel ] );
        }

        times.push_back ( t1 );
        t0 = t1;
    }

    if ( ImPlot::BeginPlot ( "Param Curve", ImVec2 ( -1, 300 ) ) ) {
        ImPlot::SetupAxes ( "Time", "Value" );
        ImPlot::SetupAxesLimits ( 0, 1, -10, 10, ImGuiCond_Once );
        ImPlot::SetupAxisLimits ( ImAxis_X1, 0.0f, 1.0f, ImPlotCond_Always );

        ImPlot::PlotLine ( "Curve", times.data ( ), values.data ( ), times.size ( ) );

        hovered_point = -1;

        for ( int i = 0; i < params.size ( ); ++i ) {
            double y = 0.0;
            double t = params [ i ].m_end_time_scaled;

            if ( mode == 1 ) y = params [ i ].m_mtx [ 4 + channel ];
            else if ( mode == 2 ) y = params [ i ].m_mtx [ 8 + channel ];

            // Draw the drag point  
            if ( ImPlot::DragPoint ( i, &t, &y, ImVec4 ( 1, 0.6f, 0, 1 ), 6 ) ) {
                params [ i ].m_end_time_scaled = glm::clamp ( ( float ) t, 0.0f, 1.0f );
                if ( mode == 1 ) params [ i ].m_mtx [ 4 + channel ] = ( float ) y;
                if ( mode == 2 ) params [ i ].m_mtx [ 8 + channel ] = ( float ) y;
            }

            // Check if this point is hovered  
            ImPlotPoint mouse_pos = ImPlot::GetPlotMousePos ( );
            double dist_x = fabs ( mouse_pos.x - t );
            double dist_y = fabs ( mouse_pos.y - y );

            // FIX: Slightly increased threshold for better reliability (0.02 -> 0.03, 0.2 -> 0.3)
            if ( dist_x < 0.03 && dist_y < 0.3 ) {
                hovered_point = i;
            }
        }

        // FIX: Reordered logic to prioritize the context menu over adding new points
        if ( ImGui::IsMouseClicked ( ImGuiMouseButton_Right ) ) {
            if ( hovered_point >= 0 ) {
                // 1. If hovering a point, open the context menu
                selected_point = hovered_point;
                ImGui::OpenPopup ( "KeyframePopup" );
            }
            else if ( ImPlot::IsPlotHovered ( ) ) {
                // 2. If NOT hovering a point but clicking the plot, add a new keyframe
                ImPlotPoint mp = ImPlot::GetPlotMousePos ( );

                int insert_pos = 0;
                while ( insert_pos < params.size ( ) && mp.x > params [ insert_pos ].m_end_time_scaled ) {
                    insert_pos++;
                }

                sparticle_element_param new_param;
                if ( insert_pos > 0 ) {
                    new_param = params [ insert_pos - 1 ];
                }
                else {
                    new_param = default_element ( );
                }

                new_param.m_end_time_scaled = glm::clamp ( ( float ) mp.x, 0.0f, 1.0f );

                if ( mode == 1 ) {
                    new_param.m_mtx [ 4 + channel ] = ( float ) mp.y;
                }
                else if ( mode == 2 ) {
                    new_param.m_mtx [ 8 + channel ] = ( float ) mp.y;
                }

                params.insert ( params.begin ( ) + insert_pos, new_param );
                selected_point = insert_pos;
            }
        }

        // Draw the context menu  
        if ( ImGui::BeginPopup ( "KeyframePopup" ) ) {
            if ( selected_point >= 0 && selected_point < params.size ( ) ) {
                ImGui::Text ( "Keyframe %d", selected_point );
                sparticle_element_param& param = params [ selected_point ];

                ImGui::DragFloat ( "Time", &param.m_end_time_scaled, 0.001f, 0.0f, 1.0f );

                if ( mode == 1 )
                    ImGui::DragFloat ( "Velocity", &param.m_mtx [ 4 + channel ], 0.01f );
                else if ( mode == 2 )
                    ImGui::DragFloat ( "Acceleration", &param.m_mtx [ 8 + channel ], 0.01f );
                else if ( mode == 0 && selected_point == 0 )
                    ImGui::DragFloat4 ( "Base", param.m_mtx, 0.01f );

                if ( ImGui::Button ( "Delete" ) && params.size ( ) > 1 ) {
                    params.erase ( params.begin ( ) + selected_point );
                    ensure_final_param ( params );
                    ImGui::CloseCurrentPopup ( );
                }

                ImGui::SameLine ( );
                if ( ImGui::Button ( "Duplicate" ) ) {
                    sparticle_element_param dup = param;
                    dup.m_end_time_scaled = glm::min ( dup.m_end_time_scaled + 0.1f, 1.0f );
                    params.insert ( params.begin ( ) + selected_point + 1, dup );
                    selected_point++;
                    ImGui::CloseCurrentPopup ( );
                }

                if ( ImGui::Button ( "Close" ) ) {
                    ImGui::CloseCurrentPopup ( );
                }
            }
            ImGui::EndPopup ( );
        }

        ImPlot::EndPlot ( );
    }

    // Display selected keyframe info
    if ( selected_point >= 0 && selected_point < params.size ( ) ) {
        ImGui::Text ( "Selected Keyframe: %d (Time: %.3f)", selected_point, params [ selected_point ].m_end_time_scaled );
    }

    // Base edit (ONLY param 0)
    if ( mode == 0 && ImGui::TreeNode ( "Base (Param 0 only)" ) ) {
        ImGui::DragFloat4 (
            "Base",
            params [ 0 ].m_mtx,
            0.01f
        );
        ImGui::TreePop ( );
    }
}


void cgame::draw_ptcl_data ( ) {
    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::particle_start );
    if ( !p || !p->m_particle_data ) {
        return;
    }

    sparticle* ptcl_data = p->m_particle_data;

    ImGuiViewport* viewport = ImGui::GetMainViewport ( );
    ImGui::SetNextWindowPos ( ImVec2 ( viewport->WorkPos.x + viewport->WorkSize.x - s_right_panel_w, viewport->WorkPos.y ) );
    ImGui::SetNextWindowSize ( ImVec2 ( s_right_panel_w, viewport->WorkSize.y ) );

    ImGui::Begin ( "Particle Editor" );

    if ( ImGui::CollapsingHeader ( "Particle Info", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        InputString ( "Name", &ptcl_data->m_name );
        ImGui::InputInt ( "ID", &ptcl_data->m_id );
        ImGui::InputInt ( "Unknown", &ptcl_data->m_unk );
    }

    ImGui::Separator ( );

    // ---------------------------------------------------------
    // EMITTERS
    // ---------------------------------------------------------
    if ( ImGui::CollapsingHeader ( "Emitters" ) ) {

        if ( ImGui::Button ( "Add Emitter" ) ) {
            ptcl_data->m_particle_params.push_back ( default_particle_param ( ) );
        }

        for ( int i = 0; i < ptcl_data->m_particle_params.size ( ); ++i ) {
            ImGui::PushID ( i );
            sparticle_param& param = ptcl_data->m_particle_params [ i ];

            bool open = ImGui::TreeNode ( "##emitter_node", "Emitter %d", i );
            ImGui::SameLine ( );
            if ( ImGui::SmallButton ( "Delete" ) ) {
                ptcl_data->m_particle_params.erase ( ptcl_data->m_particle_params.begin ( ) + i );
                ImGui::PopID ( );
                if ( open ) ImGui::TreePop ( );
                continue;
            }

            if ( open ) {
                if ( ImGui::BeginTabBar ( "EmitterTabs" ) ) {

                    // Tab 1: Base Parameters
                    if ( ImGui::BeginTabItem ( "Base" ) ) {
                        ImGui::Checkbox ( "Is Enabled", ( bool* ) &param.m_is_enabled );
                        ImGui::DragIntRange2 ( "Cycle Min/Max", &param.m_cycle_min, &param.m_cycle_max, 1.0f, 0, 10000 );
                        ImGui::Combo ( "Emit Shape", &param.m_emit_shape, emit_shape_names, IM_ARRAYSIZE ( emit_shape_names ) );
                        ImGui::Checkbox ( "Use Normals", ( bool* ) &param.m_use_surface_normals );
                        ImGui::InputInt ( "Transform Flag", &param.m_transform_flag );
                        ImGui::DragFloatRange2 ( "Radius Min/Max", &param.m_radius_min, &param.m_radius_max, 0.1f );
                        ImGui::DragInt ( "Distribution", &param.m_angle );
                        ImGui::DragInt2 ( "Rot Y/Z", &param.m_rot_y );

                        ImGui::Text ( "Scaling" );
                        ImGui::DragFloatRange2 ( "Scale Y", &param.m_scale_y_min, &param.m_scale_y_max, 0.01f );
                        ImGui::DragFloatRange2 ( "Scale XZ", &param.m_scale_xz_min, &param.m_scale_xz_max, 0.01f );

                        ImGui::DragIntRange2 ( "Dir Angle", &param.m_dir_angle_min, &param.m_dir_angle_max );
                        ImGui::InputInt ( "Special Orient", &param.m_special_orient_flag );
                        ImGui::EndTabItem ( );
                    }

                    // Tab 2: Config & Vector
                    if ( ImGui::BeginTabItem ( "Config and Vector" ) ) {
                        sparticle_emitter_param& e_param = param.m_emitter_param;

                        ImGui::TextDisabled ( "Emitter Params" );
                        ImGui::DragIntRange2 ( "Delay Duration", &e_param.m_delay_min, &e_param.m_delay_max );
                        ImGui::DragIntRange2 ( "Emit Duration", &e_param.m_emit_min, &e_param.m_emit_max );
                        ImGui::DragFloat ( "Life Time", &e_param.m_life_time, 0.1f );
                        ImGui::DragFloat ( "Inv Speed", &e_param.m_inverse_speed, 0.01f );
                        ImGui::InputInt ( "Emit Pool Max", &e_param.m_pool_size );
                        ImGui::InputInt ( "Vertex Type", &e_param.m_vertex_type );
                        ImGui::DragInt ( "Unk Angle", &e_param.m_unknown4 );
                        ImGui::InputInt ( "Model ID", &e_param.m_model_id );
                        ImGui::InputInt ( "Texture ID", &e_param.m_texture_id );
                        ImGui::InputFloat ( "Time Scale", &e_param.m_time_scale );
                        ImGui::InputFloat ( "Generate Rate", &e_param.m_frame_rate );

                        ImGui::Separator ( );
                        ImGui::TextDisabled ( "Vector Params" );
                        sparticle_vector_param& v_param = e_param.m_vector_param;
                        ImGui::DragFloat ( "Base", &v_param.m_base );
                        ImGui::DragFloat ( "Vertical Base", &v_param.m_vertical_base );
                        ImGui::DragFloat ( "Multiplier", &v_param.m_multiplier );
                        ImGui::DragInt ( "Angle Base", &v_param.m_angle_base );
                        ImGui::DragFloat ( "Vertical Pan", &v_param.m_vertical_pan );
                        ImGui::DragFloat ( "Pos Pan", &v_param.m_pos_pan );
                        ImGui::DragFloat ( "Pos Random", &v_param.m_pos_random );

                        ImGui::EndTabItem ( );
                    }

                    // Tab 3: Visuals (Vertex)
                    if ( ImGui::BeginTabItem ( "Visuals" ) ) {
                        sparticle_vertex_param& vtx = param.m_emitter_param.m_vertex_param;

                        ImGui::ColorEdit4 ( "Color Base", glm::value_ptr ( vtx.m_color_base ) );
                        ImGui::ColorEdit4 ( "Color Range", glm::value_ptr ( vtx.m_color_range ) );

                        ImGui::Separator ( );
                        ImGui::InputInt ( "Scale Flag", &vtx.m_scale_flag );
                        ImGui::DragFloat4 ( "Scale Base", glm::value_ptr ( vtx.m_scale_base ), 0.01f );
                        ImGui::DragFloat4 ( "Scale Range", glm::value_ptr ( vtx.m_scale_range ), 0.01f );

                        ImGui::Separator ( );
                        ImGui::DragFloat3 ( "Rot Base", glm::value_ptr ( vtx.m_rotation_base ) );
                        ImGui::DragFloat3 ( "Rot Range", glm::value_ptr ( vtx.m_rotation_range ) );
                        ImGui::DragFloat3 ( "Ang Accel Base", glm::value_ptr ( vtx.m_angular_accel_base ) );
                        ImGui::DragFloat3 ( "Ang Accel Range", glm::value_ptr ( vtx.m_angular_accel_range ) );

                        ImGui::Separator ( );
                        ImGui::Text ( "UV & Frame" );
                        ImGui::DragFloat2 ( "UV Base", glm::value_ptr ( vtx.m_uv_base ), 0.01f );
                        ImGui::DragFloat2 ( "UV Range", glm::value_ptr ( vtx.m_uv_range ), 0.01f );
                        ImGui::InputInt ( "Cols", &vtx.m_columns );
                        ImGui::InputInt ( "Rows", &vtx.m_rows );
                        ImGui::DragIntRange2 ( "Anim Frame", &vtx.m_start_frame, &vtx.m_end_frame );
                        ImGui::DragFloat ( "Width", &vtx.m_width );
                        ImGui::DragFloat ( "Height", &vtx.m_height );

                        ImGui::EndTabItem ( );
                    }

                    // Tab 4: Elements (Logic)
                    if ( ImGui::BeginTabItem ( "Elements" ) ) {
                        sparticle_emitter_param& e_param = param.m_emitter_param;

                        if ( ImGui::Button ( "Add Element" ) ) {
                            e_param.m_element_chains.push_back ( { default_element ( ) } );
                            e_param.m_element_count++;
                        }

                        int to_delete = -1;

                        for ( int c = 0; c < e_param.m_element_chains.size ( ); ++c ) {
                            ImGui::PushID ( c );
                            auto& element = e_param.m_element_chains [ c ];
                            bool tree = ImGui::TreeNode ( "Element", "Element %d", c );

                            ImGui::SameLine ( ImGui::GetWindowWidth ( ) - 30 );
                            if ( ImGui::SmallButton ( "X" ) ) {
                                to_delete = c;
                            }

                            if ( tree ) {
                                sparticle_element_param& head = element [ 0 ];
                                if ( ImGui::Combo (
                                    "Type",
                                    &head.m_type,
                                    element_type_names,
                                    IM_ARRAYSIZE ( element_type_names )
                                ) ) {
                                    for ( auto& param : element ) {
                                        param.m_type = head.m_type;
                                    }
                                };

                                if ( ImGui::Combo (
                                    "Effect",
                                    &head.m_effect_type,
                                    element_effect_names,
                                    IM_ARRAYSIZE ( element_effect_names )
                                ) ) {
                                    for ( auto& param : element ) {
                                        param.m_effect_type = head.m_effect_type;
                                    }
                                };

                                draw_improved_curve_editor ( element );
                                ImGui::TreePop ( );
                            }

                            ImGui::PopID ( );
                        }
                        if ( to_delete != -1 ) {
                            e_param.m_element_chains.erase ( e_param.m_element_chains.begin ( ) + to_delete );
                            e_param.m_element_count--;
                        }
                        ImGui::EndTabItem ( );
                    }

                    ImGui::EndTabBar ( );
                }
                ImGui::TreePop ( );
            }
            ImGui::PopID ( );
        }
    }

    ImGui::End ( );
}
