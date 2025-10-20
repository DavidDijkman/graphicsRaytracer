#include "cylinder.h"

#include <cmath>

using namespace std;

Hit Cylinder::intersect(Ray const &ray) {
   Vector axisDir = direction.normalized();
   double height = direction.length();

   Vector x = ray.O - position;
   double d_ax = ray.D.dot(axisDir);
   double d2 = ray.D.dot(ray.D);
   double x_ax = x.dot(axisDir);

   double a = d2 - d_ax * d_ax;
   if (fabs(a) < __DBL_EPSILON__) {
      // parallel
      return Hit::NO_HIT();
   }
   double b = 2 * (ray.D - axisDir * d_ax).dot((x - axisDir * x_ax));
   double c = x.dot(x) - x_ax * x_ax - radius * radius;

   double discriminant = b * b - 4 * a * c;
   Vector N;

   if (discriminant < 0.0) {
      return Hit::NO_HIT();
   } else if (fabs(discriminant) < __DBL_EPSILON__) {
      // 1 intersection
      double t = -b / (2 * a);
      double k = d_ax * t + x_ax;
      if (k < height && k >= 0.0) {
         N = (ray.at(t) - (position + axisDir * k)).normalized();
         return Hit(t, N);
      }
   }
   double t;
   double t1 = (-b + sqrt(discriminant)) / (2 * a);
   double t2 = (-b - sqrt(discriminant)) / (2 * a);

   double k;
   double k1 = d_ax * t1 + x_ax;
   double k2 = d_ax * t2 + x_ax;

   if ((k1 < 0.0 && k2 < 0.0) || (k1 > height && k2 > height)) {
      // above or below cilinder
      return Hit::NO_HIT();
   }

   // check if fully through shaft of cilinder
   if (k1 < height && k1 > 0 && k2 < height && k2 > 0) {
      // check behind origin
      if (t1 < 0 && t2 < 0) {
         return Hit::NO_HIT();
      }
      // if origin within cilinder

      if (min(t1, t2) == t1 && t1 > 0) {
         t = t1;
         k = k1;
      } else if (min(t1, t2) == t2 && t2 > 0) {
         t = t2;
         k = k2;
      }
      N = (ray.at(t) - (position + axisDir * k)).normalized();
      return Hit(t, N);
   }

   double t_shaft = -1;
   double k_shaft;
   Vector N_shaft;

   if (!((k1 < 0.0 && k2 > height) || (k1 > height && k2 < 0.0))) {
      // intersects with cilinder.
      t_shaft = (k1 > 0.0 && k1 < height ? t1 : t2);
      k_shaft = (k1 > 0.0 && k1 < height ? k1 : k2);
      N_shaft = (ray.at(t_shaft) - (position + axisDir * k_shaft)).normalized();
   }

   // we intersect the ray with the base and extruded plane of the cilinder
   double nl = axisDir.dot(ray.D);

   if (fabs(nl) < __DBL_EPSILON__) {
      // parallel to caps, shouldn't be possible at this stage
      return Hit::NO_HIT();
   }

   Vector roof_position = position + direction;

   double t_cap = -1;
   Vector N_cap = axisDir;
   if (N_cap.dot(ray.D) > 0) {
      N_cap = -N_cap;  // flip to face the ray
   }

   double tb = (position - ray.O).dot(-N_cap) / nl;
   double tt = (roof_position - ray.O).dot(N_cap) / nl;

   if (tb > 0.0 && (ray.at(tb) - position).length_2() <= radius * radius) {
      t_cap = tb;
   }
   
   if (tt > 0.0 && (ray.at(tt) - roof_position).length_2() <= radius * radius &&
       (tt < t_cap || t_cap < 0.0)) {
      t_cap = tt;
   }
   

   if (t_shaft < 0.0 && t_cap < 0.0) {
      //shouldn't happen
      return Hit::NO_HIT();
   }

   if (t_cap < 0.0) {
      return Hit(t_shaft, N_shaft);
   }

   if (t_shaft < 0.0) {
      return Hit(t_cap, N_cap);
   }

   if (t_cap < t_shaft) {
      return Hit(t_cap, N_cap);
   }
   return Hit::NO_HIT();
}

Cylinder::Cylinder(Point const &pos, Vector const &direction, double radius)
    : position(pos), direction(direction), radius(radius) {}
