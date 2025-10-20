#ifndef CYLINDER_H_
#define CYLINDER_H_

#include "../object.h"

class Cylinder : public Object {
  Point const position;
  Vector const direction;
  double const radius;

 public:
  Cylinder(Point const &pos, Vector const &direction, double radius);

  virtual Hit intersect(Ray const &ray);
};

#endif