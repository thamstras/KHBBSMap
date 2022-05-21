#include "Math.h"

Sphere::Sphere(glm::vec3 c, float r) : center(c), radius(r) {};

Sphere::Sphere(float x, float y, float z, float r) : center(x, y, z), radius(r) {};

Sphere Sphere::Translate(Sphere sphere, glm::vec3 transform)
{
	return Sphere(sphere.center + transform, sphere.radius);
}

AABBox::AABBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {};

AABBox AABBox::FromExtents(glm::vec3 center, glm::vec3 extents)
{
	glm::vec3 min = center - extents;
	glm::vec3 max = center + extents;
	return AABBox(min, max);
}

glm::vec3 AABBox::center()
{
	return (max + min) / 2.0f;
}

glm::vec3 AABBox::halfExtents()
{
	return (max - min) / 2.0f;
}

std::vector<glm::vec3> AABBox::GetCorners()
{
	std::vector<glm::vec3> corners = std::vector<glm::vec3>(8);

	corners.push_back(glm::vec3(min.x, min.y, min.z));
	corners.push_back(glm::vec3(min.x, min.y, max.z));
	corners.push_back(glm::vec3(min.x, max.y, min.z));
	corners.push_back(glm::vec3(min.x, max.y, max.z));
	corners.push_back(glm::vec3(max.x, min.y, min.z));
	corners.push_back(glm::vec3(max.x, min.y, max.z));
	corners.push_back(glm::vec3(max.x, max.y, min.z));
	corners.push_back(glm::vec3(max.x, max.y, max.z));

	return corners;
}

AABBox AABBox::Translate(AABBox box, glm::vec3 transform)
{
	return AABBox(box.min + transform, box.max + transform);
}

Plane::Plane(glm::vec3 norm, float d) : normal(norm), distance(d) {}

Plane::Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	glm::vec3 ba = b - a;
	glm::vec3 cb = c - b;
	glm::vec3 n = glm::cross(ba, cb);
	n = glm::normalize(n);
	float k = a.x * n.x + a.y + n.y + a.z + n.z;
	this->normal = n;
	this->distance = -k;
}

Plane Plane::FromNumbers(glm::vec4 numbers)
{
	glm::vec3 abc = glm::vec3(numbers.x, numbers.y, numbers.z);
	float mag = glm::length(abc);
	abc = glm::normalize(abc);
	Plane p = Plane(abc, numbers.w / mag);
	return p;
}

Plane::Plane() : normal(1.0f, 0.0f, 0.0f), distance(0.0f) {}

Line::Line(glm::vec3 start, glm::vec3 end) : start(start), end(end) {};

glm::vec3 Line::ToVector() { return end - start; }

Ray::Ray(glm::vec3 position, glm::vec3 direction) : position(position), direction(direction) {};

Frustum Frustum::FromVPMatrix(glm::mat4 matrix)
{
	glm::vec4 row1 = glm::vec4(matrix[0]);
	glm::vec4 row2 = glm::vec4(matrix[1]);
	glm::vec4 row3 = glm::vec4(matrix[2]);
	glm::vec4 row4 = glm::vec4(matrix[3]);
	Frustum frustum;
	frustum.planes[FRUSTUM_LEFT]   = Plane::FromNumbers(row4 + row1);
	frustum.planes[FRUSTUM_RIGHT]  = Plane::FromNumbers(row4 - row1);
	frustum.planes[FRUSTUM_BOTTOM] = Plane::FromNumbers(row4 + row2);
	frustum.planes[FRUSTUM_TOP]    = Plane::FromNumbers(row4 - row2);
	frustum.planes[FRUSTUM_NEAR]   = Plane::FromNumbers(row4 + row3);
	frustum.planes[FRUSTUM_FAR]    = Plane::FromNumbers(row4 - row3);
	return frustum;
}

Frustum::Frustum() {}

bool PointInside(Sphere s, Point p)
{
	glm::vec3 d = p - s.center;
	float sqDist = glm::dot(d, d);

	float sqRad = s.radius * s.radius;

	return sqDist <= sqRad;
}

bool PointInside(Point p, Sphere s)
{
	return PointInside(s, p);
}

glm::vec3 ClosestPointOnSphere(Sphere s, Point p)
{
	glm::vec3 dir = p - s.center;
	dir = glm::normalize(dir) * s.radius;
	return s.center + dir;
}

glm::vec3 ClosestPointOnSphere(Point p, Sphere s)
{
	return ClosestPointOnSphere(s, p);
}

bool PointInside(AABBox b, Point p)
{
	return (p.x >= b.min.x && p.y >= b.min.y && p.z >= b.min.z)
		&& (p.x <= b.max.x && p.y <= b.max.y && p.z <= b.max.z);
}

bool PointInside(Point p, AABBox b)
{
	return PointInside(b, p);
}

glm::vec3 ClosestPointInBox(AABBox b, Point p)
{
	return glm::clamp(p, b.min, b.max);
}

glm::vec3 ClosestPointInBox(Point p, AABBox b)
{
	return ClosestPointInBox(b, p);
}

float DistanceFromPlane(Plane pl, Point po)
{
	return glm::dot(pl.normal, po) - pl.distance;
}

float DistanceFromPlane(Point po, Plane pl)
{
	return DistanceFromPlane(pl, po);
}

bool PointOnPlane(Plane pl, Point po, float epsilon)
{
	return fabs(DistanceFromPlane(pl, po)) <= epsilon;
}

bool PointOnPlane(Point po, Plane pl, float epsilon)
{
	return PointOnPlane(pl, po, epsilon);
}

glm::vec3 ClosestPointOnPlane(Plane pl, Point po)
{
	float distance = glm::dot(pl.normal, po) - pl.distance;
	return po - distance * pl.normal;
}

glm::vec3 ClosestPointOnPlane(Point po, Plane pl)
{
	return ClosestPointOnPlane(pl, po);
}

glm::vec3 ThreePlaneIntersect(Plane a, Plane b, Plane c, float epsilon)
{
	// No, I don't really understand this either.

	glm::vec3 m1 = glm::vec3(a.normal.x, b.normal.x, c.normal.x);
	glm::vec3 m2 = glm::vec3(a.normal.y, b.normal.y, c.normal.y);
	glm::vec3 m3 = glm::vec3(a.normal.z, b.normal.z, c.normal.z);
	glm::vec3 d = glm::vec3(a.distance, b.distance, c.distance);

	glm::vec3 v = glm::cross(m2, m3);
	glm::vec3 u = glm::cross(m1, d);

	float denom = glm::dot(m1, u);

	if (fabs(denom) < epsilon)
		// TODO: Proper error
		return glm::vec3(0.0f);

	return glm::vec3(
		glm::dot(d, u) / denom,
		glm::dot(m3, v) / denom,
		-glm::dot(m2, v) / denom
	);
}

bool PointOnLine(Line l, Point p, float epsilon)
{
	float m = (l.end.y - l.start.y) / (l.end.x - l.start.x);
	float b = l.start.y - m * l.start.x;

	if (fabs(p.y - (m * p.x + b)) < epsilon)
	{
		return true;
	}

	return false;
}

bool PointOnLine(Point p, Line l, float epsilon)
{
	return PointOnLine(l, p, epsilon);
}

glm::vec3 ClosestPointOnLine(Line l, Point p, float* out_t)
{
	glm::vec3 a = l.start;
	glm::vec3 b = l.end;
	glm::vec3 ab = l.ToVector();

	float t = glm::dot(p - a, ab) / glm::dot(ab, ab);

	t = glm::clamp(t, 0.f, 1.f);

	glm::vec3 d = glm::vec3(a + t * ab);
	
	if (out_t != nullptr) *out_t = t;
	return d;
}

glm::vec3 ClosestPointOnLine(Point p, Line l, float* out_t)
{
	return ClosestPointOnLine(l, p, out_t);
}

bool PointInside(Frustum f, Point p)
{
	for (int i = 0; i < Frustum::FRUSTUM_PCOUNT; i++)
	{
		if (DistanceFromPlane(p, f.planes[i]) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool PointInside(Point p, Frustum f)
{
	return PointInside(f, p);
}

bool Intersect(Sphere a, Sphere b)
{
	glm::vec3 d = a.center - b.center;
	float sqDist = glm::dot(d, d);

	float sqRad = a.radius + b.radius;
	sqRad = sqRad * sqRad;

	return sqDist <= sqRad;
}

bool Intersect(Sphere s, AABBox b)
{
	glm::vec3 closestPoint = ClosestPointInBox(b, s.center);
	glm::vec3 differenceVec = s.center - closestPoint;
	float distSq = glm::dot(differenceVec, differenceVec);
	float radSq = s.radius * s.radius;
	return distSq < radSq;
}

bool Intersect(AABBox b, Sphere s)
{
	return Intersect(s, b);
}

bool Intersect(Sphere s, Plane p)
{
	glm::vec3 closestPoint = ClosestPointOnPlane(p, s.center);
	float dist = DistanceFromPlane(p, closestPoint);
	return dist <= s.radius;
}

bool Intersect(Plane p, Sphere s)
{
	return Intersect(s, p);
}

bool Intersect(AABBox a, AABBox b)
{
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool Intersect(AABBox b, Plane p)
{
	glm::vec3 c = b.center();
	glm::vec3 e = b.halfExtents();

	// Project box extents onto plane normal
	float r = e.x * abs(p.normal.x) + e.y * abs(p.normal.y) + e.z * abs(p.normal.z);

	// distance of box from plane
	float d = DistanceFromPlane(c, p);

	// intersect when s falls within [-r, +r] interval
	return abs(d) <= r;
}

bool Intersect(Plane p, AABBox b)
{
	return Intersect(b, p);
}

bool Intersect(Plane a, Plane b, float epsilon)
{
	glm::vec3 d = glm::cross(a.normal, b.normal);
	return (glm::dot(d, d) > epsilon);
}

bool Intersect(Frustum f, Sphere s)
{
	for (int i = 0; i < Frustum::FRUSTUM_PCOUNT; i++)
	{
		if (DistanceFromPlane(f.planes[i], s.center) < -s.radius)
		{
			return false;
		}
	}

	return true;
}

bool Intersect(Sphere s, Frustum f)
{
	return Intersect(f, s);
}

bool Intersect(Frustum f, AABBox b)
{
	std::vector<glm::vec3> corners = b.GetCorners();
	for (int i = 0; i < Frustum::FRUSTUM_PCOUNT; i++)
	{
		int incount = 8;
		for (glm::vec3& point : corners)
		{
			if (DistanceFromPlane(f.planes[i], point) < 0.0f)
			{
				incount -= 1;
			}
		}

		if (incount <= 0) return false;
	}

	return true;
}

bool Intersect(AABBox b, Frustum f)
{
	return Intersect(f, b);
}