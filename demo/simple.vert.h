#include <stdint.h>

#if 0
C:/msys64/home/sydlu/CProjects/clustered-forward-demo-vk/demo/simple.vert
// Module Version 10000
// Generated by (magic number): 80001
// Id's are bound by 43

                              Capability Shader
               1:             ExtInstImport  "GLSL.std.450"
                              MemoryModel Logical GLSL450
                              EntryPoint Vertex 4  "main" 10 33
                              Source GLSL 450
                              SourceExtension  "GL_ARB_separate_shader_objects"
                              Name 4  "main"
                              Name 8  "gl_PerVertex"
                              MemberName 8(gl_PerVertex) 0  "gl_Position"
                              Name 10  ""
                              Name 18  "UBO"
                              MemberName 18(UBO) 0  "view"
                              MemberName 18(UBO) 1  "normal"
                              MemberName 18(UBO) 2  "model"
                              MemberName 18(UBO) 3  "projection_clip"
                              MemberName 18(UBO) 4  "tile_size"
                              MemberName 18(UBO) 5  "grid_dim"
                              MemberName 18(UBO) 6  "cam_pos"
                              MemberName 18(UBO) 7  "cam_far"
                              MemberName 18(UBO) 8  "resolution"
                              MemberName 18(UBO) 9  "num_lights"
                              Name 20  "ubo_in"
                              Name 33  "pos_in"
                              MemberDecorate 8(gl_PerVertex) 0 BuiltIn Position
                              Decorate 8(gl_PerVertex) Block
                              MemberDecorate 18(UBO) 0 ColMajor
                              MemberDecorate 18(UBO) 0 Offset 0
                              MemberDecorate 18(UBO) 0 MatrixStride 16
                              MemberDecorate 18(UBO) 1 ColMajor
                              MemberDecorate 18(UBO) 1 Offset 64
                              MemberDecorate 18(UBO) 1 MatrixStride 16
                              MemberDecorate 18(UBO) 2 ColMajor
                              MemberDecorate 18(UBO) 2 Offset 128
                              MemberDecorate 18(UBO) 2 MatrixStride 16
                              MemberDecorate 18(UBO) 3 ColMajor
                              MemberDecorate 18(UBO) 3 Offset 192
                              MemberDecorate 18(UBO) 3 MatrixStride 16
                              MemberDecorate 18(UBO) 4 Offset 256
                              MemberDecorate 18(UBO) 5 Offset 264
                              MemberDecorate 18(UBO) 6 Offset 272
                              MemberDecorate 18(UBO) 7 Offset 284
                              MemberDecorate 18(UBO) 8 Offset 288
                              MemberDecorate 18(UBO) 9 Offset 296
                              Decorate 18(UBO) Block
                              Decorate 20(ubo_in) DescriptorSet 0
                              Decorate 20(ubo_in) Binding 0
                              Decorate 33(pos_in) Location 0
               2:             TypeVoid
               3:             TypeFunction 2
               6:             TypeFloat 32
               7:             TypeVector 6(float) 4
 8(gl_PerVertex):             TypeStruct 7(fvec4)
               9:             TypePointer Output 8(gl_PerVertex)
              10:      9(ptr) Variable Output
              11:             TypeInt 32 1
              12:     11(int) Constant 0
              13:             TypeMatrix 7(fvec4) 4
              14:             TypeVector 6(float) 2
              15:             TypeInt 32 0
              16:             TypeVector 15(int) 2
              17:             TypeVector 6(float) 3
         18(UBO):             TypeStruct 13 13 13 13 14(fvec2) 16(ivec2) 17(fvec3) 6(float) 14(fvec2) 15(int)
              19:             TypePointer Uniform 18(UBO)
      20(ubo_in):     19(ptr) Variable Uniform
              21:     11(int) Constant 3
              22:             TypePointer Uniform 13
              28:     11(int) Constant 2
              32:             TypePointer Input 17(fvec3)
      33(pos_in):     32(ptr) Variable Input
              35:    6(float) Constant 1065353216
              41:             TypePointer Output 7(fvec4)
         4(main):           2 Function None 3
               5:             Label
              23:     22(ptr) AccessChain 20(ubo_in) 21
              24:          13 Load 23
              25:     22(ptr) AccessChain 20(ubo_in) 12
              26:          13 Load 25
              27:          13 MatrixTimesMatrix 24 26
              29:     22(ptr) AccessChain 20(ubo_in) 28
              30:          13 Load 29
              31:          13 MatrixTimesMatrix 27 30
              34:   17(fvec3) Load 33(pos_in)
              36:    6(float) CompositeExtract 34 0
              37:    6(float) CompositeExtract 34 1
              38:    6(float) CompositeExtract 34 2
              39:    7(fvec4) CompositeConstruct 36 37 38 35
              40:    7(fvec4) MatrixTimesVector 31 39
              42:     41(ptr) AccessChain 10 12
                              Store 42 40
                              Return
                              FunctionEnd
#endif

static const uint32_t simple_vert[414] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000002b,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000a, 0x00000021, 0x00030003,
    0x00000002, 0x000001c2, 0x00090004, 0x415f4c47,
    0x735f4252, 0x72617065, 0x5f657461, 0x64616873,
    0x6f5f7265, 0x63656a62, 0x00007374, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00060005,
    0x00000008, 0x505f6c67, 0x65567265, 0x78657472,
    0x00000000, 0x00060006, 0x00000008, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005,
    0x0000000a, 0x00000000, 0x00030005, 0x00000012,
    0x004f4255, 0x00050006, 0x00000012, 0x00000000,
    0x77656976, 0x00000000, 0x00050006, 0x00000012,
    0x00000001, 0x6d726f6e, 0x00006c61, 0x00050006,
    0x00000012, 0x00000002, 0x65646f6d, 0x0000006c,
    0x00070006, 0x00000012, 0x00000003, 0x6a6f7270,
    0x69746365, 0x635f6e6f, 0x0070696c, 0x00060006,
    0x00000012, 0x00000004, 0x656c6974, 0x7a69735f,
    0x00000065, 0x00060006, 0x00000012, 0x00000005,
    0x64697267, 0x6d69645f, 0x00000000, 0x00050006,
    0x00000012, 0x00000006, 0x5f6d6163, 0x00736f70,
    0x00050006, 0x00000012, 0x00000007, 0x5f6d6163,
    0x00726166, 0x00060006, 0x00000012, 0x00000008,
    0x6f736572, 0x6974756c, 0x00006e6f, 0x00060006,
    0x00000012, 0x00000009, 0x5f6d756e, 0x6867696c,
    0x00007374, 0x00040005, 0x00000014, 0x5f6f6275,
    0x00006e69, 0x00040005, 0x00000021, 0x5f736f70,
    0x00006e69, 0x00050048, 0x00000008, 0x00000000,
    0x0000000b, 0x00000000, 0x00030047, 0x00000008,
    0x00000002, 0x00040048, 0x00000012, 0x00000000,
    0x00000005, 0x00050048, 0x00000012, 0x00000000,
    0x00000023, 0x00000000, 0x00050048, 0x00000012,
    0x00000000, 0x00000007, 0x00000010, 0x00040048,
    0x00000012, 0x00000001, 0x00000005, 0x00050048,
    0x00000012, 0x00000001, 0x00000023, 0x00000040,
    0x00050048, 0x00000012, 0x00000001, 0x00000007,
    0x00000010, 0x00040048, 0x00000012, 0x00000002,
    0x00000005, 0x00050048, 0x00000012, 0x00000002,
    0x00000023, 0x00000080, 0x00050048, 0x00000012,
    0x00000002, 0x00000007, 0x00000010, 0x00040048,
    0x00000012, 0x00000003, 0x00000005, 0x00050048,
    0x00000012, 0x00000003, 0x00000023, 0x000000c0,
    0x00050048, 0x00000012, 0x00000003, 0x00000007,
    0x00000010, 0x00050048, 0x00000012, 0x00000004,
    0x00000023, 0x00000100, 0x00050048, 0x00000012,
    0x00000005, 0x00000023, 0x00000108, 0x00050048,
    0x00000012, 0x00000006, 0x00000023, 0x00000110,
    0x00050048, 0x00000012, 0x00000007, 0x00000023,
    0x0000011c, 0x00050048, 0x00000012, 0x00000008,
    0x00000023, 0x00000120, 0x00050048, 0x00000012,
    0x00000009, 0x00000023, 0x00000128, 0x00030047,
    0x00000012, 0x00000002, 0x00040047, 0x00000014,
    0x00000022, 0x00000000, 0x00040047, 0x00000014,
    0x00000021, 0x00000000, 0x00040047, 0x00000021,
    0x0000001e, 0x00000000, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x0003001e, 0x00000008,
    0x00000007, 0x00040020, 0x00000009, 0x00000003,
    0x00000008, 0x0004003b, 0x00000009, 0x0000000a,
    0x00000003, 0x00040015, 0x0000000b, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000b, 0x0000000c,
    0x00000000, 0x00040018, 0x0000000d, 0x00000007,
    0x00000004, 0x00040017, 0x0000000e, 0x00000006,
    0x00000002, 0x00040015, 0x0000000f, 0x00000020,
    0x00000000, 0x00040017, 0x00000010, 0x0000000f,
    0x00000002, 0x00040017, 0x00000011, 0x00000006,
    0x00000003, 0x000c001e, 0x00000012, 0x0000000d,
    0x0000000d, 0x0000000d, 0x0000000d, 0x0000000e,
    0x00000010, 0x00000011, 0x00000006, 0x0000000e,
    0x0000000f, 0x00040020, 0x00000013, 0x00000002,
    0x00000012, 0x0004003b, 0x00000013, 0x00000014,
    0x00000002, 0x0004002b, 0x0000000b, 0x00000015,
    0x00000003, 0x00040020, 0x00000016, 0x00000002,
    0x0000000d, 0x0004002b, 0x0000000b, 0x0000001c,
    0x00000002, 0x00040020, 0x00000020, 0x00000001,
    0x00000011, 0x0004003b, 0x00000020, 0x00000021,
    0x00000001, 0x0004002b, 0x00000006, 0x00000023,
    0x3f800000, 0x00040020, 0x00000029, 0x00000003,
    0x00000007, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005,
    0x00050041, 0x00000016, 0x00000017, 0x00000014,
    0x00000015, 0x0004003d, 0x0000000d, 0x00000018,
    0x00000017, 0x00050041, 0x00000016, 0x00000019,
    0x00000014, 0x0000000c, 0x0004003d, 0x0000000d,
    0x0000001a, 0x00000019, 0x00050092, 0x0000000d,
    0x0000001b, 0x00000018, 0x0000001a, 0x00050041,
    0x00000016, 0x0000001d, 0x00000014, 0x0000001c,
    0x0004003d, 0x0000000d, 0x0000001e, 0x0000001d,
    0x00050092, 0x0000000d, 0x0000001f, 0x0000001b,
    0x0000001e, 0x0004003d, 0x00000011, 0x00000022,
    0x00000021, 0x00050051, 0x00000006, 0x00000024,
    0x00000022, 0x00000000, 0x00050051, 0x00000006,
    0x00000025, 0x00000022, 0x00000001, 0x00050051,
    0x00000006, 0x00000026, 0x00000022, 0x00000002,
    0x00070050, 0x00000007, 0x00000027, 0x00000024,
    0x00000025, 0x00000026, 0x00000023, 0x00050091,
    0x00000007, 0x00000028, 0x0000001f, 0x00000027,
    0x00050041, 0x00000029, 0x0000002a, 0x0000000a,
    0x0000000c, 0x0003003e, 0x0000002a, 0x00000028,
    0x000100fd, 0x00010038,
};
