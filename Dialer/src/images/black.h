#ifdef USE_BLACK_COLORS
const ULONG black_colors[12] =
{
	0x9a9a9a9a,0x9a9a9a9a,0x9a9a9a9a,
	0x00000000,0x00000000,0x00000000,
	0xf3f3f3f3,0xf3f3f3f3,0xf3f3f3f3,
	0x51515151,0x51515151,0x51515151,
};
#endif

#define BLACK_WIDTH         7
#define BLACK_HEIGHT        7
#define BLACK_DEPTH         2
#define BLACK_COMPRESSION   1
#define BLACK_MASKING       2

#ifdef USE_BLACK_HEADER
const struct BitMapHeader black_header =
{ 7,7,0,0,2,2,1,0,0,22,22,640,480 };
#endif

#ifdef USE_BLACK_BODY
const UBYTE black_body[40] = {
0x01,0x38,0x00,0xff,0x00,0x01,0x78,0x00,0x01,0x3c,0x00,0x01,0xdc,0x00,0x01,
0x7e,0x00,0x01,0xfc,0x00,0x01,0x7e,0x00,0x01,0xfc,0x00,0x01,0x7e,0x00,0x01,
0x78,0x00,0x01,0x3c,0x00,0xff,0x00,0x01,0x38,0x00, };
#endif
