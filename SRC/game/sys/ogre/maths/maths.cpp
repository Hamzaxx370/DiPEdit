#include "maths.h"

static unsigned int rnd_seed = 1;

double frandom ( ) {
    rnd_seed = rnd_seed * 0x55 + 0x7f;
    unsigned int rnd16 = rnd_seed & 0xffff;
    return static_cast< double >( rnd16 ) / 65535.0;
}

unsigned int random ( ) {
    rnd_seed = rnd_seed * 0x55 + 0x7f;
    return rnd_seed & 0x7fff;
}

struct ssin_cos {
    float sin;
    float cos;
};

static std::array<ssin_cos, 4096> s_sincos_table;

void init_sincos ( ) {
    for ( int i = 0; i < 4096; i++ ) {
        float angle = i * ( 2.0f * std::numbers::pi / 4096.0f );
        s_sincos_table [ i ] = { std::sin ( angle ), std::cos ( angle ) };
    }
}

void sin_cos ( float* out_sin, float* out_cos, unsigned short angle ) {
    // angle is a 0-65535 value representing 0-360 degrees
    unsigned int index = ( angle & 0xffff  ) >> 4;
    index = index & 0xfff; // Wrap around
    *out_sin = s_sincos_table [ index ].sin;
    *out_cos = s_sincos_table [ index ].cos;
}


void init_ogre_maths ( ) {
    init_sincos ( );
}