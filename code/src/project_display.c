/*
 * Minimal OpenGL viewer behind the project_display.h API: GL 1.1 immediate
 * mode + GLFW
 *
 * There is one window. display_mesh() and display_edges() copy their arrays
 * into `scene` and handle the window's events until the user presses space
 * (return 1) or closes the window (return 0). display_wait() handles them
 * until the window is closed, then destroys it.
 *
 *   drag    pan
 *   scroll  zoom
 *   w / f   toggle wireframe / filled
 *   h       half-edges: off -> arrows -> arrows + numbers
 *   n       toggle node indices
 *   r       reset view
 *   space   next step (hold it to run through the steps)
 *   q, esc  quit
 */

#include "project_display.h"

#include <GLFW/glfw3.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Every length below is a fraction of something else (the view, an element),
 * never a number of pixels, so the drawing looks the same at any zoom. */
#define WINDOW_PX 800     // initial window size
#define TEXT_HEIGHT 0.036 // digit height / half-height of the view
#define TEXT_PADDING 0.25 // plate margin around the digits / digit height
#define ARROW_INSET 0.42  // how far a side is pulled toward the element centre
#define ARROW_TRIM 0.24   // how much of each end of that side is cut away
#define ARROW_HEAD 0.22   // barb length / arrow length
#define ARROW_BARB 0.09   // barb width  / arrow length
#define NUMBER_INSET 0.15 // how far a half-edge number sits from its arrow

static const float BLACK[3] = {0.00f, 0.00f, 0.00f};
static const float BLUE[3] = {0.05f, 0.25f, 0.75f};
static const float GRAY[3] = {0.70f, 0.70f, 0.70f};

/* ========================================================================== */
/*  Points                                                                    */
/* ========================================================================== */

typedef struct
{
  double x, y;
} Vec2;

// the point t of the way from a to b (t = 0 -> a, t = 1 -> b)
static Vec2 lerp(Vec2 a, Vec2 b, double t)
{
  return (Vec2){a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

static void vertex(Vec2 p)
{
  glVertex2d(p.x, p.y);
}

/* ========================================================================== */
/*  Scene : what we draw, and everything we precomputed about it              */
/* ========================================================================== */

typedef struct
{
  double xmin, xmax, ymin, ymax;
} Box;

typedef struct
{
  double *xy; // 2*nNodes coordinates
  uint32_t nNodes;
  uint32_t *elems; // nElems * nodesPerElem node indices
  uint32_t nElems;
  int nodesPerElem; // 3: triangles, 2: plain edges
  int sidesPerElem; // 3 sides per triangle, 1 per edge
  double *attr;     // one value per node, NULL -> flat gray
  double attrMin, attrMax;
  Box box; // bounding box of the nodes
} Scene;

// malloc() that stops the program when out of memory
static void *allocate(size_t bytes)
{
  void *p = malloc(bytes > 0 ? bytes : 1);
  if (p == NULL)
  {
    fprintf(stderr, "[display] out of memory\n");
    exit(EXIT_FAILURE);
  }
  return p;
}

// a copy of the first `bytes` bytes of src, in memory of its own
static void *copyOf(const void *src, size_t bytes)
{
  void *dst = allocate(bytes);
  if (bytes > 0)
    memcpy(dst, src, bytes);
  return dst;
}

static void sceneFree(Scene *s)
{
  free(s->xy);
  free(s->elems);
  free(s->attr);
  *s = (Scene){0};
}

/* Replaces s by a copy of the caller's arrays, so that the caller may change
 * them as soon as the display call returns. Field f at node v is
 * fields[nfields*v + f]; only the rendered one is kept. A missing or invalid
 * field leaves the mesh flat gray. */
static void sceneSet(Scene *s,
                     const double *xy,
                     uint32_t nNodes,
                     const uint32_t *elems,
                     uint32_t nElems,
                     int nodesPerElem,
                     const double *fields,
                     uint32_t nfields,
                     int field)
{
  sceneFree(s);
  s->nNodes = (xy != NULL) ? nNodes : 0;
  s->nElems = (elems != NULL) ? nElems : 0;
  s->nodesPerElem = nodesPerElem;
  s->sidesPerElem = (nodesPerElem == 3) ? 3 : 1;
  s->xy = copyOf(xy, 2 * (size_t)s->nNodes * sizeof(double));
  s->elems = copyOf(elems, (size_t)s->nElems * nodesPerElem * sizeof(uint32_t));

  /* bounding box, the view is framed on it */
  s->box = (Box){0.0, 1.0, 0.0, 1.0};
  if (s->nNodes > 0)
    s->box = (Box){xy[0], xy[0], xy[1], xy[1]};
  for (uint32_t v = 0; v < s->nNodes; v++)
  {
    s->box.xmin = fmin(s->box.xmin, xy[2 * v]);
    s->box.xmax = fmax(s->box.xmax, xy[2 * v]);
    s->box.ymin = fmin(s->box.ymin, xy[2 * v + 1]);
    s->box.ymax = fmax(s->box.ymax, xy[2 * v + 1]);
  }
  if (s->box.xmax - s->box.xmin < 1e-30 && s->box.ymax - s->box.ymin < 1e-30)
  { // all nodes on one spot
    s->box.xmax += 1;
    s->box.ymax += 1;
  }

  /* the rendered field, and its range: the colormap is stretched over it */
  s->attrMin = 0.0;
  s->attrMax = 1.0;
  if (fields == NULL || field < 0 || (uint32_t)field >= nfields ||
      s->nNodes == 0)
    return; // flat gray
  s->attr = allocate(s->nNodes * sizeof(double));
  for (uint32_t v = 0; v < s->nNodes; v++)
    s->attr[v] = fields[(size_t)nfields * v + field];
  s->attrMin = s->attrMax = s->attr[0];
  for (uint32_t v = 0; v < s->nNodes; v++)
  {
    s->attrMin = fmin(s->attrMin, s->attr[v]);
    s->attrMax = fmax(s->attrMax, s->attr[v]);
  }
}

static Vec2 nodePos(const Scene *s, uint32_t v)
{
  return (Vec2){s->xy[2 * v], s->xy[2 * v + 1]};
}

static uint32_t elemNode(const Scene *s, uint32_t e, int k)
{
  return s->elems[s->nodesPerElem * e + k];
}

// side k of element e, as the directed pair of nodes (from -> to)
static void
elemSide(const Scene *s, uint32_t e, int k, uint32_t *from, uint32_t *to)
{
  *from = elemNode(s, e, k);
  *to = elemNode(s, e, (k + 1) % s->nodesPerElem);
}

// centroid of a triangle, midpoint of an edge
static Vec2 elemCentre(const Scene *s, uint32_t e)
{
  Vec2 c = {0.0, 0.0};
  for (int k = 0; k < s->nodesPerElem; k++)
  {
    Vec2 p = nodePos(s, elemNode(s, e, k));
    c.x += p.x / s->nodesPerElem;
    c.y += p.y / s->nodesPerElem;
  }
  return c;
}

/* ========================================================================== */
/*  View state : what the keyboard and the mouse act on                       */
/* ========================================================================== */

static GLFWwindow *window; // NULL until the first display_*() call
static Scene scene;        // a copy of what the last display_*() call got
static int step;           // number of display_*() calls in this window

static struct
{
  Box frame;               // the box the view is centred on
  double zoom, panX, panY; // how far we zoomed and moved away from it
  double cursorX, cursorY; // last cursor position, while dragging
  int dragging;
  int filled, wireframe; // what we show
  int halfEdges;         // 0 off, 1 arrows, 2 arrows + numbers
  int nodeIndices;
  int nextStep; // space was pressed
} view = {.filled = 1, .wireframe = 1};

/* Frames the current scene. Only done for the first step and on 'r': later
 * steps keep the frame, so the picture does not jump when the mesh grows. */
static void viewReset(void)
{
  view.frame = scene.box;
  view.zoom = 1.0;
  view.panX = view.panY = 0.0;
  view.dragging = 0;
}

static void onKey(GLFWwindow *win, int key, int scancode, int action, int mods)
{
  (void)scancode;
  (void)mods;
  if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE)
    view.nextStep = 1; // GLFW_REPEAT too: holding space runs through steps
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
    glfwSetWindowShouldClose(win, 1);
  if (key == GLFW_KEY_W)
    view.wireframe = !view.wireframe;
  if (key == GLFW_KEY_F)
    view.filled = !view.filled;
  if (key == GLFW_KEY_H)
    view.halfEdges = (view.halfEdges + 1) % 3;
  if (key == GLFW_KEY_N)
    view.nodeIndices = !view.nodeIndices;
  if (key == GLFW_KEY_R)
    viewReset();
}

static void onScroll(GLFWwindow *win, double dx, double dy)
{
  (void)win;
  (void)dx;
  view.zoom *= exp(0.1 * dy);
}

static void onButton(GLFWwindow *win, int button, int action, int mods)
{
  (void)mods;
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    view.dragging = (action == GLFW_PRESS);
    glfwGetCursorPos(win, &view.cursorX, &view.cursorY);
  }
}

/* ========================================================================== */
/*  Drawing primitives                                                        */
/* ========================================================================== */

// jet-like colormap, t in [0,1]
static void colormap(double t, float *rgb)
{
  double r = 1.5 - fabs(4 * t - 3), g = 1.5 - fabs(4 * t - 2),
         b = 1.5 - fabs(4 * t - 1);
  rgb[0] = (float)(r < 0 ? 0 : r > 1 ? 1 : r);
  rgb[1] = (float)(g < 0 ? 0 : g > 1 ? 1 : g);
  rgb[2] = (float)(b < 0 ? 0 : b > 1 ? 1 : b);
}

// the colour of a node: its attribute through the colormap, or flat gray
static void setNodeColor(const Scene *s, uint32_t v)
{
  float rgb[3];
  if (s->attr == NULL)
  {
    glColor3fv(GRAY);
    return;
  }
  double range = s->attrMax - s->attrMin;
  double t = range > 0.0 ? (s->attr[v] - s->attrMin) / range : 0.0;
  colormap(t, rgb);
  glColor3fv(rgb);
}

/* 5x7 bitmap digits: row r of digit d is DIGIT[d][r], top row first, bit 4
 * the leftmost column. Each lit bit is drawn as a filled square cell, so the
 * digits are as tall as asked at any zoom. That is the whole font. */
#define GLYPH_COLS 5
#define GLYPH_ROWS 7
static const unsigned char DIGIT[10][GLYPH_ROWS] = {
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},  // 0
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},  // 1
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},  // 2
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},  // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},  // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},  // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},  // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},  // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},  // 8
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}}; // 9

// the four corners of a rectangle, for GL_QUADS or GL_LINE_LOOP
static void emitRect(Vec2 lo, Vec2 hi)
{
  vertex(lo);
  vertex((Vec2){hi.x, lo.y});
  vertex(hi);
  vertex((Vec2){lo.x, hi.y});
}

/* The screen pixel grid, in world coordinates: a pixel is `size` wide and
 * the grid lines are at `origin` + k * size. Set by beginFrame(), it lets the
 * font land on whole pixels, which is what keeps small digits sharp. */
static struct
{
  Vec2 origin;
  double size;
} pixel = {{0.0, 0.0}, 1.0};

// the pixel grid line nearest to x, for a grid with a line at origin
static double snap(double x, double origin)
{
  return origin + round((x - origin) / pixel.size) * pixel.size;
}

// integer `value` centred on `at`, digits about `height` tall, on a white plate
static void drawNumber(int value, Vec2 at, double height, const float *color)
{
  char text[16];
  int len = sprintf(text, "%d", value);
  // one bit of the font: a whole number of pixels, at least one
  double cell = fmax(1.0, round(height / GLYPH_ROWS / pixel.size)) * pixel.size;
  double advance = (GLYPH_COLS + 1) * cell; // a digit and a one-cell gap
  double width = len * advance - cell;
  height = GLYPH_ROWS * cell;
  double pad = snap(TEXT_PADDING * height, 0.0);
  Vec2 top = {snap(at.x - 0.5 * width, pixel.origin.x),
              snap(at.y + 0.5 * height, pixel.origin.y)}; // top-left of digit 0

  /* the plate keeps the number readable over the mesh, the outline keeps it
   * visible over the white background, the padding keeps the digits clear of
   * the outline */
  Vec2 lo = {top.x - pad, top.y - height - pad};
  Vec2 hi = {top.x + width + pad, top.y + pad};
  glColor3f(1, 1, 1);
  glBegin(GL_QUADS);
  emitRect(lo, hi);
  glEnd();
  glColor3fv(color);
  glBegin(GL_LINE_LOOP);
  emitRect(lo, hi);
  glEnd();

  glBegin(GL_QUADS);
  for (int i = 0; i < len; i++, top.x += advance)
  {
    const unsigned char *rows = DIGIT[text[i] - '0'];
    for (int r = 0; r < GLYPH_ROWS; r++)
      for (int c = 0; c < GLYPH_COLS; c++)
        if (rows[r] & (1 << (GLYPH_COLS - 1 - c)))
        {
          Vec2 cellLo = {top.x + c * cell, top.y - (r + 1) * cell};
          emitRect(cellLo, (Vec2){cellLo.x + cell, cellLo.y + cell});
        }
  }
  glEnd();
}

/* Shaft plus a single barb, on the left of tail -> tip, which for a
 * counter-clockwise triangle is the side the half-edge belongs to.
 * Call between glBegin(GL_LINES) and glEnd(). */
static void emitArrow(Vec2 tail, Vec2 tip)
{
  double dx = tip.x - tail.x, dy = tip.y - tail.y;

  Vec2 back = {-ARROW_HEAD * dx,
               -ARROW_HEAD * dy}; // from the tip, back along the shaft
  Vec2 side = {-ARROW_BARB * dy, ARROW_BARB * dx}; // and out to its left
  vertex(tail);
  vertex(tip);
  vertex(tip);
  vertex((Vec2){tip.x + back.x + side.x, tip.y + back.y + side.y});
}

/* ========================================================================== */
/*  The four drawing passes                                                   */
/* ========================================================================== */

// triangles, one colour per node, interpolated inside the triangle
static void drawFilled(const Scene *s)
{
  glBegin(GL_TRIANGLES);
  for (uint32_t e = 0; e < s->nElems; e++)
    for (int k = 0; k < 3; k++)
    {
      uint32_t v = elemNode(s, e, k);
      setNodeColor(s, v);
      vertex(nodePos(s, v));
    }
  glEnd();
}

// every side of every element, in black
static void drawWireframe(const Scene *s)
{
  glColor3fv(BLACK);
  glBegin(GL_LINES);
  for (uint32_t e = 0; e < s->nElems; e++)
    for (int k = 0; k < s->sidesPerElem; k++)
    {
      uint32_t from, to;
      elemSide(s, e, k, &from, &to);
      vertex(nodePos(s, from));
      vertex(nodePos(s, to));
    }
  glEnd();
}

/* One arrow per directed side. The side is first pulled toward the centre of
 * its element and then trimmed at both ends, which is what keeps the three
 * arrows of a triangle inside it and clear of each other.
 * Side k of element e is half-edge 3*e + k, the numbering of HeObject.h; for an
 * edge list there is one side per element, so the number is the edge index. */
static void drawHalfEdges(const Scene *s, double textHeight, int withNumbers)
{
  for (uint32_t e = 0; e < s->nElems; e++)
  {
    Vec2 centre = elemCentre(s, e);
    for (int k = 0; k < s->sidesPerElem; k++)
    {
      uint32_t from, to;
      elemSide(s, e, k, &from, &to);
      Vec2 a = lerp(nodePos(s, from), centre, ARROW_INSET);
      Vec2 b = lerp(nodePos(s, to), centre, ARROW_INSET);
      Vec2 tail = lerp(a, b, ARROW_TRIM);
      Vec2 tip = lerp(b, a, ARROW_TRIM);

      glColor3fv(BLUE);
      glBegin(GL_LINES);
      emitArrow(tail, tip);
      glEnd();

      if (withNumbers)
      {
        Vec2 mid = lerp(tail, tip, 0.5);
        drawNumber((int)(s->sidesPerElem * e + k),
                   lerp(mid, centre, NUMBER_INSET),
                   0.8 * textHeight,
                   BLUE);
      }
    }
  }
}

static void drawNodeIndices(const Scene *s, double textHeight)
{
  for (uint32_t v = 0; v < s->nNodes; v++)
    drawNumber((int)v, nodePos(s, v), textHeight, BLACK);
}

/* ========================================================================== */
/*  The window                                                                */
/* ========================================================================== */

// fit the frame at the framebuffer aspect ratio (0.55: a 10% margin)
static double viewHalfHeight(double aspect)
{
  double w = view.frame.xmax - view.frame.xmin;
  double h = view.frame.ymax - view.frame.ymin;
  return 0.55 * fmax(h, w / aspect) / view.zoom;
}

// left mouse button held: move the view along with the cursor
static void applyDrag(double halfHeight)
{
  int w, h;
  double x, y;
  if (!view.dragging)
    return;

  glfwGetCursorPos(window, &x, &y);
  glfwGetWindowSize(window, &w, &h);
  if (h > 0)
  {
    view.panX -= (x - view.cursorX) * 2 * halfHeight / h;
    view.panY += (y - view.cursorY) * 2 * halfHeight / h;
  }
  view.cursorX = x;
  view.cursorY = y;
}

// white background and an orthographic projection centred on the frame
static void beginFrame(double halfHeight, int fbW, int fbH)
{
  int winW;
  glfwGetWindowSize(window, &winW, NULL);

  double cx = 0.5 * (view.frame.xmin + view.frame.xmax) + view.panX;
  double cy = 0.5 * (view.frame.ymin + view.frame.ymax) + view.panY;
  double halfWidth = halfHeight * fbW / fbH;
  pixel.origin = (Vec2){cx - halfWidth, cy - halfHeight};
  pixel.size = 2 * halfHeight / fbH;

  /* on a HiDPI screen the framebuffer is bigger than the window: widen the
   * strokes in the same proportion, or they end up one physical pixel thin */
  glLineWidth((float)(1.5 * (winW > 0 ? (double)fbW / winW : 1.0)));

  glViewport(0, 0, fbW, fbH);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(
      cx - halfWidth, cx + halfWidth, cy - halfHeight, cy + halfHeight, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void drawScene(void)
{
  int fbW, fbH;
  glfwGetFramebufferSize(window, &fbW, &fbH);
  if (fbW <= 0 || fbH <= 0)
    return; // no drawable framebuffer, e.g. while minimized

  double halfHeight = viewHalfHeight((double)fbW / fbH);
  double textHeight = TEXT_HEIGHT * halfHeight;

  applyDrag(halfHeight);
  beginFrame(halfHeight, fbW, fbH);

  if (scene.nodesPerElem == 2)
  {
    drawWireframe(&scene); // an edge list has nothing to fill
  }
  else
  {
    if (view.filled)
      drawFilled(&scene);
    if (view.wireframe)
      drawWireframe(&scene);
  }
  if (view.halfEdges)
    drawHalfEdges(&scene, textHeight, view.halfEdges == 2);
  if (view.nodeIndices)
    drawNodeIndices(&scene, textHeight);

  glfwSwapBuffers(window);
}

/* Draws the scene again after every event (key, mouse, resize...), sleeping
 * in between. Returns 1 when space is pressed, if stopOnSpace, or 0 when the
 * window is closed. */
static int handleEvents(int stopOnSpace)
{
  view.nextStep = 0;
  while (!glfwWindowShouldClose(window))
  {
    drawScene();
    glfwWaitEvents(); // sleeps until the next event, then calls onKey & co
    if (stopOnSpace && view.nextStep)
      return 1;
  }
  glfwHideWindow(window); // the caller may run on before display_wait()
  return 0;
}

static int openWindow(void)
{
  if (!glfwInit())
  {
    fprintf(stderr, "[display] glfwInit() failed\n");
    return 0;
  }
  window = glfwCreateWindow(WINDOW_PX, WINDOW_PX, "2D viewer", NULL, NULL);
  if (window == NULL)
  {
    fprintf(stderr, "[display] could not create the window\n");
    glfwTerminate();
    return 0;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetKeyCallback(window, onKey);
  glfwSetScrollCallback(window, onScroll);
  glfwSetMouseButtonCallback(window, onButton);
  return 1;
}

// what display_mesh() and display_edges() both do
static int show(const double *xy,
                uint32_t nNodes,
                const uint32_t *elems,
                uint32_t nElems,
                int nodesPerElem,
                const double *fields,
                uint32_t nfields,
                int field)
{
  if (window == NULL && !openWindow())
    return 0;
  if (glfwWindowShouldClose(window))
    return 0; // closed at an earlier step: stop showing

  sceneSet(
      &scene, xy, nNodes, elems, nElems, nodesPerElem, fields, nfields, field);
  step++;
  if (step == 1)
    viewReset();

  char title[64];
  snprintf(title, sizeof(title), "2D viewer - step %d", step);
  glfwSetWindowTitle(window, title);
  return handleEvents(1);
}

/* ========================================================================== */
/*  Public API                                                                */
/* ========================================================================== */

int display_mesh(double const *nodes,
                 uint32_t nnodes,
                 uint32_t const *elems,
                 uint32_t nelems,
                 double const *fields,
                 uint32_t nfields,
                 int render_field)
{
  return show(nodes, nnodes, elems, nelems, 3, fields, nfields, render_field);
}

int display_edges(double const *nodes,
                  uint32_t nnodes,
                  uint32_t const *edges,
                  uint32_t nedges)
{
  return show(nodes, nnodes, edges, nedges, 2, NULL, 0, 0);
}

void display_wait(void)
{
  if (window == NULL)
    return;
  handleEvents(0);
  glfwDestroyWindow(window);
  glfwTerminate();
  window = NULL;
  step = 0;
  sceneFree(&scene);
}
