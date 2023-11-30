#include "editor.h"

#include <glad/glad.h>

#include "imgui.h"

#include "batch.h"
#include "fbo.h"
#include "shader.h" 
#include "image.h"

void ShowWindow(const char* title, Framebuffer& fbo, Shader& shader, Image& image, Batch& batch)
{
    // Setup Window to put the framebuffer into

    ImGui::Begin(title);

    float imguiWindowWidth = ImGui::GetContentRegionAvail().x;
    float imguiWindowHeight = ImGui::GetContentRegionAvail().y;
    float imguiWindowPosX = ImGui::GetCursorScreenPos().x;
    float imguiWindowPosY = ImGui::GetCursorScreenPos().y;
    float mousePosImGuiWindowX = ImGui::GetMousePos().x - imguiWindowPosX;
    float mousePosImGuiWindowY = ImGui::GetMousePos().y - imguiWindowPosY;

    //printf("Mouse pos imgui window: %f, %f\n", mousePosImGuiWindowX, mousePosImGuiWindowY);
    //fbo.Resize(imguiWindowWidth, imguiWindowWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float aspect = 0.0f;
    float srcAspect = 640.0f / 480.0f;
    float dstAspect = imguiWindowWidth / imguiWindowHeight;
    float newWidth = 0.0f;
    float newHeight = 0.0f;
    if (srcAspect > dstAspect) { // horizontal letterbox
        newWidth = imguiWindowWidth;
        newHeight = imguiWindowWidth / srcAspect;
    }
    else { // vertical letterbox
        newWidth = imguiWindowHeight * srcAspect;
        newHeight = imguiWindowHeight;
    }
    float posOffsetX = (imguiWindowWidth - newWidth) / 2.0f;
    float posOffsetY = (imguiWindowHeight - newHeight) / 2.0f;
    ImGui::GetWindowDrawList()->AddImage(
        (void*)fbo.Texture(),
        ImVec2(pos.x + posOffsetX, pos.y + posOffsetY),
        ImVec2(pos.x + newWidth + posOffsetX, pos.y + newHeight + posOffsetY),
        ImVec2(0, 0),
        ImVec2(1, 1)
    );

    ImGui::End();


    // Draw into the framebuffer

    // TODO:
    // 1.) bind a quad with the images dimension.
    // 2.) bind texture
    // 3.) render
    // 4.) Bind batch and render (will be the lines later)

    fbo.Bind();
    glViewport(0, 0, 640, 480);
    //glm::mat4 ortho = glm::ortho(0.0f, 500.0f, 0.0f, 500.0f, 0.1f, 100.0f);
    //glm::mat4 ortho = glm::perspective(glm::radians(90.0f), imguiWindowWidth / imguiWindowHeight, 0.1f, 100.0f);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader.Activate();
    glBindTexture(GL_TEXTURE_2D, fbo.Texture());
    batch.Bind();
    //GLint orthoMatrixLocation = glGetUniformLocation(imageShader.Program(), "u_Ortho");
    //glUniformMatrix4fv(orthoMatrixLocation, 1, GL_FALSE, glm::value_ptr(ortho));
    glDrawElements(GL_TRIANGLES, batch.IndexCount(), GL_UNSIGNED_INT, nullptr);
    fbo.Unbind();
}

