"%VULKAN_SDK%\Bin\glslc.exe" mainShader.vert -o .\shaders\vert.spv
"%VULKAN_SDK%\Bin\glslc.exe" mainShader.frag -o .\shaders\frag.spv
"%VULKAN_SDK%\Bin\glslc.exe" offscreenShader.vert -o .\shaders\offscreenVert.spv
"%VULKAN_SDK%\Bin\glslc.exe" offscreenShader.frag -o .\shaders\offscreenFrag.spv
pause