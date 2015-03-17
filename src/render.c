// C Source File
// Created 3/7/2015; 6:28:25 PM

#include "geo.h"
#include "extgraph.h"
#include "error.h"

#include <tigcclib.h>

// The table of the vertex numbers for each face
// The last vertex of each face is duplicated to make loops easy
const int cube_vertex_tab[6][5] = {
	{2, 1, 0, 3, 2},
	{6, 7, 4, 5, 6},
	{0, 1, 5, 4, 0},
	{2, 3, 7, 6, 2},
	{2, 6, 5, 1, 2},
	{3, 0, 4, 7, 3}
};

// Given a vertex id a and id b, edge_table[a][b] gets the edge id between a and b
char edge_table[8][8];
short edge_vertex_table[12][2];

// Given the face id, this gets the bitset of the edges in that face
unsigned short edge_face_table[6];


// Initializes a render context
void init_render_context(short w, short h, short x, short y, unsigned char fov, RenderContext* c) {
	ADDR(c);
	
	c->w = w;
	c->h = h;
	c->x = x;
	c->y = y;
	c->fov = fov;
	c->center_x = w / 2;
	c->center_y = h / 2;

	// Calulate the distance to the project plane (which is the scale)
	// dist = (w / 2) / tan(fov / 2)
	c->dist = FIXDIV8(w / 2, tanfp(fov / 2));
	c->frame = 0;
	
	// Calculate the unrotated planes of the view frustum
	calculate_frustum_plane_normals(c);
}

long line_count;

// Forces a point to be on the screen
void clip_point(Vex2D* v) {
	ADDR(v);
	
	if(v->x < 0)
		v->x = 0;
	else if(v->x >= (short)LCD_WIDTH)
		v->x = LCD_WIDTH - 1;
		
	if(v->y < 0)
		v->y = 0;
	else if(v->y >= (short)LCD_HEIGHT)
		v->y = LCD_HEIGHT - 1;
}

// Draws a polygon
void draw_polygon(Polygon2D* p, RenderContext* context) {
	int i, next;
	
	ADDR(p);
	ADDR(context);
	
	clip_point(&p->p[0].v);
	
	for(i = 0; i < p->total_v; i++) {
		//next = (i + 1) % p->total_v;
		next = i + 1;
		
		if(next == p->total_v)
			next = 0;
			
		clip_point(&p->p[next].v);
		
		
		//draw_clip_line(p->p[i].v.x, p->p[i].v.y, p->p[next].v.x, p->p[next].v.y, context->screen);
		
		if(p->line[i].draw) {
			//draw_clip_line(p->p[i].v.x, p->p[i].v.y, p->p[next].v.x, p->p[next].v.y, context->screen);
			FastLine_Draw_R(context->screen, p->p[i].v.x, p->p[i].v.y, p->p[next].v.x, p->p[next].v.y);
			line_count++;
		}
	}
}

short cube_id;

// Renders a single cube in writeframe
// Note: make sure the cube isn't off the screen at all!
void render_cube(Cube* c, RenderContext* context, Polygon2D* clip, short id) {
	Vex3D rot[8];
	int i, d;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z}; 
	
	ADDR(c);
	ADDR(context);
	ADDR(clip);
	
	cube_id = id;
	
	//if(id != 0 && id != 1)
	//	return;
	
	//printf("Visit %d\n", id);
	
	if(id == 5) {
		//print_polygon2d(clip);
	}
	
	for(i = 0; i < 8; i++) {
		Vex3D temp;
		
		// Translate the point to the opposite of the camera position
		add_vex3d(&c->v[i], &ncam_pos, &temp);
		
		// Rotate the point around the origin
		rotate_vex3d(&temp, &context->cam.mat, &rot[i]);
		
		//project_vex3d(context, &rot[i], &screen[i]);
	}
	
	union {
		Polygon3D poly3D;
		Polygon2D out_2d;
	} set_a;
	
	union {
		Polygon3D poly_out;
		Polygon2D dest;
	} set_b;
	
	for(i = 0; i < 6; i++) {
		cube_get_face(c->v, i, set_a.poly3D.v);
		
		set_a.poly3D.total_v = 4;
		
		//=========================================
			
		// Which edges of the current face we need to draw
		unsigned char draw_edges = 0;


		// If the angle between the view directions and the polygon normal < 45 deg,
		// we're going to assume that the polygon isn't visible
		if(dot_product(&c->normal[i], &context->cam.dir) > 23170)
			continue;
			
		// If we're on the wrong side of the plane, it must be invisible (backface culling)
		short dist = dist_to_plane(&c->normal[i], &context->cam.pos, &c->v[cube_vertex_tab[i][0]]);
		
		if(dist > 0)
			continue;

		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];
			
			draw_edges = (draw_edges >> 1) | ((unsigned short)((c->edge_bits & (1 << edge_table[a][b])) == 0) << 3);
			c->edge_bits |= (1 << edge_table[a][b]);
		}
		
		// If none of the edges need to be drawn and this isn't a portal, we can skip clipping it
		if(draw_edges == 0 && c->cube[i] == -1)
			continue;
		
		// Now that we know which edges need to be drawn, copy it over to the 3D polygon
		for(d = 0; d< set_a.poly3D.total_v; d++) {
			set_a.poly3D.draw[d] = draw_edges & 1;
			draw_edges >>= 1;
		}
	
		//=========================================
		
		Polygon2D* res = NULL;
		
		char draw_face = clip_polygon_to_frustum(&set_a.poly3D, &context->frustum, &set_b.poly_out);
		
		if(draw_face) {
	

			
			for(d = 0; d < set_b.poly_out.total_v; d++) {
				Vex3D temp;
				add_vex3d(&set_b.poly_out.v[d], &ncam_pos, &temp);
				rotate_vex3d(&temp, &context->cam.mat, &set_b.poly_out.v[d]);
			}
			
			project_polygon3d(&set_b.poly_out, context, &set_a.out_2d);

			for(d = 0; d < set_b.poly_out.total_v; d++)
				set_a.out_2d.line[d].draw = set_b.poly_out.draw[d];
			
			res = &set_b.dest;
			
			clip_polygon(&set_a.out_2d, clip, &set_b.dest, 0);//c->cube[i] == -1);
			
			
		
			draw_polygon(res, context);
		}
		
	//#endif
	
		
	#if 1
		// If there's a cube connected to this face
		if(c->cube[i] != -1) {
			Cube* next_cube = &cube_tab[c->cube[i]];
			
			//printf("i: %d Dist: %d\n", i, dist);
			
			Polygon2D* new_clip;
			
			if(draw_face) {
				new_clip = res;
			}
			else {
				new_clip = NULL;
			}
			
			if(dist > -120 && dist < 0 && id == context->cam.current_cube) {
				draw_face = 1;
				new_clip = clip;
			}
			
			if(id == 1) {
				//printf("ONE: %d i: %d\n", new_clip == clip, i);
				
				//if(i == PLANE_FRONT)
				//	print_polygon2d(clip);
			}
			
			//errorif(new_clip->total_v < 3, "Too few points in clip");
			
			// Make sure we haven't rendered it yet
			if(next_cube->last_frame != context->frame && draw_face && new_clip->total_v > 2) {
				// Pass over which edges have already been drawn
				cube_pass_edges(context, next_cube, i);
				
				if(!(next_cube->edge_bits & (1 << 15))) {
					render_cube(next_cube, context, new_clip, c->cube[i]);
				}
			}
			
			//printf(
		}
	#endif
	}
}

// Renders the level, starting from the cube the camera is currently in
void render_level(RenderContext* c) {
	ADDR(c);
	
	// Create the clipping region
	Vex2D screen_clip[] = {
		{
			2, 2
		},
		{
			LCD_WIDTH - 3, 2
		},
		{
			LCD_WIDTH - 3, LCD_HEIGHT - 5
		},
		{
			2, LCD_HEIGHT - 5
		}
	};
	
	Polygon2D clip_region;
	
	make_polygon2d(screen_clip, 4, &clip_region);
	
	cube_tab[c->cam.current_cube].edge_bits = cube_tab[c->cam.current_cube].invisible_edges;
	cube_tab[c->cam.current_cube].last_frame = c->frame;
	
	render_cube(&cube_tab[c->cam.current_cube], c, &clip_region, c->cam.current_cube);
	
	//draw_polygon(&clip_region, c);
}

// Passes information to another cube about which edges have already been drawn
// This prevents the 4x overdraw problem
void cube_pass_edges(RenderContext* c, Cube* to, short face) {
	ADDR(c);
	ADDR(to);
	
	if(c->frame != to->last_frame) {
		to->edge_bits = to->invisible_edges;
		to->last_frame = c->frame;
	}
	
	
	to->edge_bits |= edge_face_table[get_opposite_face(face)];
}


// Sets the position of the camera and updates the plane distances
// of the viewing frustum
void set_cam_pos(RenderContext* c, short x, short y, short z) {
	//ADDR(c):
	
	c->cam.pos = (Vex3D){x, y, z};
	
	calculate_frustum_plane_distances(c);
	
	// Calculate the distance from the origin to the camera
	//c->cam.dist_from_origin = get_vex3d_magnitude(&c->cam.pos);
	
	// Update the plane equations
	int i;
	
	//for(i = 0; i < c->frustum.total_p; i++)
	//	c->frustum.p[i].d = c->frustum_unrotated.p[i].d + c->cam.dist_from_origin;
	
}

// Sets the angle of the camera and updates the planes of the view
// frustum
void set_cam_angle(RenderContext* c, unsigned char x, unsigned char y, unsigned char z) {
	ADDR(c);
	
	c->cam.angle = (Vex3Ds){x, y, z};
	
	// Construct the new rotation matrix
	construct_mat3x3(&c->cam.angle, &c->cam.mat);
	
	// Get the direction the camera is facing
	// This exploits the fact that the first column of the rotation matrix turns
	// out to be the direction the camera is facing
	c->cam.dir.x = c->cam.mat[2][0];
	c->cam.dir.y = c->cam.mat[2][1];
	c->cam.dir.z = c->cam.mat[2][2];
	
	c->cam.straif_dir.x = c->cam.mat[0][0];
	c->cam.straif_dir.y = c->cam.mat[0][1];
	c->cam.straif_dir.z = c->cam.mat[0][2];
	
	// Update the view frustum
	calculate_frustum_rotated_normals(c);
	//calculate_frustum_plane_distances(c);
	
	set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
	
	
}

// Builds the edge table, which allows us to determine which face has what
// edges
void build_edge_table() {
	int i, d;
	
	short id = 0;
	
	for(i = 0; i < 8; i++)
		for(d = 0; d < 8; d++)
			edge_table[i][d] = -1;
	
	// There's probably a better way to do this... oh well
	for(i = 0; i < 6; i++) {
		unsigned short bit = 0;
		
		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];
			
			if(edge_table[a][b] == -1) {
				bit |= ((unsigned short)1 << id);
				
				edge_vertex_table[id][0] = a;
				edge_vertex_table[id][1] = b;
				
				edge_table[a][b] = id;
				edge_table[b][a] = id++;
			}
			else {
				bit |= ((unsigned short)1 << edge_table[a][b]);
			}
		}
		
		edge_face_table[i] = bit;
	}
}


// Initializes the renderer
void init_render() {
	build_edge_table();
}

//#define MIN_FAIL_DIST 20

// Returns whether the point is inside the cube or not
// Fail plane will contain the id of the plane it fails against
char point_in_cube(int id, Vex3D* point, char* fail_plane) {
	ADDR(point);
	
	
	Cube* c = &cube_tab[id];
	int i;
	
	for(i = 0; i < 6; i++) {
		const Vex3D normal = c->normal[i];
		const Vex3D p = c->v[cube_vertex_tab[i][0]];
		
		if(normal.x || normal.y || normal.z) {
			long dot = (long)normal.x * p.x + (long)normal.y * p.y + (long)normal.z * p.z;
			long val = (long)normal.x * point->x + (long)normal.y * point->y + (long)normal.z * point->z - dot;
			
			val >>= NORMAL_BITS;
			
			//printf("Val: %ld\n", val);
			//LCD_restore(buf->dark_plane);
			
			if(c->cube[i] == -1) {
				if(val < DIST_TO_NEAR_PLANE) {
					*fail_plane = i;
					return 0;
				}
			}
			else if(val < 0) {
				*fail_plane = i;
				return 0;
			}
		}
	}
		
	return 1;
}

// Attemps to move the camera and update which cube the camera is in
void attempt_move_cam(RenderContext* c, Vex3D* dir, short speed) {
	char fail_plane;
	
	
	
	Vex3D add = {
		((long)dir->x * speed) >> NORMAL_BITS,
		((long)dir->y * speed) >> NORMAL_BITS,
		((long)dir->z * speed) >> NORMAL_BITS
	};
	
	Vex3D new_pos = {c->cam.pos.x + add.x, c->cam.pos.y + add.y, c->cam.pos.z + add.z}; 
	
	if(point_in_cube(c->cam.current_cube, &new_pos, &fail_plane)) {
		c->cam.pos = new_pos;
		
		set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
	}
	else {
		if(cube_tab[c->cam.current_cube].cube[(short)fail_plane] != -1) {
			c->cam.pos = new_pos;
			c->cam.current_cube = cube_tab[c->cam.current_cube].cube[(short)fail_plane];
			set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
		}
	}
}





















