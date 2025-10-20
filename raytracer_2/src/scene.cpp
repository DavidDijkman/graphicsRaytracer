#include "scene.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

#include "hit.h"
#include "image.h"
#include "material.h"
#include "ray.h"

using namespace std;

pair<ObjectPtr, Hit> Scene::castRay(Ray const &ray) const {
   // Find hit object and distance
   Hit min_hit(numeric_limits<double>::infinity(), Vector());
   ObjectPtr obj = nullptr;
   for (unsigned idx = 0; idx != objects.size(); ++idx) {
      Hit hit(objects[idx]->intersect(ray));
      if (hit.t < min_hit.t) {
         min_hit = hit;
         obj = objects[idx];
      }
   }

   return pair<ObjectPtr, Hit>(obj, min_hit);
}

Color Scene::shadowCast(Ray const &ray, LightPtr const &light) const {
   // We ignore refraction here, cause screw that
   pair<ObjectPtr, Hit> mainHit = castRay(ray);
   ObjectPtr obj = mainHit.first;
   Hit min_hit = mainHit.second;
   Point hit = ray.at(min_hit.t);
   if (!obj || (light->position - hit).dot(light->position - ray.O) < 0) {
      // light is first
      return light->color;
   }

   Material mat = obj->material;

   if (!mat.isTransparent) {
      // opaque object blocking, no light.
      return Color(0, 0, 0);
   }

   Color transmission = Color(1.0, 1.0, 1.0);

   if (min_hit.N.dot(ray.D) > 0) {
      // we are inside a material.
      transmission *= (1 - mat.kd);

      // double coeff = 1 - exp(-min_hit.t * mat.absorption);
      // Color absorption = coeff * (Color(1, 1, 1) - mat.color);
      if (mat.absorption > 0.0) {
         Color attenuation =
          mat.color *
          (mat.ka + exp(-min_hit.t * mat.absorption));
         transmission = transmission * attenuation;
      } if (mat.haze > 0.0) {
      // add mist effect in material

         double coeff = 1 - exp(-min_hit.t * mat.haze);
         transmission = (1 - coeff) * transmission;
      }
   }

   Ray newRay = Ray(hit + ray.D * epsilon, ray.D);

   return transmission * shadowCast(newRay, light);
}

Color Scene::trace(Ray const &ray, unsigned depth) {
   pair<ObjectPtr, Hit> mainhit = castRay(ray);
   ObjectPtr obj = mainhit.first;
   Hit min_hit = mainhit.second;

   // No hit? Return background color.
   if (!obj) return backgroundColor;

   Material const &material = obj->material;
   Point hit = ray.at(min_hit.t);
   Vector V = -ray.D;
   double dist = min_hit.t;

   // N can still point both ways
   Vector N = min_hit.N;

   // The shading normal always points in the direction of the view,
   // as required by the Phong illumination model.
   Vector shadingN;
   if (N.dot(V) >= 0.0)
      shadingN = N;
   else
      shadingN = -N;

   Color matColor;
   if (material.hasTexture) {
      double u = obj->toUV(hit).x;
      double v = obj->toUV(hit).y;
      matColor = material.texture.colorAt(u, 1.0 - v);
   } else {
      matColor = material.color;
   }

   // Add ambient once, regardless of the number of lights.
   Color color =
       (material.isTransparent ? Color(0, 0, 0)
                                                      : material.ka * matColor);

   // Add diffuse and specular components.
   for (auto const &light : lights) {
      Vector L = (light->position - hit).normalized();

      double dotNormal = shadingN.dot(L);

      if (dotNormal < 0.0) {
         continue;
      }

      Color shadowColor = Color(1, 1, 1);
      if (renderShadows && dotNormal > 0.0) {
         Ray shadowRay = Ray(hit + epsilon * shadingN, L);
         pair<ObjectPtr, Hit> shadowHit = castRay(shadowRay);
         if (shadowHit.first != nullptr) {
            shadowColor = shadowCast(shadowRay, light);
         }
      }

      // silhouette? testing
      if (material.silhouette > 0.0 && abs(ray.D.dot(shadingN)) < material.silhouette) {
         continue;  // don't add any color.
      } else {
         // Add diffuse.

         double diffuse = std::max(dotNormal, 0.0);
         color += diffuse * material.kd * shadowColor * light->color * matColor;

         // Add specular.
         if (dotNormal > 0) {
            Vector reflectDir = reflect(-L, shadingN);
            double specAngle = std::max(reflectDir.dot(V), 0.0);
            double specular = std::pow(specAngle, material.n);

            color += specular * material.ks * shadowColor * light->color;
         }
      }
   }

   if (depth < 0) {
      color = matColor;
   }

   if (depth > 0 and material.isTransparent) {
      // The object is transparent, and thus refracts and reflects light.
      // Use Schlick's approximation to determine the ratio between the two.
      double ni;
      double nt;
      if (ray.D.dot(N) > 0) {
         // we are coming from inside the material.
         ni = material.nt;
         nt = 1;
      } else {
         // we are coming from outside the material.
         ni = 1;
         nt = material.nt;
      }

      double eta = ni / nt;

      double cosi = ray.D.dot(-shadingN);  // D & N are already normalized
      // double sini = sqrt(1 - pow(cosi, 2));

      double k = 1.0 - eta * eta * (1.0 - cosi * cosi);

      // double sinr = sini * ni / nt;
      // double cosr = sqrt(1 - sinr * sinr);

      double kr;
      double kt;
      if (k < 0.0) {
         // total internal reflection
         kr = 1.0;
         kt = 0.0;
      } else {
         double kr0 = pow((ni - nt) / (ni + nt), 2);

         kr = kr0 + (1 - kr0) * (pow(1 - cosi, 5));
         kt = 1 - kr;
      }

      // in case the material is translucent
      kt *= 1 - (material.kd + material.ka + material.ks);
      kr *= 1 - (material.kd + material.ka + material.ks);

      if (kr > 0.0) {
         // Add reflection component
         Ray reflectRay =
             Ray(hit + epsilon * shadingN, reflect(ray.D, shadingN));
         Color reflectColor = trace(reflectRay, depth - 1);
         color += kr * reflectColor;
      }

      if (kt > 0.0) {
         // Add refraction component

         Vector Nr = -shadingN;

         Vector refractionDir =
             (eta * ray.D + (eta * cosi - sqrt(k)) * shadingN).normalized();

         // Vector refractionVector = (T_parallel +
         // T_perpendicular).normalized();
         Ray refractRay = Ray(hit + epsilon * Nr, refractionDir);
         Color refractColor = trace(refractRay, depth - 1);
         color += kt * refractColor;
      }

   } else if (depth > 0 and material.ks > 0.0) {
      Ray reflectRay = Ray(hit + epsilon * shadingN, reflect(ray.D, shadingN));
      Color reflectColor = trace(reflectRay, depth - 1);
      color += material.ks * reflectColor;
   }

   Color transmission(1.0, 1.0, 1.0);
   if (ray.D.dot(N) > 0 && material.absorption > 0.0 &&
       material.isTransparent) {
      //double coeff = 1 - exp(-dist * material.absorption);
      // Color absorption = Color(1, 1, 1) - coeff * material.color;
      //transmission = material.color *
                     //(obj->material.ka + exp(-dist * material.absorption));

      transmission.r *= exp(-dist * (1.0 - material.color.r) * material.absorption);
      transmission.g *= exp(-dist * (1.0 - material.color.g) * material.absorption);
      transmission.b *= exp(-dist * (1.0 - material.color.b) * material.absorption);
   } else if (ray.D.dot(N) > 0 && material.haze > 0.0) {
      // add mist effect in material

      double coeff = 1 - exp(-dist * material.haze);
      color = (1 - coeff) * color + coeff * matColor;
   } else if (ray.D.dot(N) < 0 && hazeAbsorption > 0.0) {
      // add mist effect

      double coeff = 1 - exp(-dist * hazeAbsorption);
      color = (1 - coeff) * color + coeff * hazeColor;
      // Color absorption = coeff * (1 - hazeColor);
      // transmission -= absorption;
   }
   color = transmission * color;

   // printf("%f\t %f\n", transmission, exp(-dist * material.absorption));

   return color;
}

void Scene::render(Image &img) {
   unsigned w = img.width();
   unsigned h = img.height();

   double framel = 0;
   double framer = 400;
   double framet = 400;
   double frameb = 0;

   double px = (framer - framel) / ((double)w);
   double py = (framet - frameb) / ((double)h);

   double ssFactorx = px / ((double)supersamplingFactor + 1);
   double ssFactory = py / ((double)supersamplingFactor + 1);
   double ssContribution =
       1 / ((double)(supersamplingFactor * supersamplingFactor));

   for (unsigned y = 0; y < h; ++y) {
      
      std::cout.flush();
      std::cout << "\r" << y + 1 << "/" << h;
      

      for (unsigned x = 0; x < w; ++x) {

         Color col = Color(0, 0, 0);

         for (unsigned xs = 1; xs < supersamplingFactor + 1; xs++) {
            for (unsigned ys = 1; ys < supersamplingFactor + 1; ys++) {
               Point pixel(x * px + xs * ssFactorx,
                           h - 1 - y * py + ys * ssFactory, 0);
               Ray ray(eye, (pixel - eye).normalized());
               col += ssContribution * trace(ray, recursionDepth);
            }
         }

         col.clamp();
         img(x, y) = col;
      }
   }
   std::cout << "\n";
}

// --- Misc functions ----------------------------------------------------------

// Defaults
Scene::Scene()
    : objects(),
      lights(),
      eye(),
      renderShadows(false),
      recursionDepth(0),
      supersamplingFactor(1) {}

void Scene::addObject(ObjectPtr obj) { objects.push_back(obj); }

void Scene::addLight(Light const &light) {
   lights.push_back(LightPtr(new Light(light)));
}

void Scene::setEye(Triple const &position) { eye = position; }

void Scene::setBackground(Color const &color) { backgroundColor = color; }

void Scene::setHaze(double const &haze, Color const &hazeCol) {
   hazeAbsorption = haze;
   hazeColor = hazeCol;
}

unsigned Scene::getNumObject() { return objects.size(); }

unsigned Scene::getNumLights() { return lights.size(); }

void Scene::setRenderShadows(bool shadows) { renderShadows = shadows; }

void Scene::setRecursionDepth(unsigned depth) { recursionDepth = depth; }

void Scene::setSuperSample(unsigned factor) { supersamplingFactor = factor; }
