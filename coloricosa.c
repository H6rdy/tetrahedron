#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

float A, B, C;

int width = 160, height = 44;
float zBuffer[160 * 44];
char buffer[160 * 44];
int background = ' ';
int distance = 80;
float K1 = 40;

float quality = 0.01;

#define PHI 1.618033988749895

float vertex[12][3] = {
    { 0,  1,  PHI}, { 0,  1, -PHI}, { 0, -1,  PHI}, { 0, -1, -PHI},
    { 1,  PHI, 0}, {-1,  PHI, 0}, { 1, -PHI, 0}, {-1, -PHI, 0},
    { PHI, 0,  1}, {-PHI, 0,  1}, { PHI, 0, -1}, {-PHI, 0, -1}
};

int faces[20][3] = {
    {0, 2, 8},  {0, 8, 4},  {0, 4, 5},  {0, 5, 9},  {0, 9, 2},
    
    {2, 6, 8},  {8, 6, 10}, {8, 10, 4}, {4, 10, 1}, {4, 1, 5},
    
    {5, 1, 11}, {5, 11, 9}, {9, 11, 7}, {9, 7, 2},  {2, 7, 6},
    
    {3, 6, 7},  {3, 7, 11}, {3, 11, 1}, {3, 1, 10}, {3, 10, 6}
};

char surfaceChars[20] = {
    '@', '#', '$', '~', '*', 
    '+', '=', '-', ':', '.', 
    'o', 'x', '%', '&', '!', 
    '?', '^', '/', '\\', '|'
};

float x, y, z;
float ooz;
int xp, yp;
int idx;

float calculateX(float i, float j, float k) {
  return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
         j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

float calculateY(float i, float j, float k) {
  return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
         j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
         i * cos(B) * sin(C);
}

float calculateZ(float i, float j, float k) {
  return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

void surface(float px, float py, float pz, int ch) {
  x = calculateX(px, py, pz);
  y = calculateY(px, py, pz);
  z = calculateZ(px, py, pz) + distance;

  ooz = 1 / z;

  xp = (int)(width / 2 + K1 * ooz * x * 2);
  yp = (int)(height / 2 + K1 * ooz * y);

  idx = xp + yp * width;
  if (idx >= 0 && idx < width * height) {
    if (ooz > zBuffer[idx]) {
      zBuffer[idx] = ooz;
      buffer[idx] = ch;
    }
  }
}

void triangle(float v1[3], float v2[3], float v3[3], char ch, float scale) {
  for (float u = 0; u < 1.0; u += quality) {
    for (float v = 0; v < 1.0 - u; v += quality) {
      float px = v1[0] + u * (v2[0] - v1[0]) + v * (v3[0] - v1[0]);
      float py = v1[1] + u * (v2[1] - v1[1]) + v * (v3[1] - v1[1]);
      float pz = v1[2] + u * (v2[2] - v1[2]) + v * (v3[2] - v1[2]);
      
      px *= scale;
      py *= scale;
      pz *= scale;
      
      surface(px, py, pz, ch);
    }
  }
}

int main() {
  printf("\x1b[2J");
  printf("\x1b[?25l");
  
  float scale = 20.0;
  
  int faceColors[20] = {
    31,  32,  33,  34,  32, //set 1
    33,  34,  31,  33,  31, //set 2 
    34,  32,  31,  34,  33, //set 3
    31,  33,  34,  32,  33  //set 4
  };
  
  while (1) {
    memset(buffer, background, width * height);
    memset(zBuffer, 0, width * height * 4);
    
    for (int i = 0; i < 20; i++) {
      triangle(
        vertex[faces[i][0]],
        vertex[faces[i][1]],
        vertex[faces[i][2]],
        surfaceChars[i],
        scale
      );
    }
    
    printf("\x1b[H");
    for (int k = 0; k < width * height; k++) {
      if (k % width == 0) {
        putchar(10);
      } else {
        int colorCode = 37;
        for (int i = 0; i < 20; i++) {
          if (buffer[k] == surfaceChars[i]) {
            colorCode = faceColors[i];
            break;
          }
        }
        printf("\x1b[%dm%c\x1b[0m", colorCode, buffer[k]);
      }
    }

    A += 0.02;
    B += 0.03;
    C += 0.01;
    usleep(16000 * 2);
  }
  
  return 0;
}