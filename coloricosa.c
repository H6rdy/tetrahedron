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

// 정이십면체 12개 꼭짓점
float vertex[12][3] = {
    // 4개 그룹 (각 평면에 4개씩)
    { 0,  1,  PHI}, { 0,  1, -PHI}, { 0, -1,  PHI}, { 0, -1, -PHI},
    { 1,  PHI, 0}, {-1,  PHI, 0}, { 1, -PHI, 0}, {-1, -PHI, 0},
    { PHI, 0,  1}, {-PHI, 0,  1}, { PHI, 0, -1}, {-PHI, 0, -1}
};

// 정이십면체 20개 면 (정삼각형)
int faces[20][3] = {
    // 상단 5개 (v0 중심)
    {0, 2, 8},  {0, 8, 4},  {0, 4, 5},  {0, 5, 9},  {0, 9, 2},
    
    // 중간 상단 5개
    {2, 6, 8},  {8, 6, 10}, {8, 10, 4}, {4, 10, 1}, {4, 1, 5},
    
    // 중간 하단 5개
    {5, 1, 11}, {5, 11, 9}, {9, 11, 7}, {9, 7, 2},  {2, 7, 6},
    
    // 하단 5개 (v3 중심)
    {3, 6, 7},  {3, 7, 11}, {3, 11, 1}, {3, 1, 10}, {3, 10, 6}
};

// 각 면의 문자
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
  
  // 정이십면체: 그래프 색칠 (4색)
  // 상단 5개 (v0 중심) - 서로 이웃하므로 모두 다른 색
  // 중간/하단 - 이웃 관계 고려하여 배치
  int faceColors[20] = {
    31,  // 면0: 빨강
    32,  // 면1: 초록 (면0과 이웃)
    33,  // 면2: 노랑 (면0, 면1과 이웃)
    34,  // 면3: 파랑 (면1, 면2와 이웃)
    32,  // 면4: 초록 (면2, 면3과 이웃, 면1과 떨어짐)
    
    33,  // 면5: 노랑 (면0, 면4와 이웃)
    34,  // 면6: 파랑 (면1, 면5와 이웃)
    31,  // 면7: 빨강 (면1, 면6과 이웃)
    33,  // 면8: 노랑 (면1, 면7과 이웃)
    31,  // 면9: 빨강 (면2, 면8과 이웃)
    
    34,  // 면10: 파랑 (면2, 면9와 이웃)
    32,  // 면11: 초록 (면4, 면10과 이웃)
    31,  // 면12: 빨강 (면4, 면11과 이웃)
    34,  // 면13: 파랑 (면0, 면12와 이웃)
    33,  // 면14: 노랑 (면0, 면5, 면13과 이웃)
    
    31,  // 면15: 빨강 (면6, 면14와 이웃)
    33,  // 면16: 노랑 (면7, 면15와 이웃)
    34,  // 면17: 파랑 (면8, 면16과 이웃)
    32,  // 면18: 초록 (면7, 면17과 이웃)
    33   // 면19: 노랑 (면6, 면15, 면18과 이웃)
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