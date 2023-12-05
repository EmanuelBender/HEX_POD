#include <pgmspace.h>


const int Xoff = TFT_WIDTH / 2;   // positions the center of the 3d conversion space into the center of the OLED screen. This is usally screen_x_size / 2.
const int Yoff = TFT_HEIGHT / 2;  // screen_y_size /2
const int Zoff = 250;             //Size of cube, larger no. = smaller cube

const float fact = 180 / pi;  // conversion from degrees to radians.

int AcX, AcY, AcZ;
int colAcX, colAcY, colAcZ;

float Xan, Yan, Zan;
double xx, xy, xz;  // float
double yx, yy, yz;
double zx, zy, zz;

int LinestoRender;     // lines to render.
int OldLinestoRender;  // lines to render just in case it changes. this makes sure the old lines all get erased.
int xOut = 0, yOut = 0;

struct Point3d {
  int x, y, z;
};

struct Point2d {
  int x, y;
};

struct Line3d {
  Point3d p0, p1;
};

struct Line2d {
  Point2d p0, p1;
};

Line3d Lines[12];  //Number of lines to render
Line2d Render[12];
Line2d ORender[12];


void initializeCube() {
  // Line segments to draw a cube, from p0 to p1, p1 to p2, p2 to p3, and so on.
  // Front Face
  Line3d lines[] = {
    { { -50, -50, 50 }, { 50, -50, 50 } }, { { 50, -50, 50 }, { 50, 50, 50 } }, { { 50, 50, 50 }, { -50, 50, 50 } }, { { -50, 50, 50 }, { -50, -50, 50 } }, { { -50, -50, -50 }, { 50, -50, -50 } }, { { 50, -50, -50 }, { 50, 50, -50 } }, { { 50, 50, -50 }, { -50, 50, -50 } }, { { -50, 50, -50 }, { -50, -50, -50 } }, { { -50, -50, 50 }, { -50, -50, -50 } }, { { 50, -50, 50 }, { 50, -50, -50 } }, { { -50, 50, 50 }, { -50, 50, -50 } }, { { 50, 50, 50 }, { 50, 50, -50 } }
  };
  std::copy(std::begin(lines), std::end(lines), std::begin(Lines));

  LinestoRender = 12;
  OldLinestoRender = LinestoRender;
}


void SetVars() {
  float Xan2, Yan2 /*, Zan2*/;
  float s1, s2, c1, c2;

  Xan = map(AcX, -mapRange, mapRange, -40, 40) % 360;  // Xan = Xan % 360;
  Yan = map(AcY, -mapRange, mapRange, -40, 40) % 360;  // Prevents overflow.
  // Zan = map(AcY, -mapRange, mapRange, -40, 40) % 360;  // Prevents overflow.

  Xan2 = Xan / fact;  // Convert degrees to radians.
  Yan2 = Yan / fact;
  // Zan2 = Zan / fact;

  s1 = sin(Yan2);  // Zan is assumed to be zero.
  s2 = sin(Xan2);

  c1 = cos(Yan2);
  c2 = cos(Xan2);

  xx = c1;
  xy = 0;
  xz = -s1;

  yx = s1 * s2;
  yy = c2;
  yz = c1 * s2;

  zx = s1 * c2;
  zy = -s2;
  zz = c1 * c2;
}



/***********************************************************************************************************************************/
// processes x1,y1,z1 and returns rx1,ry1 transformed by the variables set in SetVars()
void ProcessLine(struct Line2d* ret, struct Line3d vec) {
  float zvt1, zvt2;  // float
  int xv1, yv1, zv1, xv2, yv2, zv2;
  int rx1, ry1, rx2, ry2;
  int x1, y1, z1, x2, y2, z2;
  int Ok;

  x1 = vec.p0.x;
  y1 = vec.p0.y;
  z1 = vec.p0.z;

  x2 = vec.p1.x;
  y2 = vec.p1.y;
  z2 = vec.p1.z;

  Ok = 0;  // defaults to not OK

  xv1 = (x1 * xx) + (y1 * xy) + (z1 * xz);
  yv1 = (x1 * yx) + (y1 * yy) + (z1 * yz);
  zv1 = (x1 * zx) + (y1 * zy) + (z1 * zz);

  zvt1 = zv1 - Zoff;

  if (zvt1 < -5) {
    rx1 = 256 * (xv1 / zvt1) + Xoff;
    ry1 = 256 * (yv1 / zvt1) + Yoff;
    Ok = 1;  // ok we are alright for point 1.
  }

  xv2 = (x2 * xx) + (y2 * xy) + (z2 * xz);
  yv2 = (x2 * yx) + (y2 * yy) + (z2 * yz);
  zv2 = (x2 * zx) + (y2 * zy) + (z2 * zz);

  zvt2 = zv2 - Zoff;

  if (zvt2 < -5) {
    rx2 = 256 * (xv2 / zvt2) + Xoff;
    ry2 = 256 * (yv2 / zvt2) + Yoff;
  } else {
    Ok = 0;
  }

  if (Ok == 1) {
    ret->p0.x = rx1;
    ret->p0.y = ry1;

    ret->p1.x = rx2;
    ret->p1.y = ry2;
  }
}




void RenderImage() {
  int cubecolors[] = { TFT_CYAN, TFT_DARKGREY, TFT_DARKCYAN };  // front, back, middle

  for (i = 0; i < LinestoRender; i++) {
    ORender[i] = Render[i];
    ProcessLine(&Render[i], Lines[i]);
  }

  for (i = 0; i < OldLinestoRender; i++) {
    tft.drawLine(ORender[i].p0.x, ORender[i].p0.y, ORender[i].p1.x, ORender[i].p1.y, TFT_BLACK);
  }

  for (i = LinestoRender - 1; i >= 0; i--) {
    int colorIndex = i < 8 ? (i < 4 ? 0 : 1) : 2;
    tft.drawLine(Render[i].p0.x, Render[i].p0.y, Render[i].p1.x, Render[i].p1.y, cubecolors[colorIndex]);
  }

  OldLinestoRender = LinestoRender;
}

void calcCube() {

  colAcX += X;
  colAcY += Y;
  colAcZ += Z;

  imuSampleCount++;
  if (imuSampleCount == imuAvg - 1) {
    AcX = colAcX / imuAvg;
    AcY = colAcY / imuAvg;
    AcZ = colAcZ / imuAvg;
    imuSampleCount = 0;
    colAcX = 0;
    colAcY = 0;
    colAcZ = 0;
    taskManager.schedule(onceMicros(1), SetVars);
    taskManager.schedule(onceMicros(5), RenderImage);
  }
}