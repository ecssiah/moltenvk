#include "render/render.h"
#include <string.h>

void render_nuklear_init(Render *render)
{
    struct nk_context* ctx = &render->nuklear_context.context;
    struct nk_font_atlas* atlas = &render->nuklear_context.atlas;

    nk_font_atlas_init_default(atlas);
    nk_font_atlas_begin(atlas);

    render->nuklear_context.font = nk_font_atlas_add_default(atlas, 13.0f, NULL);

    int width;
    int height;
    const void* image = nk_font_atlas_bake(atlas, &width, &height, NK_FONT_ATLAS_RGBA32);

    /* upload image to Vulkan texture here */
    /* create image, image view, sampler, descriptor */

    nk_font_atlas_end(
        atlas,
        nk_handle_ptr(render),
        &render->nuklear_context.null_texture
    );

    nk_init_default(ctx, &render->nuklear_context.font->handle);

    nk_buffer_init_default(&render->nuklear_context.commands);
    nk_buffer_init_default(&render->nuklear_context.vertices);
    nk_buffer_init_default(&render->nuklear_context.indices);

    vkMapMemory(
        render->vulkan_device_context.device, 
        render->nuklear_context.vertex_buffer_memory, 
        0, 
        VK_WHOLE_SIZE, 
        0,
        &render->nuklear_context.vertex_buffer_mapped
    );

    vkMapMemory(
        render->vulkan_device_context.device, 
        render->nuklear_context.index_buffer_memory, 
        0, 
        VK_WHOLE_SIZE, 
        0,
        &render->nuklear_context.index_buffer_mapped
    );
}

void render_nuklear_convert(Render* render)
{
    struct nk_context* ctx = &render->nuklear_context.context;

    nk_buffer_clear(&render->nuklear_context.commands);
    nk_buffer_clear(&render->nuklear_context.vertices);
    nk_buffer_clear(&render->nuklear_context.indices);

    static const struct nk_draw_vertex_layout_element vertex_layout[] =
    {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(NkVertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(NkVertex, uv)},
        {NK_VERTEX_COLOR,    NK_FORMAT_R8G8B8A8, NK_OFFSETOF(NkVertex, color)},
        {NK_VERTEX_LAYOUT_END}
    };

    struct nk_convert_config config =
    {
        .vertex_layout = vertex_layout,
        .vertex_size = sizeof(NkVertex),
        .vertex_alignment = NK_ALIGNOF(NkVertex),
        .tex_null = render->nuklear_context.null_texture,
        .circle_segment_count = 22,
        .curve_segment_count = 22,
        .arc_segment_count = 22,
        .global_alpha = 1.0f,
        .shape_AA = NK_ANTI_ALIASING_ON,
        .line_AA = NK_ANTI_ALIASING_ON
    };

    nk_convert(
        ctx,
        &render->nuklear_context.commands,
        &render->nuklear_context.vertices,
        &render->nuklear_context.indices,
        &config
    );
}

void render_nuklear_upload(Render *render)
{
    /* Raw memory produced by nk_convert() */
    const void* vertex_data =
        nk_buffer_memory_const(&render->nuklear_context.vertices);

    const void* index_data =
        nk_buffer_memory_const(&render->nuklear_context.indices);

    /* Sizes of the buffers */
    nk_size vertex_size = render->nuklear_context.vertices.size;
    nk_size index_size  = render->nuklear_context.indices.size;

    /* Copy into persistently mapped Vulkan buffers */
    if (render->nuklear_context.vertex_buffer_mapped && vertex_size > 0)
    {
        memcpy(
            render->nuklear_context.vertex_buffer_mapped,
            vertex_data,
            vertex_size
        );
    }

    if (render->nuklear_context.index_buffer_mapped && index_size > 0)
    {
        memcpy(
            render->nuklear_context.index_buffer_mapped,
            index_data,
            index_size
        );
    }

    render->nuklear_context.vertex_count =
        (u32)(vertex_size / sizeof(NkVertex));

    render->nuklear_context.index_count =
        (u32)(index_size / sizeof(u16));
}

void render_nuklear_draw(Render* render)
{
    struct nk_context* ctx = &render->nuklear_context.context;

    if (
        nk_begin(
            ctx,
            "Demo",
            nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE
        )
    ) {
        nk_layout_row_dynamic(ctx, 30, 1);

        nk_label(ctx, "Nuklear + Vulkan", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 2);

        nk_button_label(ctx, "Button A");
        nk_button_label(ctx, "Button B");

        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "This UI is rendered via Nuklear.", NK_TEXT_LEFT);
    }

    nk_end(ctx);
}

void render_nuklear_record(Render* render, VkCommandBuffer cmd)
{
    struct nk_context* ctx = &render->nuklear_context.context;
    const struct nk_draw_command* draw_cmd;

    u32 index_offset = 0;

    /* Bind Nuklear pipeline */
    vkCmdBindPipeline(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        render->nuklear_pipeline_context.pipeline
    );

    /* Bind vertex buffer */
    VkBuffer vertex_buffers[] = { render->nuklear_context.vertex_buffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(
        cmd,
        0,
        1,
        vertex_buffers,
        offsets
    );

    /* Bind index buffer */
    vkCmdBindIndexBuffer(
        cmd,
        render->nuklear_context.index_buffer,
        0,
        VK_INDEX_TYPE_UINT16
    );

    /* Bind font atlas descriptor set */
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        render->nuklear_pipeline_context.layout,
        0,
        1,
        &render->nuklear_pipeline_context.descriptor_set,
        0,
        NULL
    );

    /* Push constants for coordinate transform */
    NuklearPushConstants push;

    push.scale[0] = 2.0f / render->vulkan_swapchain_context.extent.width;
    push.scale[1] = -2.0f / render->vulkan_swapchain_context.extent.height;

    push.translate[0] = -1.0f;
    push.translate[1] = 1.0f;

    vkCmdPushConstants(
        cmd,
        render->nuklear_pipeline_context.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(NuklearPushConstants),
        &push
    );

    /* Iterate Nuklear draw commands */
    nk_draw_foreach(draw_cmd, ctx, &render->nuklear_context.commands)
    {
        if (!draw_cmd->elem_count)
            continue;

        VkRect2D scissor;

        scissor.offset.x = (i32)draw_cmd->clip_rect.x;
        scissor.offset.y = (i32)draw_cmd->clip_rect.y;

        scissor.extent.width  = (u32)draw_cmd->clip_rect.w;
        scissor.extent.height = (u32)draw_cmd->clip_rect.h;

        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdDrawIndexed(
            cmd,
            draw_cmd->elem_count,
            1,
            index_offset,
            0,
            0
        );

        index_offset += draw_cmd->elem_count;
    }
}