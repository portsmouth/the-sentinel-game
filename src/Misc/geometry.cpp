
#include "geometry.h"


/**********************************************************************/
/*                          draw routines                             */
/**********************************************************************/

void drawtriangle(float *v[], float *n[], float *t[])
{	
	glNormal3fv(n[0]);		
	glTexCoord2f(t[0][0], t[0][1] );
	glVertex3fv(v[0]);
	
	glNormal3fv(n[1]);		
	glTexCoord2f(t[1][0], t[1][1]);
	glVertex3fv(v[1]);
	
	glNormal3fv(n[2]);		
	glTexCoord2f(t[2][0], t[2][1]);
	glVertex3fv(v[2]);	
}


void gltDrawSphere(GLfloat *pos, GLfloat fRadius, GLint iSlices, GLint iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;
	GLfloat ds = 1.0f / (GLfloat) iSlices;
	GLfloat dt = 1.0f / (GLfloat) iStacks;
	GLfloat t = 1.0f;	
	GLfloat s = 0.0f;
    GLint i, j;     // Looping variables
	
	glShadeModel (GL_SMOOTH);
	
	glPushMatrix();
	glTranslatef(pos[0],pos[1],pos[2]);
	
	for (i = 0; i < iStacks; i++) 
	{
		GLfloat rho = (GLfloat)i * drho;
		GLfloat srho = (GLfloat)(sin(rho));
		GLfloat crho = (GLfloat)(cos(rho));
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));
		
        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing 
        // artifacts at the poles on some OpenGL implementations
		glBegin(GL_TRIANGLE_STRIP);
        s = 0.0f;
		for ( j = 0; j <= iSlices; j++) 
		{
			GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
			GLfloat stheta = (GLfloat)(-sin(theta));
			GLfloat ctheta = (GLfloat)(cos(theta));
			
			GLfloat x = stheta * srho;
			GLfloat y = ctheta * srho;
			GLfloat z = crho;
            
            glTexCoord2f(s, t);
            glNormal3f(x, y, z);
            glVertex3f(x * fRadius, y * fRadius, z * fRadius);
			
            x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;
			glTexCoord2f(s, t - dt);
            s += ds;
            glNormal3f(x, y, z);
            glVertex3f(x * fRadius, y * fRadius, z * fRadius);
		}
        glEnd();
		
        t -= dt;
	}
	glPopMatrix();
}

/* draw an uncapped cone of length l, radius r, with apex at pos[], aligned along the z axis.
orient = +1 or -1, controls whether cone is pointing up or down.
*/
void drawCone (GLfloat *pos, float l, float theta, float *rotaxis, float r, int N_lod, int orient)
{
	int i;
	float tmp,ny,nz,a,ca,sa;
	const int n = 4*N_lod;	// number of sides to the cylinder (divisible by 4)
	
	l *= 0.5;
	a = (float)M_PI*2.0/(float)n;
	sa = (float) sin(a);
	ca = (float) cos(a);
	
	glPushMatrix();
	glTranslatef(pos[0],pos[1],pos[2]);
	glRotatef(theta,rotaxis[0],rotaxis[1],rotaxis[2]);
	
	glShadeModel (GL_SMOOTH);
	
	ny=1; nz=0;		  // normal vector = (0,ny,nz)
	glBegin (GL_TRIANGLE_STRIP);
	for (i=0; i<=n; i++) {
		glNormal3d (ny,nz,0);
		glVertex3d (0,0,orient*l);
		glNormal3d (ny,nz,0);
		glVertex3d (ny*r,nz*r,0);
		
		// rotate ny,nz
		tmp = ca*ny - sa*nz;
		nz = sa*ny + ca*nz;
		ny = tmp;
	}
	glEnd();
	
	glPopMatrix();
}

inline void drawQuad(const float *v1, const float *v2, 
					 const float *v3, const float *v4, 
					 const float *norm)
{
	glNormal3fv(norm);	
	
	glTexCoord2f(0.0f, 0.0f);
	glVertex3fv(v1); 
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3fv(v2);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3fv(v3); 
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3fv(v4);
}


/* draw box with center at (xbox, ybox, zbox) of side length 2*halfside */

static float x_pos[3] = { 1, 0, 0};
static float x_neg[3] = {-1, 0, 0};

static float y_pos[3] = { 0, 1, 0};
static float y_neg[3] = { 0,-1, 0};

static float z_pos[3] = { 0, 0, 1};
static float z_neg[3] = { 0, 0,-1};


void drawBox(float halfextents[3])
{
	glBegin(GL_QUADS);
	
	float v[8][3];
	unsigned int i,j;
	
	for (i = 0; i < 8; ++i) 
	for (j = 0; j < 3; ++j) 
			v[i][j] = halfextents[j] * ( -1.0f + 2.0f*(float) ( (i >> j) & 1 ) );
	
	/*
	 
	 y        
	 |        
	 2----3     6----7
	 | -z |     | +z |
	 0----1-x   4----5
	 
	 */
	
	drawQuad(v[0],v[2],v[3],v[1], z_neg);
	drawQuad(v[5],v[7],v[6],v[4], z_pos);
	drawQuad(v[0],v[1],v[5],v[4], y_neg);
	drawQuad(v[2],v[6],v[7],v[3], y_pos);
	drawQuad(v[0],v[4],v[6],v[2], x_neg);
	drawQuad(v[3],v[7],v[5],v[1], x_pos);
				
	glEnd();
}


inline void drawTriangle( const Vec4 (&v)[3], const Vec4& n, const Vec4 (&t)[3] )
{
	glNormal3f( n[0], n[1], n[2] );
	for (int i=0; i<3; ++i)
	{
		glTexCoord2f( t[i][0], t[i][1] );
		glVertex3f( v[i][0], v[i][1], v[i][2] );
	}
}

