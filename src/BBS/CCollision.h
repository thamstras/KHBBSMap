#pragma once
#include "../Common.h"
#include "Core/CoreRender.h"
#include "FileTypes/BbsBcd.h"

namespace BBS
{
	class CCollision
	{
	public:
		void LoadBcdFile(std::string filePath);
	};

	class CCollisionSet : public CRenderObject
	{
		//CRenderObject
		virtual void DoDraw(RenderContext& context) override;
		virtual float CalcZ(const RenderContext& context) const override;

	protected:
		CCollision* parent;
	public:
		CCollisionSet(CCollision* parent, BcdData& data);
	};
}