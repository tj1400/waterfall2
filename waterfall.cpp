//
//modified by:Maximillian Wolfe
//date:1/26/2018
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// . general animation framework
// . animation loop
// . object definitions and movement
// . collision detection
// . mouse/keyboard interaction
// . object constructor
// . coding style
// . defined constants
// . use of static variables
// . dynamic memory allocation
// . simple opengl components
// . git
//
//elements we will add to program...
//   . Game constructor
//   . multiple particles
//   . gravity
//   . collision detection
//   . more objects
//
#include <iostream>
#include <unistd.h>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"
#include "log.h"
const int MAX_POINTS = 100000;
const int MAX_PARTICLES = 1000000;
const float GRAVITY = 0.1;
//some structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
    int c;
    int hit;
    Particle(){
		hit=0;
	}
};
struct Point{
	float x,y;
};

class Global {
    public:
	int xres, yres;
	Particle particle[MAX_PARTICLES];
	Point point[MAX_POINTS];
	Point texpoint[MAX_POINTS];
	int npoints;
	Shape box[5];
	Shape circle;
	int n;
	Global() {
	    xres = 800;
	    yres = 600;
	    npoints = 200;
	    //define a box shape
	    for(int i=0;i<5;i++)
	    {
		box[i].width = 100;
		box[i].height = 10;
		box[i].center.x = 100 + 90*i;
		box[i].center.y = yres-30-60*i;
		n = 0;
	    }
	    circle.radius=300;
	    circle.center.x=650;
	    circle.center.y=-100;
	    
	}
} g;

class X11_wrapper {
    private:
	Display *dpy;
	Window win;
	GLXContext glc;
	GC gc;
    public:
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
	}
	X11_wrapper() {
	    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	    int w = g.xres, h = g.yres;
	    dpy = XOpenDisplay(NULL);
	    if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	    }
	    Window root = DefaultRootWindow(dpy);
	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	    } 
	    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    XSetWindowAttributes swa;
	    swa.colormap = cmap;
	    swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	    set_title();
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	    XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
	    //See if there are pending events.
	    return XPending(dpy);
	}
	XEvent getXNextEvent() {
	    //Get a pending event.
	    XEvent e;
	    XNextEvent(dpy, &e);
	    return e;
	}
	void swapBuffers() {
	    glXSwapBuffers(dpy, win);
	}
} x11;


//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
    srand(time(NULL));
    init_opengl();
    //Main animation loop
    int done = 0;
    while (!done) {
	//Process external events.
	while (x11.getXPending()) {
	    XEvent e = x11.getXNextEvent();
	    check_mouse(&e);
	    done = check_keys(&e);
	}
	movement();
	render();
	x11.swapBuffers();
    }
    cleanup_fonts();
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();

}

void makeParticle(int x, int y)
{
    if (g.n >= MAX_PARTICLES)
	return;
    if(MAX_PARTICLES-g.n>10){
	for(int i=0;i<10;i++){
	    Particle *p = &g.particle[g.n];
	    float r = ((float)rand()/(float)RAND_MAX)-.7;
	    r=r*80;
	    g.particle[g.n].c=r;
	    p->s.center.x = x;
	    p->s.center.y = y;
	    p->velocity.y =1;
	    p->velocity.x =((float)rand()/(float)RAND_MAX)+.1;
	    ++g.n;
	}
    }
}

void check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    if (e->type != ButtonRelease &&
	    e->type != ButtonPress &&
	    e->type != MotionNotify) {
	//This is not a mouse event that we care about.
	return;
    }
    //
    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    //int y = g.yres - e->xbutton.y;
	    //makeParticle(0, g.yres);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    if (e->type == MotionNotify) {
	//The mouse moved!
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
	    savex = e->xbutton.x;
	    savey = e->xbutton.y;
	    //int y = g.yres;
	    //makeParticle(0, y);


	}
    }
}

int check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
	return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
	switch (key) {
	    case XK_1:
		//Key 1 was pressed
		break;
	    case XK_a:
		//Key A was pressed
		break;
	    case XK_Escape:
		//Escape key was pressed
		return 1;
	}
    }
    return 0;
}

void movement()
{
    int top;
    int bottom;
    int right;
    int left;
    if (g.n <= 0)
	return;
    for(int i=0;i<g.n;i++){
	Particle *p = &g.particle[i];

	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	if(p->velocity.y>-5){
	    p->velocity.y-=0.05;
	}
	//check for collision with shapes...
	//Shape *s;
	for(int j=0;j<5;j++)
	{
	    Shape *s = &g.box[j];
	    top = s->center.y+(s->height);
	    bottom = s->center.y-(s->height);
	    left = s->center.x-(s->width);
	    right = s->center.x+(s->width);
	    float rnd = ((float)rand()/(float)RAND_MAX-.5)/2;
	    if(p->s.center.x<right-.5&&p->s.center.x>left+.5&&p->s.center.y<top&&p->s.center.y>bottom){
		if(p->s.center.y-top<5&&p->s.center.y-top>-5){
		    p->velocity.y= (0-p->velocity.y)/20+rnd;
		    p->s.center.y += .6;
		}
		if(p->s.center.y-bottom<5&&p->s.center.y-bottom>-5){
		    p->velocity.y= (0-p->velocity.y)/20+rnd;
		    p->s.center.y -= .6;
		}
		if(p->s.center.x-right<5&&p->s.center.x-right>-5&&p->velocity.x<0){
		    p->velocity.x=(0-p->velocity.x)/20+rnd;
		    p->s.center.x += .5;
		}
		if(p->s.center.x-left<5&&p->s.center.x-left>-5&&p->velocity.x>0){
		    p->velocity.x=(0-p->velocity.x)/20+rnd;
		    p->s.center.x -= .5;
		}


	    }
	}
	    float distx = p->s.center.x-g.circle.center.x;
	    if(distx<0)
		distx=0-distx;
	    float disty = p->s.center.y-g.circle.center.y;
	    if(disty<0)
		disty=0-disty;
	    float dist = sqrt(distx*distx+disty*disty);
	    if(dist<g.circle.radius){
		float temp[2];
	       	temp[0] = p->s.center.x-g.circle.center.x;
		temp[1] = p->s.center.y-g.circle.center.y;
		float norm[2];
		norm[0]=temp[0]/g.circle.radius;
		norm[1]=temp[1]/g.circle.radius;
		float dot = -(p->velocity.x*norm[0]+p->velocity.y*norm[1]);
		float len = 2.0*dot;
		p->s.center.x+=norm[0]*5;
		p->s.center.y+=norm[1]*5;
		//pnorm[0]=norm[1];
		//pnorm[1]=norm[0];
		//else{
			p->velocity.x = ((len *norm[0]+p->velocity.x)*(norm[1]/1.25));
			p->velocity.y = ((len *norm[1]+p->velocity.y)*(-norm[0]/1.25));
		//}
	} 
	    //check for off-screen
	    if (p->s.center.y<0) {
		cout << "off screen" << endl;
		g.particle[i] = g.particle[--g.n];
	    }
	}
	
    }
const float PI = 3.14159265358979;
void makeCircle(){
	
	float angle = 0.0;
    float inc = PI * 2 / (float)g.npoints;
    for (int i=0;i<g.npoints;i++){
        g.point[i].x = cos(angle) * g.circle.radius;
        g.point[i].y = sin(angle) * g.circle.radius;
		g.texpoint[i].x = cos(angle)/2+.46;
		g.texpoint[i].y = -sin(angle)/2+.51;
        angle += inc;
    }
    glPushMatrix();
    glTranslatef(g.circle.center.x, g.circle.center.y, 0.0);
    glBegin(GL_LINE_LOOP);
    glColor3ub(90,160,90);
    for(int i=0;i<g.npoints;i++){
        glVertex2f(g.point[i].x, g.point[i].y);
    }
    glEnd();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(g.circle.center.x,g.circle.center.y, 0.0);
    glBegin(GL_TRIANGLE_FAN);
    for(int i=0;i<g.npoints-1;i++){
        glColor3ub(90,160,90);
        glVertex2f(g.point[0].x,g.point[0].y);
        glVertex2f(g.point[i].x,g.point[i].y);
        glVertex2f(g.point[i+1].x,g.point[i+1].y);
    }

    glEnd();
    glPopMatrix();

}

	
	
	
void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    //
    //Draw a box here.
    float w, h;
    Shape *s;
    glColor3ub(90, 160, 90);
    for(int i=0;i<5;i++)
    {
	s = &g.box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w, -h);
	glVertex2i(-w,  h);
	glVertex2i( w,  h);
	glVertex2i( w, -h);
	glEnd();
	glPopMatrix();
    }
    glColor3ub(30,60,90);
    makeCircle();
    makeParticle(30,g.yres-10);
    //
    //Draw the particle here.
    for(int i=0;i<g.n;i++){
	glPushMatrix();
	Vec *c = &g.particle[i].s.center;
	glColor3ub(g.particle[i].c+140,g.particle[i].c+150,200);
	w =
	    h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
	//
	//Draw your 2D text here

    }
    	for(int i=0;i<5;i++)
	{
	    s=&g.box[i];
	Rect r;
	r.bot=s->center.y-6;
	r.center=1;
	r.left = s->center.x;
	//r.center=0;
	if(i==0)
	ggprint8b(&r, 16, 0x00ff0000, "REQUIREMENTS");
	else if(i==1)
	    ggprint8b(&r, 16, 0x00ff0000, "DESIGN");
	
	    else if(i==2)
		ggprint8b(&r, 16, 0x00ff0000, "CODING");
	    else if(1==3)
		ggprint8b(&r, 16, 0x00ff0000, "TESTING");
	    else
		ggprint8b(&r, 16, 0x00ff0000, "MAINTENANCE");
	
	}
}






