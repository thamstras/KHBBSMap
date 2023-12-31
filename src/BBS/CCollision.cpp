#include "CCollision.h"
#include <fstream>

using namespace BBS;

void CCollision::LoadBcdFile(std::string filePath)
{
	Clear();
	std::ifstream fs = std::ifstream(filePath, std::ios_base::binary);
	BcdFile file = BcdFile::ReadBcdFile(fs, 0);
	for (auto& set : file.data)
		sets.emplace_back(this, set);
}

void CCollision::Clear()
{
	sets.clear();
}

void CCollision::Update(WorldContext& context)
{
	for (auto& set : sets)
	{
		context.render->AddToDrawList(ERenderLayer::LAYER_OVERLAY, &set);
	}
}

void CCollision::GUI()
{
	
	ImGui::Text("Collision Sets Loaded: %d", sets.size());
	sets[0].GUI();
}

CCollisionSet::CCollisionSet(CCollision* parent, BcdData& data) : parent(parent)
{
	filterAttribs = std::bit_cast<ATTR_BIT>(0);
	this->data = data;
	std::vector<CPlaneSet::Plane> planes = std::vector<CPlaneSet::Plane>();
	for (auto& plane : data.planes)
	{
		CPlaneSet::Plane p = CPlaneSet::Plane();
		int c = plane.IsQuad() ? 4 : 3;
		for (int v = 0; v < c; v++)
		{
			FVECTOR4 vert = data.verts[plane.vertIdx[v]];
			ICOLOR color = data.colors[plane.colorIdx[v]];
			p.verts[v] = glm::vec4(vert.x, vert.y, vert.z, vert.w);
			p.colors[v] = glm::vec4(color.r / (float)UINT8_MAX, color.g / (float)UINT8_MAX, color.b / (float)UINT8_MAX, color.b / (float)UINT8_MAX);
		}
		p.isTri = !plane.IsQuad();
		p.filterData = std::bit_cast<uint32_t>(plane.attrib);
		planes.push_back(p);
	}
	draw = std::make_unique<CPlaneSet>(planes);
}

void CCollisionSet::DoDraw(RenderContext& context)
{
	if (enabled && draw != nullptr)
	{
		if (filter)
			draw->DrawFiltered(context, std::bit_cast<uint32_t>(filterAttribs), selectedPlane);
		else
			draw->Draw(context, selectedPlane);
	}
}

float CCollisionSet::CalcZ(const RenderContext& context) const
{
	return context.render.nearClip + FLT_EPSILON;
}

void CCollisionSet::GUI()
{
	ImGui::Checkbox("Show in viewport", &enabled);
	ImGui::SliderFloat("Opacity", &draw->alpha, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Text("Verts: %d", data.verts.size());
	ImGui::Text("Faces: %d", data.planes.size());
	ImGui::Text("Grid:");
	ImGui::Text("  Extents X: %.2f -> %.2f", data.minX, data.maxX);
	ImGui::Text("  Extents Y: %.2f -> %.2f", data.minY, data.maxY);
	ImGui::Text("  Extents Z: %.2f -> %.2f", data.minZ, data.maxZ);
	ImGui::Text("  Cell Size: %.2f", data.cellSize);
	ImGui::Text("  Cells: %d, %d, %d", data.divX, data.divY, data.divZ);
	//ImGui::DragInt("Selected Face", &selectedPlane, 1.0f, -1, data.size() - 1);
	ImGui::InputInt("Selected Face", &selectedPlane, 1, 10);
	if (selectedPlane < -1) selectedPlane = -1;
	else if (selectedPlane >= (int)data.planes.size()) selectedPlane = data.planes.size() - 1;
	if (selectedPlane >= 0)
	{
		auto& plane = data.planes[selectedPlane];
		ImGui::Text("Info: %X", plane.info);
		ImGui::Text("Attrib: %X", std::bit_cast<uint32_t>(plane.attrib));
		ImGui::Text("Kind: %d", plane.attrib.uiKind);
		ImGui::Text("Material: %d", plane.attrib.uiMaterial);
		ImGui::Text("Hit: %s%s%s%s%s%s%s",
			plane.attrib.uiHitPlayer ? "player " : "",
			plane.attrib.uiHitEnemy ? "enemy " : "",
			plane.attrib.uiHitFlyEnemy ? "flying " : "",
			plane.attrib.uiHitAttack ? "attack " : "",
			plane.attrib.uiHitCamera ? "camera " : "",
			plane.attrib.uiHitPrize ? "prize " : "",
			plane.attrib.uiHitGimmick ? "gimmick" : "");
		ImGui::Text("Extra: %s%s%s%s%s%s",
			plane.attrib.uiDangle ? "dangle " : "",
			plane.attrib.uiLadder ? "ladder " : "",
			plane.attrib.uiBarrier ? "barrier " : "",
			plane.attrib.uiIK ? "ik " : "",
			plane.attrib.uiSwim ? "swim " : "",
			plane.attrib.uiNoPut ? "no put" : "");
		ImGui::Text("Zero: %X", plane.attrib.dummy);
	}

	ImGui::Separator();

	ImGui::Checkbox("Enable Filter", &filter);
	ImGui::Indent();

	bool val;
#define ATTRIB_CHECKBOX(field, label) val = filterAttribs.field; \
		ImGui::Checkbox(label, &val); \
		filterAttribs.field = val
	ATTRIB_CHECKBOX(uiHitPlayer, "Hit Player");
	ATTRIB_CHECKBOX(uiHitEnemy, "Hit Enemy");
	ATTRIB_CHECKBOX(uiHitFlyEnemy, "Hit Fly Enemy");
	ATTRIB_CHECKBOX(uiHitAttack, "Hit Attack");
	ATTRIB_CHECKBOX(uiHitCamera, "Hit Camera");
	ATTRIB_CHECKBOX(uiDangle, "Dangle");
	ATTRIB_CHECKBOX(uiLadder, "Ladder");
	ATTRIB_CHECKBOX(uiBarrier, "Barrier");
	ATTRIB_CHECKBOX(uiIK, "IK");
	ATTRIB_CHECKBOX(uiHitPrize, "Hit Prize");
	ATTRIB_CHECKBOX(uiSwim, "Swim");
	ATTRIB_CHECKBOX(uiNoPut, "No Put");
	ATTRIB_CHECKBOX(uiHitGimmick, "Hit Gimmick");
#undef ATTRIB_CHECKBOX

	ImGui::Unindent();
}