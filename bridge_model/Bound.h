#pragma once
#include <concepts>

#include "glm.hpp"

enum INTERSECTION_TEST_RESULT
{
	INTERSECTION_TEST_OUTSIDE = 0,
	INTERSECTION_TEST_INSIDE = 1,
	INTERSECTION_TEST_INTERSECT = 2,
};

struct BoundAABB
{
	glm::vec3 min;
	glm::vec3 max;

	BoundAABB() = default;
	BoundAABB(const glm::vec3& min, const glm::vec3& max) :min(min), max(max) {}
	BoundAABB(const BoundAABB& local, const glm::mat4& transform);

	glm::vec3 center() const { return 0.5f * (min + max); }
	glm::vec3 size() const { return max - min; }

	glm::vec3 support(const glm::vec3& dir) const;
	void biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const;
};

struct BoundOBB
{
	glm::vec3 half_size;
	glm::vec3 center;
	glm::mat3 rotation;

	BoundOBB() = default;
	BoundOBB(const glm::vec3& half_size, const glm::vec3& center, const glm::mat3& rotation) :half_size(half_size), center(center), rotation(rotation) {}
	BoundOBB(const BoundAABB& local, const glm::mat4& transform) : half_size(0.5f * local.size()), center(transform* glm::vec4(local.center(), 1.0f)), rotation(transform) {}

	glm::vec3 support(const glm::vec3& dir) const;
	void biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const;
};

struct BoundSphere
{
	glm::vec3 center;
	float r;

	BoundSphere() = default;
	BoundSphere(const glm::vec3& center, float r) :center(center), r(r) {}

	glm::vec3 support(const glm::vec3& dir) const { return center + r * dir; }
	void biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const { rsupport_point = center - r * dir; support_point = center + r * dir; }
};

struct BoundFrustum
{
	glm::vec3 points[8];

	BoundFrustum() = default;
	BoundFrustum(const glm::mat4& transform, float fov_y, float aspect, float z_near, float z_far);

	glm::vec3 support(const glm::vec3& dir) const;
	void biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const;
};

struct BoundEllipticFrustum
{
	float near_far_ratio;
	glm::vec3 x_far;
	glm::vec3 y_far;
	glm::vec3 center_near;
	glm::vec3 center_far;

	BoundEllipticFrustum() = default;
	BoundEllipticFrustum(const glm::mat4& transform, float fov_y, float aspect, float z_near, float z_far);

	glm::vec3 support(const glm::vec3& dir) const;
	void biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const;
};

bool intersectionTest(const BoundAABB& bound1, const BoundEllipticFrustum& bound2);
bool intersectionTest(const BoundEllipticFrustum& bound1, const BoundAABB& bound2);
bool intersectionTest(const BoundOBB& bound1, const BoundEllipticFrustum& bound2);
bool intersectionTest(const BoundEllipticFrustum& bound1, const BoundOBB& bound2);

template<class T>
concept HasSupport = requires(const T & t, const glm::vec3 & dir) { { t.support(dir) }->std::convertible_to<glm::vec3>; } ||
	requires(const T & t, const glm::vec3 & dir, glm::vec3 & support_point, glm::vec3 & rsupport_point) { t.biSupport(dir, support_point, rsupport_point); };

template<HasSupport BoundT, HasSupport BoundU>
bool GJKIntersectionTest(const BoundT& bound1, const BoundU& bound2)
{
	using namespace glm;
	return true;
}