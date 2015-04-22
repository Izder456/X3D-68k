// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.




// C Source File
// Created 3/5/2015; 9:25:21 AM

#include <tigcclib.h>

#include "geo.h"
#include "math.h"
#include "screen.h"
#include "error.h"

// Returns the distance between two points
inline short dist(Vex3D* a, Vex3D* b) {
	short dx = a->x - b->x;
	short dy = a->y - b->y;
	short dz = a->z - b->z;

	return fastsqrt((long)dx * dx + (long)dy * dy + (long)dz * dz);
}

// Rotates a Vex3D around the origin
inline void rotate_vex3d(Vex3D* src, Mat3x3* mat, Vex3D* dest) {
	Vex3D_rot rot;
	asm_rotate_vex3d(src, mat, &rot);

	dest->x = rot.x;
	dest->y = rot.y;
	dest->z = rot.z;
}

// Construct a plane given 3 points on the plane
void construct_plane(Vex3D* a, Vex3D* b, Vex3D* c, Plane* dest) {
	Vex3D v1, v2;

	// Calculate the normal of the plane
	sub_vex3d(a, b, &v1);
	sub_vex3d(c, b, &v2);

	cross_product(&v1, &v2, &dest->normal);

	// D = (AX + BY + CZ)
	dest->d = dot_product(&dest->normal, a);
}

// Calculates the normals of the unrotated planes of the view frustum
void calculate_frustum_plane_normals(X3D_RenderContext* c) {
	short w = c->w;
	short h = c->h;

	//c->dist = 120;

	Vex3D top_left = {-w / 2, -h / 2, c->dist};
	Vex3D top_right = {w / 2, -h / 2, c->dist};

	Vex3D bottom_left = {-w / 2, h / 2, c->dist};
	Vex3D bottom_right = {w / 2, h / 2, c->dist};

	Vex3D cam_pos = {0, 0, 0};

	//error("ERROR\n");

#if 1
	// Top plane
	construct_plane(&cam_pos, &top_right, &top_left, &c->frustum_unrotated.p[FRUSTUM_TOP]);

	// Bottom plane
	construct_plane(&cam_pos, &bottom_left, &bottom_right, &c->frustum_unrotated.p[FRUSTUM_BOTTOM]);

	// Left plane
	construct_plane(&cam_pos, &top_left, &bottom_left, &c->frustum_unrotated.p[FRUSTUM_LEFT]);

	// Right plane
	construct_plane(&cam_pos, &bottom_right, &top_right, &c->frustum_unrotated.p[FRUSTUM_RIGHT]);

	// Near plane
	construct_plane(&bottom_right, &top_right, &top_left, &c->frustum_unrotated.p[FRUSTUM_NEAR]);

	c->frustum_unrotated.p[0].normal = (Vex3D){0, 0, 32767};

	// Hack...
	c->frustum_unrotated.p[0].d = 15;//c->dist - DIST_TO_NEAR_PLANE;
#else
	test_construct_frustum_from_polygon3D(c, &c->frustum_unrotated);
	c->frustum_unrotated.p[4].normal = (Vex3D){0, 0, 32767};
	c->frustum_unrotated.p[0].d = 15;
	//exit(-1);
#endif







	c->frustum_unrotated.total_p = 5;
}

// Calculates the distance from each plane in the view frustum to the origin
void calculate_frustum_plane_distances(X3D_RenderContext* c) {
	int i;

	Vex3D eye_cam_pos = {c->cam.pos.x, c->cam.pos.y - PLAYER_HEIGHT, c->cam.pos.z};

	for(i = 1; i < c->frustum.total_p; i++) {
		c->frustum.p[i].d = dot_product(&c->frustum.p[i].normal, &eye_cam_pos);
	}

	Vex3D out = c->cam.dir;

	short dist = 15;//c->dist - DIST_TO_NEAR_PLANE;

	out.x = ((long)out.x * dist) >> NORMAL_BITS;
	out.y = ((long)out.y * dist) >> NORMAL_BITS;
	out.z = ((long)out.z * dist) >> NORMAL_BITS;

	out.x += c->cam.pos.x;
	out.y += c->cam.pos.y - PLAYER_HEIGHT;
	out.z += c->cam.pos.z;

	c->frustum.p[0].d = dot_product(&c->frustum.p[0].normal, &out);
}

// Calculates the rotated plane normals of the view frustum
void calculate_frustum_rotated_normals(X3D_RenderContext* c) {
	int i, d;

	Mat3x3 transpose;

	for(i = 0; i < 3; i++)
		for(d = 0; d < 3; d++)
			transpose[i][d] = c->cam.mat[d][i];


	for(i = 0; i < c->frustum_unrotated.total_p; i++) {
		Vex3D n = c->frustum_unrotated.p[i].normal;

		n.x >>= 1;
		n.y >>= 1;
		n.z >>= 1;

		rotate_vex3d(&n, &transpose, &c->frustum.p[i].normal);

		c->frustum.p[i].normal.x <<= 1;
		c->frustum.p[i].normal.y <<= 1;
		c->frustum.p[i].normal.z <<= 1;
	}



	c->frustum.p[FRUSTUM_NEAR].normal = c->cam.dir;

	//printf("P: %d\nNear plane normal: ", c->frustum_unrotated.total_p);
	//print_vex3d(&c->frustum.p[4].normal);

	c->frustum.total_p = c->frustum_unrotated.total_p;
}

// Multiplies two 3x3 matricies i.e. concatenates them
void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res) {
	int i, j, k;

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			(*res)[i][j] = 0;

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			for(k = 0; k < 3; k++) {
				(*res)[i][j] += FIXMUL15((*a)[i][k], (*b)[k][j]);
			}
		}
	}
}

// Constructs a 3D rotation matrix with the given angles
// TODO: rewrite this to not use matrix multiplication
void construct_mat3x3(Vex3Ds *angle, Mat3x3 *dest) {
	short sin_y = sinfp(angle->y);
	short cos_y = cosfp(angle->y);

	short sin_x = sinfp(angle->x);
	short cos_x = cosfp(angle->x);

	Mat3x3 mat_y = {
		{ cos_y, 0, sin_y},
		{ 0, 32767, 0},
		{ -sin_y, 0, cos_y}
	};

	Mat3x3 mat_x = {
		{32767, 0, 0},
		{0, cos_x, -sin_x},
		{0, sin_x, cos_x},
	};

	Mat3x3 mul_res;

	mul_mat3x3(&mat_x, &mat_y, &mul_res);

	memcpy(dest, &mul_res, sizeof(Mat3x3));
}

// Given the input angle in DEG256, this returns the fixed-point sine of it
// The number is in 0:15 format
inline short sinfp(unsigned char angle) {
	return sintab[angle];
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 0:15 format
inline short cosfp(unsigned char angle) {
	// We exploit the fact that cos(x) = sin(90 - x)
	return sinfp(ANG_90 - angle);
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 8:8 format
inline short tanfp(unsigned char angle) {
	// Prevent division by 0
	if(angle == ANG_90 || angle == ANG_180)
		return VERTICAL_LINE_SLOPE;

	return ((long)sinfp(angle) << 8) / cosfp(angle);
}

// Constructs a cube of size (x, y, z) at position (posx, posy, posz)
// rotated by the given angle
void construct_cube(short x, short y, short z, short posx, short posy, short posz, Vex3Ds* angle, Cube* c) {
	x /= 2;
	y /= 2;
	z /=2 ;

	c->v[VEX_UBL] =  (Vex3D){-x, y, -z};
	c->v[VEX_UTL] =  (Vex3D){-x, y, z};
	c->v[VEX_UTR] =  (Vex3D){x, y, z};
	c->v[VEX_UBR] =  (Vex3D){x, y, -z};

	c->v[VEX_DBL] =  (Vex3D){-x, -y, -z};
	c->v[VEX_DTL] =  (Vex3D){-x, -y, z};
	c->v[VEX_DTR] =  (Vex3D){x, -y, z};
	c->v[VEX_DBR] =  (Vex3D){x, -y, -z};

	short v = -32767;

	c->normal[PLANE_BOTTOM] = (Vex3D){0, v, 0};
	c->normal[PLANE_TOP] = (Vex3D){0, -v, 0};
	c->normal[PLANE_FRONT] = (Vex3D){0, 0, -v};
	c->normal[PLANE_BACK] = (Vex3D){0, 0, v};
	c->normal[PLANE_LEFT] = (Vex3D){-v, 0, 0};
	c->normal[PLANE_RIGHT] =(Vex3D){v, 0, 0};


	int i;

	for(i = 0; i < 6; i++)
		c->cube[i] = -1;

#if 0
	Mat3x3 mat, mat2;
	x3d_construct_mat3_old(angle, &mat);

	x3d_construct_mat3(angle, &mat2);

	for(i = 0; i < 8; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->v[i], &mat);
		c->v[i].x = rot.x;
		c->v[i].y = rot.y;
		c->v[i].z = rot.z;
	}

	for(i = 0; i < 6; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->normal[i], &mat);
		c->normal[i].x = rot.x;
		c->normal[i].y = rot.y;
		c->normal[i].z = rot.z;
	}
#endif

	for(i = 0; i < 8; i++) {
		c->v[i].x += posx;
		c->v[i].y += posy;
		c->v[i].z += posz;
	}
}

// Takes a 3D polygon and projects all of the points into a 2D polygon
void project_polygon3d(Polygon3D* src, X3D_RenderContext* c, Polygon2D* dest) {
	int i;
	Vex2D temp[MAX_POINTS];

	for(i = 0; i < src->total_v; i++) {
		project_vex3d(c, &src->v[i], &temp[i]);
	}

	make_polygon2d(temp, src->total_v, dest);
}

// Given an array of 8 3D points that define a cube, this selects those
// points that belong to the given face
void cube_get_face(Vex3D v[8], short face,  Vex3D dest[4]) {
	int i;

	for(i = 0; i < 4; i++)
		dest[i] = v[cube_vertex_tab[face][i]];
}

// Gets the face opposite to the given face
// e.g. if you give it PLANE_LEFT it will give back PLANE_RIGHT
inline short get_opposite_face(short face) {
	if(face & 1)
		return face - 1;
	else
		return face + 1;
}

extern char render_method;

// Creates a 2D polygon from a list of 2D points
void make_polygon2d(Vex2D* v, int points, Polygon2D* p) {
	if(render_method == 0) {
		p->total_v = points;

		int i;
		Vex2D center = {0, 0};

		for(i = 0; i < points; i++) {
			center.x += v[i].x;
			center.y += v[i].y;
		}

		center.x /= points;
		center.y /= points;

		p->center = center;

		for(i = 0; i < points; i++) {
			short next = i + 1;

			if(next == points)
				next = 0;



			p->p[i].v = v[i];
			p->p[i].clipped = 0;
			get_line_info(&p->line[i], &v[i], &v[next], &center);
			p->line[i].draw = 1;
		}
	}
	else {
		short i;

		p->total_v = points;

		for(i = 0; i < points; i++) {
			p->line[i].draw = 1;
			p->p[i].v = v[i];
		}
	}
}

// Determines the distance a point is from a plane
// Note: this is the signed distance. If the point is on the
// normal side of the plane, this distance will be negative.
// If on the other side, it's positive. If on the plane, the distance
// will be 0
//
// Normal is the plane normal, v is a point on the plane, and point
// is the point you want to test
inline short dist_to_plane(Vex3D* normal, Vex3D* point, Vex3D* v) {
	Vex3D diff = {v->x - point->x, v->y - point->y, v->z - point->z};

	long x = (long)normal->x * diff.x;
	long y = (long)normal->y * diff.y;
	long z = (long)normal->z * diff.z;

	return (x + y + z) >> NORMAL_BITS;
}
