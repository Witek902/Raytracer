
**!!! NOTE !!!**

**The project is not developed in this repository anymore.
The source code was moved into NFEngine project:
https://github.com/nfprojects/nfengine/tree/devel/Src/Engine/Raytracer**


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
* Highly optimized using SSE and AVX intrinsics
* Parsing scene description from a JSON file

Rendering
---------

* Low discrepancy sampling (scrambled Halton sequence)
* Blue noise dithered sampling
* Adaptive rendering (using more samples in noisy image areas)
* Spectral rendering using _hero wavelength_ method (_Note: disabled by default_)
* Debug rendering mode (for visualizing depth, normal vectors, material parameters, etc.)
* Camera simulation:
  * Depth of Field (circle or polygonal bokeh)
  * Barrel distortion
  * Motion blur
  
Geometry
--------

* Bounding Volume Hierarchy (BVH) used for scene and mesh traversal (two levels of BVH)
* Supported shape types: triangle meshes, sphere, box, rectangle
* Full per-object motion blur (translational and rotational)
* Both single-ray and AVX-optimized stream packet traversal

Lighting
--------
* Supported light transport algorithms:
  * Naive Path Tracing (sampling only BSDF)
  * Path Tracing with multiple importance sampling (sampling both lights and material BSDF)
  * Bidirectional Path Tracing (with MIS)
  * Light Tracing (for debugging) 
* Supported light types: point, directional, background, area (any shape)
* 

Materials
---------

* Physically based BSDFs: diffuse, metal, dielectric, plastic
* Cook-Torrance BSDF for specular reflection with GGX normal distribution
* Transparency and refraction
* Normal mapping support

Textures
--------

* 2D bitmap textures
* Supports most of DXGI formats, including bit-packed formats, BC1, BC4 and BC5 block compression
* Supported file formats: BMP, DDS, EXR (thanks to https://github.com/syoyo/tinyexr library)


TODO list
=========

* Better material model and multilayer materials (e.g. introduce Disney-like "principled" material)
* Procedural textures
* Volumetric rendering
* Optimize traversal and shading with SSE/AVX
* Port to ARM NEON
