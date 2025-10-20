#include "mesh.h"

#include <cmath>
#include <iostream>
#include <limits>

#include "../objloader.h"
#include "../vertex.h"
#include "triangle.h"

using namespace std;

Hit Mesh::intersect(Ray const &ray) {
  // Replace the return of a NO_HIT by determining the intersection based
  // on the ray and this class's data members.
  double t = 0.0;
  Vector N;
  bool hitFound = false;
  for (int i = 0; i < (int) d_tris.size(); i++) {
    if (std::isnan(d_tris[i]->intersect(ray).t)) continue;
    double tempT = d_tris[i]->intersect(ray).t;
    if (t == 0.0 || tempT < t) {
      t = tempT;
      N = d_tris[i]->intersect(ray).N;
      hitFound = true;
    }
  }
  if (hitFound) {
    return Hit(t, N);
  } else {
    return Hit::NO_HIT();
  }
}

Mesh::Mesh(string const &filename, Point const &position, Vector const &rotation, Vector const &scale) {
  OBJLoader model(filename);
  model.unitize();
  d_tris.reserve(model.numTriangles());
  vector<Vertex> vertices = model.vertex_data();
  for (size_t tri = 0; tri != model.numTriangles(); ++tri) {
    Vertex one = vertices[tri * 3];
    Point v0(one.x, one.y, one.z);

    Vertex two = vertices[tri * 3 + 1];
    Point v1(two.x, two.y, two.z);

    Vertex three = vertices[tri * 3 + 2];
    Point v2(three.x, three.y, three.z);

    // Apply non-uniform scaling, rotation and translation to the three points
    // of the triangle (v0, v1, and v2) here.

    // Non-uniform scaling
    //        v0 = ...;
    //        v1 = ...;
    //        v2 = ...;
    v0 = scale*v0;
    v1 = scale*v1;
    v2 = scale*v2;

    // Rotation
    // ...
    v0 = v0.rotate(rotation*M_PI/180.0);
    v1 = v1.rotate(rotation*M_PI/180.0);
    v2 = v2.rotate(rotation*M_PI/180.0);

    // Translation
    // ...
    v0 = position+v0;
    v1 = position+v1;
    v2 = position+v2;

    d_tris.push_back(ObjectPtr(new Triangle(v0, v1, v2)));
  }

  cout << "Loaded model: " << filename << " with " << model.numTriangles() << " triangles.\n";
}
