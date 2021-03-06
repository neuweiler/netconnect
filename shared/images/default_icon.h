#ifdef USE_DEFAULT_ICON_COLORS
const ULONG default_icon_colors[24] =
{
	0x9a9a9a9a,0x9a9a9a9a,0x9a9a9a9a,
	0x00000000,0x00000000,0x00000000,
	0xefefefef,0xefefefef,0xefefefef,
	0x55555555,0x75757575,0xaaaaaaaa,
	0x8a8a8a8a,0x8a8a8a8a,0x8a8a8a8a,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0xaaaaaaaa,0x9a9a9a9a,0x8a8a8a8a,
	0xefefefef,0xaaaaaaaa,0x9a9a9a9a,
};
#endif

#define DEFAULT_ICON_WIDTH        45
#define DEFAULT_ICON_HEIGHT       28
#define DEFAULT_ICON_DEPTH         3
#define DEFAULT_ICON_COMPRESSION   1
#define DEFAULT_ICON_MASKING       2

#ifdef USE_DEFAULT_ICON_HEADER
const struct BitMapHeader default_icon_header =
{ 45,28,0,0,3,2,1,0,5,22,22,1024,768 };
#endif

#ifdef USE_DEFAULT_ICON_BODY
const UBYTE default_icon_body[583] = {
0x05,0xfb,0xbf,0xef,0xdf,0xff,0xf8,0x00,0x04,0xfe,0x20,0xff,0x00,0x05,0xfb,
0xc0,0x1f,0xdf,0xff,0xf8,0x05,0xbe,0xff,0xfb,0xff,0xff,0xf8,0x02,0x40,0x1f,
0xc0,0xfe,0x00,0x05,0xbf,0x00,0x07,0xff,0xff,0xf8,0x05,0xfb,0x1f,0xfe,0xbb,
0xff,0x78,0x05,0x02,0xf6,0xfa,0x04,0x00,0x00,0x05,0xfc,0xe9,0x01,0xfb,0xff,
0xf8,0x05,0xf6,0x6f,0xff,0x7f,0xff,0xf8,0x02,0x01,0xff,0xfc,0xfe,0x00,0x05,
0xf9,0xe0,0x00,0xff,0xff,0xf8,0x05,0xee,0xb7,0xff,0xab,0xbf,0xf8,0x02,0x03,
0xfd,0xff,0xfe,0x00,0x05,0xf0,0xba,0x00,0x7f,0xff,0xf8,0x05,0xdd,0x17,0xff,
0xdf,0xf7,0xb0,0x05,0x07,0xfb,0xff,0x80,0x08,0x08,0x05,0xe3,0x14,0x00,0x3f,
0xf7,0xf0,0x05,0xfc,0x37,0xfb,0x63,0xff,0xf8,0x05,0x2f,0xfd,0xff,0xa4,0x00,
0x00,0x05,0xc2,0x3a,0x07,0x9b,0xff,0xf8,0x05,0xbf,0x5f,0xf6,0x3b,0xff,0xb8,
0x05,0x0b,0xf7,0xff,0xd0,0x00,0x40,0x05,0xc5,0x58,0x0f,0xcf,0xff,0xb8,0x05,
0x7e,0xef,0xf7,0x35,0x7d,0xf8,0x05,0x5f,0xfb,0xff,0xd0,0x02,0x00,0x05,0x81,
0xf4,0x4f,0xcf,0xfd,0xf8,0x05,0xff,0x7f,0xea,0x9b,0xff,0xf8,0x05,0x1d,0xff,
0xff,0xe8,0x00,0x00,0x05,0x82,0xe0,0x1f,0xe7,0xff,0xf8,0x05,0x7f,0xef,0xdf,
0x19,0x7f,0xf8,0x05,0x3f,0xff,0xff,0xe8,0x80,0x00,0x05,0x80,0x70,0x3f,0xe7,
0x7f,0xf8,0x05,0x7f,0xf7,0xbd,0x1f,0xff,0xf8,0x05,0x3f,0xff,0xff,0xec,0x00,
0x00,0x05,0x80,0x78,0x7f,0xe3,0xff,0xf8,0x05,0x7f,0x7b,0xf6,0x19,0x7f,0xb8,
0x05,0x3f,0xff,0xff,0xe8,0x00,0x40,0x05,0x80,0xfc,0x0f,0xe7,0xff,0xb8,0x05,
0xfe,0xfd,0xfb,0x7b,0xff,0xf8,0x05,0x1f,0xff,0xff,0xc8,0x00,0x00,0x05,0x81,
0xfe,0x06,0x87,0xff,0xf8,0x05,0x7f,0x1d,0xf5,0xf5,0x5f,0xf8,0x05,0x5f,0xff,
0xff,0x54,0x20,0x00,0x05,0x80,0xfe,0x1e,0x0b,0xdf,0xf8,0x05,0xbf,0xeb,0xc9,
0xfb,0xff,0xe8,0x05,0x0f,0xff,0xff,0x98,0x00,0x10,0x05,0xc0,0x1c,0x3e,0x04,
0x7f,0xe8,0x05,0xff,0xf3,0xf1,0xe7,0x6b,0xf8,0x05,0x27,0xff,0x7e,0xa3,0xa0,
0x00,0x05,0xc0,0x0c,0x3e,0x1b,0x9f,0xf8,0x05,0xdf,0xff,0x83,0xfa,0xbe,0xf8,
0x05,0x05,0xfd,0xfd,0x37,0xc0,0x00,0x05,0xe0,0x00,0x7c,0x03,0xc1,0xf8,0x05,
0xef,0xff,0xff,0xd5,0x6d,0xa8,0x05,0x01,0x7f,0x54,0x4e,0xd6,0x10,0x05,0xf0,
0x00,0x00,0x26,0xce,0x68,0x05,0xf7,0xff,0xff,0xeb,0xfb,0x58,0x05,0x00,0xaa,
0xab,0x1d,0x2f,0x80,0x05,0xf8,0x00,0x03,0x0c,0x3b,0xb8,0x05,0xfb,0xff,0xfd,
0xf6,0xb7,0xe8,0x05,0x02,0x15,0x43,0xf8,0x5d,0xa0,0x05,0xfc,0x00,0x03,0xfb,
0x37,0x98,0x05,0xf4,0xff,0xfe,0x1f,0x7f,0x20,0xff,0x00,0x03,0x05,0xf4,0xab,
0xc0,0x05,0xff,0x00,0x01,0xe2,0x7f,0xd8,0x05,0xfd,0x5f,0xfb,0xfb,0x7e,0xb8,
0x05,0x00,0x40,0x3a,0x08,0xd7,0x50,0x05,0xff,0xa0,0x04,0x06,0x7f,0x88,0x05,
0xda,0xaf,0xd5,0xf7,0x3c,0x70,0x05,0x20,0x2f,0xd5,0xf4,0xed,0x50,0x05,0xdf,
0xd0,0x2a,0x0a,0xbe,0x88,0x05,0xff,0x52,0xaa,0x97,0x90,0xe8,0x05,0x00,0x02,
0xa0,0x00,0xe2,0xa0,0x05,0xff,0xfd,0x5f,0xfe,0x7d,0x18,0x05,0xff,0xfe,0xff,
0x6a,0xc1,0xd0,0x05,0x00,0x01,0x00,0x00,0x5d,0x40,0x05,0xff,0xfe,0xff,0xff,
0x3a,0x38,0x05,0x5f,0xff,0xff,0xf7,0x7f,0xa8,0x00,0xa0,0xfd,0x00,0x00,0x80,
0x00,0x5f,0xfe,0xff,0x01,0x80,0x78,0xff,0xff,0x03,0xfd,0xde,0xbf,0x58,0xff,
0x00,0x03,0x02,0x01,0x3f,0x00,0xff,0xff,0x03,0xfd,0xfe,0xc0,0xf8, };
#endif
