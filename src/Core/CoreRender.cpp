#include "CoreRender.h"

// Just stashing this here because CoreRender.h doesn't have a corresponding cpp file.
CRenderObject::~CRenderObject() {};

void RenderContext::AddToDrawList(ERenderLayer pass, CRenderObject* obj)
{
	switch (pass)
	{
	case LAYER_SKY:
		this->render.skyDrawList.push_back(obj);
		return;
	case LAYER_STATIC:
		this->render.staticDrawList.push_back(obj);
		return;
	case LAYER_DYNAMIC:
		this->render.dynamicDrawList.push_back(obj);
		return;
	case LAYER_OVERLAY:
		this->render.overlayDrawList.push_back(obj);
		return;
	case LAYER_GUI:
		this->render.guiDrawList.push_back(obj);
		return;
	default:
		throw std::exception("FATAL: Invalid pass in AddToDrawList");
	}
}