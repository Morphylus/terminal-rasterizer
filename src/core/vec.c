#include "vec.h"
#include <math.h>

bool mat4f_inv(Mat4f *out, const Mat4f *in) {
  Mat4f invOut;
  float *m = (float *)in;

  float inv[16], det;
  int i;

  inv[0] = m[5] * m[10] * m[15] - m[5] * m[14] * m[11] - m[6] * m[9] * m[15] +
           m[6] * m[13] * m[11] + m[7] * m[9] * m[14] - m[7] * m[13] * m[10];
  inv[1] = -m[1] * m[10] * m[15] + m[1] * m[14] * m[11] + m[2] * m[9] * m[15] -
           m[2] * m[13] * m[11] - m[3] * m[9] * m[14] + m[3] * m[13] * m[10];
  inv[2] = m[1] * m[6] * m[15] - m[1] * m[14] * m[7] - m[2] * m[5] * m[15] +
           m[2] * m[13] * m[7] + m[3] * m[5] * m[14] - m[3] * m[13] * m[6];
  inv[3] = -m[1] * m[6] * m[11] + m[1] * m[10] * m[7] + m[2] * m[5] * m[11] -
           m[2] * m[9] * m[7] - m[3] * m[5] * m[10] + m[3] * m[9] * m[6];
  inv[4] = -m[4] * m[10] * m[15] + m[4] * m[14] * m[11] + m[6] * m[8] * m[15] -
           m[6] * m[12] * m[11] - m[7] * m[8] * m[14] + m[7] * m[12] * m[10];
  inv[5] = m[0] * m[10] * m[15] - m[0] * m[14] * m[11] - m[2] * m[8] * m[15] +
           m[2] * m[12] * m[11] + m[3] * m[8] * m[14] - m[3] * m[12] * m[10];
  inv[6] = -m[0] * m[6] * m[15] + m[0] * m[14] * m[7] + m[2] * m[4] * m[15] -
           m[2] * m[12] * m[7] - m[3] * m[4] * m[14] + m[3] * m[12] * m[6];
  inv[7] = m[0] * m[6] * m[11] - m[0] * m[10] * m[7] - m[2] * m[4] * m[11] +
           m[2] * m[8] * m[7] + m[3] * m[4] * m[10] - m[3] * m[8] * m[6];
  inv[8] = m[4] * m[9] * m[15] - m[4] * m[13] * m[11] - m[5] * m[8] * m[15] +
           m[5] * m[12] * m[11] + m[7] * m[8] * m[13] - m[7] * m[12] * m[9];
  inv[9] = -m[0] * m[9] * m[15] + m[0] * m[13] * m[11] + m[1] * m[8] * m[15] -
           m[1] * m[12] * m[11] - m[3] * m[8] * m[13] + m[3] * m[12] * m[9];
  inv[10] = m[0] * m[5] * m[15] - m[0] * m[13] * m[7] - m[1] * m[4] * m[15] +
            m[1] * m[12] * m[7] + m[3] * m[4] * m[13] - m[3] * m[12] * m[5];
  inv[11] = -m[0] * m[5] * m[11] + m[0] * m[9] * m[7] + m[1] * m[4] * m[11] -
            m[1] * m[8] * m[7] - m[3] * m[4] * m[9] + m[3] * m[8] * m[5];
  inv[12] = -m[4] * m[9] * m[14] + m[4] * m[13] * m[10] + m[5] * m[8] * m[14] -
            m[5] * m[12] * m[10] - m[6] * m[8] * m[13] + m[6] * m[12] * m[9];
  inv[13] = m[0] * m[9] * m[14] - m[0] * m[13] * m[10] - m[1] * m[8] * m[14] +
            m[1] * m[12] * m[10] + m[2] * m[8] * m[13] - m[2] * m[12] * m[9];
  inv[14] = -m[0] * m[5] * m[14] + m[0] * m[13] * m[6] + m[1] * m[4] * m[14] -
            m[1] * m[12] * m[6] - m[2] * m[4] * m[13] + m[2] * m[12] * m[5];
  inv[15] = m[0] * m[5] * m[10] - m[0] * m[9] * m[6] - m[1] * m[4] * m[10] +
            m[1] * m[8] * m[6] + m[2] * m[4] * m[9] - m[2] * m[8] * m[5];

  det = m[0] * inv[0] + m[4] * inv[1] + m[8] * inv[2] + m[12] * inv[3];

  if (det == 0)
    return false;

  det = 1.f / det;

  for (i = 0; i < 16; i++)
    ((float *)out)[i] = inv[i] * det;

  return true;
}

void mul_mat_vec4f(Vec4f *out, const Mat4f *mat, const Vec4f *vec) {
  Vec4f *m = (Vec4f *)mat->m;
  float *v = (float *)vec;

  ((float *)out)[0] =
      v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + v[3] * m[0][3];
  ((float *)out)[1] =
      v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + v[3] * m[1][3];
  ((float *)out)[2] =
      v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + v[3] * m[2][3];
  ((float *)out)[3] =
      v[0] * m[3][0] + v[1] * m[3][1] + v[2] * m[3][2] + v[3] * m[3][3];
}

Vec3f normalize(const Vec3f in) {
  float len = in.x * in.x + in.y * in.y + in.z * in.z;
  len = sqrtf(len);
  return (Vec3f){.x = in.x / len, .y = in.y / len, .z = in.z / len};
}

Vec3f sub(const Vec3f a, const Vec3f b) {
  return (Vec3f){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

Vec3f cross(const Vec3f a, const Vec3f b) {
  return (Vec3f){.x = a.y * b.z - a.z * b.y,
                 .y = a.z * b.x - b.z * a.x,
                 .z = a.x * b.y - a.y * b.x};
}

Mat4f rotation_matrix(const Vec3f r, const Vec3f pos) {
  return (Mat4f){
      {{cosf(r.y) * cosf(r.z), -cosf(r.y) * sinf(r.z), sinf(r.y), pos.x},
       {cosf(r.x) * sinf(r.z) + sinf(r.x) * sinf(r.y) * cos(r.z),
        cosf(r.x) * cosf(r.z) - sinf(r.x) * sinf(r.y) * sinf(r.z),
        -sinf(r.x) * cosf(r.y), pos.y},
       {sinf(r.x) * sinf(r.z) - cosf(r.x) * sinf(r.y) * cosf(r.z),
        sinf(r.x) * cosf(r.z) + cosf(r.x) * sinf(r.y) * sinf(r.z),
        cosf(r.x) * cosf(r.y), pos.z},
       {0.f, 0.f, 0.f, 1.f}}};
}
