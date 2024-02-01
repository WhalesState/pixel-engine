/**************************************************************************/
/*  geometry_3d.h                                                         */
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

#ifndef GEOMETRY_3D_H
#define GEOMETRY_3D_H

#include "core/math/face3.h"
#include "core/object/object.h"
#include "core/templates/local_vector.h"
#include "core/templates/vector.h"

class Geometry3D {
public:
	static void get_closest_points_between_segments(const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_q0, const Vector3 &p_q1, Vector3 &r_ps, Vector3 &r_qt);
	static real_t get_closest_distance_between_segments(const Vector3 &p_p0, const Vector3 &p_p1, const Vector3 &p_q0, const Vector3 &p_q1);

	static inline bool ray_intersects_triangle(const Vector3 &p_from, const Vector3 &p_dir, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2, Vector3 *r_res = nullptr) {
		Vector3 e1 = p_v1 - p_v0;
		Vector3 e2 = p_v2 - p_v0;
		Vector3 h = p_dir.cross(e2);
		real_t a = e1.dot(h);
		if (Math::is_zero_approx(a)) { // Parallel test.
			return false;
		}

		real_t f = 1.0f / a;

		Vector3 s = p_from - p_v0;
		real_t u = f * s.dot(h);

		if ((u < 0.0f) || (u > 1.0f)) {
			return false;
		}

		Vector3 q = s.cross(e1);

		real_t v = f * p_dir.dot(q);

		if ((v < 0.0f) || (u + v > 1.0f)) {
			return false;
		}

		// At this stage we can compute t to find out where
		// the intersection point is on the line.
		real_t t = f * e2.dot(q);

		if (t > 0.00001f) { // ray intersection
			if (r_res) {
				*r_res = p_from + p_dir * t;
			}
			return true;
		} else { // This means that there is a line intersection but not a ray intersection.
			return false;
		}
	}

	static inline bool segment_intersects_triangle(const Vector3 &p_from, const Vector3 &p_to, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2, Vector3 *r_res = nullptr) {
		Vector3 rel = p_to - p_from;
		Vector3 e1 = p_v1 - p_v0;
		Vector3 e2 = p_v2 - p_v0;
		Vector3 h = rel.cross(e2);
		real_t a = e1.dot(h);
		if (Math::is_zero_approx(a)) { // Parallel test.
			return false;
		}

		real_t f = 1.0f / a;

		Vector3 s = p_from - p_v0;
		real_t u = f * s.dot(h);

		if ((u < 0.0f) || (u > 1.0f)) {
			return false;
		}

		Vector3 q = s.cross(e1);

		real_t v = f * rel.dot(q);

		if ((v < 0.0f) || (u + v > 1.0f)) {
			return false;
		}

		// At this stage we can compute t to find out where
		// the intersection point is on the line.
		real_t t = f * e2.dot(q);

		if (t > (real_t)CMP_EPSILON && t <= 1.0f) { // Ray intersection.
			if (r_res) {
				*r_res = p_from + rel * t;
			}
			return true;
		} else { // This means that there is a line intersection but not a ray intersection.
			return false;
		}
	}

	static Vector3 get_closest_point_to_segment(const Vector3 &p_point, const Vector3 *p_segment) {
		Vector3 p = p_point - p_segment[0];
		Vector3 n = p_segment[1] - p_segment[0];
		real_t l2 = n.length_squared();
		if (l2 < 1e-20f) {
			return p_segment[0]; // Both points are the same, just give any.
		}

		real_t d = n.dot(p) / l2;

		if (d <= 0.0f) {
			return p_segment[0]; // Before first point.
		} else if (d >= 1.0f) {
			return p_segment[1]; // After first point.
		} else {
			return p_segment[0] + n * d; // Inside.
		}
	}

	static inline bool point_in_projected_triangle(const Vector3 &p_point, const Vector3 &p_v1, const Vector3 &p_v2, const Vector3 &p_v3) {
		Vector3 face_n = (p_v1 - p_v3).cross(p_v1 - p_v2);

		Vector3 n1 = (p_point - p_v3).cross(p_point - p_v2);

		if (face_n.dot(n1) < 0) {
			return false;
		}

		Vector3 n2 = (p_v1 - p_v3).cross(p_v1 - p_point);

		if (face_n.dot(n2) < 0) {
			return false;
		}

		Vector3 n3 = (p_v1 - p_point).cross(p_v1 - p_v2);

		if (face_n.dot(n3) < 0) {
			return false;
		}

		return true;
	}

	static inline bool triangle_sphere_intersection_test(const Vector3 *p_triangle, const Vector3 &p_normal, const Vector3 &p_sphere_pos, real_t p_sphere_radius, Vector3 &r_triangle_contact, Vector3 &r_sphere_contact) {
		real_t d = p_normal.dot(p_sphere_pos) - p_normal.dot(p_triangle[0]);

		if (d > p_sphere_radius || d < -p_sphere_radius) {
			// Not touching the plane of the face, return.
			return false;
		}

		Vector3 contact = p_sphere_pos - (p_normal * d);

		/** 2nd) TEST INSIDE TRIANGLE **/

		if (Geometry3D::point_in_projected_triangle(contact, p_triangle[0], p_triangle[1], p_triangle[2])) {
			r_triangle_contact = contact;
			r_sphere_contact = p_sphere_pos - p_normal * p_sphere_radius;
			//printf("solved inside triangle\n");
			return true;
		}

		/** 3rd TEST INSIDE EDGE CYLINDERS **/

		const Vector3 verts[4] = { p_triangle[0], p_triangle[1], p_triangle[2], p_triangle[0] }; // for() friendly

		for (int i = 0; i < 3; i++) {
			// Check edge cylinder.

			Vector3 n1 = verts[i] - verts[i + 1];
			Vector3 n2 = p_sphere_pos - verts[i + 1];

			///@TODO Maybe discard by range here to make the algorithm quicker.

			// Check point within cylinder radius.
			Vector3 axis = n1.cross(n2).cross(n1);
			axis.normalize();

			real_t ad = axis.dot(n2);

			if (ABS(ad) > p_sphere_radius) {
				// No chance with this edge, too far away.
				continue;
			}

			// Check point within edge capsule cylinder.
			/** 4th TEST INSIDE EDGE POINTS **/

			real_t sphere_at = n1.dot(n2);

			if (sphere_at >= 0 && sphere_at < n1.dot(n1)) {
				r_triangle_contact = p_sphere_pos - axis * (axis.dot(n2));
				r_sphere_contact = p_sphere_pos - axis * p_sphere_radius;
				// Point inside here.
				return true;
			}

			real_t r2 = p_sphere_radius * p_sphere_radius;

			if (n2.length_squared() < r2) {
				Vector3 n = (p_sphere_pos - verts[i + 1]).normalized();

				r_triangle_contact = verts[i + 1];
				r_sphere_contact = p_sphere_pos - n * p_sphere_radius;
				return true;
			}

			if (n2.distance_squared_to(n1) < r2) {
				Vector3 n = (p_sphere_pos - verts[i]).normalized();

				r_triangle_contact = verts[i];
				r_sphere_contact = p_sphere_pos - n * p_sphere_radius;
				return true;
			}

			break; // It's pointless to continue at this point, so save some CPU cycles.
		}

		return false;
	}

	// Create a "wrap" that encloses the given geometry.
	static Vector<Face3> wrap_geometry(Vector<Face3> p_array, real_t *p_error = nullptr);

	struct MeshData {
		struct Face {
			Plane plane;
			LocalVector<int> indices;
		};

		LocalVector<Face> faces;

		struct Edge {
			int vertex_a, vertex_b;
			int face_a, face_b;
		};

		LocalVector<Edge> edges;

		LocalVector<Vector3> vertices;

		void optimize_vertices();
	};

	static MeshData build_convex_mesh(const Vector<Plane> &p_planes);
	static Vector<Plane> build_sphere_planes(real_t p_radius, int p_lats, int p_lons, Vector3::Axis p_axis = Vector3::AXIS_Z);
	static Vector<Plane> build_capsule_planes(real_t p_radius, real_t p_height, int p_sides, int p_lats, Vector3::Axis p_axis = Vector3::AXIS_Z);

	static Vector<Vector3> compute_convex_mesh_points(const Plane *p_planes, int p_plane_count);

#define FINDMINMAX(x0, x1, x2, min, max) \
	min = max = x0;                      \
	if (x1 < min) {                      \
		min = x1;                        \
	}                                    \
	if (x1 > max) {                      \
		max = x1;                        \
	}                                    \
	if (x2 < min) {                      \
		min = x2;                        \
	}                                    \
	if (x2 > max) {                      \
		max = x2;                        \
	}

	_FORCE_INLINE_ static bool planeBoxOverlap(Vector3 normal, float d, Vector3 maxbox) {
		int q;
		Vector3 vmin, vmax;
		for (q = 0; q <= 2; q++) {
			if (normal[q] > 0.0f) {
				vmin[q] = -maxbox[q];
				vmax[q] = maxbox[q];
			} else {
				vmin[q] = maxbox[q];
				vmax[q] = -maxbox[q];
			}
		}
		if (normal.dot(vmin) + d > 0.0f) {
			return false;
		}
		if (normal.dot(vmax) + d >= 0.0f) {
			return true;
		}

		return false;
	}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)                 \
	p0 = a * v0.y - b * v0.z;                      \
	p2 = a * v2.y - b * v2.z;                      \
	if (p0 < p2) {                                 \
		min = p0;                                  \
		max = p2;                                  \
	} else {                                       \
		min = p2;                                  \
		max = p0;                                  \
	}                                              \
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

#define AXISTEST_X2(a, b, fa, fb)                  \
	p0 = a * v0.y - b * v0.z;                      \
	p1 = a * v1.y - b * v1.z;                      \
	if (p0 < p1) {                                 \
		min = p0;                                  \
		max = p1;                                  \
	} else {                                       \
		min = p1;                                  \
		max = p0;                                  \
	}                                              \
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)                 \
	p0 = -a * v0.x + b * v0.z;                     \
	p2 = -a * v2.x + b * v2.z;                     \
	if (p0 < p2) {                                 \
		min = p0;                                  \
		max = p2;                                  \
	} else {                                       \
		min = p2;                                  \
		max = p0;                                  \
	}                                              \
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

#define AXISTEST_Y1(a, b, fa, fb)                  \
	p0 = -a * v0.x + b * v0.z;                     \
	p1 = -a * v1.x + b * v1.z;                     \
	if (p0 < p1) {                                 \
		min = p0;                                  \
		max = p1;                                  \
	} else {                                       \
		min = p1;                                  \
		max = p0;                                  \
	}                                              \
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

	/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)                 \
	p1 = a * v1.x - b * v1.y;                      \
	p2 = a * v2.x - b * v2.y;                      \
	if (p2 < p1) {                                 \
		min = p2;                                  \
		max = p1;                                  \
	} else {                                       \
		min = p1;                                  \
		max = p2;                                  \
	}                                              \
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

#define AXISTEST_Z0(a, b, fa, fb)                  \
	p0 = a * v0.x - b * v0.y;                      \
	p1 = a * v1.x - b * v1.y;                      \
	if (p0 < p1) {                                 \
		min = p0;                                  \
		max = p1;                                  \
	} else {                                       \
		min = p1;                                  \
		max = p0;                                  \
	}                                              \
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y; \
	if (min > rad || max < -rad) {                 \
		return false;                              \
	}

	_FORCE_INLINE_ static bool triangle_box_overlap(const Vector3 &boxcenter, const Vector3 boxhalfsize, const Vector3 *triverts) {
		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */
		Vector3 v0, v1, v2;
		float min, max, d, p0, p1, p2, rad, fex, fey, fez;
		Vector3 normal, e0, e1, e2;

		/* This is the fastest branch on Sun */
		/* move everything so that the boxcenter is in (0,0,0) */

		v0 = triverts[0] - boxcenter;
		v1 = triverts[1] - boxcenter;
		v2 = triverts[2] - boxcenter;

		/* compute triangle edges */
		e0 = v1 - v0; /* tri edge 0 */
		e1 = v2 - v1; /* tri edge 1 */
		e2 = v0 - v2; /* tri edge 2 */

		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */
		fex = Math::abs(e0.x);
		fey = Math::abs(e0.y);
		fez = Math::abs(e0.z);
		AXISTEST_X01(e0.z, e0.y, fez, fey);
		AXISTEST_Y02(e0.z, e0.x, fez, fex);
		AXISTEST_Z12(e0.y, e0.x, fey, fex);

		fex = Math::abs(e1.x);
		fey = Math::abs(e1.y);
		fez = Math::abs(e1.z);
		AXISTEST_X01(e1.z, e1.y, fez, fey);
		AXISTEST_Y02(e1.z, e1.x, fez, fex);
		AXISTEST_Z0(e1.y, e1.x, fey, fex);

		fex = Math::abs(e2.x);
		fey = Math::abs(e2.y);
		fez = Math::abs(e2.z);
		AXISTEST_X2(e2.z, e2.y, fez, fey);
		AXISTEST_Y1(e2.z, e2.x, fez, fex);
		AXISTEST_Z12(e2.y, e2.x, fey, fex);

		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */

		/* test in X-direction */
		FINDMINMAX(v0.x, v1.x, v2.x, min, max);
		if (min > boxhalfsize.x || max < -boxhalfsize.x) {
			return false;
		}

		/* test in Y-direction */
		FINDMINMAX(v0.y, v1.y, v2.y, min, max);
		if (min > boxhalfsize.y || max < -boxhalfsize.y) {
			return false;
		}

		/* test in Z-direction */
		FINDMINMAX(v0.z, v1.z, v2.z, min, max);
		if (min > boxhalfsize.z || max < -boxhalfsize.z) {
			return false;
		}

		/* Bullet 2: */
		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */
		normal = e0.cross(e1);
		d = -normal.dot(v0); /* plane eq: normal.x+d=0 */
		return planeBoxOverlap(normal, d, boxhalfsize); /* if true, box and triangle overlaps */
	}

	static Vector<uint32_t> generate_edf(const Vector<bool> &p_voxels, const Vector3i &p_size, bool p_negative);
	static Vector<int8_t> generate_sdf8(const Vector<uint32_t> &p_positive, const Vector<uint32_t> &p_negative);
};

#endif // GEOMETRY_3D_H
