/* Drawing nearly perfect 2D line segments in OpenGL
 * You can use this code however you want.
 * I just hope you to cite my name and the page of this technique:
 * http://artgrammer.blogspot.com/2011/05/drawing-nearly-perfect-2d-line-segments.html
 * http://www.codeproject.com/KB/openGL/gllinedraw.aspx
 * 
 * Enjoy. Chris Tsang.*/
#ifndef VASE_REN_DRAFT_H
#define VASE_REN_DRAFT_H
#include <math.h>
static inline double GET_ABS(double x) {return x>0?x:-x;}
/*
 * this implementation uses vertex array (opengl 1.1)
 *   choose only 1 from vase_rend_draft_1.h and vase_rend_draft_2.h
 *   to your need. if you have no preference, just use vase_rend_draft_2.h
 */
/* this is the master line() function which features:
 * -premium quality anti-aliased line drawing
 * -smaller CPU overhead than other CPU rasterizing algorithms
 * -line thickness control
 * -line color control
 * -can choose to use alpha blend or not
 * 
 * sample usage using alpha blending:
 * 
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho( 0,context_width,context_height,0,0.0f,100.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
		line ( 10,100,100,300,		//coordinates
			1.2,			//thickness in px
			0.5, 0.0, 1.0, 1.0,	//line color RGBA
			0,0,			//not used
			true);			//enable alphablend

		//more line() or glDrawArrays() calls
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//other drawing code...
  glPopMatrix();
  glDisable(GL_BLEND); //restore blending options
 *
 * and not using alpha blending (blend to background color):
 *
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho( 0,context_width,context_height,0,0.0f,100.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
		line ( 20,100,110,300,		//coordinates
			1.2,			//thickness in px
			0.5, 0.0, 1.0,		//line color *RGB*
			1.0, 1.0, 1.0,		//background color
			false);			//not using alphablend

		//more line() or glDrawArrays() calls
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//other drawing code...
  glPopMatrix();
*/
void line( double x1, double y1, double x2, double y2, //coordinates of the line
	float w,			//width/thickness of the line in pixel
	float Cr, float Cg, float Cb,	//RGB color components
	float Br, float Bg, float Bb,	//color of background when alphablend=false,
					//  Br=alpha of color when alphablend=true
	bool alphablend)		//use alpha blend or not
{
	double t; double R; double f=w-static_cast<int>(w);
	float A;
	
	if ( alphablend)
		A=Br;
	else
		A=1.0f;
	
	//determine parameters t,R
	/*   */
	if ( w>=0.0 && w<1.0) {
		t=0.05; R=0.48+0.32*f;
		if ( !alphablend) {
			Cr+=0.88*(1-f);
			Cg+=0.88*(1-f);
			Cb+=0.88*(1-f);
			if ( Cr>1.0) Cr=1.0;
			if ( Cg>1.0) Cg=1.0;
			if ( Cb>1.0) Cb=1.0;
		} else {
			A*=f;
		}
	} else if ( w>=1.0 && w<2.0) {
		t=0.05+f*0.33; R=0.768+0.312*f;
	} else if ( w>=2.0 && w<3.0){
		t=0.38+f*0.58; R=1.08;
	} else if ( w>=3.0 && w<4.0){
		t=0.96+f*0.48; R=1.08;
	} else if ( w>=4.0 && w<5.0){
		t=1.44+f*0.46; R=1.08;
	} else if ( w>=5.0 && w<6.0){
		t=1.9+f*0.6; R=1.08;
	} else if ( w>=6.0){
		double ff=w-6.0;
		t=2.5+ff*0.50; R=1.08;
	}
	//printf( "w=%f, f=%f, C=%.4f\n", w,f,C);
	
	//determine angle of the line to horizontal
	double tx=0,ty=0; //core thinkness of a line
	double Rx=0,Ry=0; //fading edge of a line
	double cx=0,cy=0; //cap of a line
	double ALW=0.01;
	double dx=x2-x1;
	double dy=y2-y1;
	if ( GET_ABS(dx) < ALW) {
		//vertical
		tx=t; ty=0;
		Rx=R; Ry=0;
		if ( w>0.0 && w<1.0)
			tx*=8;
		else if ( w==1.0)
			tx*=10;
	} else if ( GET_ABS(dy) < ALW) {
		//horizontal
		tx=0; ty=t;
		Rx=0; Ry=R;
		if ( w>0.0 && w<1.0)
			ty*=8;
		else if ( w==1.0)
			ty*=10;
	} else {
		if ( w < 3) { //approximate to make things even faster
			double m=dy/dx;
			//and calculate tx,ty,Rx,Ry
			if ( m>-0.4142 && m<=0.4142) {
				// -22.5< angle <= 22.5, approximate to 0 (degree)
				tx=t*0.1; ty=t;
				Rx=R*0.6; Ry=R;
			} else if ( m>0.4142 && m<=2.4142) {
				// 22.5< angle <= 67.5, approximate to 45 (degree)
				tx=t*-0.7071; ty=t*0.7071;
				Rx=R*-0.7071; Ry=R*0.7071;
			} else if ( m>2.4142 || m<=-2.4142) {
				// 67.5 < angle <=112.5, approximate to 90 (degree)
				tx=t; ty=t*0.1;
				Rx=R; Ry=R*0.6;
			} else if ( m>-2.4142 && m<-0.4142) {
				// 112.5 < angle < 157.5, approximate to 135 (degree)
				tx=t*0.7071; ty=t*0.7071;
				Rx=R*0.7071; Ry=R*0.7071;
			} else {
				// error in determining angle
				//printf( "error in determining angle: m=%.4f\n",m);
			}
		} else { //calculate to exact
			dx=y1-y2;
			dy=x2-x1;
			double L=sqrt(dx*dx+dy*dy);
			dx/=L;
			dy/=L;
			cx=-0.6*dy; cy=0.6*dx;
			tx=t*dx; ty=t*dy;
			Rx=R*dx; Ry=R*dy;
		}
	}

	//draw the line by triangle strip
	float line_vertex[]=
	{
		x1-tx-Rx, y1-ty-Ry,	//fading edge1
		x2-tx-Rx, y2-ty-Ry,
		x1-tx,y1-ty,		//core
		x2-tx,y2-ty,
		x1+tx,y1+ty,
		x2+tx,y2+ty,
		x1+tx+Rx, y1+ty+Ry,	//fading edge2
		x2+tx+Rx, y2+ty+Ry
	};
	glVertexPointer(2, GL_FLOAT, 0, line_vertex);
	
	if ( !alphablend) {
		float line_color[]=
		{
			Br,Bg,Bb,			
			Br,Bg,Bb,
			Cr,Cg,Cb,
			Cr,Cg,Cb,
			Cr,Cg,Cb,
			Cr,Cg,Cb,
			Br,Bg,Bb,			
			Br,Bg,Bb
		};
		glColorPointer(3, GL_FLOAT, 0, line_color);
	} else {
		float line_color[]=
		{
			Cr,Cg,Cb,0,
			Cr,Cg,Cb,0,
			Cr,Cg,Cb,A,
			Cr,Cg,Cb,A,
			Cr,Cg,Cb,A,
			Cr,Cg,Cb,A,
			Cr,Cg,Cb,0,
			Cr,Cg,Cb,0
		};
		glColorPointer(4, GL_FLOAT, 0, line_color);
	}
	
	if ( (GET_ABS(dx) < ALW || GET_ABS(dy) < ALW) && w<=1.0) {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	} else {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
	}
	
	//cap
	if ( w < 3) {
		//do not draw cap
	} else {
		//draw cap
		float line_vertex[]=
		{
			x1-Rx+cx, y1-Ry+cy,	//cap1
			x1+Rx+cx, y1+Ry+cy,
			x1-tx-Rx, y1-ty-Ry,
			x1+tx+Rx, y1+ty+Ry,
			x2-Rx-cx, y2-Ry-cy,	//cap2
			x2+Rx-cx, y2+Ry-cy,
			x2-tx-Rx, y2-ty-Ry,
			x2+tx+Rx, y2+ty+Ry
		};
		glVertexPointer(2, GL_FLOAT, 0, line_vertex);
				
		if ( !alphablend) {
			float line_color[]=
			{
				Br,Bg,Bb,	//cap1
				Br,Bg,Bb,
				Cr,Cg,Cb,
				Cr,Cg,Cb,
				Br,Bg,Bb,	//cap2
				Br,Bg,Bb,
				Cr,Cg,Cb,
				Cr,Cg,Cb
			};
			glColorPointer(3, GL_FLOAT, 0, line_color);
		} else {
			float line_color[]=
			{
				Cr,Cg,Cb,0,	//cap1
				Cr,Cg,Cb,0,
				Cr,Cg,Cb,A,
				Cr,Cg,Cb,A,
				Cr,Cg,Cb,0,	//cap2
				Cr,Cg,Cb,0,
				Cr,Cg,Cb,A,
				Cr,Cg,Cb,A
			};
			glColorPointer(4, GL_FLOAT, 0, line_color);
		}
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	}
}


/*a skimmed version of line(); no color, no thickness control
 * draws near-perfectly a black "hair line" of thickness 1px
 * when alphablend is false, it assumes drawing on a white surface
 * when alphablend is true, it draws with alpha */
void hair_line( double x1, double y1, double x2, double y2, bool alphablend=0)
{
	double t=0.05; double R=0.768;
	// double C=0.0;

	//determine angle of the line to horizontal
	double tx=0,ty=0, Rx=0,Ry=0;
	double ALW=0.01;
	double dx=x2-x1;
	double dy=y2-y1;
	if ( GET_ABS(dx) < ALW) {
		tx=t*10; ty=0;
		Rx=R; Ry=0;
	} else if ( GET_ABS(dy) < ALW) {
		tx=0; ty=t*10;
		Rx=0; Ry=R;
	} else {
		double m=dy/dx;
		if ( m>-0.4142 && m<=0.4142) {
			// -22.5< angle <= 22.5, approximate to 0 (degree)
			tx=t*0.1; ty=t;
			Rx=R*0.6; Ry=R;
		} else if ( m>0.4142 && m<=2.4142) {
			// 22.5< angle <= 67.5, approximate to 45 (degree)
			tx=t*-0.7071; ty=t*0.7071;
			Rx=R*-0.7071; Ry=R*0.7071;
		} else if ( m>2.4142 || m<=-2.4142) {
			// 67.5 < angle <=112.5, approximate to 90 (degree)
			tx=t; ty=t*0.1;
			Rx=R; Ry=R*0.6;
		} else if ( m>-2.4142 && m<-0.4142) {
			// 112.5 < angle < 157.5, approximate to 135 (degree)
			tx=t*0.7071; ty=t*0.7071;
			Rx=R*0.7071; Ry=R*0.7071;
		}
	}
	
	//draw the line by triangle strip
	float line_vertex[]=
	{
		x1-tx-Rx, y1-ty-Ry,	//fading edge1
		x2-tx-Rx, y2-ty-Ry,
		x1-tx,y1-ty,		//core
		x2-tx,y2-ty,
		x1+tx,y1+ty,
		x2+tx,y2+ty,
		x1+tx+Rx, y1+ty+Ry,	//fading edge2
		x2+tx+Rx, y2+ty+Ry
	};
	glVertexPointer(2, GL_FLOAT, 0, line_vertex);
	
	if ( !alphablend) {
		float line_color[]=
		{	1,1,1,
			1,1,1,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			1,1,1,
			1,1,1
		};
		glColorPointer(3, GL_FLOAT, 0, line_color);
	} else {
		float line_color[]=
		{	0,0,0,0,
			0,0,0,0,
			0,0,0,1,
			0,0,0,1,
			0,0,0,1,
			0,0,0,1,
			0,0,0,0,
			0,0,0,0
		};
		glColorPointer(4, GL_FLOAT, 0, line_color);
	}
	
	if ( (GET_ABS(dx) < ALW || GET_ABS(dy) < ALW)) {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	} else {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
	}
}

/*as a fall back to line()*/
void line_raw( double x1, double y1, double x2, double y2,
	double w,
	double Cr, double Cg, double Cb,
	double,double,double, bool)
{
	glLineWidth(w);
	float line_vertex[]=
	{
		x1,y1,
		x2,y2
	};
	float line_color[]=
	{
		Cr,Cg,Cb,
		Cr,Cg,Cb
	};
	glVertexPointer(2, GL_FLOAT, 0, line_vertex);
	glColorPointer(3, GL_FLOAT, 0, line_color);
	glDrawArrays(GL_LINES, 0, 2);
}

#endif
