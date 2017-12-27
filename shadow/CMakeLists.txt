set(TARGET_NAME shadow)
set(SPIRV_DIR ${CMAKE_CURRENT_SOURCE_DIR})

glsl_to_spirv(shadow_map.vert ${SPIRV_DIR})
glsl_to_spirv(shadow_map.geom ${SPIRV_DIR})
glsl_to_spirv(shadow_map.frag ${SPIRV_DIR})
glsl_to_spirv(filter.vert ${SPIRV_DIR})
glsl_to_spirv(filter.frag ${SPIRV_DIR})

glsl_to_spirv(onscreen.vert ${SPIRV_DIR})
glsl_to_spirv(onscreen.frag ${SPIRV_DIR})

add_executable(${TARGET_NAME}
    main.cpp
    Prog_info.hpp
    Shell.hpp
    Cubemap_render_target.hpp
    Program.hpp
    shadow_map.vert.h
    shadow_map.geom.h
    shadow_map.frag.h
    filter.vert.h
    filter.frag.h
    onscreen.vert.h
    onscreen.frag.h
    )
target_link_libraries(${TARGET_NAME}
    ${Vulkan_LIBRARY}
    assimp
    )
