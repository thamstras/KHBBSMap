#pragma once
#include "../Common.h"
#include "Core/CoreRender.h"
#include "FileTypes/BbsBcd.h"
#include "Core/CPlaneSet.h"
#include "Core/World.h"

namespace BBS
{
	class CCollision;
	class CCollisionSet;
	class CCollisionGrid;

	class CCollisionSet : public CRenderObject
	{
		//CRenderObject
		virtual void DoDraw(RenderContext& context) override;
		virtual float CalcZ(const RenderContext& context) const override;

	protected:
		CCollision* parent;
		//std::vector<BcdPlane> data;
		BcdData data;
		std::unique_ptr<CPlaneSet> draw;
		int selectedPlane = -1;
		bool enabled = true;
		bool filter = false;
		ATTR_BIT filterAttribs;
	public:
		CCollisionSet(CCollision* parent, BcdData& data);
		void GUI();
	};

	class CCollision
	{
	protected:
		std::vector<CCollisionSet> sets;
	public:
		void LoadBcdFile(std::string filePath);
		void Clear();
		void Update(WorldContext& context);
		void GUI();

		float visOpacity = 0.5f;
	};
}