// ============================================================================
// File: game.cpp
// 
// Purpose: Implements the game module
// 
// ============================================================================

#include "game.h"

#include "game\actor\root.h"
#include "engine\core\engine.h"
#include "engine\core\components.h"
#include "engine\sys\actor\actor.h"
#include "engine\sys\render\render.h"

#include "game\sys\ogre\particle\particle.h"
#include "game\sys\ogre\maths\maths.h"
#include "game\file\file.h"
#include "game\actor\dummy.h"
#include "game\actor\authoring_common.h"
#include "game\actor\yact_manager.h"
#include "game\actor\camera.h"

#include "imgui\imgui.h"
#include "imgui\backends\imgui_impl_glfw.h"
#include "imgui\backends\imgui_impl_opengl3.h"

#include "game\imgui_file_dialog\ImGuiFileDialog.h"

#include "game\implot\implot.h"

#include <filesystem>
#include <algorithm>

static float seed = frandom ( );

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

cgame::cgame ( ) {
    m_particle_path = "";
    m_got_folder = false;
    m_is_y2 = false;
    m_particle_looped = false;

    m_got_motion = false;
    m_motion_looped = false;
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
    ImGui::SetNextWindowSize ( ImVec2 ( s_left_panel_w, viewport->WorkSize.y ) );
    ImGui::Begin ( "Main Window", nullptr, main_window_flags );

    //if ( ImGui::Button ( "Open YAct Folder" ) ) {
    //    ImGuiFileDialog::Instance ( )->OpenDialog ( "ChooseYActFolder", "Choose a folder", nullptr );
    //}

    if ( ImGui::Button ( "Open PTCL Folder" ) ) {
        ImGuiFileDialog::Instance ( )->OpenDialog ( "ChoosePTCLFolder", "Choose a folder", nullptr );
    }

    if ( ImGui::Button ( "Open Motion" ) ) {
        ImGuiFileDialog::Instance ( )->OpenDialog ( "ChooseMotFile", "Choose Motion file", ".dat,.omt" );
    }

    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );

    if ( ImGui::Checkbox ( "Is Yakuza 2", &m_is_y2 ) ) {
        m_ptcl_info.clear ( );
        populate_info ( );
    };

    if ( m_got_folder ) {
        ImGui::Text ( "PTCL Files" );

        draw_ptcl_tree ( );
    }

    // messy
    bool ok = false;
    if ( ImGuiFileDialog::Instance ( )->Display ( "ChooseMotFile" ) ) {
        if ( ImGuiFileDialog::Instance ( )->IsOk ( ) ) {
            m_motion_path = ImGuiFileDialog::Instance ( )->GetFilePathName ( );
            ok = true;
        }
        ImGuiFileDialog::Instance ( )->Close ( );
    }

    if ( ok )
        ImGuiFileDialog::Instance ( )->OpenDialog ( "ChoosePmmFile", "Choose PMM file", ".dat" );

    if ( ImGuiFileDialog::Instance ( )->Display ( "ChoosePmmFile" ) ) {
        if ( ImGuiFileDialog::Instance ( )->IsOk ( ) ) {
            m_pmm_path = ImGuiFileDialog::Instance ( )->GetFilePathName ( );
            m_got_motion = true;

            cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );
            if ( !d ) {
                d = new cact_dummy ( cengine::get ( )->act_man->get_actor ( e_actid::root ), e_actid::dummy );
            }
            d->reload_motion ( m_motion_path, m_pmm_path, true );

            p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );
            if ( p ) {
                p->set_exec_flag ( e_act_exec::pause );
                p->set_exec_flag ( e_act_exec::done );
            }
        }
        ImGuiFileDialog::Instance ( )->Close ( );
    }


    if ( ImGuiFileDialog::Instance ( )->Display ( "ChoosePTCLFolder" ) ) {
        if ( ImGuiFileDialog::Instance ( )->IsOk ( ) ) {
            m_particle_path = ImGuiFileDialog::Instance ( )->GetCurrentPath ( );
            p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );
            if ( p ) {
                p->set_exec_flag ( e_act_exec::pause );
                p->set_exec_flag ( e_act_exec::done );
            }

            cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );
            if ( d ) {
                d->set_exec_flag ( e_act_exec::pause );
                d->set_exec_flag ( e_act_exec::done );
            }

            cact_camera* cam = ( cact_camera* ) cengine::get ( )->act_man->get_actor ( e_actid::camera );

            cam->set_target ( glm::vec3 ( 0.0f ) );

            m_got_folder = true;

            m_mesh_list.clear ( );
            m_tex_list.clear ( );
            m_ptcl_info.clear ( );

            cengine::get ( )->mesh_man->clear_mesh ( );
            cengine::get ( )->tex_man->clear_tex ( );

            for ( auto& entry : fs::directory_iterator ( m_particle_path ) )
            {
                if ( entry.path ( ).extension ( ).string ( ) == ".OME" ) {
                    int index = std::stoi ( entry.path ( ).stem ( ).string ( ) );

                    if ( m_mesh_list.size ( ) <= index ) m_mesh_list.resize ( index + 1 );

                    m_mesh_list [ index ] = read_ogre_mesh_file ( entry.path ( ).string ( ).c_str ( ), "Shaders\\vertex_general.glsl",
                        "Shaders\\fragment_particle.glsl" );
                }
                else if ( entry.path ( ).extension ( ).string ( ) == ".TXB" ) {
                    int index = std::stoi ( entry.path ( ).stem ( ).string ( ) );

                    if ( m_tex_list.size ( ) <= index ) m_tex_list.resize ( index + 1 );

                    m_tex_list [ index ] = read_ogre_tex_file ( entry.path ( ).string ( ).c_str ( ) );
                }
            }
        }
        ImGuiFileDialog::Instance ( )->Close ( );
    }

    /*
    if ( ImGuiFileDialog::Instance ( )->Display ( "ChooseYActFolder" ) ) {
        if ( ImGuiFileDialog::Instance ( )->IsOk ( ) ) {
            m_yact_path = ImGuiFileDialog::Instance ( )->GetCurrentPath ( );

            cact_yact_manager* ym = ( cact_yact_manager* ) cengine::get ( )->act_man->get_actor ( e_actid::yact_manager );

            if ( ym ) ym->reset ( );
            else ym = new cact_yact_manager (
                cengine::get ( )->act_man->get_actor ( e_actid::root ),
                e_actid::yact_manager
            );

        }
        ImGuiFileDialog::Instance ( )->Close ( );
    }
    */

    p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );

    if ( m_particle_looped && p && p->m_particles.empty ( ) ) {
        p->create_blank ( );
    }

    cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );

    if ( m_motion_looped && d && d->m_animator->m_anim->m_frame_num < d->m_animator->m_frame_counter ) {
        d->reload_motion ( m_motion_path, m_pmm_path );
    }

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

    d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );

    if ( d ) {
        IGFD::FileDialogConfig cfg;
        cfg.fileName = "prop.dat";
        cfg.filePathName = ".";
        cfg.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        if ( ImGui::Button ( "Save Property" ) )
        {
            ImGuiFileDialog::Instance ( )->OpenDialog (
                "SaveProp",
                "Save Property",
                ".dat",
                cfg
            );
        }

        if ( ImGuiFileDialog::Instance ( )->Display ( "SaveProp" ) )
        {
            if ( ImGuiFileDialog::Instance ( )->IsOk ( ) )
            {
                std::string path =
                    ImGuiFileDialog::Instance ( )->GetFilePathName ( );

                write_ogre_pmm ( path.c_str ( ), d->m_pmm_data );
            }

            ImGuiFileDialog::Instance ( )->Close ( );
        }
    }

    draw_ptcl_data ( );
    draw_pmm_data ( );
    populate_info ( );

    ImGui::End ( );

    ImGui::Render ( );
    ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData ( ) );
}

void cgame::populate_info ( ) {
    if ( !m_got_folder ) return;
    if ( m_ptcl_info.empty ( ) ) {
        if ( m_is_y2 ) {
            for ( auto& entry : fs::directory_iterator ( m_particle_path ) )
            {
                if ( entry.path ( ).extension ( ) == ".ptcl" )
                {
                    const std::string filename = entry.path ( ).string ( );
                    std::string name;
                    int id;
                    get_particle_info ( filename, name, id );
                    m_ptcl_info.push_back ( { name,id } );
                }
            }
        }
        else {
            for ( int i = 0; i < 697; i++ ) {
                m_ptcl_info.push_back ( g_ptcl_info [ i ] );
            }
        }
    }
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
                    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );
                    if ( !p )
                        new cact_particle ( cengine::get ( )->act_man->get_actor ( e_actid::root ),
                            e_actid::test_particle,
                            m_particle_name );
                    else {
                        p->m_ptcl_name = m_particle_name;
                        delete p->m_particle_data;
                        p->m_particle_data = load_particle_file ( p->m_ptcl_name );
                        p->create_blank ( );
                    }

                    cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );

                    if ( d ) {
                        d->set_exec_flag ( e_act_exec::done );
                        d->set_exec_flag ( e_act_exec::pause );
                    }

                    cact_camera* cam = ( cact_camera* ) cengine::get ( )->act_man->get_actor ( e_actid::camera );

                    cam->set_target ( glm::vec3 ( 0.0f ) );
                }
            }
        }

        ImGui::TreePop ( );
    }
}

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

const char* uv_type_names [ ] = {
    "Normal", "Pattern"
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

    std::sort ( 
        params.begin ( ), 
        params.end ( ), 
        [ ] ( const sparticle_element_param& a, const sparticle_element_param& b ) { 
            return a.m_end_time_scaled < b.m_end_time_scaled; 
        } );

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
    case e_element_type::pattern:
    case e_element_type::uv: return chan_uv;
    default: return chan_s;
    }
}

static void get_channel_min_max ( e_element_type type, float& min, float& max ) {
    switch ( type ) {
    case e_element_type::scale: 
        min = -100.0f;
        max = 100.0f;
        break;
    case e_element_type::color_8bit:
    case e_element_type::color_7bit:
        min = -255.0f;
        max = 255.0f;
        break;
    case e_element_type::angle:
        min = -65535.0f;
        max = 65535.0f;
        break;
    case e_element_type::pattern:
    case e_element_type::uv:
        min = -1.0f;
        max = 1.0f;
        break;
    default:
        min = -100.0f;
        max = 100.0f;
        break;
    }
}

void draw_param_curve_editor ( std::vector<sparticle_element_param>& params, sparticle_emitter_param& e_param ) {
    if ( params.empty ( ) ) return;
    ensure_final_param ( params );

    static bool show_channels [ 4 ] = { true, true, true, true };
    static int active_channel = 0;
    static float y_min = -5.0f;
    static float y_max = 5.0f;

    static int selected_pt_idx = -2;
    static int hovered_pt_idx = -2;

    e_element_type type = ( e_element_type ) params [ 0 ].m_type;
    const char** chan_names = get_channel_names ( type );

    float total_life = e_param.m_life_time;

    get_channel_min_max ( type, y_min, y_max );

    ImGui::BeginChild ( "CurveToolbar", ImVec2 ( 0, 35 ), true );
    for ( int i = 0; i < 4; ++i ) {
        if ( strcmp ( chan_names [ i ], "None" ) == 0 ) continue;
        ImGui::Checkbox ( chan_names [ i ], &show_channels [ i ] ); ImGui::SameLine ( );
    }
    if ( ImGui::Button ( "Fit View" ) ) { ImPlot::SetNextAxisToFit ( ImAxis_Y1 ); }
    ImGui::EndChild ( );

    if ( ImPlot::BeginPlot ( "##CURVEEDIT", ImVec2 ( -1, 400 ) ) ) {
        ImPlot::SetupAxes ( "Normalized Time (0-1)", "Value", ImPlotAxisFlags_None, ImPlotAxisFlags_None );
        ImPlot::SetupAxisLimits ( ImAxis_X1, 0, 1, ImPlotCond_Always );
        ImPlot::SetupAxisLimits ( ImAxis_Y1, y_min, y_max, ImPlotCond_Once );

        float t_phys_max = ( total_life > 0 ) ? total_life : 1.0f;
        ImVec4 colors [ 4 ] = { {1,0,0,1}, {0,1,0,1}, {0.2f,0.5f,1,1}, {1,1,1,1} };

        ImPlotPoint plot_mouse_pos = ImPlot::GetPlotMousePos ( );
        ImVec2 screen_mouse_pos = ImGui::GetMousePos ( );

        hovered_pt_idx = -2;

        auto is_hovered_in_pixels = [ & ] ( double x, double y, float radius ) -> bool {
            ImVec2 pt_screen = ImPlot::PlotToPixels ( x, y );
            float dx = screen_mouse_pos.x - pt_screen.x;
            float dy = screen_mouse_pos.y - pt_screen.y;
            return ( dx * dx + dy * dy ) < ( radius * radius );
            };

        for ( int ch = 0; ch < 4; ch++ ) {
            if ( !show_channels [ ch ] || strcmp ( chan_names [ ch ], "None" ) == 0 ) continue;
            ImGui::PushID ( ch );

            std::vector<double> plot_x, plot_y;
            float p_start_t = 0.0f;
            float p_start_val = params [ 0 ].m_mtx [ ch ];

            double sx = 0, sy = p_start_val;
            if ( active_channel == ch ) {
                if ( ImPlot::DragPoint ( 100, &sx, &sy, colors [ ch ], 7.0f ) ) {
                    params [ 0 ].m_mtx [ ch ] = ( float ) sy;
                }

                if ( is_hovered_in_pixels ( sx, sy, 8.0f ) ) {
                    hovered_pt_idx = -1;
                }
            }

            for ( int i = 0; i < ( int ) params.size ( ); i++ ) {
                sparticle_element_param& p = params [ i ];
                float end_t = p.m_end_time_scaled;
                float v = p.m_mtx [ 4 + ch ];
                float a = ( p.m_effect_type == 1 ) ? p.m_mtx [ 8 + ch ] : 0.0f;

                plot_x.clear ( ); plot_y.clear ( );
                for ( int s = 0; s <= 20; s++ ) {
                    float t_norm = p_start_t + ( ( float ) s / 20.0f ) * ( end_t - p_start_t );
                    float dt = ( t_norm - p_start_t ) * t_phys_max;
                    float val = p_start_val + ( v * dt ) + ( 0.5f * a * dt * dt );
                    plot_x.push_back ( t_norm );
                    plot_y.push_back ( val );
                }

                ImPlot::SetNextLineStyle ( colors [ ch ], ( active_channel == ch ) ? 3.0f : 1.0f );
                ImPlot::PlotLine ( "##seg", plot_x.data ( ), plot_y.data ( ), ( int ) plot_x.size ( ) );

                if ( active_channel == ch ) {
                    double ex = end_t, ey = plot_y.back ( );

                    // Acceleration Handle
                    if ( ImPlot::DragPoint ( i * 2, &ex, &ey, colors [ ch ], 6.0f ) ) {
                        p.m_end_time_scaled = glm::clamp ( ( float ) ex, p_start_t + 0.01f, 1.0f );
                        float dt = ( p.m_end_time_scaled - p_start_t ) * t_phys_max;
                        if ( dt > 0 ) {
                            p.m_mtx [ 8 + ch ] = 2.0f * ( ( float ) ey - p_start_val - ( v * dt ) ) / ( dt * dt );
                        }
                    }

                    if ( is_hovered_in_pixels ( ex, ey, 8.0f ) ) {
                        hovered_pt_idx = i;
                    }

                    // Velocity Handle
                    double tx = p_start_t + ( end_t - p_start_t ) * 0.2f;
                    double ty = p_start_val + v * ( ( tx - p_start_t ) * t_phys_max );
                    if ( ImPlot::DragPoint ( i * 2 + 1, &tx, &ty, ImVec4 ( 1, 1, 1, 0.5f ), 4.0f ) ) {
                        float dt_h = ( ( float ) tx - p_start_t ) * t_phys_max;
                        if ( dt_h > 0 ) p.m_mtx [ 4 + ch ] = ( ( float ) ty - p_start_val ) / dt_h;
                    }
                }

                p_start_val = ( float ) plot_y.back ( );
                p_start_t = end_t;
            }
            ImGui::PopID ( );
        }

        if ( ImGui::IsMouseClicked ( ImGuiMouseButton_Right ) && ImPlot::IsPlotHovered ( ) ) {
            selected_pt_idx = hovered_pt_idx;

            if ( selected_pt_idx >= -1 ) ImGui::OpenPopup ( "EditPointPopup" );
            else ImGui::OpenPopup ( "AddPointPopup" );
        }

        if ( ImGui::BeginPopup ( "EditPointPopup" ) ) {
            if ( selected_pt_idx == -1 ) {
                ImGui::Text ( "Initial Value" );
                ImGui::DragFloat ( "Value", &params [ 0 ].m_mtx [ active_channel ], 0.1f );
            }
            else if ( selected_pt_idx >= 0 && selected_pt_idx < params.size ( ) ) {
                auto& p = params [ selected_pt_idx ];
                ImGui::Text ( "Segment %d", selected_pt_idx );
                if ( selected_pt_idx < ( int ) params.size ( ) - 1 )
                    ImGui::DragFloat ( "End Time", &p.m_end_time_scaled, 0.01f, 0.0f, 1.0f );

                ImGui::DragFloat ( "Velocity", &p.m_mtx [ 4 + active_channel ], 0.1f );
                ImGui::DragFloat ( "Acceleration", &p.m_mtx [ 8 + active_channel ], 0.1f );

                ImGui::Separator ( );
                if ( ImGui::Selectable ( "Delete Segment" ) && params.size ( ) > 1 ) {
                    params.erase ( params.begin ( ) + selected_pt_idx );
                    ensure_final_param ( params );
                }
            }
            ImGui::EndPopup ( );
        }

        if ( ImGui::BeginPopup ( "AddPointPopup" ) ) {
            ImGui::Text ( "Add segment at %.3f?", plot_mouse_pos.x );
            if ( ImGui::Selectable ( "Confirm Add" ) ) {
                sparticle_element_param new_p = params.back ( );
                new_p.m_end_time_scaled = ( float ) plot_mouse_pos.x;
                params.push_back ( new_p );
                ensure_final_param ( params );
            }
            ImGui::EndPopup ( );
        }

        ImPlot::EndPlot ( );
    }

    ImGui::Text ( "Active Channel:" ); ImGui::SameLine ( );
    for ( int i = 0; i < 4; ++i ) {
        if ( strcmp ( chan_names [ i ], "None" ) == 0 ) continue;
        ImGui::PushID ( i );
        if ( ImGui::RadioButton ( chan_names [ i ], active_channel == i ) ) active_channel = i;
        ImGui::PopID ( );
        ImGui::SameLine ( );
    }
    ImGui::NewLine ( );

    ensure_final_param ( params );
}


static void texture_tooltip ( ctex_buffer* tex )
{
    if ( !tex || !tex->m_tex )
        return;

    float max_size = 256.0f;
    float w = ( float ) tex->m_width;
    float h = ( float ) tex->m_height;

    float scale = 1.0f;
    if ( w > max_size || h > max_size )
        scale = max_size / glm::max ( w, h );

    ImGui::BeginTooltip ( );
    ImGui::Image (
        ( ImTextureID ) tex->m_tex,
        ImVec2 ( w * scale, h * scale ),
        ImVec2 ( 0, 1 ),
        ImVec2 ( 1, 0 )
    );
    ImGui::Text ( "%dx%d", tex->m_width, tex->m_height );
    ImGui::EndTooltip ( );
}

void draw_texture_selector (
    int& selected_tex_id,
    std::vector<std::vector<ctex_ref>> tex_list )
{
    const char* preview =
        ( selected_tex_id >= 0 && selected_tex_id < ( int ) tex_list.size ( ) )
        ? std::to_string ( selected_tex_id ).c_str ( )
        : "<none>";

    if ( ImGui::BeginCombo ( "Texture", preview ) )
    {
        for ( int i = 0; i < ( int ) tex_list.size ( ); i++ )
        {
            if ( tex_list [ i ].empty ( ) )
                continue;

            bool is_selected = ( i == selected_tex_id );

            ImGui::Selectable (
                ( "Texture " + std::to_string ( i ) ).c_str ( ),
                is_selected
            );

            if ( ImGui::IsItemHovered ( ) )
            {
                selected_tex_id = i;
                texture_tooltip ( tex_list [ i ][ 0 ].get ( ) );
            }

            if ( is_selected )
                ImGui::SetItemDefaultFocus ( );
        }

        ImGui::EndCombo ( );
    }
}


void cgame::draw_ptcl_data ( ) {
    cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( e_actid::test_particle );
    if ( !p || !p->m_particle_data ) {
        return;
    }

    sparticle* ptcl_data = p->m_particle_data;

    ImGuiViewport* viewport = ImGui::GetMainViewport ( );
    ImGui::SetNextWindowPos ( ImVec2 ( viewport->WorkPos.x + viewport->WorkSize.x - s_right_panel_w, viewport->WorkPos.y ) );
    ImGui::SetNextWindowSize ( ImVec2 ( s_right_panel_w, viewport->WorkSize.y ) );

    ImGui::Begin ( "Particle Editor" );

    if ( ImGui::Button ( "Play" ) ) {
        if ( p ) {
            p->create_blank ( );
        }
    };

    ImGui::SameLine ( );

    ImGui::Checkbox ( "Loop", &m_particle_looped );

    ImGui::SameLine ( );

    if ( ImGui::Button ( "Exit" ) ) {
        p->set_exec_flag ( e_act_exec::pause );
        p->set_exec_flag ( e_act_exec::done );
        ImGui::End ( );
        return;
    }

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

                        draw_texture_selector (
                            e_param.m_texture_id,
                            m_tex_list
                        );

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
                        ImGui::DragFloat3 ( "Ang Base", glm::value_ptr ( vtx.m_rotation_base ) );
                        ImGui::DragFloat3 ( "Ang Range", glm::value_ptr ( vtx.m_rotation_range ) );
                        ImGui::DragFloat3 ( "Ang Accel Base", glm::value_ptr ( vtx.m_angular_accel_base ) );
                        ImGui::DragFloat3 ( "Ang Accel Range", glm::value_ptr ( vtx.m_angular_accel_range ) );

                        ImGui::Separator ( );
                        ImGui::Text ( "UV & Frame" );
                        ImGui::Combo ( "UV Type", &vtx.m_uv_flag, uv_type_names, IM_ARRAYSIZE ( uv_type_names ) );
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

                                draw_param_curve_editor ( element, e_param );

                                //for ( int z = 0; z < element.size ( ); z++ ) {
                                //    if ( ImGui::BeginChild ( "Param" ) ) {
                                //        if ( ImGui::Button ( "Delete" ) ) {
                                //            element.erase ( element.begin ( ) + z );
                                //        }
                                //        ImGui::EndChild ( );
                                //    }
                                //}

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

static bool parse_motion_number ( const std::string& name, int& out_num )
{
    // Expected: 0-<number>.dat
    if ( !name.starts_with ( "0-" ) || !name.ends_with ( ".dat" ) )
        return false;

    try {
        out_num = std::stoi ( name.substr ( 2, name.size ( ) - 6 ) );
        return true;
    }
    catch ( ... ) {
        return false;
    }
}

cpmm_property default_property ( ) {
    cpmm_property prop;

    prop.m_type = 3;
    prop.m_start = 0.0f;
    prop.m_end = 3.0f;
    prop.m_tmp0 = 0;
    prop.m_tmp1 = 0;
    prop.m_tmp2 = 0;
    prop.m_tmp3 = 0;
    prop.m_tmp4 = 0;

    return prop;
}

ceffect_authoring default_effect ( ) {
    ceffect_authoring effect;

    effect.m_play_type = 1;
    effect.m_start = 0.0f;
    effect.m_end = 3.0f;
    effect.m_speed = 1.0f;
    effect.m_bone_idx = 0;
    effect.m_type = 1;
    effect.m_id = 1169;
    effect.m_xyz = glm::vec3 ( 0.0f );
    effect.m_normal = glm::vec3 ( 0.0f );
    effect.m_tmp0 = glm::vec3 ( 0.0f );
    effect.m_tmp1 = glm::vec3 ( 0.0f );
    effect.m_tmp1_int = 0;

    return effect;
}

void set_bit ( int& flag, int bit, bool enabled ) {
    if ( !enabled )
        flag &= ~( 1 << bit );
    else
        flag |= ( 1 << bit );
}

const char* effect_play_types [ ] = {
    "Normal", "One Shot"
};

struct vec_rot {
    float yaw;
    float pitch;
    float length;
};

static vec_rot vector_to_ui ( const glm::vec3& v ) {
    vec_rot ui {};

    ui.length = glm::length ( v );

    if ( ui.length > 1e-6f ) {
        glm::vec3 d = v / ui.length;
        ui.yaw = atan2 ( d.x, d.z );
        ui.pitch = asin ( glm::clamp ( d.y, -1.0f, 1.0f ) );
    }
    else {
        ui.yaw = 0.0f;
        ui.pitch = 0.0f;
        ui.length = 1.0f;
    }

    return ui;
}

static glm::vec3 ui_to_vector ( const vec_rot& ui ) {
    float cp = cos ( ui.pitch );

    glm::vec3 dir {
        sin ( ui.yaw ) * cp,
        sin ( ui.pitch ),
        cos ( ui.yaw ) * cp
    };

    return dir * ui.length;
}

bool edit_rot_vec (
    const char* label,
    glm::vec3& inout_vec,
    float max_length = 1000.0f
) {
    vec_rot ui {};
    bool changed = false;

    ImGui::PushID ( label );

    ui = vector_to_ui ( inout_vec );

    ImGui::TextUnformatted ( label );
    ImGui::Indent ( );

    changed |= ImGui::SliderAngle ( "Yaw", &ui.yaw, -180.0f, 180.0f );
    changed |= ImGui::SliderAngle ( "Pitch", &ui.pitch, -89.0f, 89.0f );
    changed |= ImGui::DragFloat (
        "Length", &ui.length,
        0.01f, 0.0001f, max_length, "%.4f"
    );

    ImGui::Unindent ( );

    if ( ui.length < 1e-6f )
        ui.length = 1e-6f;

    if ( changed ) {
        inout_vec = ui_to_vector ( ui );
    }

    ImGui::PopID ( );
    return changed;
}

void cgame::draw_pmm_data ( ) {
    cact_dummy* d = ( cact_dummy* ) cengine::get ( )->act_man->get_actor ( e_actid::dummy );
    if ( !d ) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport ( );
    ImGui::SetNextWindowPos ( ImVec2 ( viewport->WorkPos.x + viewport->WorkSize.x - s_right_panel_w, viewport->WorkPos.y ) );
    ImGui::SetNextWindowSize ( ImVec2 ( s_right_panel_w, viewport->WorkSize.y ) );

    ImGui::Begin ( "Motion data editor" );

    if ( ImGui::SliderFloat ( "Time", &d->m_animator->m_frame_counter, 0.0f, d->m_animator->m_anim->m_frame_num ) ) {
        d->set_time ( d->m_animator->m_frame_counter );
    }

    if ( ImGui::Button ( "Play" ) ) {
        d->reload_motion ( m_motion_path, m_pmm_path );
    }

    ImGui::SameLine ( );

    ImGui::Checkbox ( "Loop", &m_motion_looped );

    ImGui::SameLine ( );

    ImGui::Checkbox ( "Pause", &d->m_pause_motion );

    ImGui::SameLine ( );

    static bool follow_dummy = false;

    ImGui::Checkbox ( "Follow", &follow_dummy );

	cact_camera* cam = ( cact_camera* ) cengine::get ( )->act_man->get_actor ( e_actid::camera );

    if ( follow_dummy ) {
        cam->set_target ( d->m_animator->m_bones [ 1 ].m_anim_pos );
    }
    else {
		cam->set_target ( glm::vec3 ( 0.0f ) );
    }

    ImGui::SameLine ( );

    if ( ImGui::Button ( "Exit" ) ) {
        d->set_exec_flag ( e_act_exec::pause );
        d->set_exec_flag ( e_act_exec::done );
        follow_dummy = false;
        ImGui::End ( );
        return;
    }

    if ( ImGui::BeginTabBar ( "Pmm data" ) ) {
        if ( ImGui::BeginTabItem ( "Properties" ) ) {
            if ( ImGui::Button ( "Add Property" ) ) {
                d->m_pmm_data.m_properties.push_back ( default_property ( ) );
            }
            for ( int i = 0; i < d->m_pmm_data.m_properties.size ( ); i++ ) {
                ImGui::PushID ( i );

                bool open = ImGui::TreeNode ( "##property_node", "Property %d", i );
                ImGui::SameLine ( );
                if ( ImGui::SmallButton ( "Delete" ) ) {
                    d->m_pmm_data.m_properties.erase ( d->m_pmm_data.m_properties.begin ( ) + i );
                    ImGui::PopID ( );
                    if ( open ) ImGui::TreePop ( );
                    continue;
                }

                if ( open ) {
                    cpmm_property& prop = d->m_pmm_data.m_properties [ i ];
                    ImGui::InputInt ( "Type", &prop.m_type );
                    ImGui::DragFloat2 ( "Start / End", &prop.m_start );
                    ImGui::InputInt ( "Tmp0", &prop.m_tmp0 );
                    ImGui::InputInt ( "Tmp1", &prop.m_tmp1 );
                    ImGui::InputInt ( "Tmp2", &prop.m_tmp2 );
                    ImGui::InputInt ( "Tmp3", &prop.m_tmp3 );
                    ImGui::InputInt ( "Tmp4", &prop.m_tmp4 );

                    ImGui::TreePop ( );
                }

                ImGui::PopID ( );
            }

            ImGui::EndTabItem ( );
        }

        if ( ImGui::BeginTabItem ( "Effects" ) ) {
            if ( ImGui::Button ( "Add Effect" ) ) {
                d->m_pmm_data.m_effects.push_back ( default_effect ( ) );
            }
            for ( int i = 0; i < d->m_pmm_data.m_effects.size ( ); i++ ) {
                ImGui::PushID ( i );

                bool open = ImGui::TreeNode ( "##effect_node", "Effect %d", i );
                ImGui::SameLine ( );
                if ( ImGui::SmallButton ( "Delete" ) ) {
                    d->m_pmm_data.m_effects.erase ( d->m_pmm_data.m_effects.begin ( ) + i );
                    ImGui::PopID ( );
                    if ( open ) ImGui::TreePop ( );
                    continue;
                }

                bool reset = false;

                if ( open ) {
                    ceffect_authoring& effect = d->m_pmm_data.m_effects [ i ];
                    ImGui::Combo ( "Play Type", &effect.m_play_type, effect_play_types, IM_ARRAYSIZE ( effect_play_types ) );
                    ImGui::DragFloat2 ( "Start / End", &effect.m_start );
                    ImGui::DragFloat ( "Speed", &effect.m_speed, 0.0f, 100.0f );

                    ImGui::InputInt ( "Type", &effect.m_type );
                    ImGui::InputInt ( "ID", &effect.m_id );

                    switch ( effect.m_type ) {
                    case 1:
                        if ( effect.m_id != 0 ) {
                            ImGui::InputInt ( "Bone Number", &effect.m_bone_idx );
                            if ( ImGui::DragFloat3 ( "Position", &effect.m_xyz.x, 0.1f ) ) reset = true;
                            if ( edit_rot_vec ( "Target", effect.m_normal ) ) reset = true;
                            if ( edit_rot_vec ( "Effect", effect.m_tmp0 ) ) reset = true;
                            if ( ImGui::DragFloat3 ( "Flags", &effect.m_tmp1.x ) ) reset = true;

                            ImGui::InputInt ( "Condition Flags", &effect.m_tmp0_int );

                            bool parent = effect.m_tmp1_int & 0x00000001;
                            bool rot = effect.m_tmp1_int & 0x00000100;

                            ImGui::Checkbox ( "Follow Parent", &parent );
                            ImGui::Checkbox ( "Follow Rotation", &rot );

                            set_bit ( effect.m_tmp1_int, 0, parent );
                            set_bit ( effect.m_tmp1_int, 8, rot );
                        }
                        else {

                        }
                        break;
                    default:
                        break;
                    }

                    ImGui::TreePop ( );
                }

                if ( reset ) {
                    d->set_time ( d->m_animator->m_frame_counter );
                }

                ImGui::PopID ( );
            }

            ImGui::EndTabItem ( );
        }

        ImGui::EndTabBar ( );
    }

    ImGui::End ( );
}