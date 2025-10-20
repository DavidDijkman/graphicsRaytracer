# Computer Graphics Raytracer Competition Submission
by David Dijkman

This extension of the raytracer template adds some material additions.
These include two types of colour diminishing in volumes

# Absorption
Glassy materials can now be given an "absorption" tag, a double > 0.0. This makes the
material absorb light passing through it based on the material colour, the absorption coefficient, 
and the distance travelled through that medium. This makes it so that larger models will require larger
absorption coefficients to have the same visual effect of a smaller model.

# Haze
Both glassy materials and the full scene can be assigned a "haze"("Haze" for the full scene) coefficient. This
scales of the material colour for materials, and off of a separate "HazeColour" triple for the scene. As opposed
to absorption, this effect blends the light colour towards the 'haze colour'. This gives a more foggy effect. 

Because of some sloppy coding, right now haze and absorption don't work in conjunction in the same material. Also,
to apply haze to a material, it needs to have an absorption coefficient, set to 0.0. Sorry.

# Materials inside glassy materials
You can put different materials inside larger transparent materials and it will correctly make shadows for the encapsulated materials.
Don't overlap surfaces with this, it only works if the volumes are well defined.

# Attenuated shadows through transparent materials
Shadow rays that are cast through transparent materials now get attenuated if the material has any haze or absorption.
Haze simply reduces the intensity of the passing light, while absorption attenuates colours seperately.
These shadows do not take refraction into account, I hope it's clear why.

# Example render

The below image shows almost all of the features of the current implementation. It shows a glassy cube encapsulating a reflective ball.
Some hazy fog bubbles are made with a material with a matching refractive index to the scene(1.0).

<img width="400" height="400" alt="31" src="https://github.com/user-attachments/assets/51689695-fcea-474f-8248-c5a4f4de4470" />
