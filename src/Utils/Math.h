#pragma once
#include <glm/glm.hpp>
#include <vector>

typedef glm::vec3 Point;

struct Sphere
{
	glm::vec3 center;
	float radius;

	Sphere(glm::vec3 c, float r);
	Sphere(float x, float y, float z, float r);
	static Sphere Translate(Sphere sphere, glm::vec3 transform);
};

struct AABBox
{
	glm::vec3 min;
	glm::vec3 max;

	glm::vec3 center();
	glm::vec3 halfExtents();
	std::vector<glm::vec3> GetCorners();

	AABBox(glm::vec3 min, glm::vec3 max);
	static AABBox FromExtents(glm::vec3 center, glm::vec3 extents);
	static AABBox Translate(AABBox box, glm::vec3 transform);
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	Plane(glm::vec3 norm, float d);
	Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	static Plane FromNumbers(glm::vec4 numbers);
private:
	friend struct Frustum;
	Plane();	// Private constructor to help Frustum
};

struct Line
{
	glm::vec3 start;
	glm::vec3 end;

	Line(glm::vec3 start, glm::vec3 end);
	glm::vec3 ToVector();
};

struct Ray
{
	glm::vec3 position;
	glm::vec3 direction;

	Ray(glm::vec3 position, glm::vec3 direction);
};

struct Frustum
{
	enum FPLANES
	{
		FRUSTUM_LEFT,
		FRUSTUM_RIGHT,
		FRUSTUM_BOTTOM,
		FRUSTUM_TOP,
		FRUSTUM_NEAR,
		FRUSTUM_FAR,
		FRUSTUM_PCOUNT,
	};

	Plane planes[6];

	static Frustum FromVPMatrix(glm::mat4 matrix);
private:
	Frustum();	// Private constructor for FromVPMatrix
};

bool PointInside(Sphere s, Point p);
bool PointInside(Point p, Sphere s);

glm::vec3 ClosestPointOnSphere(Sphere s, Point p);
glm::vec3 ClosestPointOnSphere(Point p, Sphere s);

bool PointInside(AABBox b, Point p);
bool PointInside(Point p, AABBox b);

float DistanceFromPlane(Plane pl, Point po);
float DistanceFromPlane(Point po, Plane pl);

float HalfSpaceTest(Plane pl, Point po);

bool PointOnPlane(Plane pl, Point po, float epsilon = 0.0001f);
bool PointOnPlane(Point po, Plane pl, float epsilon = 0.0001f);

glm::vec3 ClosestPointOnPlane(Plane pl, Point po);
glm::vec3 ClosestPointOnPlane(Point po, Plane pl);

glm::vec3 ThreePlaneIntersect(Plane a, Plane b, Plane c, float epsilon = 0.0001f);

bool PointOnLine(Line l, Point p, float epsilon);
bool PointOnLine(Point p, Line l, float epsilon);

glm::vec3 ClosestPointOnLine(Line l, Point p, float* out_t = nullptr);
glm::vec3 ClosestPointOnLine(Point p, Line l, float* out_t = nullptr);

bool PointInside(Frustum f, Point p);
bool PointInside(Point p, Frustum f);

bool Intersect(Sphere a, Sphere b);
bool Intersect(Sphere s, AABBox b);
bool Intersect(AABBox b, Sphere s);
bool Intersect(Sphere s, Plane p);
bool Intersect(Plane p, Sphere s);
bool Intersect(AABBox a, AABBox b);
bool Intersect(AABBox b, Plane p);
bool Intersect(Plane p, AABBox b);
bool Intersect(Plane a, Plane b, float epsilon = 0.0001f);
bool Intersect(Frustum f, Sphere s);
bool Intersect(Sphere s, Frustum f);
bool Intersect(Frustum f, AABBox b);
bool Intersect(AABBox b, Frustum f);