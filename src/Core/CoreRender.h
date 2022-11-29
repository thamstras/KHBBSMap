#pragma once
#include "..\Common.h"
#include "ShaderLibrary.h"

enum ERenderLayer
{
	/// <summary>
	/// The Sky "box".
	/// Positionally fixed to the camera and drawn behind everything.
	/// Drawn in order without depth.
	/// </summary>
	LAYER_SKY,

	/// <summary>
	/// Static opaque geometry.
	/// Drawn near-to-far with depth.
	/// </summary>
	LAYER_STATIC,

	/// <summary>
	/// Anything that moves or requires blending.
	/// Drawn far-to-near with read only depth.
	/// </summary>
	LAYER_DYNAMIC,

	/// <summary>
	/// Anything that gets drawn after everything else.
	/// Envisioning something like a collision visualization that's
	/// drawn on top of the normal map.
	/// Drawn in order with read only depth
	/// </summary>
	LAYER_OVERLAY,

	/// <summary>
	/// Any UI elements we end up supporting.
	/// Drawn in order without depth.
	/// </summary>
	LAYER_GUI,
};

struct RenderContext;
class CCamera;

class CRenderObject
{
public:
	CRenderObject() = default;
	virtual ~CRenderObject();

	virtual void DoDraw(RenderContext& context) = 0;
	virtual float CalcZ(const RenderContext& context) const = 0;
};

struct RenderContext
{
	struct {
		glm::vec4 fogColor;
		float fogNear;
		float fogFar;
		glm::vec4 clearColor;
	} env;

	struct {
		bool wireframe;
		bool no_cull;
		bool no_blend;
		bool no_texture;
		bool no_fog;
		bool no_lighting;

		CCamera* current_camera;
		glm::mat4 skyViewMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		float nearClip;
		float farClip;

		// TODO: DrawLists
		ERenderLayer currentPass;
		std::vector<CRenderObject*> skyDrawList;
		std::vector<CRenderObject*> staticDrawList;
		std::vector<CRenderObject*> dynamicDrawList;
		std::vector<CRenderObject*> overlayDrawList;
		std::vector<CRenderObject*> guiDrawList;

		std::string default_shader;
		std::string highlight_shader;
		std::string textureless_shader;

		std::shared_ptr<ShaderLibrary> shaderLibrary;

	} render;

	struct {
		bool active;
		bool sectionOnly;
		bool highlight;
		bool no_texture;
		bool wireframe;
		bool peel;
		unsigned int obj_id;
		unsigned int section_id;
		glm::vec4 highlight_color;
	} debug;

	struct {
		unsigned int obj_drawn;
		unsigned int draw_calls;
		unsigned int tris_drawn;
	} stats;
};