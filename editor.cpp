#include "editor.h"

#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "tinyfiledialogs.h"

#include "stb_image_write.h"
#include "gif.h"

#include "batch.h"
#include "fbo.h"
#include "shader.h" 
#include "image.h"
#include "input.h"
#include "static_geometry.h"
#include "static_image_data.h"
#include "render_common.h"
#include "common.h"
#include "beierneely.h"
#include "parser.h"
#include "event_handler.h"
#include "utils.h"


// TODO: Unify Result window and morph windows

std::string GetProjectNameFromFilePath(std::string pathAndFilename) {
    int length = (int)pathAndFilename.size();
    
    if (length < 1) return "";

    int pos = length - 1;
    while (pos >= 0 && pathAndFilename[pos] != '\\' && pathAndFilename[pos] != '/') {
        pos--;
    }

    std::string result = pathAndFilename.substr(pos+1, length - pos);

    return result;
}

std::string LineToString(Line& line) {
    std::string result;
    // Image coordinates that the beier-neely algorithm uses
    glm::vec3 aPos = line.a.pos;
    glm::vec3 bPos = line.b.pos;
    // Info for the editor to display lines correctly
    ImVec2 absA = line.absA;
    ImVec2 absB = line.absB;
    ImVec2 editorScale = line.buttonSize; // TODO: Rename buttonSize -> widgetSize or something like that

    result += std::to_string(aPos.x); result += " ";
    result += std::to_string(aPos.y); result += " ";
    result += std::to_string(bPos.x); result += " ";
    result += std::to_string(bPos.y); result += " ";
    result += std::to_string(absA.x); result += " ";
    result += std::to_string(absA.y); result += " ";
    result += std::to_string(absB.x); result += " ";
    result += std::to_string(absB.y); result += " ";
    result += std::to_string(editorScale.x); result += " ";
    result += std::to_string(editorScale.y);

    return result;
}

void RenderToDiskAsTGA(std::string pathAndFilename, std::vector<Image>& images) {
    for (size_t i = 0; i < images.size(); i++) {
        std::string fileName = pathAndFilename + std::to_string(i) + ".tga";
        Image& image = images[i];
        int ok = stbi_write_tga(fileName.c_str(), image.m_Width, image.m_Height, image.m_Channels, image.m_Data);
        if (!ok) {
            SDL_Log("Error writing image # %d for %s\n", (int)i, pathAndFilename.c_str());
        }
    }
    SDL_Log("Written images to: %s\n", pathAndFilename.c_str());
}

void RenderToDiskAsGIF(std::string pathAndFilename, std::vector<Image>& images) {
    std::string fileName = pathAndFilename + ".gif";
    GifWriter gf;
    int width = (int)images[0].m_Width;
    int height = (int)images[0].m_Height;
    int delay = 1;
    GifBegin(&gf, fileName.c_str(), width, height, delay, 8, true);
    for (size_t i = 0; i < images.size(); i++) {
        Image& image = images[i];
        Image imageRGBA = Image::ToRGBA(image);
        GifWriteFrame(&gf, imageRGBA.m_Data, width, height, delay);
    }
    GifEnd(&gf);

    SDL_Log("Written GIF to: %s\n", pathAndFilename.c_str());
}

void WriteProjectFile(std::string pathAndFileName, std::vector<Line>& sourceLines, std::vector<Line>& destLines, std::string sourceImagePath, std::string destImagePath, float a, float b, float p) {
    // Create string to write
    const char* dataToSave = "This is the data to be saved in the file.";
    std::string result;
    result += "src_img_path " + sourceImagePath + "\n";
    result += "dst_img_path " + destImagePath+ "\n";

    result += "weight " + std::to_string(a) + " " + std::to_string(b) + " " + std::to_string(p) + "\n";

    size_t numLinesPairs = sourceLines.size();
    // TODO: Give warning if sourceLines.size() != destLines.size()
    result += "src\n";
    for (size_t i = 0; i < numLinesPairs; i++) {
        std::string lineString = std::to_string(i) + " " + LineToString(sourceLines[i]);
        result += lineString + "\n";
    }
    result += "dst\n";
    for (size_t i = 0; i < numLinesPairs; i++) {
        std::string lineString = std::to_string(i) + " " + LineToString(destLines[i]);
        result += lineString + "\n";
    }

    FILE* file = fopen(pathAndFileName.c_str(), "w"); // Open the file in write mode
    if (file != NULL) {
        fwrite(result.data(), sizeof(char), result.size(), file);
        fclose(file);
        SDL_Log("Data saved to %s\n", pathAndFileName.c_str());
    }
    else {
        SDL_Log("Error opening file for writing: %s\n", pathAndFileName.c_str());
    }
}

void Editor::InitFromProjectFile(std::string pathAndFilename) {    
    ATP_File projectFile;
    if (atp_read_file(pathAndFilename.c_str(), &projectFile) != ATP_SUCCESS) {
        SDL_Log("Failed to read project file: %s.\n", pathAndFilename.c_str());
        return;
    }
    SDL_Log("Reading project file: %s\n", pathAndFilename.c_str());
    MorphProjectData projectData = ParseProjectFile(projectFile);
    
    ResetState();
    m_sourceImage = Image(projectData.sourceImagePath);
    m_destImage = Image(projectData.destImagePath);
    m_sourceImageTexture = Texture(m_sourceImage.m_Data, m_sourceImage.m_Width, m_sourceImage.m_Height);
    m_destImageTexture = Texture(m_destImage.m_Data, m_destImage.m_Width, m_destImage.m_Height);
    m_sourceFBO->Resize(m_sourceImage.m_Width, m_sourceImage.m_Height);
    m_destFBO->Resize(m_destImage.m_Width, m_destImage.m_Height);
    m_A = projectData.weightParams.x;
    m_B = projectData.weightParams.y;
    m_P = projectData.weightParams.z;
    m_sourceLines = projectData.sourceLines;
    m_destLines = projectData.destLines;
}

void Editor::NewProject()
{
    ResetState();
    m_sourceLines.clear();
    m_destLines.clear();
}

void Editor::SaveProject()
{
    const char* fileFilterList[] = { "*.mph" };
    char const* retSaveFile = tinyfd_saveFileDialog(
        "Save Project",
        m_OpenedProject.c_str(),
        1,
        fileFilterList,
        "Morph MPH files");
    if (retSaveFile == NULL) {
        SDL_Log("Save Project cancelled\n");
    }
    else {
        WriteProjectFile(retSaveFile, m_sourceLines, m_destLines, m_sourceImage.m_FilePath, m_destImage.m_FilePath, m_A, m_B, m_P);
        m_OpenedProject = GetProjectNameFromFilePath(retSaveFile);
        glfwSetWindowTitle(m_Window, ("PowerMorph - " + m_OpenedProject).c_str());
    }
}

void Editor::OpenProject()
{
    const char* fileFilterList[] = { "*.mph" };
    char const* retOpenFile = tinyfd_openFileDialog(
        "Open Project",
        "",
        1,
        fileFilterList,
        "Morph MPH files",
        0
    );
    if (retOpenFile == NULL) {
        SDL_Log("Open Project cancelled\n");
    }
    else {
        InitFromProjectFile(retOpenFile);
        m_OpenedProject = GetProjectNameFromFilePath(retOpenFile);
        glfwSetWindowTitle(m_Window, ("PowerMorph - " + m_OpenedProject).c_str());
    }
}

bool Editor::OpenImage(std::string& pathAndFilename)
{
    const char* fileFilterList[] = { "*.png", "*.jpg", "*.bmp" };
    char const* retOpenFile = tinyfd_openFileDialog(
        "Open Image",
        "",
        3,
        fileFilterList,
        "image files",
        0
    );
    if (retOpenFile == NULL) {
        SDL_Log("Open Image cancelled\n");
    }
    else {
        pathAndFilename = std::string(retOpenFile);
        return true;
    }

    return false;
}

void Editor::RenderTGA(std::vector<Image>& images)
{
    if (ImGui::Button("Render (TGA)")) {
        char const* retSaveFile = tinyfd_saveFileDialog(
            "Render",
            "",
            0,
            nullptr,
            "TGA Image sequence");
        if (retSaveFile == NULL) {
            SDL_Log("Rendering cancelled\n");
        }
        else {
            SDL_Log("Rendering...\n");
            RenderToDiskAsTGA(retSaveFile, m_blendedImages);
        }
    }
}

void Editor::RenderGIF(std::vector<Image>& images)
{
    if (ImGui::Button("Render (GIF)")) {
        char const* retSaveFile = tinyfd_saveFileDialog(
            "Render",
            "",
            0,
            nullptr,
            "GIF");
        if (retSaveFile == NULL) {
            SDL_Log("Rendering cancelled\n");
        }
        else {
            SDL_Log("Rendering...\n");
            RenderToDiskAsGIF(retSaveFile, m_blendedImages);
        }
    }
}

void Editor::RenderTGA(std::vector<Texture>& textures)
{
    if (ImGui::Button("Render (TGA)")) {
        char const* retSaveFile = tinyfd_saveFileDialog(
            "Render",
            "",
            0,
            nullptr,
            "TGA Image sequence");
        if (retSaveFile == NULL) {
            SDL_Log("Rendering cancelled\n");
        }
        else {
            SDL_Log("Rendering...\n");
            std::vector<Image> images = CreateImagesFromTexture(textures);
            RenderToDiskAsTGA(retSaveFile, images);
        }
    }
}

void Editor::RenderGIF(std::vector<Texture>& textures)
{
    if (ImGui::Button("Render (GIF)")) {
        char const* retSaveFile = tinyfd_saveFileDialog(
            "Render",
            "",
            0,
            nullptr,
            "GIF");
        if (retSaveFile == NULL) {
            SDL_Log("Rendering cancelled\n");
        }
        else {
            SDL_Log("Rendering...\n");
            std::vector<Image> images = CreateImagesFromTexture(textures);
            RenderToDiskAsGIF(retSaveFile, images);
        }
    }
}

// TODO: Maybe use command pattern that allows us to store commands
// that we can replay as we want.
void Editor::Undo()
{
    if (m_editorState == ED_IDLE) {        
        if (m_sourceLines.size() > m_destLines.size()) {
            if (!m_sourceLines.empty()) {
                m_sourceLines.pop_back();
            }
        }
        else if (m_sourceLines.size() == m_destLines.size()) {
            if (!m_destLines.empty()) {
                m_destLines.pop_back();
                m_editorState = ED_PLACE_DEST_LINE_1;
            }
        }
    }
    
    else if (m_editorState == ED_PLACE_SOURCE_LINE) {
        m_editorState = ED_IDLE;
    }
    else if (m_editorState == ED_PLACE_DEST_LINE_1) {
        m_sourceLines.pop_back();
        m_editorState = ED_IDLE;
    }
    else if (m_editorState == ED_PLACE_DEST_LINE_2) {
        m_editorState = ED_PLACE_DEST_LINE_1;
    }    
}

void Editor::ResetState()
{
    m_editorState = ED_IDLE;
    m_editorMouseState = ED_MOUSE_IDLE;
    m_editorMouseInfo = { ImVec2(0, 0), ImVec2(0, 0) };
    m_Dirty = false;
    m_OpenedProject = "Untitled Project";
}

bool IsPointInsideRect(ImVec2 p, ImVec2 rectPos, ImVec2 rectSize) {
    if (p.x > rectPos.x && p.x < rectPos.x + rectSize.x
        && p.y > rectPos.y && p.y < rectPos.y + rectSize.y) {
        return true;
    }

    return false;
}

void Editor::Update(IEvent* event)
{
    switch (event->m_Type) {
    case EVENT_TYPE_DROP: {
        DropEvent* de = (DropEvent*)event;
        printf("Editor: received file drop event: %s\n", de->m_pathAndFilename.c_str());
        m_newImagePathAndFilename = de->m_pathAndFilename;
        m_Dirty = true;    
    } break;
    case EVENT_TYPE_RENDER_UPDATE: {
        RenderUpdateEvent* rue = (RenderUpdateEvent*)event;
        m_RenderPctDone = rue->m_pctDone;
    } break;
    case EVENT_TYPE_RENDER_DONE: {
        SDL_Log("Editor: Received render done event\n");
        m_sourceToDestMorphs.clear();
        m_destToSourceMorphs.clear();
        m_blendedImages.clear();
        m_blendedImageTextures.clear();
        m_SourceToDestMorphTextures.clear();
        m_DestToSourceMorphTextures.clear();
        RenderDoneEvent* rde = (RenderDoneEvent*)event;
        m_sourceToDestMorphs = rde->m_sourceToDestMorphs;
        m_destToSourceMorphs = rde->m_destToSourceMorphs;        
        std::reverse(m_destToSourceMorphs.begin(), m_destToSourceMorphs.end()); // TODO: Causes mem-leak because Image move ctor is broken or something
        m_blendedImages = BlendImages(m_sourceToDestMorphs, m_destToSourceMorphs);        
        if (m_ImageIndex >= m_blendedImages.size()) {
            m_ImageIndex = 0;
        }        
        for (int i = 0; i < m_blendedImages.size(); i++) {
            m_blendedImageTextures.push_back(Texture(m_blendedImages[i].m_Data, m_blendedImages[i].m_Width, m_blendedImages[i].m_Height));
            m_SourceToDestMorphTextures.push_back(Texture(m_sourceToDestMorphs[i].m_Data, m_sourceToDestMorphs[i].m_Width, m_sourceToDestMorphs[i].m_Height));
            m_DestToSourceMorphTextures.push_back(Texture(m_destToSourceMorphs[i].m_Data, m_destToSourceMorphs[i].m_Width, m_destToSourceMorphs[i].m_Height));
        }
        m_isRendering = false;
        m_RenderPctDone = 0.0f;
    } break;
    default: {};
    }    
}

std::string Editor::GetProjectName()
{
    return m_OpenedProject;
}

static ImVec2 MousePosToImageCoords(ImVec2 mousePos, ImVec2 widgetMins, ImVec2 widgetSize, ImVec2 imageSize) {
    ImVec2 mousePosInButton = ImVec2(mousePos.x - widgetMins.x, mousePos.y - widgetMins.y);
    ImVec2 pictureCoords = ImVec2(
        imageSize.x * (mousePosInButton.x / widgetSize.x),
        imageSize.y * (mousePosInButton.y / widgetSize.y)
    );
    
    return pictureCoords;
}


// NOTE: Keep this as a reference if we want to render own geometry on top of the imgui image FBO
// 
//void ShowWindow(const char* title, Framebuffer& fbo, 
//    Shader& shader, Image& image, Batch& batch, std::vector<Line>& lines, EditorWindowType windowType)
//{
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
//}

Editor::Editor(Image sourceImage, Image destImage, EventHandler* eventHandler, GLFWwindow* window)
{
    // TODO: Should we allow source and destination images to be of different size?
    //       If yes, what is the size of the result image?
    assert(sourceImage.m_Width == destImage.m_Width);
    assert(sourceImage.m_Height == destImage.m_Height);

    ResetState();

    m_sourceImage = sourceImage;
    m_destImage = destImage;

    m_Window = window;

    m_sourceImageTexture = Texture(m_sourceImage.m_Data, m_sourceImage.m_Width, m_sourceImage.m_Height);
    m_destImageTexture = Texture(m_destImage.m_Data, m_destImage.m_Width, m_destImage.m_Height);

    m_A = 0.001f;
    m_B = 2.5f;
    m_P = 0.0f;

    m_NumIterations = 2;
    m_MaxIterations = 100;
    m_ImageIndex = 0;

    // Create Framebuffers for windows
    m_sourceFBO = new Framebuffer(m_sourceImage.m_Width, m_sourceImage.m_Height);
    m_destFBO = new Framebuffer(m_destImage.m_Width, m_destImage.m_Height);
    m_resultFBO = new Framebuffer(m_sourceImage.m_Width, m_sourceImage.m_Height);

    // Shader
    std::string exePath = com_GetExePath();
#ifdef WIN32
    if (!m_imageShader.Load(exePath + "shaders/basic.vert", exePath + "shaders/basic.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }   
#elif __APPLE__
    if (!m_imageShader.Load(exePath + "/../shaders/basic.vert", exePath + "/../shaders/basic.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }
#endif

    m_EventHandler = eventHandler;
    m_EventHandler->Attach(this);    

    m_OpenedProject = "Untitled Project";

    m_RenderPctDone = 0.0f;
    m_sourceRenderDone = false;
    m_destRenderDone = false;
    m_isRendering = false;
}

Editor::~Editor()
{
    delete m_sourceFBO;
    delete m_destFBO;
    delete m_resultFBO;
}

// TODO: Dest and Source windows should probably be in their own class! It is *very* annoying to check for
// windowType each time we want to know if stuff is being updated (such as images loaded via drag and drop)
// on either the source or the destination window.
void Editor::ShowWindow(const char* title, Image& image, Framebuffer* fbo, std::vector<Line>& lines, EditorWindowType windowType)
{
    // Setup Window to put the framebuffer into

    ImGui::Begin(title);    

    if (ImGui::Button("Select picture...")) {
        std::string pathAndFilename;
        if (windowType == ED_WINDOW_TYPE_SOURCE) {
            if (OpenImage(pathAndFilename)) {
                m_sourceImage = Image(pathAndFilename);
            }
        }
        else if (windowType == ED_WINDOW_TYPE_DEST) {
            if (OpenImage(pathAndFilename)) {
                m_destImage = Image(pathAndFilename);
            }
        }
    }

    // TODO: This is wastefule. Build the string only on change.
    ImGui::Text((image.m_FilePath + " (" + std::to_string(image.m_Width) + ", " + std::to_string(image.m_Height) + ")").c_str());

    // TODO: Kinda ugly to have this here...

    if (m_Dirty) {            
        if (ImGui::IsWindowHovered()) {
            if (windowType == ED_WINDOW_TYPE_SOURCE) {
                Image newImage(m_newImagePathAndFilename);
                if (newImage.m_Data) {
                    m_sourceImage = newImage;
                    m_sourceImageTexture = Texture(m_sourceImage.m_Data, m_sourceImage.m_Width, m_sourceImage.m_Height);
                    m_sourceFBO->Resize(newImage.m_Width, newImage.m_Height);
                }
            }
            else if (windowType == ED_WINDOW_TYPE_DEST) {
                Image newImage(m_newImagePathAndFilename);
                if (newImage.m_Data) {
                    m_destImage = newImage;
                    m_destImageTexture = Texture(m_destImage.m_Data, m_destImage.m_Width, m_destImage.m_Height);
                    m_destFBO->Resize(newImage.m_Width, newImage.m_Height);                    
                }                
            }            
        }        
    }

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
    float srcAspect = (float)fbo->m_Width / (float)fbo->m_Height;
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
        (void*)fbo->GetTexture().GetHandle(),
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

    ImVec2 mousePos = ImGui::GetMousePos();

    ImVec2 pictureCoords = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
    if (ImGui::IsItemHovered()) {        
        std::string pictureCoordsText;
        pictureCoordsText += std::to_string((int)pictureCoords.x) + " " + std::to_string((int)pictureCoords.y);
        ImVec2 labelPos = mousePos;
        labelPos.y -= 20.0;
        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 1.2,
            labelPos,
            ImGui::GetColorU32(ImVec4(255, 230, 0, 100)),
            &pictureCoordsText[0], &pictureCoordsText[0] + pictureCoordsText.size());
    }

    if (windowType == ED_WINDOW_TYPE_SOURCE) {
        if (m_editorState == ED_IDLE) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                printf("mousePos: %f, %f\n", mousePos.x, mousePos.y);
                m_editorMouseInfo.pos1 = mousePos;
                ImVec2 pictureCoords = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                printf("mouse %f, %f:\n", pictureCoords.x, pictureCoords.y);
                m_editorState = ED_PLACE_SOURCE_LINE;
            }
        }
        else if (m_editorState == ED_PLACE_SOURCE_LINE) {            
            drawList->AddLine(m_editorMouseInfo.pos1, mousePos,
                ImGui::GetColorU32(ImVec4(255, 250, 0, 255)),
                5.0);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                ImVec2 pictureCoordsA = MousePosToImageCoords(m_editorMouseInfo.pos1, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 pictureCoordsB = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 mousePosInButtonA = ImVec2(m_editorMouseInfo.pos1.x - buttonMin.x, m_editorMouseInfo.pos1.y - buttonMin.y);
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
                m_editorState = ED_PLACE_DEST_LINE_1;
            }
        }
    }
    else if (windowType == ED_WINDOW_TYPE_DEST) {
        if (m_editorState == ED_PLACE_DEST_LINE_1) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {                
                m_editorMouseInfo.pos1 = mousePos;
                ImVec2 pictureCoords = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                m_editorState = ED_PLACE_DEST_LINE_2;
            }
        }
        else if (m_editorState == ED_PLACE_DEST_LINE_2) {
            drawList->AddLine(m_editorMouseInfo.pos1, mousePos,
                ImGui::GetColorU32(ImVec4(255, 250, 0, 255)),
                5.0);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                ImVec2 pictureCoordsA = MousePosToImageCoords(m_editorMouseInfo.pos1, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 pictureCoordsB = MousePosToImageCoords(mousePos, buttonMin, buttonSize, ImVec2(imageWidth, imageHeight));
                ImVec2 mousePosInButtonA = ImVec2(m_editorMouseInfo.pos1.x - buttonMin.x, m_editorMouseInfo.pos1.y - buttonMin.y);
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
                m_editorState = ED_IDLE;
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
    fbo->Bind();

    glViewport(0, 0, fbo->m_Width, fbo->m_Height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 1.) bind a quad with the images dimension.
    Batch& unitQuadBatch = GetUnitQuadBatch();
    unitQuadBatch.Bind();
    
    // 2.) bind texture
    if (windowType == ED_WINDOW_TYPE_SOURCE) {
        m_sourceImageTexture.Bind();
    }
    else if (windowType == ED_WINDOW_TYPE_DEST) {
        m_destImageTexture.Bind();
    }

    // 3.) render
    m_imageShader.Activate();
    glDrawElements(GL_TRIANGLES, unitQuadBatch.IndexCount(), GL_UNSIGNED_INT, nullptr);

    fbo->Unbind();
}

void Editor::ShowResultWindow(const char* title)
{
    ImGui::Begin(title);

    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    // Do not drag the window when left clicking and dragging

    // TODO: This is wrong! This sets global state but we don't want that. For individual input
    // capture there is something else. google it!
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
    }
    else {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    }

    float imguiWindowWidth  = ImGui::GetContentRegionAvail().x;
    float imguiWindowHeight = ImGui::GetContentRegionAvail().y - 60.0f; // -30 to leave some room for the slider widget below

    // safe guard for potential div by 0

    if (imguiWindowWidth <= 0) {
        imguiWindowWidth = 1.0;
    }
    if (imguiWindowHeight <= 0) {
        imguiWindowHeight = 1.0;
    }

    float srcAspect = (float)m_resultFBO->m_Width / (float)m_resultFBO->m_Height;
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

    ImVec2 imageSize(newWidth, newHeight);
    ImVec2 imagePosition(ImGui::GetCursorPosX() + posOffsetX, ImGui::GetCursorPosY() + posOffsetY);

    ImGui::SetCursorPos(imagePosition);
    
    ImGui::Image((void*)(intptr_t)m_blendedImageTextures[m_ImageIndex].GetHandle(), ImVec2(newWidth, newHeight));

    ImGui::SetCursorPosX(imagePosition.x);    
    ImGui::PushItemWidth(imageSize.x);
    ImGui::SliderInt("##imageIndexSlider", &m_ImageIndex, 0, m_blendedImages.size() - 1);
    ImGui::PopItemWidth();

    RenderTGA(m_blendedImages);
    ImGui::SameLine();
    RenderGIF(m_blendedImages);

    ImGui::End();
}



void Editor::ShowTextureSequenceWindow(const char* title, std::vector<Texture>& textures)
{
    ImGui::Begin(title);
    
    // TODO: This is wrong! This sets global state but we don't want that. For individual input
    // capture there is something else. google it!
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
    }
    else {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    }

    float imguiWindowWidth = ImGui::GetContentRegionAvail().x;
    float imguiWindowHeight = ImGui::GetContentRegionAvail().y - 60.0f; // -60 to leave some room for the slider widget below

    // safe guard for potential div by 0

    if (imguiWindowWidth <= 0) {
        imguiWindowWidth = 1.0;
    }
    if (imguiWindowHeight <= 0) {
        imguiWindowHeight = 1.0;
    }

    float srcAspect = (float)m_resultFBO->m_Width / (float)m_resultFBO->m_Height;
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

    ImVec2 imageSize(newWidth, newHeight);
    ImVec2 imagePosition(ImGui::GetCursorPosX() + posOffsetX, ImGui::GetCursorPosY() + posOffsetY);

    ImGui::SetCursorPos(imagePosition);

    ImGui::Image((void*)(intptr_t)textures[m_ImageIndex].GetHandle(), ImVec2(newWidth, newHeight));

    ImGui::SetCursorPosX(imagePosition.x);
    ImGui::PushItemWidth(imageSize.x);
    ImGui::SliderInt("##imageIndexSlider", &m_ImageIndex, 0, m_blendedImages.size() - 1);
    RenderTGA(textures);
    ImGui::SameLine();
    RenderGIF(textures);

    ImGui::PopItemWidth();

    ImGui::End();
}

void Editor::Run()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());    

    // Main Menu bar

    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("New","Ctrl+N")) {
                NewProject();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                SaveProject();
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                OpenProject();
            }
            ImGui::EndMenu();
        }        

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                Undo();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // ! Main menu bar

    // Keyboard shortcuts

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N)) && ImGui::GetIO().KeyCtrl) {        
        NewProject();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) && ImGui::GetIO().KeyCtrl) {
        SaveProject();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)) && ImGui::GetIO().KeyCtrl) {
        OpenProject();
    }

    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)) && ImGui::GetIO().KeyCtrl) {
        Undo();
    }

    // ! Keyboard shortcuts

    ShowWindow("Source", m_sourceImage, m_sourceFBO, m_sourceLines, ED_WINDOW_TYPE_SOURCE);
    ShowWindow("Destination", m_destImage, m_destFBO, m_destLines, ED_WINDOW_TYPE_DEST);

    ImGui::Begin("Control Panel");

    ImGui::SliderFloat("a", &m_A, 0.0f, 20.0f);
    ImGui::SliderFloat("b", &m_B, 0.0f, 20.0f);
    ImGui::SliderFloat("p", &m_P, 0.0f, 1.0f);
    ImGui::SliderInt("Iterations", &m_NumIterations, 1, 100);

    if (ImGui::Button("MAGIC!")) {
        if (m_sourceImage.m_Width != m_destImage.m_Width 
            || m_sourceImage.m_Height != m_destImage.m_Height) {
            tinyfd_messageBox("Image dimension mismatch", "Source and destination images must have the same dimensions!", "ok", "warning", 1);
        }
        else if (m_sourceLines.size() != m_destLines.size()) {
            tinyfd_messageBox("Linecount mismatch", "The number of lines in the source window and the destination window do not match!", "ok", "warning", 1);
        }
        else {
            RenderStartEvent rse(m_sourceLines, m_destLines, m_sourceImage, m_destImage, m_NumIterations, m_A, m_B, m_P);
            m_EventHandler->Notify(&rse);
            m_resultFBO->Resize(m_destImage.m_Width, m_destImage.m_Height);
            m_isRendering = true;
        }
    }
    
    if (m_isRendering) {
        ImGui::ProgressBar(m_RenderPctDone);
        if (ImGui::Button("Cancel Render")) {
            RenderStopEvent rse;
            m_EventHandler->Notify(&rse);
            m_RenderPctDone = 0.0f;
            m_isRendering = false;
        }
    }
    
    if (!m_blendedImages.empty()) {
        ImGui::Checkbox("Show source morphs", &m_ShowSourceToDestImages);
        ImGui::Checkbox("Show destination morphs", &m_ShowDestToSourceImages);

        if (m_ShowSourceToDestImages) {
            ShowTextureSequenceWindow("source morphs", m_SourceToDestMorphTextures);
        }
        if (m_ShowDestToSourceImages) {
            ShowTextureSequenceWindow("destination morphs", m_DestToSourceMorphTextures);
        }
    }

    ImGui::End();

    if (!m_blendedImages.empty()) {
        ShowResultWindow("Result");
    }    

    m_Dirty = false;
}
