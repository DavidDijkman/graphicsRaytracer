#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "image.h"
#include "triple.h"

class Material {
 public:
  Color color;  // base color
  double silhouette;
  double ka;    // ambient coefficient
  double kd;    // diffuse coefficient
  double ks;    // specular coefficient
  double n;     // exponent for specular highlight size
  

  bool hasTexture = false;
  Image texture;

  bool isTransparent = false;
  double nt = 1.0;
  double absorption;
  double haze;

  Material() = default;

  Material(Color const &color, double sil, double ka, double kd, double ks, double n)
      : color(color), silhouette(sil), ka(ka), kd(kd), ks(ks), n(n), hasTexture(false), texture() {}

  Material(Image const &texture, double sil, double ka, double kd, double ks, double n)
      : color(), silhouette(sil), ka(ka), kd(kd), ks(ks), n(n), hasTexture(true), texture(texture) {}

  Material(Color const &color, double sil, double ka, double kd, double ks, double n, double nt, double absorption)
      : color(color), silhouette(sil), ka(ka), kd(kd), ks(ks), n(n), isTransparent(true), nt(nt), absorption(absorption) {}

  Material(Color const &color, double sil, double ka, double kd, double ks, double n, double nt, double absorption, double haze)
      : color(color), silhouette(sil), ka(ka), kd(kd), ks(ks), n(n), isTransparent(true), nt(nt), absorption(absorption), haze(haze) {}
};

#endif
