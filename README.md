Voxels
==============

Voxel Engine Written in Pure C using OpenGL
--------------

This is a basic voxel engine that I wrote primarily during spring break (2015).

It is a basic voxel engine written is C using OpenGL for rendering.

![Screenshot from the program](http://i.imgur.com/GjtyJOd.png)

Features include:
  * Basic physics engine
  * Place / destroy blocks of a chosen color
  * "Models", which are 16x16x16 chunks that fit in the space of a single normal block
  * Models can be saved to, and loaded from, files
  * A wiring system including basic logic gates
  * Ambient, diffuse, and specular light
  * Point light shadows
  * Face culling
  * Frustum culling
  * Triangle meshing

<img src="http://i.imgur.com/QrhrGA1.png" alt="A 5-to-32 decoder made in the game" height="300px">
<img src="http://i.imgur.com/iGvxO6X.png" alt="A 4-bit register made in the game" height="300px">

For now, the world is finite.

I intended to add multiple light sources, but for now only one works (adding more gets ridiculously slow).

The code to handle textures is there, but I removed their use, mostly for speed purposes.

Everything was written by hand; nearly none of the code is directly from other sources. :)

Uses GLEW and GLFW.

Compiles on my 2014 Retina MacBook Pro (MacBookPro11,1); I haven't tried on other setups.
