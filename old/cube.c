// C Source File
// Created 2/16/2015; 11:24:28 PM

#include <tigcclib.h>

#include "3D.h"
#include "extgraph.h"
#include "graphics.h"
#include "clip.h"

enum {
	PLANE_BOTTOM,
	PLANE_TOP,
	PLANE_LEFT,
	PLANE_RIGHT,
	PLANE_BACK,
	PLANE_FRONT
};

const int cube_vertex_tab[6][5] = {
	{2, 1, 0, 3, 2},
	{6, 7, 4, 5, 6},
	{0, 1, 5, 4, 0},
	{2, 3, 7, 6, 2},
	{2, 6, 5, 1, 2},
	{3, 0, 4, 7, 3}
};

typedef struct {
	Vex3D v[8];
	Vex3D normal[8];
	char color[8];
	short cube[6];
	
	unsigned short last_frame;
	unsigned short edge_bits;
} Cube;

typedef struct {
	Vex3D v[4];
} Face;

typedef struct{
	short x;
	short pad1;
	short y;
	short pad2;
	short z;
	short pad3;
} Vex3D_rot;

short rotate_point_local(Vex3D_rot *dest asm("a3"), Vex3D *src asm("a4"), Mat3x3* mat asm("a5"));

inline void cube_get_face_3D(Face* dest, Vex3D_rot v[], int face) {
	int i;
	
	for(i = 0; i < 4; i++)
		dest->v[i] = (Vex3D){v[cube_vertex_tab[face][i]].x, v[cube_vertex_tab[face][i]].y, v[cube_vertex_tab[face][i]].z};
}

extern void* Vscreen0;
extern void* Vscreen1;

inline long dot_product(Vex3D* a, Vex3D* b) {
	return (((long)a->x * b->x) >> 2) + (((long)a->y * b->y) >> 2) + (((long)a->z * b->z) >> 2);
}

short scale_factor;
extern short lcd_w, lcd_h;

// Takes a 3D point and projects it onto the 2D screen
inline void project_3D(CBuffer* buf, Vex3D* src, Vex2D* dest) {
	short cx = buf->width / 2;
	short cy = buf->height / 2;
	
	dest->x = (((long)buf->scale_factor * src->x) / (src->z)) + cx;
	dest->y = (((long)buf->scale_factor * src->y) / (src->z)) + cy;
}

// Determines whether a polygon passes the "facing test" i.e. the camera
// lies on the visible side of the polygon
inline char polygon_visible(Vex3D* normal, Vex3D* cam_pos, Vex3D* v) {
	Vex3D diff = {v->x - cam_pos->x, v->y - cam_pos->y, v->z - cam_pos->z};
	
	return (long)normal->x * diff.x + (long)normal->y * diff.y + (long)normal->z * diff.z < 0;
}


#define MIN 15

int was_clipped;


char clip_ray_double(Vex3D* v1, Vex3D* v2) {
	was_clipped = 0;
	if(v1->z < MIN && v2->z < MIN) return 0;

	if(v1->z >= MIN && v2->z >= MIN) return 1;
	
	double t;

	if(v1->z < MIN) {
	
		double dz = v1->z - v2->z;
		t = ((double)MIN - v1->z) / dz;

		v1->x = ((double)v1->x - v2->x) * t + v1->x;
		v1->y = ((double)v1->y - v2->y) * t + v1->y;
		v1->z = ((double)v1->z - v2->z) * t + v1->z;
		was_clipped = 0;
	}
	else {
		double dz = v2->z - v1->z;
		t = ((double)MIN - v2->z) / dz;

		v2->x = ((double)v2->x - v1->x) * t + v2->x;
		v2->y = ((double)v2->y - v1->y) * t + v2->y;
		v2->z = ((double)v2->z - v1->z) * t + v2->z;
		was_clipped = 1;
	}
	
	return 1;
}

// Clips a 3D line segment against the near clipping plane
// Returns whether the line was actually clipped
char clip_ray(Vex3D* v1, Vex3D* v2, short min) {
	was_clipped = 0;
	if(v1->z < min && v2->z < min) return 0;

	if(v1->z >= min && v2->z >= min) return 1;

	if(v1->z < min) {
		short dx = v1->x - v2->x;
		short dy = v1->y - v2->y;
		short dz = v1->z - v2->z;
		
		long t = ((((long)(min - v1->z)) << 8) / dz);
		
		//if(abs(t) > 32767)
		//	error("t out of range");
		
		v1->x = (((long)dx * t) >> 8) + v1->x;
		v1->y = (((long)dy * t) >> 8) + v1->y;
		//v1->z = (((long)dz * t) >> 8) + v1->z;
		v1->z = min;
		was_clipped = 0;
	}
	else {
		short dx = v2->x - v1->x;
		short dy = v2->y - v1->y;
		short dz = v2->z - v1->z;
		
		long t = ((((long)(min - v2->z)) << 8) / dz);
		
		//if(abs(t) > 32767)
		//	error("t out of range");
		
		v2->x = (((long)dx * t) >> 8) + v2->x;
		v2->y = (((long)dy * t) >> 8) + v2->y;
		//v2->z = (((long)dz * t) >> 8) + v2->z;
		v2->z = min;
		was_clipped = 1;
	}

	return 1;
}

// Determines the distance a point is from a plane
// Note: this is the signed distance. If the point is on the
// normal side of the plane, this distance will be negative.
// If on the other side, it's positive. If on the plane, the distance
// will be 0
inline short plane_dist(Vex3D* normal, Vex3D* cam_pos, Vex3D* v) {
	Vex3D diff = {v->x - cam_pos->x, v->y - cam_pos->y, v->z - cam_pos->z};
	
	long x = (long)normal->x * diff.x;
	long y = (long)normal->y * diff.y;
	long z = (long)normal->z * diff.z;
	
	return (x + y + z) >> NORMAL_BITS;//dot >> NORMAL_BITS;
}


void XGrayFilledTriangle_R(short x1 asm("%d0"),short y1 asm("%d1"),short x2 asm("%d2"),short y2 asm("%d3"),short x3 asm("%d4"),short y3 asm("%a1"),void *planes asm("%a0"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) asm("%a2"));

//
inline void line_info(Line* dest, Vex2D* start, Vex2D* end) {
	short dx = end->x - start->x;
	short dy = end->y - start->y;
	
	dest->slope = ((long)dx << LINE_BITS) / dy;
	dest->start = *start;
}

short line_intersect(Line* line, short y) {
	return line->start.x + (((long)line->slope * (y - line->start.y)) >> LINE_BITS);
}

void save_polygon(FILE* file, Polygon* p) {
	int i;
	
	for(i = 0; i < p->total_points; i++) {
		fprintf(file, "{%d,%d} s: %ld b: %d\n", p->p[i].v.x, p->p[i].v.y, p->line[i].slope, p->line[i].b);
	}
}

void draw_line(Line2D* line);

void draw_polygon_slope(CBuffer* buf, Polygon* p) {
	int i;
	
	PortRestore();
	clrscr();
	
	for(i = 0; i < p->total_points; i++) {
		draw_line(&p->line[i]);
	}
	
	DrawChar(200, 110, 'W', A_NORMAL);
	ngetchx();
	PortSet(buf->dark_plane, 239 - 1, 127 - 1);
}

Polygon* clip_quad(CBuffer* buf, Vex3D* v, Polygon* dest, Polygon* temp_a, Polygon* temp_b, Polygon* clip, char save, char draw, short near_dist) {
	int point;
	int next_point;
	char next;
	short out[20];
	short out_pos = 0;
	int i;
	Vex2D project;
	int pos[20];
	int line_pos[20];
	
	Vex3D v2[20];
	int v2_pos = 0;
	
	draw = 0b1111;
	
	for(i = 0; i < 20; i++)
		pos[i] = -1;
	
	
	char side = v[0].z < near_dist ? -1 : 1;
	char next_side;
	char clipped;
	
	dest->total_points = 0;
	
	for(point = 0; point < 4; point++) {
		next_point = (point + 1) % 4;
		next_side = v[next_point].z < near_dist ? -1 : 1;
		
		if(side != -1) {
			project_3D(buf, &v[point], &project);
			dest->p[dest->total_points++] = (Point){0, project};
			pos[point] = dest->total_points - 1;
			
			v2[v2_pos++] = v[point];
		}
		
		clipped = 0;
		if(side + next_side == 0 && side) {
			Vex3D p1 = v[point];
			Vex3D p2 = v[next_point];
			
			Vex3D p1t = v[point];
			Vex3D p2t = v[next_point];
			
			Vex3D p1s = v[point];
			Vex3D p2s = v[next_point];
			
			char ray = clip_ray(&p1, &p2, near_dist);
			
			if(ray) {
				Vex3D* po = (side == -1 ? &p1 : &p2);
				
			#if 0
				if(buf->save_poly) {
					clip_ray_double(&p1t, &p2t);
					
					fprintf(buf->file, "====================\n");
					fprintf(buf->file, "int: {%d, %d, %d} to {%d, %d, %d}\n", p1s.x, p1s.y, p1s.z, p2s.x, p2s.y, p2s.z);
					fprintf(buf->file, "res: {%d, %d, %d} to {%d, %d, %d}\n", p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
					fprintf(buf->file, "dub: {%d, %d, %d} to {%d, %d, %d}\n", p1s.x, p1s.y, p1s.z, p2s.x, p2s.y, p2s.z);
					fprintf(buf->file, "res: {%d, %d, %d} to {%d, %d, %d}\n", p1t.x, p1t.y, p1t.z, p2t.x, p2t.y, p2t.z);
					fprintf(buf->file, "====================\n");
				}
			#endif
				
				
				project_3D(buf, po, &project);
				dest->p[dest->total_points] = (Point){1, project};
				
				pos[side == -1 ? point : next_point] = dest->total_points++;
				clipped = 1;
				
				v2[v2_pos++] = *po;
			}
		}
		
		if(clipped || side == 0) {
			out[out_pos++] = dest->total_points - 1;
		}
		
		side = next_side;
	}
	
	Vex2D center = {0, 0};
	
	for(i = 0; i < dest->total_points; i++) {
		center.x += dest->p[i].v.x;
		center.y += dest->p[i].v.y;
	}
	
	if(dest->total_points != 0) {
		center.x /= dest->total_points;
		center.y /= dest->total_points;
	}
	
	for(i = 0; i < dest->total_points; i++)
		dest->line[i].draw = 1;
	
#if 0
	for(i = 0; i < 4; i++) {
		if(pos[i] != -1) {
			dest->line[pos[i]].draw = draw & 1;
		}
		
		draw >>= 1;
	}
#endif
	
	for(i = 0; i < dest->total_points; i++) {
		short next = (i + 1) % dest->total_points;
		
		get_line_info(&dest->line[i], &dest->p[i].v, &dest->p[next].v, &center);
		
		//dest->line[i].draw = draw & 1;
		
		//draw >>= 1;
	}
	
	
/*
	if(out_pos == 2) {
		short min_p = min(out[0], out[1]);
		short max_p = max(out[0], out[1]);
		
		//printf("Min out: %d\n", min_p);
		//printf("Max out: %d\n", max_p);
	
#if 1	
		if(min_p == 0 && max_p == dest->total_points - 1) {
			dest->line[dest->total_points - 1].draw = 0;
		}
		else {
			dest->line[min_p].draw = 0;
		}
#endif
	}
*/

	for(i = 0; i < out_pos - 1; i++) {
		if(out[i] == out[i + 1] - 1) {
			//printf("Case\n");
			//dest->line[out[i]] = *edge;
			dest->line[out[i]].draw = 0;
		}
	}
	
	if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_points - 1) {
		//dest->line[dest->total_points - 1] = *edge;
		dest->line[dest->total_points - 1].draw = 0;
	}

	
	
	if(out_pos != 0 && out_pos != 2)
		error("Invalid out A\n");
	
	if(dest->total_points != 0) {
		Polygon* res = clip_polygon(dest, clip, temp_a, temp_b);
		
		//if(res->total_points != 0)
		//	draw_polygon2(res, buf->dark_plane);
		
	#if 1
		if(buf->save_poly && save) {
			fprintf(buf->file, "------------------------\n");
			fprintf(buf->file, "===Clip region\n");
			save_polygon(buf->file, clip);
			
			fprintf(buf->file, "===Original\n");
			
			int j;
			for(j = 0; j < 4; j++)
				fprintf(buf->file, "{%d, %d, %d}\n", v[j].x, v[j].y, v[j].z);
			
			fprintf(buf->file, "===3D Clipped\n");
			for(j = 0; j < v2_pos; j++)
				fprintf(buf->file, "{%d, %d, %d}\n", v2[j].x, v2[j].y, v2[j].z);
				
			for(j = 0; j < res->total_points; j++)
				res->line[j].draw = 1;
			
			fprintf(buf->file, "===Before clipping\n");
			save_polygon(buf->file, dest);
			fprintf(buf->file, "===After clipping\n");
			save_polygon(buf->file, res);
			fprintf(buf->file, "------------------------\n");
			
			//draw_polygon_slope(buf, res);
		}
	#endif
	
		for(i = 0; i < res->total_points; i++)
			res->line[i].draw = 1;
		
		return res;
	}
	else {
		//printf("removed\n");
	}
	
	return dest;
}


#define TOTAL_CUBES 5

Cube cube_tab[TOTAL_CUBES];

// Given a vertex id a and id b, edge_table[a][b] gets the edge id between a and b
char edge_table[8][8];
short edge_vertex_table[12][2];

// Given the face id, this gets the bitset of the edges in that face
unsigned short edge_face_table[6];

short get_opposite_face(short face) {
	if(face & 1)
		return face - 1;
	else
		return face + 1;
}

void cube_pass_edges(CBuffer* buf, Cube* to, short face) {
	if(buf->frame != to->last_frame) {
		to->edge_bits = 0;
		to->last_frame = buf->frame;
	}
	
	
	to->edge_bits |= edge_face_table[get_opposite_face(face)];
}

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
	
	for(i = 0; i < 6; i++) {
		unsigned short b = edge_face_table[i];
		
		for(d = 0; d < 12; d++) {
			if(b & (1 << 11)) {
				printf("1");
			}
			else {
				printf("0");
			}
			
			b <<= 1;
		}
		printf("\n");
	}
	
	ngetchx();
	
	for(i = 0; i < 8; i++) {
		for(d = 0; d < 8; d++) {
			printf("%d ", edge_table[i][d]);
		}
		printf("\n");
	}
	
	if(id != 12)
		error("ERR");
}

void save_bin(FILE* file, unsigned short val, char bits) {
	int i;
	
	for(i = 0; i < bits; i++) {
		fprintf(file, "%d", (val & (1 << (bits - 1))) != 0);
		val <<= 1;
	}
	
	fprintf(file, "\n");
}

void render_cube(CBuffer* buf, Cube* c, Cam* cam, char outline, Polygon* clip, short id) {
	Mat3x3* mat = &cam->mat;
	Vex3D* view = &cam->dir;
	Vex3D* cam_pos = &cam->pos;
	
	if(buf->save_poly)
		fprintf(buf->file, "enter cube %d\n", id);
	
	//if(buf->lines_left == 0)
	//	return;
		
	c->edge_bits |= (1 << 15);
	
	if(id != 0 && id != 1)
		return;
	
	
	
	short cx = buf->width / 2;
	short cy = buf->height / 2;
	Vex3D_rot output[8];
	short x[8];
	short y[8];
	char vis[6];
	
	int i, d;
	
	for(i = 0; i < 8; i++) {
		Vex3D p = {c->v[i].x - cam_pos->x, c->v[i].y - cam_pos->y, c->v[i].z - cam_pos->z};
		
		rotate_point_local(&output[i], &p, mat); 
		
		x[i] = (((long)buf->scale_factor * output[i].x) / (output[i].z)) + cx;
		y[i] = (((long)buf->scale_factor * output[i].y) / (output[i].z)) + cy;
		
		//printf("x[i] = %d\n", y[i]);
	}
	
	//unsigned char edge_table[8];
	
	short ctab[4];
	
	//for(i = 0; i < 8; i++)
	//	edge_table[i] = 0;
	
	if(1) {
	#if 0
		for(i = 0; i < 6; i++) {
			Vex3D_rot normal;
			
			//rotate_point_local(&normal, &c->normal[i], mat);
			vis[i] = polygon_visible(&c->normal[i], cam_pos, &c->v[cube_vertex_tab[i][0]]);
			
			if(vis[i] && c->cube[i] != -1) {
				Cube* c2 = &cube_tab[c->cube[i]];
			
				cube_pass_edges(buf, c2, i);

				if(!(c2->edge_bits & (1 << 15))) {
					//render_cube(buf, c2, cam, outline);
				}
			}
		}
	#endif
		
		if(1) {
			if(buf->save_poly) {
				fprintf(buf->file, "Vis for cube %d\n", id);
			}
				
			
			for(i = 0; i < 6; i++) {	
				vis[i] = polygon_visible(&c->normal[i], cam_pos, &c->v[cube_vertex_tab[i][0]]);
				
				long dist = plane_dist(&c->normal[i], cam_pos, &c->v[cube_vertex_tab[i][0]]);
				
				if(buf->save_poly)
					fprintf(buf->file, "%d: %d\n", i, (short)vis[i]);
				
				if(vis[i]) {
					//if(i != 5)
					//	error("NOT 5");
					
					if(1) {
						Face face;
						cube_get_face_3D(&face, output, i);
						
						Polygon* res;
						Polygon temp_a, temp_b;
						Polygon temp_c;
						
						unsigned char draw = 0;
							
					#if 0
						if(buf->save_poly) {
							fprintf(buf->file, "face: %d\nTemplate: ", i);
							save_bin(buf->file, edge_face_table[i], 12);
							save_bin(buf->file, c->edge_bits, 12);
						}
					#endif
						
						//if(c->cube[i] == -1) {
							for(d = 0; d < 4; d++) {
								short a = cube_vertex_tab[i][d];
								short b = cube_vertex_tab[i][d + 1];
								
								draw = (draw >> 1) | ((unsigned short)((c->edge_bits & (1 << edge_table[a][b])) == 0) << 3);
								c->edge_bits |= (1 << edge_table[a][b]);
								
								
								//printf("Draw: %u\n", draw);
								//printf("Edge: %u\n", c->edge_bits);
								//LCD_restore(buf->dark_plane);
								//ngetchx();
								
							}
						//}
						
					#if 0
						if(buf->save_poly) {
							save_bin(buf->file, c->edge_bits, 12);
							save_bin(buf->file, draw, 4);
							fprintf(buf->file,"-------------------\n");
						}
					#endif
						
						//draw = 0b00001100;

						char force = 0;
						

						res = clip_quad(buf, face.v, &temp_c, &temp_a, &temp_b, clip, c->cube[i] != -1, draw, 
							c->cube[i] != -1 ? 10 : 10);

							
						if(dist < -15)
							res = clip_quad(buf, face.v, &temp_c, &temp_a, &temp_b, clip, c->cube[i] != -1, draw, 10);
						else {
							force = 1;
						}
						
						if((id == 0 && i == 4) || (id == 1 && i == PLANE_FRONT))
							printf("\n\ni: %d, DIFF: %ld\n force: %d\n", i, dist, force);
						
						if(res->total_points != 0 || force) {		
							if(c->cube[i] != -1 && id == 1)
								draw_polygon2(res, buf->dark_plane);
		
							if(c->cube[i] != -1) {
								Cube* c2 = &cube_tab[c->cube[i]];
				
								if(res->total_points != 0 || force) {
									cube_pass_edges(buf, c2, i);
					
									if(!(c2->edge_bits & (1 << 15))) {										
										if(buf->save_poly) {
											//fprintf(buf->file, "=========CLIP AFTER\n");
											//save_polygon(buf->file, res);
										}
										
									#if 1
										if(dist < -15)
											render_cube(buf, c2, cam, outline, res, c->cube[i]);
										else {
											render_cube(buf, c2, cam, outline, clip, c->cube[i]);
											//printf("DIFF\n");
										}
									#endif
										//render_cube(buf, c2, cam, outline, clip, c->cube[i]);
										
									}
								}
								
								int j;
								
								//draw_polygon2(res, buf->dark_plane);
								
								for(j = 0; j < res->total_points; j++) {
									//res->p[j].v.x++;
									//res->p[j].v.y++;
								}
							}
						}
						
						
						Vex3D tri1[] = {face.v[0], face.v[1], face.v[2]};
						Vex3D tri2[] = {face.v[0], face.v[3], face.v[2]};
						
						short color = get_color(&cam->dir, &c->normal[i]);
						
						if(outline)
							color = c->color[i];
							
						ctab[i] = color;
						
						//char draw[4];
							
						
						//clip_triangle(buf, tri1, color, draw[0], draw[1]);
						
						//clip_triangle(buf, tri2, color, draw[3], draw[2]);
					}
					
					
					/*
					if(outline) {
						for(d = 0; d < 3; d++){
							render_ray(&face.v[d], &face.v[d + 1]);
						}
				
						render_ray(&face.v[0], &face.v[3]);
					}*/
					
					
					
					//render_ray(list2[v1],list2[v4]);
					
					
					#if 0
					Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
					Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
					asm_gray_tri(face.v[0].x, face.v[0].y, face.v[1].x, face.v[1].y, face.v[2].x, face.v[2].y, c->color[i]);
					//printf("%d is visible\n", i);
					
					Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
					Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
					asm_gray_tri(face.v[0].x, face.v[0].y, face.v[3].x, face.v[3].y, face.v[2].x, face.v[2].y, c->color[i]);
					#endif
				
					
					//printf("Vis: %d\n", i);
					
				}
			}
		
			//GrayDBufToggleSync();
			//ngetchx();
			//GrayDBufToggleSync();
			
			
				
		}
	#ifdef MODE_GRAY
		for(i = 0; i < 6; i++) {
			if(vis[i] && c->cube[i] != -1) {
				Cube* c2 = &cube_tab[c->cube[i]];
			
				cube_pass_edges(buf, c2, i);

				if(!(c2->edge_bits & (1 << 15))) {
					render_cube(buf, c2, cam, outline);
				}
			}
		}
	#endif
	
	
#if 0	
	// New edge drawing code!
	unsigned short edge = c->edge_bits;
	
	for(i = 0; i < 12; i++) {
		if((edge & 1) == 0) {
			// This edge hasn't been drawn yet, so draw it
			short a = edge_vertex_table[i][0];
			short b = edge_vertex_table[i][1];
			
			Vex3D aa = {output[a].x, output[a].y, output[a].z};
			Vex3D bb = {output[b].x, output[b].y, output[b].z};
			
			render_ray(buf, &aa, &bb);
			
			if(outline) {
				LCD_restore(buf->dark_plane);
				ngetchx();
			}
		}
		
		edge >>= 1;
	}
#endif
	
	
		
	#if 0
		if(1) {
			for(i = 0; i < 6; i++) {
				if(vis[i]) {// && ctab[i] == COLOR_WHITE) {
					
					unsigned char edge_table[8];
					
					for(d = 0; d < 8; d++)
						edge_table[i] = 0;	
					
					seg_calls++;
					
					Face face;
					cube_get_face_3D(&face, output, i);	
				
					for(d = 0; d < 3; d++) {
						short a = cube_vertex_tab[i][d];
						short b = cube_vertex_tab[i][d + 1];
						
						if(b < a) {
							short temp = a;
							a = b;
							b = temp;
						}
						
						if(!(edge_table[a] & (1 << b))) {
							render_ray(buf, &face.v[d], &face.v[d + 1]);
							edge_table[a] |= (1 << b);
							
							
							
							if(outline) {
								LCD_restore(buf->dark_plane);
								ngetchx();
							}
						}
					}
					
					short a = cube_vertex_tab[i][0];
					short b = cube_vertex_tab[i][3];
					
					if(b < a) {
						short temp = a;
						a = b;
						b = temp;
					}
					
					if(!(edge_table[a] & (1 << b))) {
						render_ray(buf, &face.v[0], &face.v[3]);
						edge_table[a] |= (1 << b);
						
						
						
						if(outline) {
							LCD_restore(buf->dark_plane);
							ngetchx();
						}
					}
				}
			}
		}
		#endif
	}
	else {
		
		#if 0
		for(i = 0; i < 6; i++) {
			//polygon_visible(view, &c->normal[i], cam_pos, &output[cube_vertex_tab[i][0]], i, outline, &c->v[cube_vertex_tab[i][0]]);
		}
		
		// Top
		for(i = 0; i < 3; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 1], y[i + 1], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		GrayDrawClipLine2B(x[0], y[0], x[3], y[3], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		
		// Bottom
		for(i = 4; i < 7; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 1], y[i + 1], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		GrayDrawClipLine2B(x[4], y[4], x[7], y[7], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		
		// Sides
		for(i = 0; i < 4; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 4], y[i + 4], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		#endif
		
	}
	
	buf->first_cube = 0;
}

enum{
	VEX_UBL,
	VEX_UTL,
	VEX_UTR,
	VEX_UBR,
	VEX_DBL,
	VEX_DTL,
	VEX_DTR,
	VEX_DBR
};

inline Vex3D mVex3D(int x, int y, int z) {
	return (Vex3D){x, y, z};
}

void make_cube(Cube* c, int x ,int y, int z, int posx, int posy, int posz, Vex3D* angle){
	x/=2;
	y/=2;
	z/=2;

	c->v[VEX_UBL] =  mVex3D(-x, y, -z);
	c->v[VEX_UTL] =  mVex3D(-x, y, z);
	c->v[VEX_UTR] =  mVex3D(x, y, z);
	c->v[VEX_UBR] =  mVex3D(x, y, -z);
	
	c->v[VEX_DBL] =  mVex3D(-x, -y, -z);
	c->v[VEX_DTL] =  mVex3D(-x, -y, z);
	c->v[VEX_DTR] =  mVex3D(x, -y, z);
	c->v[VEX_DBR] =  mVex3D(x, -y, -z);
	
	int v = -32767;
	
	#if 1
	c->normal[PLANE_BOTTOM] = mVex3D(0, v, 0);
	c->normal[PLANE_TOP] = mVex3D(0, -v, 0);
	c->normal[PLANE_FRONT] = mVex3D(0, 0, -v);
	c->normal[PLANE_BACK] = mVex3D(0, 0, v);
	c->normal[PLANE_LEFT] = mVex3D(-v, 0, 0);
	c->normal[PLANE_RIGHT] = mVex3D(v, 0, 0);
	
	c->color[PLANE_BOTTOM] = COLOR_BLACK;
	c->color[PLANE_TOP] = COLOR_BLACK;
	c->color[PLANE_LEFT] = COLOR_LIGHTGRAY;
	c->color[PLANE_RIGHT] = COLOR_LIGHTGRAY;
	c->color[PLANE_FRONT] = COLOR_DARKGRAY;
	c->color[PLANE_BACK] = COLOR_DARKGRAY;
	#endif
	
	c->normal[PLANE_BOTTOM] = mVex3D(0, v, 0);
	c->normal[PLANE_TOP] = mVex3D(0, -v, 0);
	c->normal[PLANE_FRONT] = mVex3D(0, 0, -v);
	c->normal[PLANE_BACK] = mVex3D(0, 0, v);
	c->normal[PLANE_LEFT] = mVex3D(-v, 0, 0);
	c->normal[PLANE_RIGHT] = mVex3D(v, 0, 0);
	
	
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

void connect_cube(int parent, int child, int plane) {
	cube_tab[parent].cube[plane] = child;
	cube_tab[child].cube[get_opposite_face(plane)] = parent;
}

/*
char point_in_cube(int id, Vex3D* point, char* fail_plane) {
	int i;
	
	Cube* c = &cube_tab[id];
	
	for(i = 0; i < 6; i++) {
		if(plane_dist(&c->normal[i], point, &c->v[cube_vertex_tab[i][0]]) > 0) {
			*fail_plane = i;
			return 0;
		}
	}
	
	return 1;
}
*/

#define MIN_FAIL_DIST 15

char point_in_cube(CBuffer* buf, int id, Vex3D* point, char* fail_plane) {
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
				if(val < MIN_FAIL_DIST) {
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


void attempt_move(CBuffer* buf, Vex3D* pos, Vex3D* add) {
	char fail_plane;
	
	Vex3D new_pos = {pos->x + add->x, pos->y + add->y, pos->z + add->z};
	
	if(point_in_cube(buf, buf->current_cube, &new_pos, &fail_plane)) {
		*pos = new_pos;
	}
	else {
		if(cube_tab[buf->current_cube].cube[fail_plane] != -1) {
			*pos = new_pos;
			buf->current_cube = cube_tab[buf->current_cube].cube[fail_plane];
		}
	}
}


void init_cubes() {
	Vex3D cube_angle = {0, 0, 0};
	
	int s = 200;
	
	make_cube(&cube_tab[0], s, s, s, 0, 0, 0, &cube_angle);
	make_cube(&cube_tab[1], s, s, s, 0, 0, s, &cube_angle);
	
	make_cube(&cube_tab[3], s, s, s, 0, -s, s, &cube_angle);
	
	make_cube(&cube_tab[2], s, s, s, -s, 0, s, &cube_angle);
	
	//cube_tab[0].cube[PLANE_BACK] = 1;
	//cube_tab[1].cube[PLANE_LEFT] = 2;
	//cube_tab[1].cube[PLANE_TOP] = 3;
	
	connect_cube(0, 1, PLANE_BACK);
	connect_cube(1, 2, PLANE_LEFT);
	connect_cube(1, 3, PLANE_TOP);
	
}

void invert_normals() {
	int i, d;
	
	for(i = 0; i < TOTAL_CUBES; i++) {
		for(d = 0; d < 6; d++) {
			cube_tab[i].normal[d].x = -cube_tab[i].normal[d].x;
			cube_tab[i].normal[d].y = -cube_tab[i].normal[d].y;
			cube_tab[i].normal[d].z = -cube_tab[i].normal[d].z;
		}
	}
}

void test_cube(CBuffer* buf, Cam* cam, char outline, Vex3D* cube_angle) {
	Cube c, c2;
	
	buf->first_cube = 1;
	cube_tab[buf->current_cube].last_frame = buf->frame;
	cube_tab[buf->current_cube].edge_bits = 0;
	
	render_cube(buf, &cube_tab[buf->current_cube], cam, outline, buf->clip, buf->current_cube);
	return;
	
	
	
	Vex3D cube_angle2 = {cube_angle->y, -cube_angle->y + 360, 0};
	
	if(cube_angle2.y >= 360)
		cube_angle2.y -= 360;
	
	make_cube(&c, 75, 75, 75, 0, 0, 0, cube_angle);
	//make_cube(&c2, 40, 40, 40, 0, -70, 200, &cube_angle2);
	
	Vex3D angle = {0, 0, 0};
	
	
	//cam->dir = cam->mat[0][1];
	
	int i, j;
	
	/*
	for(j = 0; j < 2; j++) {
		for(i = 0; i < 5; i++) {
			render_cube(&c, &mat, &view, &pos, 30 + i * 40, 64 - 25 + 45 * j, outline);
		}
	}
	*/
	
	Cube c3;
	
	//make_cube(&c3, 20, 20, 20, 0, 0, -200, cube_angle);
	
	//for(i = 0; i < 8; i++) {
	//	c3.v[i].z -= 200;
	//}
	
	//render_cube(buf, &c, cam, outline);
	//render_cube(&c2, cam, outline);
	//render_cube(&c3, cam, outline);
	
	//render_cube_outline(&c, &mat, &view, &pos, 120 + 40, 64, outline);
}











































