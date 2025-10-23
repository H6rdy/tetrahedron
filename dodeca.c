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

// 황금비 (Golden Ratio)
#define PHI 1.618033988749895

// 정십이면체 꼭짓점 20개
float vertices[20][3] = {
    // 정육면체 8개 꼭짓점 (±1, ±1, ±1)
    { 1,  1,  1}, { 1,  1, -1}, { 1, -1,  1}, { 1, -1, -1},
    {-1,  1,  1}, {-1,  1, -1}, {-1, -1,  1}, {-1, -1, -1},
    
    // XY 평면 4개 (0, ±1/φ, ±φ)
    { 0,  1.0/PHI,  PHI}, { 0,  1.0/PHI, -PHI},
    { 0, -1.0/PHI,  PHI}, { 0, -1.0/PHI, -PHI},
    
    // YZ 평면 4개 (±1/φ, ±φ, 0)
    { 1.0/PHI,  PHI, 0}, {-1.0/PHI,  PHI, 0},
    { 1.0/PHI, -PHI, 0}, {-1.0/PHI, -PHI, 0},
    
    // XZ 평면 4개 (±φ, 0, ±1/φ)
    { PHI, 0,  1.0/PHI}, { PHI, 0, -1.0/PHI},
    {-PHI, 0,  1.0/PHI}, {-PHI, 0, -1.0/PHI}
};

// 정십이면체 12개 면 (각 면은 정오각형, 5개 꼭짓점)
int faces[12][5] = {
    {0, 8, 10, 2, 16},    // 면0
    {0, 16, 17, 1, 12},   // 면1
    {0, 12, 13, 4, 8},    // 면2
    {1, 17, 3, 11, 9},    // 면3
    {1, 9, 5, 13, 12},    // 면4
    {2, 10, 6, 15, 14},   // 면5
    {2, 14, 3, 17, 16},   // 면6
    {3, 14, 15, 7, 11},   // 면7
    {4, 13, 5, 19, 18},   // 면8
    {4, 18, 6, 10, 8},    // 면9
    {5, 9, 11, 7, 19},    // 면10
    {6, 18, 19, 7, 15}    // 면11
};

// 각 면의 문자
char surChars[12] = {'@', 'r', '=', 'x', '*', '+', ':', '-', '$', '.', 'o', '~'};

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

// 삼각형 렌더링 (정사면체와 동일)
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

// 정오각형을 삼각형 3개로 분할하여 렌더링
void pentagon(int faceIdx, float scale) {
  float v1[3], v2[3], v3[3], v4[3], v5[3];
  
  // 5개 꼭짓점 가져오기
  for (int i = 0; i < 3; i++) {
    v1[i] = vertices[faces[faceIdx][0]][i];
    v2[i] = vertices[faces[faceIdx][1]][i];
    v3[i] = vertices[faces[faceIdx][2]][i];
    v4[i] = vertices[faces[faceIdx][3]][i];
    v5[i] = vertices[faces[faceIdx][4]][i];
  }
  
  char ch = surChars[faceIdx];
  
  // 정오각형을 3개 삼각형으로 분할
  // 팬 삼각분할 (Fan Triangulation): v1을 중심으로
  triangle(v1, v2, v3, ch, scale);
  triangle(v1, v3, v4, ch, scale);
  triangle(v1, v4, v5, ch, scale);
}

int main() {
  printf("\x1b[2J");
  printf("\x1b[?25l");
  
  float scale = 20.0;
  
  while (1) {
    memset(buffer, background, width * height);
    memset(zBuffer, 0, width * height * 4);
    
    // 12개 면 렌더링
    for (int i = 0; i < 12; i++) {
      pentagon(i, scale);
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