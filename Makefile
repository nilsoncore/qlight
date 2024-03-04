qlight.out: qlight.o render_vk.o allocator.o math.o
	clang++ qlight.o render_vk.o allocator.o math.o -L $(VULKAN_SDK)/lib -L libs/GLFW -L libs/GLEW -Wl,-rpath,$(VULKAN_SDK)/lib libvulkan.so libglfw3.a

qlight.o: src/qlight.cpp src/qlight.h
	clang++ -c src/qlight.cpp -I libs -I $(VULKAN_SDK)/include -L $(VULKAN_SDK)/lib -L libs/GLFW -L libs/GLEW -Wl,-rpath,$(VULKAN_SDK)/lib libvulkan.so libglfw3.a

render_vk.o: src/render_vk.cpp src/render_vk.h
	clang++ -c src/render_vk.cpp -I libs -I $(VULKAN_SDK)/include -L $(VULKAN_SDK)/lib -L libs/GLFW -L libs/GLEW -Wl,-rpath,$(VULKAN_SDK)/lib libvulkan.so libglfw3.a

allocator.o: src/allocator.cpp src/allocator.h
	clang++ -c src/allocator.cpp -I libs -I $(VULKAN_SDK)/include -L $(VULKAN_SDK)/lib -L libs/GLFW -L libs/GLEW -Wl,-rpath,$(VULKAN_SDK)/lib libvulkan.so libglfw3.a

math.o: src/math.cpp src/math.h
	clang++ -c src/math.cpp -I libs -I $(VULKAN_SDK)/include-L $(VULKAN_SDK)/lib -L libs/GLFW -L libs/GLEW -Wl,-rpath,$(VULKAN_SDK)/lib libvulkan.so libglfw3.a