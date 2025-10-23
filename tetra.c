#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

float A, B, C;

int width = 160, height = 44;
float zBuffer[160 * 44];
char buffer[160 * 44];
int background = ' ';
int distance = 60;
float K1 = 40;

float quality = 0.01;

float vertex[4][3] = {
    { 1.0,  1.0,  1.0},  
    { 1.0, -1.0, -1.0},  
    {-1.0,  1.0, -1.0},
    {-1.0, -1.0,  1.0}   
};

int faces[4][3] = {
    {0, 1, 2}, 
    {0, 1, 3}, 
    {0, 2, 3}, 
    {1, 2, 3}  
};

char surfaceChars[4] = {'@', '*', '.', '~'};

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

void surface(float tetraX, float tetraY, float tetraZ, int ch) {
  x = calculateX(tetraX, tetraY, tetraZ);
  y = calculateY(tetraX, tetraY, tetraZ);
  z = calculateZ(tetraX, tetraY, tetraZ) + distance;

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
  
  float scale = 15.0; 
  
  while (1) {
    memset(buffer, background, width * height);
    memset(zBuffer, 0, width * height * 4);
    
    for (int i = 0; i < 4; i++) {
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
      putchar(k % width ? buffer[k] : 10);
    }

    A += 0.02;
    B += 0.03;
    C += 0.01;
    usleep(16000 * 2); 
  }
  
  return 0;
}