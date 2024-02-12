/**************************************************************************/
/*  test_geometry_3d.h                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef TEST_GEOMETRY_3D_H
#define TEST_GEOMETRY_3D_H

#include "core/math/geometry_3d.h"
#include "tests/test_macros.h"

namespace TestGeometry3D {
TEST_CASE("[Geometry3D] Closest Points Between Segments") {
	Vector3 ps, qt;
	Geometry3D::get_closest_points_between_segments(Vector3(1, -1, 1), Vector3(1, 1, -1), Vector3(-1, -2, -1), Vector3(-1, 1, 1), ps, qt);
	CHECK(ps.is_equal_approx(Vector3(1, -0.2, 0.2)));
	CHECK(qt.is_equal_approx(Vector3(-1, -0.2, 0.2)));
}

TEST_CASE("[Geometry3D] Closest Distance Between Segments") {
	CHECK(Geometry3D::get_closest_distance_between_segments(Vector3(1, -2, 0), Vector3(1, 2, 0), Vector3(-1, 2, 0), Vector3(-1, -2, 0)) == 2.0f);
}

TEST_CASE("[Geometry3D] Compute Convex Mesh Points") {
	Vector<Vector3> cube;
	cube.push_back(Vector3(-5, -5, -5));
	cube.push_back(Vector3(5, -5, -5));
	cube.push_back(Vector3(-5, 5, -5));
	cube.push_back(Vector3(5, 5, -5));
	cube.push_back(Vector3(-5, -5, 5));
	cube.push_back(Vector3(5, -5, 5));
	cube.push_back(Vector3(-5, 5, 5));
	cube.push_back(Vector3(5, 5, 5));
}

TEST_CASE("[Geometry3D] Get Closest Point To Segment") {
	Vector3 segment[2] = { Vector3(1, 1, 1), Vector3(5, 5, 5) };
	Vector3 output = Geometry3D::get_closest_point_to_segment(Vector3(2, 1, 4), segment);
	CHECK(output.is_equal_approx(Vector3(2.33333, 2.33333, 2.33333)));
}

TEST_CASE("[Geometry3D] Plane and Box Overlap") {
	CHECK(Geometry3D::planeBoxOverlap(Vector3(3, 4, 2), 5, Vector3(5, 5, 5)) == true);
	CHECK(Geometry3D::planeBoxOverlap(Vector3(0, 1, 0), -10, Vector3(5, 5, 5)) == false);
	CHECK(Geometry3D::planeBoxOverlap(Vector3(1, 0, 0), -6, Vector3(5, 5, 5)) == false);
}

TEST_CASE("[Geometry3D] Is Point in Projected Triangle") {
	CHECK(Geometry3D::point_in_projected_triangle(Vector3(1, 1, 0), Vector3(3, 0, 0), Vector3(0, 3, 0), Vector3(-3, 0, 0)) == true);
	CHECK(Geometry3D::point_in_projected_triangle(Vector3(5, 1, 0), Vector3(3, 0, 0), Vector3(0, 3, 0), Vector3(-3, 0, 0)) == false);
	CHECK(Geometry3D::point_in_projected_triangle(Vector3(3, 0, 0), Vector3(3, 0, 0), Vector3(0, 3, 0), Vector3(-3, 0, 0)) == true);
}

TEST_CASE("[Geometry3D] Does Ray Intersect Triangle") {
	Vector3 result;
	CHECK(Geometry3D::ray_intersects_triangle(Vector3(0, 1, 1), Vector3(0, 0, -10), Vector3(0, 3, 0), Vector3(-3, 0, 0), Vector3(3, 0, 0), &result) == true);
	CHECK(Geometry3D::ray_intersects_triangle(Vector3(5, 10, 1), Vector3(0, 0, -10), Vector3(0, 3, 0), Vector3(-3, 0, 0), Vector3(3, 0, 0), &result) == false);
	CHECK(Geometry3D::ray_intersects_triangle(Vector3(0, 1, 1), Vector3(0, 0, 10), Vector3(0, 3, 0), Vector3(-3, 0, 0), Vector3(3, 0, 0), &result) == false);
}

TEST_CASE("[Geometry3D] Segment Intersects Triangle") {
	Vector3 result;
	CHECK(Geometry3D::segment_intersects_triangle(Vector3(1, 1, 1), Vector3(-1, -1, -1), Vector3(-3, 0, 0), Vector3(0, 3, 0), Vector3(3, 0, 0), &result) == true);
	CHECK(Geometry3D::segment_intersects_triangle(Vector3(1, 1, 1), Vector3(3, 0, 0), Vector3(-3, 0, 0), Vector3(0, 3, 0), Vector3(3, 0, 0), &result) == true);
	CHECK(Geometry3D::segment_intersects_triangle(Vector3(1, 1, 1), Vector3(10, -1, -1), Vector3(-3, 0, 0), Vector3(0, 3, 0), Vector3(3, 0, 0), &result) == false);
}

TEST_CASE("[Geometry3D] Triangle and Box Overlap") {
	Vector3 good_triangle[3] = { Vector3(3, 2, 3), Vector3(2, 2, 1), Vector3(2, 1, 1) };
	CHECK(Geometry3D::triangle_box_overlap(Vector3(0, 0, 0), Vector3(5, 5, 5), good_triangle) == true);
	Vector3 bad_triangle[3] = { Vector3(100, 100, 100), Vector3(-100, -100, -100), Vector3(10, 10, 10) };
	CHECK(Geometry3D::triangle_box_overlap(Vector3(1000, 1000, 1000), Vector3(1, 1, 1), bad_triangle) == false);
}

TEST_CASE("[Geometry3D] Triangle and Sphere Intersect") {
	Vector<Vector3> triangle;
	triangle.push_back(Vector3(3, 0, 0));
	triangle.push_back(Vector3(-3, 0, 0));
	triangle.push_back(Vector3(0, 3, 0));
	Vector3 triangle_contact, sphere_contact;
	CHECK(Geometry3D::triangle_sphere_intersection_test(&triangle[0], Vector3(0, -1, 0), Vector3(0, 0, 0), 5, triangle_contact, sphere_contact) == true);
	CHECK(Geometry3D::triangle_sphere_intersection_test(&triangle[0], Vector3(0, 1, 0), Vector3(0, 0, 0), 5, triangle_contact, sphere_contact) == true);
	CHECK(Geometry3D::triangle_sphere_intersection_test(&triangle[0], Vector3(0, 1, 0), Vector3(20, 0, 0), 5, triangle_contact, sphere_contact) == false);
}
} // namespace TestGeometry3D

#endif // TEST_GEOMETRY_3D_H
