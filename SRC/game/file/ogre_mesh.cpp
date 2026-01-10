#include "file.h"

#include "engine\core\engine.h"

#include "engine\file\binary_helper.h"
#include "engine\sys\render\render.h"

enum eODBP_TYPE
{
    eODBP_TYPE_NORMAL,
    eODBP_TYPE_NO_WEIGHTS,
    eODBP_TYPE_STAGE
};

struct SOMEHeader {
    // stores header info
    std::string magic {};
    int version {};
    int OBDP_pointer {};
    int misctable_count {};
    int unk {};
    int sheen_strength {};
    int unk_flag {};
};

struct SODBPHeader {
    // stores subheader info (OBDP)
    std::string submagic {};
    int substruct_size {}; // idk if it can be called like that
    int unk_id {};
    int filesize {};
    int mesh_pointer {};
    int mesh_count {};
    int armature_pointer {};
    int bone_count {};
    int unkvar {};
    int mat_pointer_1 {};
    int physicsobj_pointer {};
    int true_mat_pointer {};
};
struct SMeshHeader
{
    int info_ptr;
    int vert_ptr;
    int vert_count;
    int face_ptr;
    int face_count;
};

std::vector<cmesh_ref> read_ogre_mesh_file ( const char* szFileName, int type, const char* VSh, const char* FSh ) {
    std::vector<cmesh_ref> names;
	cbinary_helper bin_help = cbinary_helper ( szFileName );
    bin_help.seek ( 0 );
    SOMEHeader header;
    header.magic = bin_help.read_fixed_string ( 4 );
    header.version = bin_help.read_uint ( );
    header.OBDP_pointer = bin_help.read_uint ( );
    bin_help.seek ( 16 );
    header.misctable_count = bin_help.read_uint ( );
    header.unk = bin_help.read_uint ( );;
    header.sheen_strength = bin_help.read_uint ( );
    header.unk_flag = bin_help.read_uint ( );

    int odbp_start = header.OBDP_pointer + 32;

    bin_help.seek ( header.OBDP_pointer );
    SODBPHeader subheader;
    subheader.submagic = bin_help.read_fixed_string ( 4 );
    subheader.substruct_size = bin_help.read_uint ( );
    subheader.unk_id = bin_help.read_uint ( );
    subheader.filesize = bin_help.read_uint ( );

    bin_help.seek ( odbp_start );
    subheader.mesh_pointer = bin_help.read_uint ( );
    subheader.mesh_count = bin_help.read_uint ( );
    subheader.armature_pointer = bin_help.read_uint ( );
    subheader.bone_count = bin_help.read_uint ( );;
    subheader.unkvar = bin_help.read_uint ( );

    bin_help.seek ( odbp_start + subheader.mesh_pointer );
    SMeshHeader* headers = new SMeshHeader [ subheader.mesh_count ];
    for ( int i = 0; i < subheader.mesh_count; i++ )
    {
        SMeshHeader mesh_header;
        mesh_header.info_ptr = bin_help.read_uint ( );
        bin_help.seek ( bin_help.get_pos ( ) + 60 );
        headers [ i ] = mesh_header;
    }

    for ( int i = 0; i < subheader.mesh_count; i++ )
    {
        SMeshHeader mesh_header = headers [ i ];
        bin_help.seek ( odbp_start + mesh_header.info_ptr + 8 );
        mesh_header.vert_ptr = bin_help.read_uint ( );
        mesh_header.vert_count = bin_help.read_uint ( );
        mesh_header.face_ptr = bin_help.read_uint ( );
        mesh_header.face_count = bin_help.read_uint ( );

        cmesh_buffer* mesh = new cmesh_buffer ( );
        mesh->m_face_num = mesh_header.face_count;
        mesh->m_vert_num = mesh_header.vert_count;
        mesh->m_verts = new glm::vec3 [ mesh->m_vert_num ];
        mesh->m_faces = new int [ mesh->m_face_num ];
        mesh->m_uvs = new glm::vec2 [ mesh->m_vert_num ];
        if ( type == eODBP_TYPE_NORMAL ) {
            mesh->m_norms = new glm::vec3 [ mesh->m_vert_num ];
            mesh->m_weights = new glm::vec4 [ mesh->m_vert_num ];
            mesh->set_flag ( e_mesh_attr::normals );
            mesh->set_flag ( e_mesh_attr::weights );
        }
        if ( type == eODBP_TYPE_STAGE ) {
            mesh->m_vert_colors = new glm::vec4 [ mesh->m_vert_num ];
            mesh->set_flag ( e_mesh_attr::colors );
        }
        bin_help.seek ( odbp_start + mesh_header.vert_ptr );
        for ( int v = 0; v < mesh_header.vert_count; v++ )
        {
            float x = bin_help.read_float ( );
            float y = bin_help.read_float ( );
            float z = bin_help.read_float ( );
            mesh->m_verts[ v ] = glm::vec3 ( x, y, z );
            if ( type == eODBP_TYPE_NORMAL )
            {
                float weight1 = bin_help.read_float ( );
                unsigned char b1 = bin_help.read_uchar ( );
                unsigned char b2 = bin_help.read_uchar ( );
                unsigned char b3 = bin_help.read_uchar ( );
                unsigned char b4 = bin_help.read_uchar ( );

                mesh->m_weights [ v ][ 0 ] = weight1;
                mesh->m_weights [ v ][ 1 ] = 1.0f - weight1;
                mesh->m_weights [ v ][ 2 ] = b1;
                mesh->m_weights [ v ][ 3 ] = b2;
            }
            else
            {
            }
            if ( type == eODBP_TYPE_STAGE )
            {
                float b = ( float ) bin_help.read_uchar ( ) / 0xff;
                float g = ( float ) bin_help.read_uchar ( ) / 0xff;
                float r = ( float ) bin_help.read_uchar ( ) / 0xff;
                float a = ( float ) bin_help.read_uchar ( ) / 0xff;
                mesh->m_vert_colors [ v ] = glm::vec4 ( r, g, b, a );
            }
            else
            {
            }
            if ( type == eODBP_TYPE_NORMAL )
            {
                float nx = bin_help.read_float ( );
                float ny = bin_help.read_float ( );
                float nz = bin_help.read_float ( );
                mesh->m_norms [ v ] = glm::vec3 ( nx, ny, nz );
            }
            else
            {
                //mesh->m_norms [ v ] = glm::vec3 ( 0.0, 0.0, 0.0 );
            }
            float uu = bin_help.read_float ( );
            float vv = bin_help.read_float ( );

            mesh->m_uvs [ v ] = glm::vec2 ( uu, vv );
        }
        bin_help.seek ( odbp_start + mesh_header.face_ptr );
        for ( int v = 0; v < mesh_header.face_count; v++ )
        {
            short f = bin_help.read_short ( );
            mesh->m_faces [ v ] = (unsigned int)f;
        }

        std::string name = std::string ( szFileName ) + "_" + std::to_string ( i );
        mesh->m_name = name;
        mesh->set_flag ( e_mesh_attr::tex );
        names.push_back ( cmesh_ref ( name ) );
        mesh->init_buffers ( );
        mesh->set_shaders ( VSh, FSh );
        cengine::get ( )->mesh_man->regist_mesh ( mesh );
    }

    delete [ ] headers;
    
    return names;
}

std::vector <cskel_bone> read_ogre_bones ( const char* filename ) {
    cbinary_helper bin_help = cbinary_helper ( filename );
    SOMEHeader header;
    header.magic = bin_help.read_string ( );
    header.version = bin_help.read_uint ( );
    header.OBDP_pointer = bin_help.read_uint ( );
    bin_help.seek ( 16 );
    header.misctable_count = bin_help.read_uint ( );
    header.unk = bin_help.read_uint ( );;
    header.sheen_strength = bin_help.read_uint ( );
    header.unk_flag = bin_help.read_uint ( );

    int odbp_start = header.OBDP_pointer + 32;


    bin_help.seek ( header.OBDP_pointer );
    SODBPHeader subheader;
    subheader.submagic = bin_help.read_fixed_string ( 4 );
    subheader.substruct_size = bin_help.read_uint ( );
    subheader.unk_id = bin_help.read_uint ( );
    subheader.filesize = bin_help.read_uint ( );

    bin_help.seek ( odbp_start );
    subheader.mesh_pointer = bin_help.read_uint ( );
    subheader.mesh_count = bin_help.read_uint ( );
    subheader.armature_pointer = bin_help.read_uint ( );
    subheader.bone_count = bin_help.read_uint ( );;
    subheader.unkvar = bin_help.read_uint ( );


    std::vector <cskel_bone> bones;

    bin_help.seek ( odbp_start + subheader.armature_pointer );
    for ( int i = 0; i < subheader.bone_count; i++ )
    {
        cskel_bone bone = cskel_bone ( );
        bone.m_index = i;
        int child_ptr = bin_help.read_uint ( );
        int sibling_ptr = bin_help.read_uint ( );
        bin_help.seek ( bin_help.get_pos ( ) + 8 );
        float x = bin_help.read_float ( );
        float y = bin_help.read_float ( );
        float z = bin_help.read_float ( );
        bin_help.seek ( bin_help.get_pos ( ) + 20 );

        glm::vec3 scale;
        scale.x = bin_help.read_float ( );
        scale.y = bin_help.read_float ( );
        scale.z = bin_help.read_float ( );
        bin_help.read_float ( );

        glm::quat rot;
        rot.w = bin_help.read_float ( );
        rot.x = bin_help.read_float ( );
        rot.y = bin_help.read_float ( );
        rot.z = bin_help.read_float ( );

        glm::mat4 translation = glm::translate ( glm::mat4 ( 1.0 ), glm::vec3 ( x, y, z ) );
        glm::mat4 rotation = glm::mat4_cast ( rot );
        bone.m_rest = translation * rotation * glm::scale ( glm::mat4 ( 1.0f ), scale );
        bone.m_rest_pos = glm::vec3 ( x, y, z );
        bone.m_rest_rot = rot;
        bone.m_anim_pos = glm::vec3 ( x, y, z );
        if ( child_ptr > 0 )
        {
            bone.m_child = ( ( child_ptr - subheader.armature_pointer ) / 80 );
        }
        else
        {
            bone.m_child = -1;
        }
        if ( sibling_ptr > 0 )
        {
            bone.m_sibling = ( ( sibling_ptr - subheader.armature_pointer ) / 80 );

        }
        else
        {
            bone.m_sibling = -1;
        }
        bones.push_back ( bone );
    }
    for ( int i = 0; i < subheader.bone_count; i++ )
    {
        if ( bones [ i ].m_child != -1 )
        {
            bones [ bones [ i ].m_child ].m_parent = bones [ i ].m_index;
        }
        if ( bones [ i ].m_sibling != -1 )
        {
            bones [ bones [ i ].m_sibling ].m_parent = bones [ i ].m_parent;
        }
    }

    return bones;
}