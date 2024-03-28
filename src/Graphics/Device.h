#pragma once

namespace Graphics
{
	struct Surface;
	struct Texture;
	struct RenderCmd;

	class Device
	{
	public:
		void Begin(Surface* pTarget);
		void Submit(RenderCmd& cmd);
		void End();
	};
}