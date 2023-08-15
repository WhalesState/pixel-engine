# Glslang Components and Status

There are several components:

### Reference Validator and GLSL/ESSL -> AST Front End

An OpenGL GLSL and OpenGL|ES GLSL (ESSL) front-end for reference validation and translation of GLSL/ESSL into an internal abstract syntax tree (AST).

**Status**: Virtually complete, with results carrying similar weight as the specifications.

### HLSL -> AST Front End

An HLSL front-end for translation of an approximation of HLSL to glslang's AST form.

**Status**: Partially complete. Semantics are not reference quality and input is not validated.
This is in contrast to the [DXC project](https://github.com/Microsoft/DirectXShaderCompiler), which receives a much larger investment and attempts to have definitive/reference-level semantics.

See [issue 362](https://github.com/KhronosGroup/glslang/issues/362) and [issue 701](https://github.com/KhronosGroup/glslang/issues/701) for current status.

### AST -> SPIR-V Back End

Translates glslang's AST to the Khronos-specified SPIR-V intermediate language.

**Status**: Virtually complete.

### Reflector

An API for getting reflection information from the AST, reflection types/variables/etc. from the HLL source (not the SPIR-V).

**Status**: There is a large amount of functionality present, but no specification/goal to measure completeness against.  It is accurate for the input HLL and AST, but only approximate for what would later be emitted for SPIR-V.

### Standalone Wrapper

`glslang` is command-line tool for accessing the functionality above.