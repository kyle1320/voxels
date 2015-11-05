Voxels
==============

Voxel Engine Written in Pure C using OpenGL
--------------

This is a basic voxel engine that I wrote primarily during spring break (2015).

It is a basic voxel engine written is C using OpenGL for rendering.

Features include:
  * Basic physics engine
  * Place / destroy blocks of a chosen color
  * "Models", which are 16x16x16 chunks that fit in the space of a single normal block
  * Models can be saved to, and loaded from, files
  * Ambient, diffuse, and specular light
  * Point light shadows
  * Face culling
  * Frustum culling
  * Triangle meshing

For now, the world is finite.

I intended to add multiple light sources, but for now only one works (adding more gets ridiculously slow).

The code to handle textures is there, but I removed their use, mostly for speed purposes.

Everything was written by hand; nearly none of the code is directly from other sources. :)

Uses GLEW and GLFW.

Compiles on my 2014 Retina MacBook Pro (MacBookPro11,1); I haven't tried on other setups.
