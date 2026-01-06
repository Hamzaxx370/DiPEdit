#include "File.h"

#include "engine\core\engine.h"
#include "engine\sys\render\render.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// Load mesh data from an FBX file
std::vector<cmesh_ref> read_mesh_file ( const char* szFileName, const char* VSh, const char* FSh ) {
	std::vector<cmesh_ref> names;

	Assimp::Importer Importer;
	Importer.SetPropertyBool ( AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true );
	Importer.SetPropertyBool ( AI_CONFIG_IMPORT_FBX_STRICT_MODE, true );
	const aiScene* lpScene = Importer.ReadFile ( szFileName, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices );
	if ( !lpScene )
		return names;

	unsigned int i;
	for ( i = 0; i < lpScene->mNumMeshes; i++ ) {
		aiMesh* lpMesh = lpScene->mMeshes [ i ];
		cmesh_buffer* pMBuf = new cmesh_buffer ( );
		unsigned int v;
		
		// Copy mesh data from fbx to a custom container
		char* copy = new char [ lpMesh->mName.length + 1 ];
		strcpy ( copy, lpMesh->mName.C_Str ( ) );

		pMBuf->m_name = std::string ( copy );
		names.push_back ( cmesh_ref ( std::string ( copy ) ) );

		pMBuf->m_verts = new glm::vec3 [ lpMesh->mNumVertices ];
		pMBuf->m_vert_num = lpMesh->mNumVertices;
		for ( v = 0; v < pMBuf->m_vert_num; v++ ) {
			pMBuf->m_verts [ v ].x = lpMesh->mVertices [ v ].x;
			pMBuf->m_verts [ v ].y = lpMesh->mVertices [ v ].z;
			pMBuf->m_verts [ v ].z = lpMesh->mVertices [ v ].y;
		}
		
		pMBuf->m_faces = new int [ lpMesh->mNumFaces * 3 ];
		pMBuf->m_face_num = lpMesh->mNumFaces * 3;
		int inx = 0;
		for ( v = 0; v < lpMesh->mNumFaces; v++) {
			pMBuf->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 0 ];
			pMBuf->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 2 ];
			pMBuf->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 1 ];
		}
		if ( lpMesh->HasNormals ( ) ) {
			pMBuf->set_flag ( e_mesh_attr::normals );
			pMBuf->m_norms = new glm::vec3 [ lpMesh->mNumVertices ];
			for ( v = 0; v < pMBuf->m_vert_num; v++ ) {
				pMBuf->m_norms [ v ].x = lpMesh->mNormals [ v ].x;
				pMBuf->m_norms [ v ].y = lpMesh->mNormals [ v ].z;
				pMBuf->m_norms [ v ].z = lpMesh->mNormals [ v ].y;
			}
		}
		pMBuf->init_buffers ( );
		pMBuf->set_shaders ( VSh, FSh );
		cengine::get ( )->mesh_man->regist_mesh ( pMBuf );
	}

	return names;
}