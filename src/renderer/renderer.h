#include <GLFW/glfw3.h>

#include "platform/platform.h"
#include "backend/vulkan/vulkan_backend.h"

typedef struct Mesh Mesh;
typedef struct Texture Texture;
typedef struct Material Material;

typedef struct 
{
    VulkanBackend* vulkan_backend;
}
Renderer;

Renderer* renderer_create(PlatformWindow* platform_window);
void renderer_init(Renderer* Renderer, PlatformWindow* window);
void renderer_draw(Renderer* Renderer);
void renderer_destroy(Renderer* Renderer);