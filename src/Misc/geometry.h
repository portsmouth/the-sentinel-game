

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Maths.h"

#define TINY_VAL 1.0e-6

/* geometry pre-computation */
extern float *sa, *ca, *sa_HS, *ca_HS;
extern void gen_cylinder_lookup_tables(int min_lod, int max_lod);
extern void free_cylinder_lookup_tables(int min_lod, int max_lod);
extern void gen_cylinder_lookup_tables_HS(int min_lod, int max_lod);
extern void free_cylinder_lookup_tables_HS(int min_lod, int max_lod);

/**********************************************************************/
/*                          draw routines                             */
/**********************************************************************/

extern void drawtriangle(float *v[], float *n[], float *t[]);
extern void gltDrawSphere(GLfloat *pos, GLfloat fRadius, GLint iSlices, GLint iStacks);
extern void drawCylinder (GLfloat *pos, float l, float r, float zoffset, int lod, int min_lod, float wraptex_x, float wraptex_z);
extern void drawCone (GLfloat *pos, float l, float theta, float *rotaxis, float r, int N_lod, int orient);
extern void drawBox(float halfextents[3]);


#endif //GEOMETRY_H
