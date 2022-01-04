# ThempX12
ThempX12 Rendering Engine

This is my first attempt at creating a decent rendering engine in with D3D12.
The goal is to have fully file driven rendering (Control by scripting language), therefore no hardcoded passes or resources (other than the fixed vertex format (for now)).
I'm wanting to add support for:
1. The Traditional pipeline (Vertex / Pixel / Geometry / Hull / Domain / Tesselation & Compute shaders)
2. The Mesh Shading pipeline (Mesh / Amplification shaders)
3. The Raytracing pipeline (RayGeneration / Intersection / AnyHit / ClosestHit & Miss shaders), Callable shaders are TBD

Scripting is done with AngelScript.

Currently the engine has the following dependencies:

AngelScript ( https://www.angelcode.com/angelscript/ ) -> Scripting,

Assimp (https://github.com/assimp/assimp ) -> Model conversion (from import type to custom output type),

DirectX Tex (https://github.com/microsoft/DirectXTex )-> DDS Texture generation,

DirectX Shader Compiler ( https://github.com/microsoft/DirectXShaderCompiler ) -> Runtime shader compilation,

FreeImage ( https://freeimage.sourceforge.io/ ) -> General purpose image reader.

And of course DirectX which is included in the Windows SDK
