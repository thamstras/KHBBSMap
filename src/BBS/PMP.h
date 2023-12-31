#pragma once
#include "..\BTypes.h"
#include "Types.h"

namespace BBS
{

	constexpr uint16 bbs_pmp_ver = 0;
	constexpr uint32 PMP_MAGIC_I = ('P' << 24) | ('M' << 16) | ('P' << 8) | ('\0' << 0);
	constexpr char PMP_MAGIC_C[4] = { 'P', 'M', 'P', '\0' };

#pragma pack(push, 1)

	struct PMP_HDR
	{
		uint32 magic;
		uint16 version;
		uint16 reserved0;
		uint32 reserved1;
		uint8 reserved2[3];
		uint8 flag;

		uint16 instance_max;
		uint16 model_max;
		uint32 reserved3;
		uint16 reserved4;
		uint16 tex_max;
		uint32 p_texList;
	};

	struct PMP_INSTANCE
	{
		FVECTOR3 pos;
		FVECTOR3 rot;
		FVECTOR3 scale;
		uint32 p_model;
		uint32 reserved0;
		uint16 flag;
		uint16 model_idx;
	};

	enum INSTANCE_FLAGS
	{
		FLAG_SKYBOX = 0x1,
		FLAG_SHADOW = 0x4,
		FLAG_UNK0 = 0x8,
		FLAG_UNK1 = 0x10,
		FLAG_UNK2 = 0x20
	};

	struct PMP_TEXINFO
	{
		uint32 p_texture;
		char name[12];
		float scrollX;
		float scrollY;
		uint32 reserved0[2];
	};

#pragma pack(pop)
}