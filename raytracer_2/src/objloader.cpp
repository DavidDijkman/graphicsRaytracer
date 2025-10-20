#include "objloader.h"

// Pro C++ Tip: here you can specify other includes you may need
// such as <iostream>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// ===================================================================
// -- Constructors and destructor ------------------------------------
// ===================================================================

// --- Public --------------------------------------------------------

OBJLoader::OBJLoader(string const &filename) : d_hasTexCoords(false) { parseFile(filename); }

// ===================================================================
// -- Member functions -----------------------------------------------
// ===================================================================

// --- Public --------------------------------------------------------

vector<Vertex> OBJLoader::vertex_data() const {
  vector<Vertex> data;

  // For all vertices in the model, interleave the data
  for (Vertex_idx const &vertex : d_vertices) {
    // Add coordinate data
    Vertex vert;

    vec3 const coord = d_coordinates.at(vertex.d_coord);
    vert.x = coord.x;
    vert.y = coord.y;
    vert.z = coord.z;

    // Add normal data
    vec3 const norm = d_normals.at(vertex.d_norm);
    vert.nx = norm.x;
    vert.ny = norm.y;
    vert.nz = norm.z;

    // Add texture data (if available)
    if (d_hasTexCoords) {
      vec2 const tex = d_texCoords.at(vertex.d_tex);
      vert.u = tex.u;  // u coordinate
      vert.v = tex.v;  // v coordinate
    } else {
      vert.u = 0;
      vert.v = 0;
    }
    data.push_back(vert);
  }

  return data;  // copy elision
}

unsigned OBJLoader::numTriangles() const { return d_vertices.size() / 3U; }

bool OBJLoader::hasTexCoords() const { return d_hasTexCoords; }

void OBJLoader::unitize() {
  // TODO: implement this yourself!

  // This is a very handy function for importing models
  // which you may reuse in other projects.
  // You may have noticed you can use arbitrary sizes for your
  // models. You may find that modelers do not always use the
  // same size for models. Therefore it might be preferable to
  // scale the object to fit inside the unit cube so you can easily
  // set the right scale of your model in OpenGL. Aditionally,
  // the model does not have to be centered around the origin
  // (0, 0, 0) which may cause troubles when translating
  // This function should fix that!

  // A common approach looks like this:

  // Determine min / max and offset in each dimension
  // Determine by which factor to scale (largest difference
  //  in min / max in a dimension (Important! Scale uniformaly in
  //  all dimensions!)
  // Loop over all coordinate data and scale the coordinates
  //  and apply the translate/scaling

  Triple mins = findMinVal();
  Triple maxs = findMaxVal();

  Triple ranges = Triple(maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z);
  double scale = max(ranges.x, max(ranges.y, ranges.z));
  Triple center = Triple(mins.x + ranges.x/2.0, mins.y + ranges.y/2.0, mins.z + ranges.z/2.0);

  for (vec3 &c : d_coordinates) {
    c.x = (c.x - center.x)/scale;
    c.y = (c.y - center.y)/scale;
    c.z = (c.z - center.z)/scale;
  }

}

// --- Private -------------------------------------------------------

void OBJLoader::parseFile(string const &filename) {
  ifstream file(filename);
  if (file) {
    string line;
    while (getline(file, line)) parseLine(line);

  } else {
    cerr << "Could not open: " << filename << " for reading!\n";
  }
}

void OBJLoader::parseLine(string const &line) {
  if (line[0] == '#') return;  // ignore comments

  StringList tokens = split(line, ' ', false);

  if (tokens[0] == "v")
    parseVertex(tokens);
  else if (tokens[0] == "vn")
    parseNormal(tokens);
  else if (tokens[0] == "vt")
    parseTexCoord(tokens);
  else if (tokens[0] == "f")
    parseFace(tokens);

  // Other data is also ignored
}

void OBJLoader::parseVertex(StringList const &tokens) {
  float x, y, z;
  x = stof(tokens.at(1));  // 0 is the "v" token
  y = stof(tokens.at(2));
  z = stof(tokens.at(3));
  d_coordinates.push_back(vec3{x, y, z});
}

void OBJLoader::parseNormal(StringList const &tokens) {
  float x, y, z;
  x = stof(tokens.at(1));  // 0 is the "vn" token
  y = stof(tokens.at(2));
  z = stof(tokens.at(3));
  d_normals.push_back(vec3{x, y, z});
}

void OBJLoader::parseTexCoord(StringList const &tokens) {
  d_hasTexCoords = true;  // Texture data will be read

  float u, v;
  u = stof(tokens.at(1));  // 0 is the "vt" token
  v = stof(tokens.at(2));
  d_texCoords.push_back(vec2{u, v});
}

void OBJLoader::parseFace(StringList const &tokens) {
  // skip the first token ("f")
  for (size_t idx = 1; idx < tokens.size(); ++idx) {
    // format is:
    // <vertex idx + 1>/<texture idx +1>/<normal idx + 1>
    // Wavefront .obj files start counting from 1 (yuck)

    StringList elements = split(tokens.at(idx), '/');
    Vertex_idx vertex{};  // initialize to zeros on all fields

    vertex.d_coord = stoul(elements.at(0)) - 1U;

    if (d_hasTexCoords)
      vertex.d_tex = stoul(elements.at(1)) - 1U;
    else
      vertex.d_tex = 0U;  // ignored

    vertex.d_norm = stoul(elements.at(2)) - 1U;

    d_vertices.push_back(vertex);
  }
}

OBJLoader::StringList OBJLoader::split(string const &line, char splitChar, bool keepEmpty) {
  StringList tokens;
  istringstream iss(line);
  string token;
  while (getline(iss, token, splitChar))
    if (token.size() > 0 || (token.size() == 0 && keepEmpty)) tokens.push_back(token);

  return tokens;
}

Triple OBJLoader::findMinVal() {
  Triple minVals = Triple(d_coordinates[0].x, d_coordinates[0].y, d_coordinates[0].z);
  for (const auto &c : d_coordinates) {
    if (c.x < minVals.x) {minVals.x = c.x;}
    if (c.y < minVals.y) {minVals.y = c.y;}
    if (c.z < minVals.z) {minVals.z = c.z;}
  }
  return minVals;
}

Triple OBJLoader::findMaxVal() {
  Triple maxVals = Triple(d_coordinates[0].x, d_coordinates[0].y, d_coordinates[0].z);
  for (const vec3 &c : d_coordinates) {
    if (c.x > maxVals.x) {maxVals.x = c.x;}
    if (c.y > maxVals.y) {maxVals.y = c.y;}
    if (c.z > maxVals.z) {maxVals.z = c.z;}
  }
  return maxVals;
}
