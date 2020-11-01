#pragma once
#include "Common.h"
#include "BTypes.h"
#include "PsPGeTypes.h"
#include "Texture.h"

namespace PSPRender
{
	struct RenderCommand
	{
		GLenum primativeType;
		GLint first;
		GLsizei count;
	};

	struct RenderList
	{
		uint16 vertexCount;
		uint8 textureId;
		uint8 vertexSize;
		uint32 dataFlags;
		uint8 jointIndices[8];
		uint32 diffuseColor;

		bool useDiffuseColor;
		bool useSkinning;

		GeVTYPE vertexType;
		std::vector<RenderCommand> renderCommands;


		GLuint VAO, VBO;
		Texture *texture;
	};
}