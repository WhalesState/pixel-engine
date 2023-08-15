# basis_universal
Basis Universal Supercompressed GPU Texture Codec

----

Basis Universal is a ["supercompressed"](http://gamma.cs.unc.edu/GST/gst.pdf) GPU texture data interchange system that supports two highly compressed intermediate file formats (.basis or the [.KTX2 open standard from the Khronos Group](https://github.khronos.org/KTX-Specification/)) that can be quickly transcoded to a [very wide variety](https://github.com/BinomialLLC/basis_universal/wiki/OpenGL-texture-format-enums-table) of GPU compressed and uncompressed pixel formats: ASTC 4x4 L/LA/RGB/RGBA, PVRTC1 4bpp RGB/RGBA, PVRTC2 RGB/RGBA, BC7 mode 6 RGB, BC7 mode 5 RGB/RGBA, BC1-5 RGB/RGBA/X/XY, ETC1 RGB, ETC2 RGBA, ATC RGB/RGBA, ETC2 EAC R11 and RG11, FXT1 RGB, and uncompressed raster image formats 8888/565/4444. 

The system now supports two modes: a high quality mode which is internally based off the [UASTC compressed texture format](https://richg42.blogspot.com/2020/01/uastc-block-format-encoding.html), and the original lower quality mode which is based off a subset of ETC1 called "ETC1S". UASTC is for extremely high quality (similar to BC7 quality) textures, and ETC1S is for very small files. The ETC1S system includes built-in data compression, while the UASTC system includes an optional Rate Distortion Optimization (RDO) post-process stage that conditions the encoded UASTC texture data in the .basis file so it can be more effectively LZ compressed by the end user. More technical details about UASTC integration are [here](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-implementation-details).

Basis files support non-uniform texture arrays, so cubemaps, volume textures, texture arrays, mipmap levels, video sequences, or arbitrary texture "tiles" can be stored in a single file. The compressor is able to exploit color and pattern correlations across the entire file, so multiple images with mipmaps can be stored very efficiently in a single file.

The system's bitrate depends on the quality setting and image content, but common usable ETC1S bitrates are .3-1.25 bits/texel. ETC1S .basis files are typically 10-25% smaller than using RDO texture compression of the internal texture data stored in the .basis file followed by LZMA. For UASTC files, the bitrate is fixed at 8bpp, but with RDO post-processing and user-provided LZ compression on the .basis file the effective bitrate can be as low as 2bpp for video or for individual textures approximately 4-6bpp.

The .basis and .KTX2 transcoders have been fuzz tested using [zzuf](https://www.linux.com/news/fuzz-testing-zzuf).

So far, we've compiled the code using MSVC 2019, under Ubuntu 18.04 and 20 x64 using cmake with either clang 3.8 or gcc 5.4, and emscripten 1.35 to asm.js. (Be sure to use this version or later of emcc, as earlier versions fail with internal errors/exceptions during compilation.) 

Basis Universal supports "skip blocks" in ETC1S compressed texture arrays, which makes it useful for basic [compressed texture video](http://gamma.cs.unc.edu/MPTC/) applications. Note that Basis Universal is still at heart a GPU texture compression system, not a dedicated video codec, so bitrates will be larger than even MPEG1.
1/10/21 release notes:

For v1.13, we've added numerous ETC1S encoder optimizations designed to greatly speed up single threaded encoding time, as well as greatly reducing overall CPU utilization when multithreading is enabled. For benchmarking, we're using "-q 128 -no_multithreading -mip_fast". The encoder now uses approximately 1/3rd as much total CPU time for the same PSNR. The encoder can now optionally utilize SSE 4.1 - see the "-no_sse" command line option.
