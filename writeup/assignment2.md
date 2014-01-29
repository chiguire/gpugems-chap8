---
title: 'Maths & Graphics - Coursework 2 - Simulating Diffraction'
author:
- name: Ciro Duran
- name: Bogdan Catana
---

Ciro Durán &lt;<ma302cd@gold.ac.uk>&gt;, Bogdan Catana &lt;<b.i.catana@gmail.com>&gt;\
*Goldsmiths, University of London*\
*MSc Computer Games & Entertainment*\
*Maths & Graphics*\
*Prof. Frederic F. Leymarie*\

*30th January, 2014*

## Introduction

The objective of this report is to describe the development of a software that explores a particular topic on this subject.

The selected topic was to re-implement the Diffraction shader from the first volume of GPU Gems (@Fernando2004). The original implementation uses the Cg shading language and deprecated OpenGL functions, so the reimplementation uses OpenGL ES 2.0 and GLSL shading language.

This topic delves into vector operations for simulating light behaviour.

## Review of Diffraction Simulation

### What is Diffraction?

This describes what diffraction is, with figures. 

### Original Implementation

The original implementation is contained in the [nVIDIA Developers site](http://www.nvidia.com/object/gpu_gems_cd.html), being implemented with OpenGL 1.x functions, with shaders written with Cg.

The shader does the vector calculations on the vertex processor, by calculating the color, and the letting the fragment processor interpolate the colors between vertices. This makes the shader highly dependent on the geometry of the object. In [Diffraction2](this figure) we can see that the diffraction effect is not really appreciable with a 6-vertices cube. We also applied the shader on much more defined mesh object, and the differences can also be appreciated at here.

## Reimplementation of the Shader

The reimplementation of the shader was made with OpenGL ES 2.0, with shaders written with GLSL, using Andy Thomason's Octet.

The reimplementation had to consider the differences in the GPU profiles that Cg is capable, and the one that OpenGL ES 2.0 is limited to. This implies the conversion of matrices which the latter is not capable of, so the conversion was made at the moment of loading the shader.

The original shader was written in 2004, a moment where most heavy calculations were done at the vertex processor, so the fragment processor could only interpolate colors. We were quite sure that current GPUs are capable of handling the calculations in this shader at the fragment level. So we did a second shader that moved the color calculation to the fragment processor. Thus, in this second shader, we only do simple vector calculations at the vertex level to move the position, normal and tangent of each vertex, and do the heavy color calculations at the fragment level.

The differences are remarkable using low-poly objects (see figures X and Y). In higher-poly objects the differences are appreciable/not apprecible/nosabenoresponde.

## Illustrations

![Diffraction2](diffraction2.jpg "Diffraction screenshot example - Using vertex processor")

![Diffraction3](diffraction3.jpg "Diffraction screenshot example - Using fragment processor")

## References
