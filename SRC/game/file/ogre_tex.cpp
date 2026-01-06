#include "file.h"

#include "engine\file\binary_helper.h"
#include "engine\sys\render\render.h"
#include "engine\core\engine.h"

void unswizzle ( unsigned char* palette )
{
    unsigned char temp [ 256 * 4 ];

    for ( int k = 0; k < 8; ++k )
    {
        for ( int j = 0; j < 2; ++j )
        {
            for ( int i = 0; i < 8; ++i )
            {
                int dst0 = ( k * 32 + j * 16 + i ) * 4;
                int dst1 = dst0 + 8 * 4;

                int src0 = ( k * 32 + 8 * j + i ) * 4;
                int src1 = ( k * 32 + 8 * j + 16 + i ) * 4;

                memcpy ( &temp [ dst0 ], &palette [ src0 ], 4 );
                memcpy ( &temp [ dst1 ], &palette [ src1 ], 4 );
            }
        }
    }

    memcpy ( palette, temp, 256 * 4 );
}

void unswizzle16 ( unsigned char* palette )
{
    unsigned char temp [ 16 * 4 ];

    for ( int j = 0; j < 2; ++j )
    {
        for ( int i = 0; i < 8; ++i )
        {
            int dst = j * 8 + i;
            int src = 8 * j + i;
            if ( src < 16 && dst < 16 )
            {
                memcpy ( &temp [ dst * 4 ], &palette [ src * 4 ], 4 );
            }
        }
    }

    memcpy ( palette, temp, 16 * 4 );
}

std::vector<ctex_ref> read_ogre_tex_file ( const char* filename ) {
	cbinary_helper bin_help = cbinary_helper ( filename );
    std::string magic = bin_help.read_fixed_string ( 4 );
    if ( magic != "TXBP" ) {
        throw std::runtime_error ( "Invalid TXB file format" );
    }

    std::vector<ctex_ref> names;

    int TextureCount = bin_help.read_uint ( );
    if ( TextureCount > 1 )
    {
        throw std::runtime_error ( "Unsupported format, multiple textures" );
    }
    bin_help.seek ( 0x20 );
    for ( unsigned int i = 0; i < TextureCount; ++i ) {
        std::string name = std::string ( filename ) + "_" + std::to_string ( i );
        names.push_back ( ctex_ref ( name ) );
        ctex_buffer* buffer = new ctex_buffer ( );
        buffer->m_name = name;
        int size = bin_help.read_int ( );

        int res = bin_help.read_int ( );
        buffer->m_width = res;
        buffer->m_height = res;
        bin_help.read_int ( );

        int format = bin_help.read_int ( );
        bin_help.read_int ( );
        bin_help.read_int ( );
        bin_help.read_int ( );
        bin_help.read_int ( );


        if ( format == 0x15 )
        {
            int RealTexSize = buffer->m_width * buffer->m_height * 4;
            unsigned char* TexColors = new unsigned char [ 256 * 4 ];
            buffer->m_buffer = new unsigned char [ RealTexSize ];
            unsigned char* Indices = new unsigned char [ buffer->m_width * buffer->m_height ];
            for ( int e = 0; e < 256 ; e++ )
            {
                TexColors [ e * 4 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 1 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 2 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 3 ] = bin_help.read_uchar ( );
            }
            unswizzle ( TexColors );
            for ( int e = 0; e < buffer->m_width * buffer->m_height; e++ )
            {
                unsigned char Index = bin_help.read_uchar ( );
                Indices [ e ] = Index;
            }
            for ( int e = 0; e < buffer->m_width * buffer->m_height; e++  )
            {
                unsigned char Index = Indices [ e ];
                
				buffer->m_buffer [ e * 4 ] = TexColors [ Index * 4 ];
                buffer->m_buffer [ e * 4 + 1 ] = TexColors [ Index * 4 + 1 ];
                buffer->m_buffer [ e * 4 + 2 ] = TexColors [ Index * 4 + 2 ];
				buffer->m_buffer [ e * 4 + 3 ] = TexColors [ Index * 4 + 3 ];
            }

            delete [ ] TexColors;
            delete [ ] Indices;
        }
        else if ( format == 0x14 )
        {
            int RealTexSize = buffer->m_width * buffer->m_height * 4;
            unsigned char* TexColors = new unsigned char [ 16 * 4 ];
            buffer->m_buffer = new unsigned char [ RealTexSize ];

            for ( int e = 0; e < 16; e++ ) {
                TexColors [ e * 4 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 1 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 2 ] = bin_help.read_uchar ( );
                TexColors [ e * 4 + 3 ] = bin_help.read_uchar ( );
            }

            unswizzle16 ( TexColors );

            int indexBufferSize = ( buffer->m_width * buffer->m_height + 1 ) / 2;
            unsigned char* packedIndices = new unsigned char [ indexBufferSize ];

            for ( int e = 0; e < indexBufferSize; e++ ) {
                packedIndices [ e ] = bin_help.read_uchar ( );
            }

            for ( int e = 0; e < buffer->m_width * buffer->m_height; e++ ) {
                unsigned char packed = packedIndices [ e / 2 ];
                unsigned char index;

                if ( e % 2 == 0 ) {
                    index = packed & 0x0F;
                }
                else {
                    index = ( packed >> 4 ) & 0x0F;
                }

                index = index & 0x0F;

                buffer->m_buffer [ e * 4 ] = TexColors [ index * 4 ];
                buffer->m_buffer [ e * 4 + 1 ] = TexColors [ index * 4 + 1 ];
                buffer->m_buffer [ e * 4 + 2 ] = TexColors [ index * 4 + 2 ];
                buffer->m_buffer [ e * 4 + 3 ] = TexColors [ index * 4 + 3 ];
            }
            delete [ ] TexColors;
            delete [ ] packedIndices;
        }
        else
        {
            throw std::runtime_error ( "Unsupported buffer format" );
        }

        buffer->init_buffer ( );
        cengine::get ( )->tex_man->regist_tex ( buffer );

        return names;
    }
}