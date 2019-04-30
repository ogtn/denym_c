C:/VulkanSDK/1.1.106.0/Bin32/glslangValidator.exe -V -S vert first_example.vs
C:/VulkanSDK/1.1.106.0/Bin32/glslangValidator.exe -V -S frag first_example.fs
pause

rem @echo off
rem for /r %%i in (*.frag, *.vert) do %VULKAN_SDK%/Bin/glslangValidator.exe -V %%i
