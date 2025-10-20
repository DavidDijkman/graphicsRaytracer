#include "triangle.h"

Hit Triangle::intersect(Ray const &ray) {
    Vector e1 = v1 - v0;
    Vector e2 = v2 - v0;
    double det = e1.dot(ray.D.cross(e2));
    double invDet = 1.0/det;
    Vector T = ray.O - v0;
    double u = (T.dot(ray.D.cross(e2))) * invDet;
    if (u < 0 || u > 1) return Hit::NO_HIT();
    double v = (ray.D.dot(T.cross(e1))) * invDet;
    if (v < 0 || u+v > 1) return Hit::NO_HIT();
    double t = (e2.dot(T.cross(e1))) * invDet;
    if (t < 0) return Hit::NO_HIT();
    
    // Note that the direction of the normal is not changed here,
    // but in scene.cpp - if necessary.
    //if (N.dot(ray.D) > 0) N = -N; we don't turn the normal around
    return Hit(t, N);  // placeholder
}

Triangle::Triangle(Point const &v0, Point const &v1, Point const &v2) : v0(v0), v1(v1), v2(v2), N() {
  // Calculate the surface normal here and store it in the N,
  // which is declared in the header. It can then be used in the intersect function.
  Vector e1 = v1 - v0;
  Vector e2 = v2 - v0;
  N = e1.cross(e2).normalized();
}
