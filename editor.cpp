#include "editor.h"

#include <glad/glad.h>

#include "imgui.h"

#include "batch.h"
#include "fbo.h"
#include "shader.h" 
#include "image.h"
#include "input.h"
#include "static_geometry.h"

void ShowWindow(const char* title, Framebuffer& fbo, Shader& shader, Image& image, Batch& batch)
{
    // Setup Window to put the framebuffer into

    ImGui::Begin(title);

    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    // Do not drag the window when left clicking and dragging

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
    }       
    else {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    }

    float imguiWindowWidth = ImGui::GetContentRegionAvail().x;
    float imguiWindowHeight = ImGui::GetContentRegionAvail().y;
    float imguiWindowPosX = ImGui::GetCursorScreenPos().x;
    float imguiWindowPosY = ImGui::GetCursorScreenPos().y;
    float mousePosImGuiWindowX = ImGui::GetMousePos().x - imguiWindowPosX;
    float mousePosImGuiWindowY = ImGui::GetMousePos().y - imguiWindowPosY;
    ImVec2 mousePosInWindow = ImVec2(mousePosImGuiWindowX, mousePosImGuiWindowY);

    // safe guard for potential div by 0

    if (imguiWindowWidth <= 0) {
        imguiWindowWidth = 1.0;
    }
    if (imguiWindowHeight <= 0) {
        imguiWindowHeight = 1.0;
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float aspect = 0.0f;
    float srcAspect = (float)fbo.m_Width / (float)fbo.m_Height;
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
    
    ImVec2 buttonSize(newWidth, newHeight); // Size of the invisible button
    ImVec2 buttonPosition(ImGui::GetCursorPosX() + posOffsetX, ImGui::GetCursorPosY() + posOffsetY); // Position of the button
    
    // Render the invisible button
    ImGui::SetCursorPos(buttonPosition);
    ImGui::InvisibleButton("##canvas", buttonSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
        ImGuiButtonFlags_MouseButtonMiddle);

    ImVec2 buttonMin = ImGui::GetItemRectMin();
    ImVec2 buttonMax = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddImage(
            (void*)fbo.GetTexture().GetHandle(),
        buttonMin,
        buttonMax,
        ImVec2(0, 0),
        ImVec2(1, 1)
    );

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        printf("ImGui suuuucks!\n");
    }
    
    
    //ImGui::PopStyleVar(2); // Pop both WindowPadding and ItemSpacing


    ImGui::End();


    // Draw into the framebuffer

    // TODO:
    fbo.Bind();

    glViewport(0, 0, fbo.m_Width, fbo.m_Height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // 1.) bind a quad with the images dimension.
    Batch& unitQuadBatch = GetUnitQuadBatch();    
    unitQuadBatch.Bind();
    // 2.) bind texture
    image.GetTexture().Bind();
    // 3.) render
    shader.Activate();
    glDrawElements(GL_TRIANGLES, unitQuadBatch.IndexCount(), GL_UNSIGNED_INT, nullptr);

    fbo.Unbind();

    
    // 4.) Bind batch and render (will be the lines later)

    //fbo.Bind();
    //glViewport(0, 0, fbo.m_Width, fbo.m_Height);
    ////glm::mat4 ortho = glm::ortho(0.0f, 500.0f, 0.0f, 500.0f, 0.1f, 100.0f);
    ////glm::mat4 ortho = glm::perspective(glm::radians(90.0f), imguiWindowWidth / imguiWindowHeight, 0.1f, 100.0f);
    //glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    //shader.Activate();
    //glBindTexture(GL_TEXTURE_2D, fbo.GetTexture().GetHandle());
    //batch.Bind();
    ////GLint orthoMatrixLocation = glGetUniformLocation(imageShader.Program(), "u_Ortho");
    ////glUniformMatrix4fv(orthoMatrixLocation, 1, GL_FALSE, glm::value_ptr(ortho));
    //glDrawElements(GL_TRIANGLES, batch.IndexCount(), GL_UNSIGNED_INT, nullptr);
    //fbo.Unbind();
}

