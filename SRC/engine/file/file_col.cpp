#include "File.h"

#include "engine\core\engine.h"
#include "engine\sys\collision\collision.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void read_col_file ( const char* szFileName ) {
	Assimp::Importer Importer;
	Importer.SetPropertyBool ( AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true );
	Importer.SetPropertyBool ( AI_CONFIG_IMPORT_FBX_STRICT_MODE, true );
	const aiScene* lpScene = Importer.ReadFile ( szFileName, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices );
	if ( !lpScene )
		return;

	unsigned int i;
	for ( i = 0; i < lpScene->mNumMeshes; i++ ) {
		aiMesh* lpMesh = lpScene->mMeshes [ i ];
		ccol_mesh* lpColMesh = new ccol_mesh ( );
		unsigned int v;

		lpColMesh->m_verts = new glm::vec3 [ lpMesh->mNumVertices ];
		lpColMesh->m_vert_num = lpMesh->mNumVertices;
		for ( v = 0; v < lpColMesh->m_vert_num; v++ ) {
			lpColMesh->m_verts [ v ].x = lpMesh->mVertices [ v ].x;
			lpColMesh->m_verts [ v ].y = lpMesh->mVertices [ v ].z;
			lpColMesh->m_verts [ v ].z = lpMesh->mVertices [ v ].y;
		}

		lpColMesh->m_faces = new int [ lpMesh->mNumFaces * 3 ];
		lpColMesh->m_face_num = lpMesh->mNumFaces * 3;
		int inx = 0;
		for ( v = 0; v < lpMesh->mNumFaces; v++ ) {
			lpColMesh->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 0 ];
			lpColMesh->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 2 ];
			lpColMesh->m_faces [ inx++ ] = lpMesh->mFaces [ v ].mIndices [ 1 ];
		}
		cengine::get ( )->col_man->regist_col_mesh ( lpColMesh );
	}
}