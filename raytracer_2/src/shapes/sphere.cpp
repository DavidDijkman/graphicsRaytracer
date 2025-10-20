#include "sphere.h"

#include <cmath>

#include "solvers.h"

using namespace std;

Hit Sphere::intersect(Ray const &ray) {
   // Sphere formula: ||x - position||^2 = r^2
   // Line formula:   x = ray.O + t * ray.D

   Vector L = ray.O - position;
   double a = ray.D.dot(ray.D);
   double b = 2.0 * ray.D.dot(L);
   double c = L.dot(L) - r * r;

   double t0;
   double t1;
   if (not Solvers::quadratic(a, b, c, t0, t1)) return Hit::NO_HIT();

   // t0 is closest hit
   if (t0 < 0.0)  // check if it is not behind the camera
   {
      t0 = t1;       // try t1
      if (t0 < 0.0)  // both behind the camera
         return Hit::NO_HIT();
   }

   // calculate normal
   Point hit = ray.at(t0);
   Vector N = (hit - position).normalized();

   // Note that the direction of the normal is not changed here,
   // but in scene.cpp - if necessary.

   return Hit(t0, N);
}

Vector Sphere::toUV(Point const &hit) {
   // placeholders
   Vector rotAxis = axis.normalized();

   // we rotate around x and change the angle because there's some weird contstant inconsistency
   /*
   Vector upAxis;
   upAxis.x = rotAxis.x;
   upAxis.y = cos(-M_PI/2)*sphereYAxis.y - sin(-M_PI/2)*sphereYAxis.z;
   upAxis.z = cos(-M_PI/2)*sphereYAxis.z + sin(-M_PI/2)*sphereYAxis.y;
   */

   Vector hitToCenter = (position - hit).normalized();
   
   Point qHit;
   if (fmod(angle, 360.0) < 0.0001) {
      // no rotation.
      qHit = hitToCenter;
   } else {
      // we take the negative, since we actually want to rotate from axis to
      // base vector.

      double theta = -M_PI*angle/180.0;

      double qw = cos(theta / 2.0);
      double sinHalfTheta = sin(theta / 2.0);
      Vector qxyz = Vector(rotAxis.x * sinHalfTheta, rotAxis.y * sinHalfTheta,
                           rotAxis.z * sinHalfTheta);
      double qm = qw * qw + qxyz.length_2();
      qw = qw/qm;
      qxyz = qxyz/qm;

      Vector t = 2 * qxyz.cross(hitToCenter);
      qHit = hitToCenter + qw * t + qxyz.cross(t);
   }

   double v = 0.5 + asin(-qHit.z) / M_PI;
   double u = 0.5 + (atan2(-qHit.y, -qHit.x)) / (2 * M_PI);

   // Use a Vector to return 2 doubles. The third value is never read.
   return Vector{u, v, 0.0};
}

Sphere::Sphere(Point const &pos, double radius, Vector const &axis,
               double angle)
    :  // Feel free to modify this constructor.
      position(pos),
      r(radius),
      axis(axis),
      angle(angle) {}
