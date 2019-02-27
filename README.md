# Raytracer
A side project which aims to build an efficient photorealistic software renderer.

<p float="left">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/1.jpg">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/2.jpg">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/sponza.jpg">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/rough_glass.jpg">
    <img width="49%" src="https://raw.githubusercontent.com/Witek902/Raytracer/master/Gallery/caustics.jpg">
</p>

Features
========

General
---------

* Written in C++14
* Developed for Windows and Linux
* Optimized using SSE and AVX intrinsics (especially in performance-critical and low level math code)
* Parsing scene description from a JSON file

Rendering
---------

* Low discrepancy sampling (scrambled Halton sequence)
* Adaptive sampling (using more samples in noisy image areas)
* Spectral rendering using _hero wavelength_ method (_Note: disabled with a macro by default_)
* Debug rendering mode (for visualizing depth, normal vectors, material parameters, etc.)
* Camera simulation:
  * Depth of Field (circle or polygonal bokeh)
  * Barrel distortion
  * Motion blur
  
Geometry
--------

* Bounding Volume Hierarchy (BVH) used for scene and mesh traversal (two levels of BVH)
* Scene object types: Triangle meshes, Sphere, Box, Plane
* Full per-object motion blur (translational and rotational)
* Both single-ray and AVX-optimized stream packet traversal

Lighting
--------
* Supported light transport algorithms:
  * Naive Path Tracing (sampling only BSDF)
  * Path Tracing with multiple importance sampling (sampling both lights and material BSDF)
  * Bidirectional Path Tracing (with MIS)
  * Vertex Connection and Merging (_Note: work in progress, not all lights and BSDFs are working properly_)
  * Light Tracing (for debugging) 
* Supported light types: point, area (triangle or quad), spherical, directional, background

Materials
---------

* Physically based BSDFs: diffuse, metal, dielectric, plastic
* Cook-Torrance BSDF for specular reflection with GGX normal distribution
* Transparency and refraction
* Normal mapping support

Textures
--------

* 2D bitmap textures
* Supported pixel formats: 8-bit unsigned int, 32-bit float, 16-bit float, block-compressed BC1, BC4 and BC5
* Supported file formats: BMP, DDS, EXR (thanks to https://github.com/syoyo/tinyexr library)


TODO list
=========

* Better material model and multilayer materials (e.g. introduce Disney-like "principled" material)
* Procedural textures
* Volumetric rendering
* Optimize traversal and shading with SSE/AVX
