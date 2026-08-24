#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <jni.h>
#include <math.h>
#include <float.h>
#include <vector>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "include/inlineHook.h"

bool g_Initialized = false;
EGLDisplay g_EglDisplay;
EGLSurface eglSurface;
EGLContext context;
ANativeWindow *g_NativeWindow;
int glWidth = 0, glHeight = 0;
ImVec2 g_MenuPos;
ImVec2 g_MenuSize;
unsigned long base = 0L;
void *localPlayer = NULL;
void *moveInputHandler = NULL;
std::vector<void *> mobs;

static bool keepSprinting = true;

static bool magnifier = true;
static float fov = 120.f;

static bool killaura = false;

static bool bHop = false;
static float bHop_speed = 1.0f;

static bool drawBox = false;
static int boxStyle = 0;

static bool drawNameTag = false;

static bool drawTracer = false;
static int tracerPos = 0;

static bool fullBright = false;
static float fullBright_val = 25.f;

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vec2
{
    float x;
    float y;
};

struct Box3D
{
    Vec3 min;
    Vec3 max;
};

static float getPitch(void *actor)
{
    return *(float *)((uint32_t)actor + 0xA8);
}

static float getYaw(void *actor)
{
    return *(float *)((uint32_t)actor + 0xAC);
}

static Vec3 getPos(void *actor)
{
    Vec3 pos;
    pos.x = *(float *)((uint32_t)actor + 0xcd4);
    pos.y = *(float *)((uint32_t)actor + 0xcd8);
    pos.z = *(float *)((uint32_t)actor + 0xcdc);
    return pos;
}

static float getSizeWidth(void *actor)
{
    return *(float *)((uint32_t)actor + 0xcc0);
}

static float getSizeHeight(void *actor)
{
    return *(float *)((uint32_t)actor + 0xcc4);
}

static int getHealth(void *actor)
{
    auto _ZNK5Actor9getHealthEv = (int (*)(void *))(base + 41090380 + 1);
    return (*_ZNK5Actor9getHealthEv)(actor);
}

static int getMaxHealth(void *actor)
{
    auto _ZNK5Actor12getMaxHealthEv = (int (*)(void *))(base + 0x0272FD74 + 1);
    return (*_ZNK5Actor12getMaxHealthEv)(actor);
}

static bool isAlive(void *actor)
{
    auto _ZNK5Actor7isAliveEv = (bool (*)(void *))(base + 41040402 + 1);
    return (*_ZNK5Actor7isAliveEv)(actor);
}

static void setSprinting(void *actor, bool enable)
{
    auto _ZN11LocalPlayer12setSprintingEb = (void *(*)(void *, bool))(base + 26096448 + 1);
    (*_ZN11LocalPlayer12setSprintingEb)(actor, enable);
}

static void attack(void *gameMode, void *actor)
{
    auto _ZN8GameMode6attackER5Actor = (void *(*)(void *, void *))(base + 0x02A67F58 + 1);
    (*_ZN8GameMode6attackER5Actor)(gameMode, actor);
}

static void swing(void *actor)
{
    auto _ZN11LocalPlayer5swingEv = (void *(*)(void *))(base + 26096100 + 1);
    (*_ZN11LocalPlayer5swingEv)(actor);
}

static bool isOnGround(void *actor){
    return *(bool *)((uint32_t)actor + 0x110);
}

static void *getGameMode(void *actor)
{
    return *(void **)((uint32_t)actor + 0x15CC);
}

static Vec3 *getVelocity(void *actor){
    return (Vec3 *)((uint32_t)actor + 0xcc8);
}

static void *getClientInstance(void *actor)
{
    auto _ZNK11LocalPlayer17getClientInstanceEvp = (void *(*)(void *))(base + 26083436 + 1);
    return (*_ZNK11LocalPlayer17getClientInstanceEvp)(actor);
}

static void *getRegion(void *actor){
    auto _ZNK5Actor9getRegionEv = (void *(*)(void *))(base + 40996504 + 1);
    return (*_ZNK5Actor9getRegionEv)(actor);
}

static void *getBlock(void *region, int x, int y, int z){
    auto _ZNK11BlockSource8getBlockEiii = (void *(*)(void *, int, int, int))(base + 47271792 + 1);
    return (*_ZNK11BlockSource8getBlockEiii)(region, x, y, z);
}

static void *toBlockLegacy(void *block){
    return **(void ***)((uint32_t)block + 8);
}

static char *getDescriptionId(void *blockLegacy){
    auto _ZNK11BlockLegacy16getDescriptionIdEv = (char *(*)(void *))(base + 46134660 + 1);
    return *(char **)(*_ZNK11BlockLegacy16getDescriptionIdEv)(blockLegacy);
}

static bool isEmptyBlock(void *block){
    auto _ZNK5Block7isEmptyEv = (bool (*)(void *))(base + 46121754 + 1);
    // return (*_ZNK5Block7isEmptyEv)(block);
    return **(uint32_t **)((uint32_t)block + 8) == *(uint32_t *)(base + 0x5068d80);
}

static void *getGuiData(void *clientInstance)
{
    auto _ZN14ClientInstance10getGuiDataEv = (void *(*)(void *))(base + 31185878 + 1);
    return (*_ZN14ClientInstance10getGuiDataEv)(clientInstance);
}

static float getGuiWidth(void *guiData)
{
    return *(float *)((uint32_t)guiData + 0xc);
}

static float getGuiHeight(void *guiData)
{
    return *(float *)((uint32_t)guiData + 0x10);
}

static float getMoveSide(void *moveInputHandler){
    return *(float *)((uint32_t)moveInputHandler + 4);
}

static float getMoveForward(void *moveInputHandler){
    return *(float *)((uint32_t)moveInputHandler + 8);
}

static bool isSneakDown(void *moveInputHandler){
    return *(bool *)((uint32_t)moveInputHandler + 0x5e);
}

static bool isJumpDown(void *moveInputHandler){
    return *(bool *)((uint32_t)moveInputHandler + 0x5e - 1);
}

float Distance3D(Vec3 a, Vec3 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

static Vec3 Normalize(Vec3 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len <= 0.00001f)
        return {0, 0, 0};
    v.x /= len;
    v.y /= len;
    v.z /= len;
    return v;
}

static Vec3 Cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

bool WorldToScreen(Vec3 world, Vec3 camera, float yaw, float pitch, float fov, float width, float height, ImVec2 &screen)
{
    float yawRad = yaw * 3.1415926535f / 180.0f;
    float pitchRad = pitch * 3.1415926535f / 180.0f;

    Vec3 forward;
    forward.x = -sinf(yawRad) * cosf(pitchRad);
    forward.y = -sinf(pitchRad);
    forward.z = cosf(yawRad) * cosf(pitchRad);
    forward = Normalize(forward);

    Vec3 worldUp = {0, 1, 0};
    Vec3 right = Cross(forward, worldUp);

    if (fabs(right.x) < 0.0001f && fabs(right.y) < 0.0001f && fabs(right.z) < 0.0001f)
    {
        worldUp = {0, 0, 1};
        right = Cross(forward, worldUp);
    }
    right = Normalize(right);

    Vec3 cameraUp = Cross(right, forward);
    cameraUp = Normalize(cameraUp);

    Vec3 delta;
    delta.x = world.x - camera.x;
    delta.y = world.y - camera.y;
    delta.z = world.z - camera.z;

    float camX = delta.x * right.x + delta.y * right.y + delta.z * right.z;
    float camY = delta.x * cameraUp.x + delta.y * cameraUp.y + delta.z * cameraUp.z;
    float camZ = delta.x * forward.x + delta.y * forward.y + delta.z * forward.z;

    if (camZ <= 0.01f)
        return false;

    float focal = (height * 0.5f) / tanf(fov * 0.5f * 3.1415926535f / 180.0f);
    screen.x = width * 0.5f + camX * focal / camZ;
    screen.y = height * 0.5f - camY * focal / camZ;

    return true;
}

void GetBoxVertices(Vec3 pos, float width, float height, Vec3 vertices[8])
{
    float x1 = pos.x - width / 2;
    float x2 = pos.x + width / 2;
    float y1 = pos.y;
    float y2 = pos.y + height;
    float z1 = pos.z - width / 2;
    float z2 = pos.z + width / 2;
    vertices[0] = {x1, y1, z1};
    vertices[1] = {x2, y1, z1};
    vertices[2] = {x2, y1, z2};
    vertices[3] = {x1, y1, z2};
    vertices[4] = {x1, y2, z1};
    vertices[5] = {x2, y2, z1};
    vertices[6] = {x2, y2, z2};
    vertices[7] = {x1, y2, z2};
}

void Draw3DBox(Vec3 pos, float w, float h, Vec3 cam, float yaw, float pitch, float fov, float screenW, float screenH)
{
    Vec3 points[8];
    GetBoxVertices(pos, w, h, points);
    ImVec2 screen[8];
    for (int i = 0; i < 8; i++)
        if (!WorldToScreen(points[i], cam, yaw, pitch, fov, screenW, screenH, screen[i]))
            return;
    auto draw = ImGui::GetBackgroundDrawList();
    int edges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    for (auto &e : edges)
        draw->AddLine(screen[e[0]], screen[e[1]], IM_COL32(255, 0, 0, 255), 2.0f);
}

void Draw2DBox(Vec3 entityPos, float entityWidth, float entityHeight, Vec3 cameraPos, float yaw, float pitch, float fov, float screenWidth, float screenHeight)
{
    float half = entityWidth * 0.5f;
    float minX = entityPos.x - half;
    float maxX = entityPos.x + half;
    float minY = entityPos.y;
    float maxY = entityPos.y + entityHeight;
    float minZ = entityPos.z - half;
    float maxZ = entityPos.z + half;
    
    Vec3 corners[8] =
    {
        {minX, minY, minZ},
        {maxX, minY, minZ},
        {maxX, minY, maxZ},
        {minX, minY, maxZ},

        {minX, maxY, minZ},
        {maxX, maxY, minZ},
        {maxX, maxY, maxZ},
        {minX, maxY, maxZ}
    };


    ImVec2 screen[8];
    float left   = FLT_MAX;
    float right  = -FLT_MAX;
    float top    = FLT_MAX;
    float bottom = -FLT_MAX;
    int visible = 0;
    for (int i = 0; i < 8; i++)
    {
        if (!WorldToScreen(corners[i], cameraPos, yaw, pitch, fov, screenWidth, screenHeight, screen[i]))
            continue;
        visible++;
        if (screen[i].x < left)
            left = screen[i].x;
        if (screen[i].x > right)
            right = screen[i].x;
        if (screen[i].y < top)
            top = screen[i].y;
        if (screen[i].y > bottom)
            bottom = screen[i].y;
    }
    if (visible == 0)
        return;
    if (right <= left || bottom <= top)
        return;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImVec2 boxMin(left, top);
    ImVec2 boxMax(right, bottom);
    draw->AddRectFilled(boxMin, boxMax, IM_COL32(255, 255, 255, 60));
    draw->AddRect(ImVec2(left - 1.5f, top - 1.5f), ImVec2(right + 1.5f, bottom + 1.5f), IM_COL32(0, 0, 0, 220), 0.0f, 0, 3.0f);
    draw->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.5f);
}

void DrawNameTag(void *target, Vec3 cam, float yaw, float pitch, float fov, float screenW, float screenH)
{
    ImVec2 pos;
    Vec3 targetPos = getPos(target);
    targetPos.y += getSizeHeight(target);
    if (WorldToScreen(targetPos, cam, yaw, pitch, fov, screenW, screenH, pos))
    {
        ImDrawList *draw = ImGui::GetBackgroundDrawList();
        ImFont *font = ImGui::GetFont();
        float distance = Distance3D(targetPos, cam);
        float scale = 20.0f / distance;
        scale = std::max(0.5f, std::min(scale, 1.f));
        char dis[128];
        sprintf(dis, "Enemy %.1fm %d/%d", distance, getHealth(target), getMaxHealth(target));
        float fontSize = ImGui::GetFontSize() * scale;

        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, dis);
        ImVec2 textPos(pos.x - textSize.x / 2, pos.y - textSize.y / 2);
        float paddingX = 8.0f;
        float paddingY = 4.0f;
        ImVec2 bgMin(textPos.x - paddingX, textPos.y - paddingY);
        ImVec2 bgMax(textPos.x + textSize.x + paddingX, textPos.y + textSize.y + paddingY);
        draw->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 120), 6.0f);
        draw->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 220), dis);
        draw->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 255), dis);
    }
}

void DrawNeonBox(Vec3 entityPos, float entityWidth, float entityHeight, Vec3 cameraPos, float yaw, float pitch, float fov, float screenWidth, float screenHeight)
{
    float halfWidth = entityWidth * 0.5f;
    float minX = entityPos.x - halfWidth;
    float maxX = entityPos.x + halfWidth;
    float minY = entityPos.y;
    float maxY = entityPos.y + entityHeight;
    float minZ = entityPos.z - halfWidth;
    float maxZ = entityPos.z + halfWidth;

    Vec3 corners[8] =
    {
        { minX, minY, minZ },
        { maxX, minY, minZ },
        { maxX, minY, maxZ },
        { minX, minY, maxZ },

        { minX, maxY, minZ },
        { maxX, maxY, minZ },
        { maxX, maxY, maxZ },
        { minX, maxY, maxZ }
    };

    ImVec2 screen[8];
    float left   = FLT_MAX;
    float right  = -FLT_MAX;
    float top    = FLT_MAX;
    float bottom = -FLT_MAX;
    int visible = 0;
    for (int i = 0; i < 8; i++)
    {
        if (!WorldToScreen(corners[i], cameraPos, yaw, pitch, fov, screenWidth, screenHeight, screen[i]))
            continue;
        visible++;
        if (screen[i].x < left) left = screen[i].x;
        if (screen[i].x > right) right = screen[i].x;
        if (screen[i].y < top) top = screen[i].y;
        if (screen[i].y > bottom) bottom = screen[i].y;
    }

    if (visible == 0) return;
    if (right <= left || bottom <= top) return;
    
    float boxWidth  = right - left;
    float boxHeight = bottom - top;
    if (boxWidth < 1.0f || boxHeight < 1.0f)
        return;
    if (boxWidth > screenWidth * 2.0f ||
        boxHeight > screenHeight * 2.0f)
        return;
        
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(left, top), ImVec2(right, bottom), IM_COL32(255, 255, 255, 10), 2.0f);
    draw->AddRect(ImVec2(left - 7.0f, top - 7.0f), ImVec2(right + 7.0f, bottom + 7.0f), IM_COL32(255, 255, 255, 10), 3.0f, 0, 5.0f);
    draw->AddRect(ImVec2(left - 5.0f, top - 5.0f), ImVec2(right + 5.0f, bottom + 5.0f), IM_COL32(255, 255, 255, 20), 3.0f, 0, 4.0f);
    draw->AddRect(ImVec2(left - 3.0f, top - 3.0f), ImVec2(right + 3.0f, bottom + 3.0f), IM_COL32(255, 255, 255, 35), 2.0f, 0, 3.0f);
    draw->AddRect(ImVec2(left - 1.5f, top - 1.5f), ImVec2(right + 1.5f, bottom + 1.5f ), IM_COL32(255, 255, 255, 80), 2.0f, 0, 2.5f);
    draw->AddRect(ImVec2(left, top), ImVec2(right, bottom), IM_COL32(255, 255, 255, 255), 2.0f, 0, 1.2f);
}

void DrawTracer(Vec3 enemyPos, Vec3 cameraPos, float yaw, float pitch, float fov, float screenWidth, float screenHeight)
{
    ImVec2 enemyScreen;
    if (!WorldToScreen(enemyPos, cameraPos, yaw, pitch, fov, screenWidth, screenHeight, enemyScreen))
        return;
    ImVec2 playerScreen(screenWidth * 0.5f, screenHeight * 0.5f);
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddLine(playerScreen, enemyScreen, IM_COL32(255, 255, 255, 35), 5.0f);
    draw->AddLine(playerScreen, enemyScreen, IM_COL32(255, 255, 255, 70), 3.0f);
    draw->AddLine(playerScreen, enemyScreen, IM_COL32(255, 255, 255, 230), 1.5f);
}

void initImGui(JNIEnv *env, jclass clazz, jobject surface)
{
    if (g_Initialized)
        return;
    EGLConfig config;
    EGLint numConfigs;
    const EGLint configAttribs[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
    eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs);
    eglSurface = eglCreateWindowSurface(g_EglDisplay, config, g_NativeWindow, nullptr);
    context = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, nullptr);
    eglMakeCurrent(g_EglDisplay, eglSurface, eglSurface, context);
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    g_NativeWindow = ANativeWindow_fromSurface(env, surface);
    ImGui::CreateContext();
    ImGuiIO *io = &ImGui::GetIO();
    io->IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().GrabMinSize = 24.0f;
    io->Fonts->Clear();
    io->Fonts->AddFontDefault();
    io->FontGlobalScale = 2.0f;
    ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueWheel);
    ImGui_ImplAndroid_Init(g_NativeWindow);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImFontConfig font_cfg;
    g_Initialized = true;
}

void onResize(JNIEnv *env, jclass clazz, jobject gl, jint width, jint height)
{
    glWidth = width;
    glHeight = height;
    glViewport(0, 0, width, height);
    ImGuiIO *io = &ImGui::GetIO();
    io->DisplaySize = ImVec2((float)width, (float)height);
}

void onImGuiRender(JNIEnv *env, jclass clazz, jobject im_gui_surface)
{
    ImGuiIO &io = ImGui::GetIO();
    if (g_EglDisplay == EGL_NO_DISPLAY)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
    ImGui::NewFrame();

    static bool show_demo_window = false;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    {
        ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
        ImGui::Begin("Minecraft:be");

        if (ImGui::CollapsingHeader("GameInfo"))
        {
            ImGui::Checkbox("ShowDemoWindow", &show_demo_window);
            ImGui::Text("Mobs : %d", mobs.size());
            ImGui::Text("LocalPlayer : %lx", (uint32_t)localPlayer);
            ImGui::Text("moveInputHandler : %lx", (uint32_t)moveInputHandler);
            if(localPlayer){
                auto region = getRegion(localPlayer);
                auto pos = getPos(localPlayer);
                pos.y -= 1;
                auto block = getBlock(region, (int)pos.x, (int)pos.y, (int)pos.z);
                ImGui::Text("%d %d %d Block : %lx %s", (int)pos.x, (int)pos.y, (int)pos.z, (uint32_t)block, getDescriptionId(toBlockLegacy(block)));
            }
        }

        if (ImGui::CollapsingHeader("KeepSprinting"))
        {
            ImGui::Checkbox("Enabled KeepSprinting", &keepSprinting);
        }

        if (ImGui::CollapsingHeader("KillAura"))
        {
            ImGui::Checkbox("Enabled KillAura", &killaura);
        }

        if (ImGui::CollapsingHeader("Magnifier"))
        {
            ImGui::Checkbox("Enabled Magnifier", &magnifier);
            ImGui::SliderFloat("Fov", &fov, -1000.f, 1000.f);
        }
        
        
        ImGui::Checkbox("FullBright", &fullBright);
        if (fullBright){
            ImGui::SameLine();
            ImGui::SliderFloat("FullBright val", &fullBright_val, -25.f, 25.f);
        }
        
        if (ImGui::CollapsingHeader("bHop")) {
            ImGui::Checkbox("Enabled bHop", &bHop);
            ImGui::SliderFloat("bHop Speed", &bHop_speed, 0.f, 3.f);
        }

        if (ImGui::CollapsingHeader("ESP"))
        {
            ImGui::Checkbox("DrawBox", &drawBox);
            if (drawBox){
                ImGui::RadioButton("3DBox", &boxStyle, 0); ImGui::SameLine();
                ImGui::RadioButton("2DBox", &boxStyle, 1); ImGui::SameLine();
                ImGui::RadioButton("NeonBox", &boxStyle, 2);
            }
            ImGui::Checkbox("DrawNameTag", &drawNameTag);
            ImGui::Checkbox("DrawTracer", &drawTracer);
            if (drawTracer){
                ImGui::RadioButton("Top", &tracerPos, 0); ImGui::SameLine();
                ImGui::RadioButton("Center", &tracerPos, 1); ImGui::SameLine();
                ImGui::RadioButton("Bottom", &tracerPos, 2);
            }
        }

        if (drawBox || drawNameTag || drawTracer)
        {
            void *clientInstance = getClientInstance(localPlayer);
            void *guiData = getGuiData(clientInstance);
            float guiWidth = getGuiWidth(guiData);
            float guiHeight = getGuiHeight(guiData);
            for (auto mob = mobs.begin(); mob != mobs.end() && *mob != localPlayer; mob++)
            {
                if (isAlive(*mob) && getHealth(*mob) > 0)
                {
                    ImVec2 pos;
                    Vec3 mPos = getPos(*mob);
                    float sizeWidth = getSizeWidth(*mob);
                    float sizeHeight = getSizeHeight(*mob);
                    Vec3 cPos = getPos(localPlayer);
                    float yaw = getYaw(localPlayer);
                    float pitch = getPitch(localPlayer);
                    if (drawBox){
                        switch(boxStyle){
                            case 0:
                                Draw3DBox(mPos, sizeWidth, sizeHeight, cPos, yaw, pitch, fov, guiWidth, guiHeight);
                                break;
                            case 1:
                                Draw2DBox(mPos, sizeWidth, sizeHeight, cPos, yaw, pitch, fov, guiWidth, guiHeight);
                                break;
                            case 2:
                                DrawNeonBox(mPos, sizeWidth, sizeHeight, cPos, yaw, pitch, fov, guiWidth, guiHeight);
                                break;
                            default:
                                break;
                        }
                    }
                    if (drawNameTag)
                        DrawNameTag(*mob, cPos, yaw, pitch, fov, guiWidth, guiHeight);
                    if (drawTracer){
                        switch(tracerPos){
                            case 0:
                                mPos.y += sizeHeight;
                                break;
                            case 1:
                                mPos.y += sizeHeight / 2;
                                break;
                        }
                        DrawTracer(mPos, cPos, yaw, pitch, fov, guiWidth, guiHeight);
                    }
                }
            }
        }
        g_MenuPos = ImGui::GetWindowPos();
        g_MenuSize = ImGui::GetWindowSize();
        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(g_EglDisplay, eglSurface);
}

void onShutdown(JNIEnv *env, jclass clazz)
{
    if (!g_Initialized)
        return;
    g_Initialized = false;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    ANativeWindow_release(g_NativeWindow);
    g_NativeWindow = nullptr;
}

bool IsInMenu(float x,float y) {
    return x >= g_MenuPos.x && x <= g_MenuPos.x + g_MenuSize.x && y >= g_MenuPos.y && y <= g_MenuPos.y + g_MenuSize.y;
}

bool nativeTouch(JNIEnv *env, jclass clazz, jint action, jfloat x, jfloat y)
{
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    if (action == 0)
        io.MouseDown[0] = true;
    if (action == 1)
        io.MouseDown[0] = false;
    // return io.WantCaptureMouse;
    return IsInMenu(x, y);
}

static unsigned long find_database_of(char *soName)
{
    char filename[32];
    char cmdline[256];
    sprintf(filename, "/proc/%d/maps", getpid());
    FILE *fp = fopen(filename, "r");
    unsigned long revalue = 0;
    if (fp)
    {
        while (fgets(cmdline, 256, fp))
        {
            if (strstr(cmdline, soName) && strstr(cmdline, "r-xp"))
            {
                char *str = strstr(cmdline, "-");
                if (str)
                {
                    *str = '\0';
                    char num[32];
                    sprintf(num, "0x%s", cmdline);
                    revalue = strtoul(num, NULL, 0);
                    return revalue;
                }
            }
            memset(cmdline, 0, 256);
        }
        fclose(fp);
    }
    return 0L;
}

void *(*_ZN3MobC2EP20ActorDefinitionGroupRK25ActorDefinitionIdentifier)(void *, void *, void *) = NULL;
void *_ZN3MobC2EP20ActorDefinitionGroupRK25ActorDefinitionIdentifier_Hook(void *thisObject, void *ActorDefinitionGroup, void *ActorDefinitionIdentifier)
{
    mobs.push_back(thisObject);
    return (*_ZN3MobC2EP20ActorDefinitionGroupRK25ActorDefinitionIdentifier)(thisObject, ActorDefinitionGroup, ActorDefinitionIdentifier);
}

void *(*_ZN3MobD2Ev)(void *) = NULL;
void *_ZN3MobD2Ev_Hook(void *thisObject)
{
    mobs.erase(std::remove(mobs.begin(), mobs.end(), thisObject), mobs.end());
    return (*_ZN3MobD2Ev)(thisObject);
}

void *(*_ZN11LocalPlayer10normalTickEv)(void *) = NULL;
void *_ZN11LocalPlayer10normalTickEv_Hook(void *thisObject)
{
    localPlayer = thisObject;
    if (keepSprinting)
    {
        setSprinting(localPlayer, keepSprinting);
    }

    if (killaura)
    {
        void *gameMode = getGameMode(localPlayer);
        for (auto mob = mobs.begin(); mob != mobs.end() && *mob != localPlayer; mob++)
        {
            if (isAlive(*mob) && getHealth(*mob) > 0)
            {
                attack(gameMode, *mob);
                swing(localPlayer);
            }
        }
    }
    
    if (bHop){
        Vec2 moveVec2d = {getMoveForward(moveInputHandler), -getMoveSide(moveInputHandler)};
        if (sqrt(pow(moveVec2d.x, 2) + pow(moveVec2d.y, 2)) > 0.01) {
            float calcYaw = (getYaw(localPlayer) + 90) * (3.1415926 / 180);
            float c = cos(calcYaw);
            float s = sin(calcYaw);
    
            moveVec2d = {moveVec2d.x * c - moveVec2d.y * s,
                         moveVec2d.x * s + moveVec2d.y * c};
    
            Vec3 *motion = getVelocity(localPlayer);
    
            if (isOnGround(localPlayer)) {
                auto _ZN11LocalPlayer14jumpFromGroundEv = (void *(*)(void *))(base + 26118930 + 1);
                (*_ZN11LocalPlayer14jumpFromGroundEv)(localPlayer);
            }
            motion->x = moveVec2d.x * bHop_speed;
            motion->z = moveVec2d.y * bHop_speed;
        }
    }
    return (*_ZN11LocalPlayer10normalTickEv)(thisObject);
}

float (*_ZN19LevelRendererPlayer6getFovEfb)(void *, float, bool) = NULL;
float _ZN19LevelRendererPlayer6getFovEfb_Hook(void *thisObject, float a1, bool a2)
{
    if (!magnifier)
        fov = (*_ZN19LevelRendererPlayer6getFovEfb)(thisObject, a1, a2);
    return fov;
}

void *(*_ZN16MoveInputHandler4tickER11LocalPlayer)(void *, void *) = NULL;
void *_ZN16MoveInputHandler4tickER11LocalPlayer_Hook(void *thisObject, void *lp){
    moveInputHandler = thisObject;
    return (*_ZN16MoveInputHandler4tickER11LocalPlayer)(thisObject, lp);
}

float (*_ZNK7Options8getGammaEv)(void *) = NULL;
float _ZNK7Options8getGammaEv_Hook(void *thisObject){
    return (*_ZNK7Options8getGammaEv)(thisObject);
}

void EnabledHook()
{
    // 找不到就用指针 不是必要的hook
    registerInlineHook((uint32_t)(base + 26084404 + 1), (uint32_t)_ZN11LocalPlayer10normalTickEv_Hook, (uint32_t **)&_ZN11LocalPlayer10normalTickEv);
    // 找不到就自己设置个值
    registerInlineHook((uint32_t)(base + 27718928 + 1), (uint32_t)_ZN19LevelRendererPlayer6getFovEfb_Hook, (uint32_t **)&_ZN19LevelRendererPlayer6getFovEfb);
    
    registerInlineHook((uint32_t)(base + 0x0295BC78 + 1), (uint32_t)_ZN3MobC2EP20ActorDefinitionGroupRK25ActorDefinitionIdentifier_Hook, (uint32_t **)&_ZN3MobC2EP20ActorDefinitionGroupRK25ActorDefinitionIdentifier);
    registerInlineHook((uint32_t)(base + 0x0295DC70 + 1), (uint32_t)_ZN3MobD2Ev_Hook, (uint32_t **)&_ZN3MobD2Ev);
    registerInlineHook((uint32_t)(base + 24363356 + 1), (uint32_t)_ZN16MoveInputHandler4tickER11LocalPlayer_Hook, (uint32_t **)&_ZN16MoveInputHandler4tickER11LocalPlayer);
    registerInlineHook((uint32_t)(base + 25800542 + 1), (uint32_t)_ZNK7Options8getGammaEv_Hook, (uint32_t **)&_ZNK7Options8getGammaEv);
    inlineHookAll();
}

static JNINativeMethod methods[] = {
    {"init", "(Landroid/view/Surface;)V", (void *)initImGui},
    {"resize", "(Ljavax/microedition/khronos/opengles/GL10;II)V", (void *)onResize},
    {"nativeTouch", "(IFF)Z", (void *)nativeTouch},
    {"onTick", "(Lcom/example/application/GameRender;)V", (void *)onImGuiRender},
    {"onShutdown", "()V", (void *)onShutdown},
};

jint registerNative(JNIEnv *env)
{
    jclass clazz = env->FindClass("com/example/application/GameRender");
    if (!clazz)
        return JNI_ERR;
    return env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) == 0 ? JNI_OK : JNI_ERR;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) != JNI_OK)
        return JNI_ERR;
    if (registerNative(env) != JNI_OK)
        return JNI_ERR;
    base = find_database_of("libminecraftpe.so");
    if (base > 0L)
        EnabledHook();
    return JNI_VERSION_1_6;
}
