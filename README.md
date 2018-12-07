# Raytracer
A side project which aims to build an efficient photorealistic software renderer.

<p float="left">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/1.jpg">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/2.jpg">
</p>

Features
========

General
---------

* Written in C++14
* Developed for Windows and Linux
* Multithreaded rendering (_obvious_)
* Optimized using SSE and AVX intrinsics (especially in performance-critical and low level math code)
* Parsing scene description from a JSON file

Rendering
---------

* Adaptive sampling (using more samples in noisy image areas)
* Spectral rendering using _hero wavelength_ method (_Note: disabled with a macro by default_)
* Camera simulation:
  * Depth of Field (circle, hexagonal or square bokeh)
  * Barrel distortion
  * Motion blur
  
Geometry
--------

* Bounding Volume Hierarchy (BVH) used for scene and mesh traversal (two levels of BVH)
* Scene object types: Triangle meshes, Sphere, Box, Plane
* Full per-object motion blur (translational and rotational)

Lighting
--------
* Algorithm: unidirectional path tracing
* Multiple importance sampling (sampling both lights and material BSDF)
* Supported light types: point, area, directional, background

Materials
---------

* Physically based metal and dielectric model (can be blend using _metalness_ parameter)
* Cook-Torrance BSDF for specular reflection with GGX normal distribution
* Transparency and refraction
* Chromatic dispersion 
* Normal mapping support

Textures
--------

* 2D bitmap textures
* Supported pixel formats: 8-bit unsigned int, 32-bit float, 16-bit float, block-compressed BC1, BC4 and BC5
* Supported file formats: BMP, DDS, EXR (thanks to https://github.com/syoyo/tinyexr library)


TODO list
=========

* Better light transport algorithm (e.g. Vertex Connection and Merging)
* Better material model and multilayer materials
* Procedural textures
* Volumetric rendering
* Optimize traversal and shading with SSE/AVX
