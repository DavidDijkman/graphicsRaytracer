#ifndef MESH_H_
#define MESH_H_

#include <string>
#include <vector>

#include "../object.h"

class Mesh : public Object {
  std::vector<ObjectPtr> d_tris;

 public:
  Mesh(std::string const &filename, Point const &position, Vector const &rotation, Vector const &scale);

  virtual Hit intersect(Ray const &ray);
};

#endif
