#include "Common.h"
#include <exception>

namespace BBS
{

	// TODO: Determine scale values for the fixed types

	enum BBS_UV_TYPE
	{
		UV_NONE = 0x0,
		UV_UINT8 = 0x1,
		UV_UINT16 = 0x2,
		UV_FLOAT32 = 0x3,
	};

	enum BBS_COLOR_TYPE
	{
		COLOR_NONE = 0x0,
		COLOR_BGR5650 = 0x4,
		COLOR_ABGR5551 = 0x5,
		COLOR_AGBR4444 = 0x6,
		COLOR_AGBR8888 = 0x7,
	};

	enum BBS_NORMAL_TYPE
	{
		NORMAL_NONE = 0x0,
		NORMAL_INT8 = 0x1,
		NORMAL_INT16 = 0x2,
		NORMAL_FLOAT32 = 0x3,
	};

	enum BBS_POSITION_TYPE
	{
		POSITION_NONE = 0x0,
		POSITION_INT8 = 0x1,
		POSITION_INT16 = 0x2,
		POSITION_FLOAT32 = 0x3,
	};

	enum BBS_WEIGHT_TYPE
	{
		WEIGHT_NONE = 0x0,
		WEIGHT_INT8 = 0x1,
		WEIGHT_INT16 = 0x2,
		WEIGHT_FLOAT32 = 0x3,
	};

	enum BBS_INDEX_TYPE
	{
		INDEX_NONE = 0x1,
		INDEX_INT8 = 0x2,
		INDEX_INT16 = 0x3,
	};

	enum BBS_PRIM_TYPE
	{
		PRIM_POINTS = 0,
		PRIM_LINES,
		PRIM_LINE_STRIP,
		PRIM_TRIANGLES,
		PRIM_TRIANGLE_STRIP,
		PRIM_TRIANGLE_FAN,
		PRIM_SPRITES
	};



#pragma pack(push, 1)

	struct PMO_SECTION_FLAGS
	{
		uint32 texCoord : 2;	// BBS_UV_TYPE
		uint32 unknown0 : 2;
		uint32 vertColor : 3;	// BBS_COLOR_TYPE
		uint32 position : 2;	// BBS_POSITION_TYPE
		uint32 skinning : 2;	// BBS_WEIGHT_TYPE? only seems to be set when there are joint weights present?
		uint32 unknown1 : 3;
		uint32 jointCnt : 4;	// maximum joint index used? (index count = joint count + 1 - or just use (jointCnt + skinning)
		uint32 unknown2 : 9;
		uint32 diffuse : 1;	// seems to only be set when a diffuse color is present in header
		uint32 unknown3 : 3;
		uint32 dataType : 4;	// BBS_PRIM_TYPE
	};

	struct PMO_SECTION_HEADER
	{
		uint16 vertexCount;
		uint8  textureID;
		uint8  vertexSize;
		PMO_SECTION_FLAGS dataFlags;	// uint32
		uint8  unknown0x08;
		uint8  triStripCount;
		uint8  unknown0x0A[2];
	};

	struct PMO_TEXTURE_ENTRY
	{
		uint32 dataOffset;
		char   resourceName[0x0C];

		uint32 unknown0x10[4];
	};

	struct PMO_JOINT
	{
		uint16 index;
		uint16 padding0;

		uint16 parent;
		uint16 padding1;

		uint16 unknown0x08; // skinning index?
		uint16 padding2;

		uint32 unknown0x0C;

		// 0x10
		char   name[0x10];
		// 0x20
		float  transform[16];
		float  transformInverse[16];
	};

	struct PMO_SKEL_HEADER
	{
		char   dataTag[4]; // 'BON'
		uint32 unknown0x04;
		uint32 jointCount;
		uint16 unknown0x0C; // number of skinned joints?
		uint16 unknown0x0E; // skinning start index
	};

	struct PMO_HEADER
	{
		// 0x00
		char   fileTag[4]; // 'PMO'
		uint8  unknown0x04[4];
		uint16 textureCount; //
		uint16 unknown0x0A;
		uint32 skeletonOffset;
		// 0x10
		uint32 meshOffset0;
		uint16 triangleCount;
		uint16 vertexCount;
		float  modelScale;
		uint32 meshOffset1;
		// 0x20
		float  boundingBox[8][4];
	};

#pragma pack(pop)

	struct BBS_DrawCall
	{
		GLenum primativeType;
		GLint first;
		GLsizei count;
	};

	GLenum TranslatePrimType(BBS_PRIM_TYPE prim)
	{
		switch (prim)
		{
		case BBS::PRIM_POINTS:
			return GL_POINTS;
		case BBS::PRIM_LINES:
			return GL_LINES;
		case BBS::PRIM_LINE_STRIP:
			return GL_LINE_STRIP;
		case BBS::PRIM_TRIANGLES:
			return GL_TRIANGLES;
		case BBS::PRIM_TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		case BBS::PRIM_TRIANGLE_FAN:
			return GL_TRIANGLE_FAN;
		case BBS::PRIM_SPRITES:
			throw std::invalid_argument("Primative type PRIM_SPRITES is not yet supported!");
		}
	}

	GLenum TranslatePositionType(BBS_POSITION_TYPE pos)
	{
		switch (pos)
		{
		case BBS::POSITION_NONE:
			return 0;
		case BBS::POSITION_INT8:
			return GL_BYTE;
		case BBS::POSITION_INT16:
			return GL_SHORT;
		case BBS::POSITION_FLOAT32:
			return GL_FLOAT;
		}
	}

	GLenum TranslateColorType(BBS_COLOR_TYPE col)
	{
		switch (col)
		{
		case BBS::COLOR_NONE:
			return 0;
		case BBS::COLOR_BGR5650:
			throw std::invalid_argument("Vertex color type COLOR_BGR5650 is not yet supported!");
		case BBS::COLOR_ABGR5551:
			throw std::invalid_argument("Vertex color type COLOR_ABGR5551 is not yet supported!");
		case BBS::COLOR_AGBR4444:
			throw std::invalid_argument("Vertex color type COLOR_AGBR4444 is not yet supported!");
		case BBS::COLOR_AGBR8888:
			break;
		}
	}


	class BBS_MeshSection
	{
	public:
		uint16 vertexCount;
		uint8 textureID;
		uint8 vertexSize;
		PMO_SECTION_FLAGS flags;
		uint8 jointIndices[8];
		uint32 diffuseColor;

		GLuint VBO, VAO;

		BBS_MeshSection(uint8* data, bool hasSkeleton);
	};

	//TODO: TranslateVertexData()
	// Probably needs to be part of BBS_Mesh

	BBS_MeshSection::BBS_MeshSection(uint8* data, bool hasSkeleton)
	{
		PMO_SECTION_HEADER* hdr = (PMO_SECTION_HEADER*)data;
		vertexCount = hdr->vertexCount;
		textureID = hdr->textureID;		// TODO: need to capture the texture header for the rendering info
		vertexSize = hdr->vertexSize;
		flags = hdr->dataFlags;
		uint8 triStripCount = hdr->triStripCount;

		uint8* fp = (uint8*)(hdr + 1);

		if (hasSkeleton)
		{
			for (int i = 0; i < 8; i++)
			{
				jointIndices[i] = *fp;
				fp++;
			}
		}
		else
		{
			for (int i = 0; i < 8; i++) jointIndices[i] = 0;
		}

		if (flags.diffuse)
		{
			diffuseColor = *((uint32*)fp);
			fp += 4;
		}
		else
		{
			diffuseColor = 0;
		}

		std::vector<uint16> triStripLengths;
		if (triStripCount)
		{
			for (int i = 0; i < triStripCount; i++)
			{
				triStripLengths.push_back(*((uint16*)fp));
				fp += 2;
			}
		}

		uint8* vdata = fp;
		size_t vdata_size = vertexCount * vertexSize;

		if (triStripCount && ((BBS_PRIM_TYPE)flags.dataType) != PRIM_TRIANGLE_STRIP)
		{
			// TODO: Error
		}
		if ((BBS_PRIM_TYPE)flags.dataType == PRIM_TRIANGLES && vertexCount % 3 != 0)
		{
			// TODO: Error
		}

		BBS_PRIM_TYPE primative = (BBS_PRIM_TYPE)flags.dataType;
		BBS_POSITION_TYPE posType = (BBS_POSITION_TYPE)flags.position;
		//BBS_NORMAL_TYPE normType = (BBS_NORMAL_TYPE)unkown;
		BBS_COLOR_TYPE colType = (BBS_COLOR_TYPE)flags.vertColor;
		//BBS_
		
		// And now, rendering...
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vdata_size, vdata, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // position

		
	}

	class BBS_Mesh
	{
	public:
		static BBS_Mesh Parse(uint8* PMOChunk);

	private:
		
	};

	// TODO: FILE* version?
	/*BBS_Model BBS_Model::Parse(uint8* PMOChunk)
	{
		if (memcmp(PMOChunk, "PMO\0", 4) != 0)
		{
			// TODO: Error
		}

		//HEADER
		PMO_HEADER* header = (PMO_HEADER*)PMOChunk;


		// TEXTURES
		PMO_TEXTURE_ENTRY* texture_list = (PMO_TEXTURE_ENTRY*)(header + 1);
		for (int i = 0; i < header->textureCount; i++)
		{
			PMO_TEXTURE_ENTRY entry = texture_list[i];
			// TODO
			//if (entry.dataOffset && !renderer.isTextureLoaded(entry.resourceName)) renderer.loadTexture(entry.resourceName);
			// model.textures.push_back(entry.resourceName);
		}


		// SKELETON
		PMO_SKEL_HEADER* skeleton = (PMO_SKEL_HEADER*)(PMOChunk + header->skeletonOffset);
		if (memcmp(skeleton, "BON\0", 4) == 0)
		{
			PMO_JOINT* bone_list = (PMO_JOINT*)(skeleton + 1);
			for (int i = 0; i < skeleton->jointCount; i++)
			{
				PMO_JOINT bone = bone_list[i];
				// TODO
				// model.bones.push_back(bone);
			}
		}


		// MESH


	}*/
}