# Variance shadow mapping for omni lights with Vulkan

![vsm_vk](https://i.imgur.com/LDP3aoh.png)

Render routine:
- Render pass shadow map
  - one draw call, write to one color attachment with 6 layers
- Render pass Gaussian filter (x direction)
  - one draw call, write to six color attachments with single layer
- Render pass Gaussian filter (y direction)
  - same as above
- Render pass on screen

A blog post discussing the details:

> [Rendering dynamic cube maps for omni light shadows with Vulkan API](https://blue2rgb.wordpress.com/2017/12/26/rendering-dynamic-cube-maps-for-omni-light-shadows-with-vulkan-api/)

Controls:
- orbit: left/right/up/down arrow keys
- pan: A/D/R/F
- forward and backward: W/S
- freeze/unfreeze light: F1
- pause animation: P
