/*
 * Vector3DMath.hpp
 *
 *  Created on: Mar 28, 2011
 *      Author: isaak
 */

#include <iostream>
#include <cmath>

using namespace std;

struct Vector {
   union {
      float x, u, r;
   };
   union {
      float y, v, g;
   };
   union {
      float z, w, b;
   };

   Vector(const float s = 0) :
      x(s), y(s), z(s) {
   }

   Vector(const float x, const float y, const float z) :
      x(x), y(y), z(z) {
   }

   // warning: memory hack!
   float& operator [](const int index) {
      return *(&x + index);
   }

   // read only elements accessor
   const float& operator [](const int index) const {
      return *(&x + index);
   }

   bool operator ==(const Vector& v) const {
      return v.x == x && v.y == y && v.z == z;
   }

   bool operator !=(const Vector& v) const {
      return v.x != x || v.y != y || v.z != z;
   }

   Vector operator -() const {
      return Vector(-x, -y, -z);
   }

   Vector operator +(const Vector& v) const {
      return Vector(x + v.x, y + v.y, z + v.z);
   }

   Vector operator -(const Vector& v) const {
      return Vector(x - v.x, y - v.y, z - v.z);
   }

   Vector operator *(const Vector& v) const {
      return Vector(x * v.x, y * v.y, z * v.z);
   }

   Vector operator *(const float s) const {
      return Vector(x * s, y * s, z * s);
   }

   Vector operator /(const Vector& v) const {
      return Vector(x / v.x, y / v.y, z / v.z);
   }

   Vector operator /(const float s) const {
      const float inverse_s = 1.0 / s;
      return Vector(x * inverse_s, y * inverse_s, z * inverse_s);
   }

   // dot (inner) product
   float operator ^(const Vector& v) const {
      return x * v.x + y * v.y + z * v.z;
   }

   // cross product
   Vector operator %(const Vector& v) const {
      return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
   }

   const Vector& operator =(const Vector& v) {
      x = v.x, y = v.y, z = v.z;
      return *this;
   }

   const Vector& operator +=(const Vector& v) {
      x += v.x, y += v.y, z += v.z;
      return *this;
   }

   const Vector& operator -=(const Vector& v) {
      x -= v.x, y -= v.y, z -= v.z;
      return *this;
   }

   const Vector& operator *=(const Vector& v) {
      x *= v.x, y *= v.y, z *= v.z;
      return *this;
   }

   const Vector& operator *=(const float s) {
      x *= s, y *= s, z *= s;
      return *this;
   }

   const Vector& operator /=(const Vector& v) {
      x /= v.x, y /= v.y, z /= v.z;
      return *this;
   }

   const Vector& operator /=(const float s) {
      const float inverse_s = 1.0 / s;
      x *= inverse_s, y *= inverse_s, z *= inverse_s;
      return *this;
   }

   // cross product
   const Vector& operator %=(const Vector& v) {
      const float tx = x, ty = y, tz = z;
      x = ty * v.z - tz * v.y;
      y = tz * v.x - tx * v.z;
      z = tx * v.y - ty * v.x;
      return *this;
   }

   float squared_length() const {
      return x * x + y * y + z * z;
   }

   float length() const {
      return sqrt(x * x + y * y + z * z);
   }

   // normalize to a unit vector per default
   Vector normalize(const float length = 1) const {
      const float s = length / sqrt(x * x + y * y + z * z);
      return Vector(x * s, y * s, z * s);
   }

   const Vector& read(istream& is) {
      is.read((char*) this, sizeof(Vector));
      return *this;
   }

   void write(ostream& os) const {
      os.write((char*) this, sizeof(Vector));
   }

};

inline istream& operator >>(istream& is, Vector& v) {
   is >> ws;
   if (is.peek() != '(')
      is.clear(std::ios::failbit);
   is.get();
   is >> v.x >> ws;
   if (is.peek() != ',')
      is.clear(std::ios::failbit);
   is.get();
   is >> v.y >> ws;
   if (is.peek() != ',')
      is.clear(std::ios::failbit);
   is.get();
   is >> v.z >> ws;
   if (is.peek() != ')')
      is.clear(std::ios::failbit);
   is.get();
   return is;
}

inline ostream& operator <<(ostream& os, const Vector& v) {
   return os << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}

struct Matrix {
   Vector a, b, c; // direct access to column vectors

   // create an identity matrix per default
   Matrix(const float s = 1) :
      a(s, 0, 0), b(0, s, 0), c(0, 0, s) {
   }

   Matrix(const float s, const float t, const float u) :
      a(s, 0, 0), b(0, t, 0), c(0, 0, u) {
   }

   Matrix(const Vector& a, const Vector& b, const Vector& c) :
      a(a), b(b), c(c) {
   }

   // warning: memory hack!
   Vector& operator [](const int index) {
      return *(&a + index);
   }

   // read only elements accessor
   const Vector& operator [](const int index) const {
      return *(&a + index);
   }

   bool operator ==(const Matrix& m) const {
      return a == m.a && b == m.b && c == m.c;
   }

   bool operator !=(const Matrix& m) const {
      return a != m.a || b != m.b || c != m.c;
   }

   Matrix operator -() const {
      return Matrix(-a, -b, -c);
   }

   Matrix operator +(const Matrix& m) const {
      return Matrix(a + m.a, b + m.b, c + m.c);
   }

   Matrix operator -(const Matrix& m) const {
      return Matrix(a - m.a, b - m.b, c - m.c);
   }

   Matrix operator *(const float s) const {
      return Matrix(a * s, b * s, c * s);
   }

   Vector operator *(const Vector& v) const {
      return a * v.x + b * v.y + c * v.z;
   }

   Matrix operator *(const Matrix& m) const {
      return Matrix(m.a * a.x + m.b * a.y + m.c * a.z,
            m.a * b.x + m.b * b.y + m.c * b.z,
            m.a * c.x + m.b * c.y + m.c * c.z);
   }

   Matrix operator /(const float s) const {
      const float inverse_s = 1.0 / s;
      return Matrix(a * inverse_s, b * inverse_s, c * inverse_s);
   }

   const Matrix& operator =(const Matrix& m) {
      a = m.a, b = m.b, c = m.c;
      return *this;
   }

   const Matrix& operator +=(const Matrix& m) {
      a += m.a, b += m.b, c += m.c;
      return *this;
   }

   const Matrix& operator -=(const Matrix& m) {
      a -= m.a, b -= m.b, c -= m.c;
      return *this;
   }

   const Matrix& operator *=(const Matrix& m) {
      a *= m.a, b *= m.b, c *= m.c;
      return *this;
   }

   const Matrix& operator *=(const float s) {
      a *= s, b *= s, c *= s;
      return *this;
   }

   const Matrix& operator /=(const float s) {
      const float inverse_s = 1.0 / s;
      a *= inverse_s, b *= inverse_s, c *= inverse_s;
      return *this;
   }

   float trace() const {
      return a.x + b.y + c.z;
   }

   float determinant() const {
      return a.x * (b.y * c.z - c.y * b.z) - b.x * (a.y * c.z - c.y * a.z)
            + c.x * (a.y * b.z - b.y * a.z);
   }

   Matrix transpose() const {
      return Matrix(Vector(a.x, b.x, c.x), Vector(a.y, b.y, c.y),
            Vector(a.z, b.z, c.z));
   }

   const Matrix& read(istream& is) {
      is.read((char*) this, sizeof(Matrix));
      return *this;
   }

   void write(ostream& os) const {
      os.write((char*) this, sizeof(Matrix));
   }
};

inline istream& operator >>(istream& is, Matrix& m) {
   is >> ws;
   if (is.peek() != '[')
      is.clear(std::ios::failbit);
   is.get();
   is >> m.a >> ws;
   if (is.peek() != ',')
      is.clear(std::ios::failbit);
   is.get();
   is >> m.b >> ws;
   if (is.peek() != ',')
      is.clear(std::ios::failbit);
   is.get();
   is >> m.c >> ws;
   if (is.peek() != ']')
      is.clear(std::ios::failbit);
   is.get();
   return is;
}

inline ostream& operator <<(ostream& os, const Matrix& m) {
   return os << '[' << m.a << ',' << m.b << ',' << m.c << ']';
}

inline Matrix outer_product(const Vector& v) {
   return Matrix(v * v.x, v * v.y, v * v.z);
}

inline void ensure_symmetry(Matrix& m) {
   m.a.y = m.b.x = (m.a.y + m.b.x) * 0.5;
   m.a.z = m.c.x = (m.a.z + m.c.x) * 0.5;
   m.b.z = m.c.y = (m.b.z + m.c.y) * 0.5;
}
