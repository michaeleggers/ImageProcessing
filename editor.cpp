#include "editor.h"

#include <vector>

#include <glad/glad.h>

#include "imgui.h"

#include "batch.h"
#include "fbo.h"
#include "shader.h" 
#include "image.h"
#include "input.h"
#include "static_geometry.h"
#include "render_common.h"

static EditorState      editorState;
static EditorMouseState editorMouseState;
static EditorMouseInfo  editorMouseInfo;

static ImVec2 MousePosToImageCoords(ImVec2 mousePos, ImVec2 widgetMins, ImVec2 widgetSize, ImVec2 imageSize) {
    ImVec2 mousePosInButton = ImVec2(mousePos.x - widgetMins.x, mousePos.y - widgetMins.y);
    ImVec2 pictureCoords = ImVec2(
        imageSize.x * (mousePosInButton.x / widgetSize.x),
        imageSize.y * (mousePosInButton.y / widgetSize.y)
    );
    
    return pictureCoords;
}

void ShowWindow(const char* title, Framebuffer& fbo, 
    Shader& shader, Image& image, Batch& batch, std::vector<Line>& lines, EditorWindowType windowType)
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

    ImGui::SetCursorPos(ImGui::GetWindowPos());

    // Do the editor logic here. This stuff is pretty messy and should be
    // cleaned up as soon as the program is working!

    float imageWidth = (float)image.m_Width;
    float imageHeight = (float)image.m_Height;

    if (windowType == ED_WINDOW_TYPE_SOURCE) {
        if (editorState == ED_IDLE) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                ImVec2 mousePos = ImGui::GetMousePos();
                printf("mousePos: %f, %f\n", mousePos.x, mousePos.y);
                editorMouseInfo.pos1 = mousePos;
                ImVec2 pictureCoords = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                printf("mouse %f, %f:\n", pictureCoords.x, pictureCoords.y);
                editorState = ED_PLACE_SOURCE_LINE;
            }
        }
        else if (editorState == ED_PLACE_SOURCE_LINE) {
            ImVec2 mousePos = ImGui::GetMousePos();
            drawList->AddLine(editorMouseInfo.pos1, mousePos,
                ImGui::GetColorU32(ImVec4(255, 250, 0, 255)),
                5.0);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                ImVec2 pictureCoordsA = MousePosToImageCoords(editorMouseInfo.pos1, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 pictureCoordsB = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 mousePosInButtonA = ImVec2(editorMouseInfo.pos1.x - buttonMin.x, editorMouseInfo.pos1.y - buttonMin.y);
                ImVec2 mousePosInButtonB = ImVec2(mousePos.x - buttonMin.x, mousePos.y - buttonMin.y);

                lines.push_back({
                        {glm::vec3(pictureCoordsA.x, pictureCoordsA.y, 0.0f), glm::vec3(0), glm::vec2(0)},
                        {glm::vec3(pictureCoordsB.x, pictureCoordsB.y, 0.0f), glm::vec3(0), glm::vec2(0)},
                        mousePosInButtonA, mousePosInButtonB, buttonSize
                    }
                );
                printf("Lines: \n");
                for (auto& line : lines) {
                    printf("(%f, %f) -> (%f, %f)\n", line.a.pos.x, line.a.pos.y, line.b.pos.x, line.b.pos.y);
                }
                editorState = ED_PLACE_DEST_LINE_1;
            }
        }
    }
    else if (windowType == ED_WINDOW_TYPE_DEST) {
        if (editorState == ED_PLACE_DEST_LINE_1) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                ImVec2 mousePos = ImGui::GetMousePos();                
                editorMouseInfo.pos1 = mousePos;
                ImVec2 pictureCoords = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));                
                editorState = ED_PLACE_DEST_LINE_2;
            }
        }
        else if (editorState == ED_PLACE_DEST_LINE_2) {
            ImVec2 mousePos = ImGui::GetMousePos();
            drawList->AddLine(editorMouseInfo.pos1, mousePos,
                ImGui::GetColorU32(ImVec4(255, 250, 0, 255)),
                5.0);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {                
                ImVec2 pictureCoordsA = MousePosToImageCoords(editorMouseInfo.pos1, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 pictureCoordsB = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 mousePosInButtonA = ImVec2(editorMouseInfo.pos1.x - buttonMin.x, editorMouseInfo.pos1.y - buttonMin.y);
                ImVec2 mousePosInButtonB = ImVec2(mousePos.x - buttonMin.x, mousePos.y - buttonMin.y);

                lines.push_back({
                        {glm::vec3(pictureCoordsA.x, pictureCoordsA.y, 0.0f), glm::vec3(0), glm::vec2(0)},
                        {glm::vec3(pictureCoordsB.x, pictureCoordsB.y, 0.0f), glm::vec3(0), glm::vec2(0)},
                        mousePosInButtonA, mousePosInButtonB, buttonSize
                    }
                );
                printf("Lines: \n");
                for (auto& line : lines) {
                    printf("(%f, %f) -> (%f, %f)\n", line.a.pos.x, line.a.pos.y, line.b.pos.x, line.b.pos.y);
                }
                editorState = ED_IDLE;
            }
        }
    }

    

    pos = ImGui::GetCursorScreenPos();


    for (auto& line : lines) {     
        ImVec2 absCoordsA = line.absA;
        ImVec2 absCoordsB = line.absB;        
        absCoordsA.x *= buttonSize.x / line.buttonSize.x;
        absCoordsA.y *= buttonSize.y / line.buttonSize.y;
        absCoordsB.x *= buttonSize.x / line.buttonSize.x;
        absCoordsB.y *= buttonSize.y / line.buttonSize.y;
        absCoordsA.x += buttonMin.x;
        absCoordsA.y += buttonMin.y;
        absCoordsB.x += buttonMin.x;
        absCoordsB.y += buttonMin.y;
        drawList->AddLine(absCoordsA, absCoordsB,
            ImGui::GetColorU32(ImVec4(255, 255, 255, 255)),
            2.0);
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

void ShowResultWindow(const char* title, Framebuffer& fbo, Shader& shader, std::vector<Image>& images) {

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
    ImGui::InvisibleButton("##resultCanvas", buttonSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
        ImGuiButtonFlags_MouseButtonMiddle);

    ImVec2 buttonMin = ImGui::GetItemRectMin();
    ImVec2 buttonMax = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (Image& image : images) {
        drawList->AddImage(
            (void*)fbo.GetTexture().GetHandle(),
            buttonMin,
            buttonMax,
            ImVec2(0, 0),
            ImVec2(1, 1)
        );
    }

    ImGui::SetCursorPos(ImGui::GetWindowPos());

    ImGui::End();

    fbo.Bind();

    glViewport(0, 0, fbo.m_Width, fbo.m_Height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // 1.) bind a quad with the images dimension.
    Batch& unitQuadBatch = GetUnitQuadBatch();
    unitQuadBatch.Bind();
    shader.Activate();
    // 2.) bind texture
    for (Image& image : images) {
        image.GetTexture().Bind();        
        glDrawElements(GL_TRIANGLES, unitQuadBatch.IndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    fbo.Unbind();

}

