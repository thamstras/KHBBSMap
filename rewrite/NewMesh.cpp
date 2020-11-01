#include "Common.h"
#include <exception>
#include <fstream>

#include "BBSTypes.h"
#include "Texture.h"
#include "Shader.h"
#include "Render.h"

// Polyfill for c++20::bit_cast
template <class To, class From>
typename std::enable_if_t<
	sizeof(To) == sizeof(From) &&
	std::is_trivially_copyable_v<From> &&
	std::is_trivially_copyable_v<To>,
	To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept
{
	static_assert(std::is_trivially_constructible_v<To>,
		"This implementation additionally requires destination type to be trivially constructible");

	To dst;
	std::memcpy(&dst, &src, sizeof(To));
	return dst;
}

namespace BBS
{
	class PMO_Skeleton;
	class PMO_Section;
	class PMO_TextureRecord;
	class PMO_File;
	

	class PMO_File
	{
	public:
		std::shared_ptr<RenderContext> renderContext;

		PMO_HEADER header;

		std::vector<PMO_TextureRecord> textureList;
		PMO_Skeleton* skeleton;

		std::vector<PMO_Section> firstPass;
		std::vector<PMO_Section> secondPass;

		PMO_File(FILE* data);
		void Draw();
		
		std::shared_ptr<Texture> GetTextureIndex(uint8 idx);
	};


	// TODO: Hook up texture
	// TODO: Hook up shader
	// TODO: VBO, VAO
	// TODO: Draw
	class PMO_Section
	{
	public:
		bool useSkeleton;
		float modelScale;

		PMO_File* parent;
		
		PMO_MESH_HEADER header;
		uint8 boneIndices[8];
		uint8* vertexData;
		uint32 diffuseColor;
		
		GLuint VBO, VAO;
		GLenum primativeType;
		std::vector<GLsizei> primativeCounts;
		std::shared_ptr<Texture> texObj;
		std::shared_ptr<Shader> shaderObj;

		PMO_Section(PMO_File* parent, FILE* data, bool useSkeleton, float modelScale);
		void Draw(const glm::mat4& transform, const glm::mat4& view, const glm::mat4& projection);

	private:
		void Construct();
		void SelectShader();
	};

	PMO_File::PMO_File(FILE* data)
	{
		long baseOff = ftell(data);
		
		size_t rcount = fread_s(&header, sizeof(PMO_HEADER), sizeof(PMO_HEADER), 1, data);
		if (rcount < 1)
		{
			throw new std::exception("File error reading PMO_File header");
		}

		if (memcmp(&PMO_MAGIC_C, &header.fileTag, sizeof(PMO_MAGIC_C)) != 0)
		{
			throw new std::exception("Invalid PMO_File!");
		}

		for (int i = 0; i < header.textureCount; i++)
		{
			// Read texture records
		}

		if (header.skeletonOffset != 0)
		{
			fseek(data, baseOff + header.skeletonOffset, SEEK_SET);
			// Read skeleton
		}

		fseek(data, baseOff + header.meshOffset0, SEEK_SET);
		int c;
		c = fgetc(data);
		while (c != NULL)
		{
			ungetc(c, data);
			firstPass.push_back(PMO_Section(this, data, (bool)header.skeletonOffset, header.modelScale));
			c = fgetc(data);
		}
		ungetc(c, data);

		fseek(data, baseOff + header.meshOffset1, SEEK_SET);
		int c;
		c = fgetc(data);
		while (c != NULL)
		{
			ungetc(c, data);
			secondPass.push_back(PMO_Section(this, data, (bool)header.skeletonOffset, header.modelScale));
			c = fgetc(data);
		}
		ungetc(c, data);
	}
	
	PMO_Section::PMO_Section(PMO_File* parent, FILE* data, bool useSkeleton, float modelScale) : useSkeleton(useSkeleton), modelScale(modelScale)
	{
		this->parent = parent;

		size_t rcount = fread_s(&header, sizeof(PMO_MESH_HEADER), sizeof(PMO_MESH_HEADER), 1, data);
		if (rcount < 1)
		{
			throw new std::exception("File error reading PMO_Section header");
		}
		
		if (useSkeleton)
		{
			rcount = fread_s(&boneIndices[0], sizeof(boneIndices), sizeof(uint8), 8, data);
			if (rcount < 8)
			{
				throw new std::exception("File error reading PMO_Section bone indices");
			}
		}

		if (header.dataFlags.diffuse & 1)
		{
			rcount = fread_s(&diffuseColor, sizeof(diffuseColor), sizeof(diffuseColor), 1, data);
			if (rcount < 1)
			{
				throw new std::exception("File error reading PMO_Section diffuseColor");
			}
		}

		size_t vdata_siz = header.vertexCount * header.vertexSize;
		vertexData = new uint8[vdata_siz];
		rcount = fread_s(&vertexData, vdata_siz, header.vertexSize, header.vertexCount, data);
		if (rcount < header.vertexCount)
		{
			throw new std::exception("File error reading PMO_Section vertex data");
		}

		if (header.triStripCount > 0)
		{
			std::vector<uint16> triCounts = std::vector<uint16>(header.triStripCount);
			rcount = fread_s(triCounts.data(), triCounts.capacity() * sizeof(uint16), sizeof(uint16), header.triStripCount, data);
			if (rcount < header.triStripCount)
			{
				throw new std::exception("File error reading PMO_Section tri strip data");
			}
			for (int i = 0; i < header.triStripCount; i++) primativeCounts.push_back(static_cast<GLsizei>(triCounts[i]));
		}
		else
		{
			primativeCounts.push_back(header.vertexCount);
		}

		Construct();
	}

	void PMO_Section::Construct()
	{
		std::vector<float> data;
		
		uint8* vertexStart = vertexData;
		int dptr;
		for (int vi = 0; vi < header.vertexCount; vi++)
		{
			dptr = 0;

			switch (header.dataFlags.weights)
			{
			case 0: // NONE
				break;
			case 1: // 8-bit fixed
				for (int ji = 0; ji < header.dataFlags.skinning + 1; ji++)
				{
					uint8 weight = vertexStart[dptr++];
					data.push_back((float)weight / 255.0f);
				}
			default:
				std::cerr << "[PMO Section] Unknown weights type: " << header.dataFlags.weights << std::endl;
				assert(false);
			}

			// TEXTURE COORDS
			switch (header.dataFlags.texCoord)
			{
			case 0:	// NONE
				break;
			case 1:	// Normalized INT
			{
				uint8 tex[2];
				tex[0] = vertexStart[dptr++];
				tex[1] = vertexStart[dptr++];
				data.push_back((float)tex[0] / 127.0f);
				data.push_back((float)tex[1] / 127.0f);
			}
				break;
			case 2:	// Normalized SHORT
			{
				assert(dptr % 2 == 0);
				uint16 tex[2];
				tex[0] = vertexStart[dptr++] | (vertexStart[dptr++] << 8);
				tex[1] = vertexStart[dptr++] | (vertexStart[dptr++] << 8);
				data.push_back((float)tex[0] / 32767.0f);
				data.push_back((float)tex[1] / 32767.0f);
			}
				break;
			case 3:	// float
			{
				static_assert(sizeof(uint32) == sizeof(float));
				assert(dptr % 4 == 0);
				uint32 tex[2];
				tex[0] = vertexStart[dptr++] | (vertexStart[dptr++] << 8) | (vertexStart[dptr++] << 16) | (vertexStart[dptr++] << 24);
				tex[1] = vertexStart[dptr++] | (vertexStart[dptr++] << 8) | (vertexStart[dptr++] << 16) | (vertexStart[dptr++] << 24);
				data.push_back(bit_cast<uint32, float>(tex[0]));
				data.push_back(bit_cast<uint32, float>(tex[1]));
			}
				break;
			default:
				std::cerr << "[PMO Section] Unknown texCoord type: " << header.dataFlags.texCoord << std::endl;
				assert(false);
			}

			// COLOR
			switch (header.dataFlags.color)
			{
			case 0:
			{
				if (header.dataFlags.diffuse)
				{
					uint8 c[4];
					c[0] = (diffuseColor >> 0) & 0xFF;
					c[1] = (diffuseColor >> 8) & 0xFF;
					c[2] = (diffuseColor >> 16) & 0xFF;
					c[3] = (diffuseColor >> 24) & 0xFF;
					data.push_back((float)c[0] / 128.0f);
					data.push_back((float)c[1] / 128.0f);
					data.push_back((float)c[2] / 128.0f);
					data.push_back((float)c[3] / 255.0f);
				}
				else
				{
					data.push_back((float)0x80 / 128.0f);
					data.push_back((float)0x80 / 128.0f);
					data.push_back((float)0x80 / 128.0f);
					data.push_back((float)0xFF / 128.0f);
				}
			}
				break;
			case 7:	// ABGR-8888
			{
				uint8 col[4];
				col[0] = vertexStart[dptr++];	col[1] = vertexStart[dptr++];
				col[2] = vertexStart[dptr++];	col[3] = vertexStart[dptr++];
				data.push_back((float)col[0] / 128.0f);
				data.push_back((float)col[1] / 128.0f);
				data.push_back((float)col[2] / 128.0f);
				data.push_back((float)col[3] / 255.0f);
			}
				break;
			default:
				std::cerr << "[PMO Section] Unknown color type: " << header.dataFlags.color << std::endl;
				assert(false);
			}

			switch (header.dataFlags.position)
			{
			case 2:	// Normalized Short
			{
				assert(dptr % 2 == 0);
				uint16 pos[3];
				pos[0] = vertexStart[dptr++] | (vertexStart[dptr++] << 8);
				pos[1] = vertexStart[dptr++] | (vertexStart[dptr++] << 8);
				pos[2] = vertexStart[dptr++] | (vertexStart[dptr++] << 8);
				data.push_back(((float)pos[0] / 32767.0f) * modelScale);
				data.push_back(((float)pos[1] / 32767.0f) * modelScale);
				data.push_back(((float)pos[2] / 32767.0f) * modelScale);
			}
			break;
			case 3:	// float
			{
				static_assert(sizeof(uint32) == sizeof(float));
				assert(dptr % 4 == 0);
				uint32 pos[3];
				pos[0] = vertexStart[dptr++] | (vertexStart[dptr++] << 8) | (vertexStart[dptr++] << 16) | (vertexStart[dptr++] << 24);
				pos[1] = vertexStart[dptr++] | (vertexStart[dptr++] << 8) | (vertexStart[dptr++] << 16) | (vertexStart[dptr++] << 24);
				pos[2] = vertexStart[dptr++] | (vertexStart[dptr++] << 8) | (vertexStart[dptr++] << 16) | (vertexStart[dptr++] << 24);
				data.push_back(bit_cast<uint32, float>(pos[0]));
				data.push_back(bit_cast<uint32, float>(pos[1]));
				data.push_back(bit_cast<uint32, float>(pos[2]));
			}
			break;
			default:
				std::cerr << "[PMO Section] Unknown position type: " << header.dataFlags.position << std::endl;
				assert(false);
			}

		}

		switch (header.dataFlags.primative)
		{
		case 0x3: primativeType = GL_TRIANGLES; break;
		case 0x4: primativeType = GL_TRIANGLE_STRIP; break;
		default:
			std::cerr << "[PMO Section] Unknown primative type: " << header.dataFlags.primative << std::endl;
			assert(false);
		}

		texObj = parent->GetTextureIndex(header.textureID);
		SelectShader();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), (void*)data.data(), GL_STATIC_DRAW);

		// TODO: This will need updating if/when we support untextured/uncolored meshes
		GLint position = glGetUniformLocation(shaderObj->ID, "position");
		GLint color = glGetUniformLocation(shaderObj->ID, "color");
		GLint tex = glGetUniformLocation(shaderObj->ID, "tex");

		glEnableVertexAttribArray(position);
		glEnableVertexAttribArray(color);
		glEnableVertexAttribArray(tex);

		glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(0 * sizeof(float)));
		glVertexAttribPointer(color,    4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(tex,      2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));

	}

	void PMO_Section::Draw(const glm::mat4& transform, const glm::mat4& view, const glm::mat4& projection)
	{
		shaderObj->use();
		shaderObj->setMat4("model", transform);
		shaderObj->setMat4("view", view);
		shaderObj->setMat4("projection", projection);
		
		glBindVertexArray(VAO);
		
		//if (twoSided) glDisable(GL_CULL_FACE);

		GLint vptr = 0;
		for (auto itr = primativeCounts.begin(); itr < primativeCounts.end(); itr++)
		{
			glDrawArrays(primativeType, vptr, *itr);
			vptr += *itr;
		}

		//if (twoSided && !parent->renderContext->render_disable_cull) glEnable(GL_CULL_FACE);

		// context stats
	}

	void PMO_Section::SelectShader()
	{
		if (header.dataFlags.texCoord == 0)
		{
			std::cerr << "[PMO Section] PMO Sections with no texture coords are not yet supported." << std::endl;
			assert(false);
		}

		if (header.dataFlags.weights != 0)
		{
			std::cerr << "[PMO Section] PMO Sections using vertex weights/skinning are not yet supported." << std::endl;
			assert(false);
		}

		// We don't yet support textureless, we force a vcolor and don't yet support weights
		// So the only option is [lighting]_vcol_tex
		std::string vertexRequire = "_vcol_tex";
		std::string lighting;
		if (parent->renderContext->render_lighting_enabled) lighting = "default_lit";
		else lighting = "unlit";
		std::string shader_name = lighting + vertexRequire;
		shaderObj = parent->renderContext->shaderLibrary->GetShader(shader_name);
	}
}