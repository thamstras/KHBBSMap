#pragma once
#include "Common.h"

struct V_P4C4T2
{
	float x, y, z, w;
	float r, g, b, a;
	float u, v;

	static void BindVAO();

	static const int memberCount = 10;
	static const bool supportsTextures = true;
};

// Map geom
struct V_P3C4T2
{
	float x, y, z;
	float r, g, b, a;
	float u, v;

	static void BindVAO();

	static const int memberCount = 9;
	static const bool supportsTextures = true;
};

// Collision geom
struct V_P4C4
{
	float x, y, z, w;
	float r, g, b, a;	

	static void BindVAO();

	static const int memberCount = 8;
	static const bool supportsTextures = false;
};

// Debug geom
struct V_P3C3
{
	float x, y, z;
	float r, g, b;

	static void BindVAO();

	static const int memberCount = 6;
	static const bool supportsTextures = false;
};

// Skel mesh?
struct V_P3C4T2W8
{
	float x, y, z;
	float r, g, b, w;
	float u, v;
	float w0, w1, w2, w3;
	float w4, w5, w6, w7;

	static void BindVAO();

	static const int memberCount = 17;
	static const bool supportsTextures = true;
};