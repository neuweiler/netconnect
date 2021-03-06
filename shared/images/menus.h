#ifdef USE_MENUS_COLORS
const ULONG menus_colors[24] =
{
	0x96969696,0x96969696,0x96969696,
	0x00000000,0x00000000,0x00000000,
	0xffffffff,0xffffffff,0xffffffff,
	0x3d3d3d3d,0x65656565,0xa2a2a2a2,
	0x79797979,0x79797979,0x79797979,
	0xaeaeaeae,0xaeaeaeae,0xaeaeaeae,
	0xaaaaaaaa,0x92929292,0x7d7d7d7d,
	0xffffffff,0xaaaaaaaa,0x96969696,
};
#endif

#define MENUS_WIDTH        26
#define MENUS_HEIGHT       17
#define MENUS_DEPTH         3
#define MENUS_COMPRESSION   1
#define MENUS_MASKING       2

#ifdef USE_MENUS_HEADER
const struct BitMapHeader menus_header =
{ 26,17,32,48,3,2,1,0,0,22,22,1024,768 };
#endif

#ifdef USE_MENUS_BODY
const UBYTE menus_body[233] = {
0xfe,0xff,0x00,0x80,0xfd,0x00,0xfd,0x00,0x03,0xad,0xd7,0xfe,0x80,0x03,0x44,
0x10,0x00,0x00,0x03,0x3b,0xef,0xff,0x40,0x03,0x80,0x00,0x00,0x80,0xfd,0x00,
0x03,0x7f,0xff,0xff,0x40,0x03,0x4b,0x7f,0xff,0x80,0x03,0x21,0x00,0x00,0x80,
0x03,0x9e,0x80,0x00,0x40,0x03,0x5f,0x7f,0xff,0x00,0x03,0x20,0x3f,0xff,0x00,
0x03,0x1f,0x80,0x00,0xc0,0x03,0x4b,0x60,0x00,0x00,0x03,0x29,0x20,0x00,0x00,
0x03,0x16,0x9f,0xff,0x80,0x03,0x5f,0x60,0x00,0x00,0xff,0x20,0xff,0x00,0x03,
0x1f,0x90,0x00,0x00,0x03,0x77,0x7f,0x00,0x00,0xfd,0x00,0x03,0x3f,0x80,0x80,
0x00,0x03,0x5e,0x16,0x80,0x00,0x03,0x21,0xe8,0x00,0x00,0x03,0x1e,0x17,0x40,
0x00,0x03,0x5e,0xff,0x80,0x00,0x00,0x2d,0xfe,0x00,0x03,0x12,0xff,0x60,0x00,
0x03,0x7e,0xb6,0xc0,0x00,0x03,0x01,0x12,0x40,0x00,0x03,0x3e,0xed,0x20,0x00,
0x03,0x50,0xfe,0x80,0x00,0x00,0x21,0xfe,0x00,0x03,0x1e,0xff,0x60,0x00,0x03,
0x3f,0xfe,0xc0,0x00,0x03,0x00,0x02,0x40,0x00,0x03,0x41,0xfd,0x20,0x00,0x03,
0x16,0xa0,0xc0,0x00,0x03,0x15,0x00,0x40,0x00,0x03,0x28,0xff,0x20,0x00,0x03,
0x03,0xff,0xc0,0x00,0x03,0x02,0x00,0xc0,0x00,0x03,0x1c,0x00,0x20,0x00,0x03,
0x00,0x5f,0x80,0x00,0x03,0x00,0x5f,0x80,0x00,0x03,0x00,0xa0,0x60,0x00,0xfd,
0x00,0xfd,0x00,0x03,0x00,0x7f,0xc0,0x00, };
#endif
