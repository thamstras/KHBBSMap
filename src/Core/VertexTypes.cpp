#include "VertexTypes.h"

// TODO: Attrib indexes
constexpr int attrib_position = 0;
constexpr int attrib_color = 1;
constexpr int attrib_tex = 2;

void V_P4C4T2::BindVAO()
{
	glEnableVertexAttribArray(attrib_position);
	glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, x));

	glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, r));

	glEnableVertexAttribArray(attrib_tex);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, u));
}

void V_P3C4T2::BindVAO()
{
	glEnableVertexAttribArray(attrib_position);
	glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, x));

	glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, r));

	glEnableVertexAttribArray(attrib_tex);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, u));
}

void V_P4C4::BindVAO()
{
	glEnableVertexAttribArray(attrib_position);
	glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, x));

	glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, r));
}

void V_P3C3::BindVAO()
{
	glEnableVertexAttribArray(attrib_position);
	glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, x));

	glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(V_P4C4T2), (void*)offsetof(V_P4C4T2, r));
}