#include <iostream>
#include <memory>

#include <sm/vec>

#include <mplot/tools.h>
#include <mplot/Visual.h>
#include <mplot/SphereVisual.h>
#include <mplot/VectorVisual.h>
#include <mplot/TriangleVisual.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct imgui_visual final : public mplot::Visual<>
{
    imgui_visual (int width, int height, const std::string& title) : mplot::Visual<> (width, height, title)
    {
        this->setContext(); // Set the OpenGL context before Dear ImGui initialization
        this->renderSwapsBuffers (false); // With Dear ImGui, we manually swapBuffers(), so set this false
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // One user suggested doing this before opengl setup...
        //auto defaultSetClipboardTextFn = io.SetClipboardTextFn;
        //auto defaultGetClipboardTextFn = io.GetClipboardTextFn;
        //auto defaultClipboardUserData = io.ClipboardUserData;

        ImGui_ImplGlfw_InitForOpenGL (this->window, true);
        ImGui_ImplOpenGL3_Init();
        constexpr int size_in_pixels = 24;
        io.Fonts->AddFontFromFileTTF ("/tmp/DejaVuSans.ttf", size_in_pixels);

        // ...then this after:
        //io.SetClipboardTextFn = defaultSetClipboardTextFn;
        //io.GetClipboardTextFn = defaultGetClipboardTextFn;
        //io.ClipboardUserData = defaultClipboardUserData;
        //
        // But it didn't work for me.


        // Clipboard?
        //ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        //platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { glfwSetClipboardString (nullptr, text); };
        //platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*) { return glfwGetClipboardString (nullptr); };
        // OR
#if 0
        io.SetClipboardTextFn = _IMGUISetClipboardText;
        io.GetClipboardTextFn = _IMGUIGetClipboardText;
#endif
    }

#if 0
    // These have to be static, so no access to this->window :(
    static const char* _IMGUIGetClipboardText(void *)
    {
        std::cout << __func__ << " called\n";
        return glfwGetClipboardString (glfwGetCurrentContext());
    }
    static void _IMGUISetClipboardText(void *, const char *text)
    {
        std::cout << __func__ << " called\n";
        glfwSetClipboardString (glfwGetCurrentContext(), text);
    }
#endif

    void gui_draw()
    {
        if (this->show_gui == true) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Options (Esc to toggle)");
            if (ImGui::SliderFloat("Thickness", &this->thickness, 0.0f, 0.005f)) { }
            if (ImGui::ColorEdit3("Colour", this->clr.data())) {
            }
            static char buf1[512] = "";
            if (ImGui::InputText("Coords/Tri", buf1, IM_ARRAYSIZE(buf1))) {
                this->geom_text = std::string (buf1);
            }

            if (ImGui::Button("Draw")) { needs_visualmodel_rebuild = true; }
            if (ImGui::Button("Clear")) { this->vm.clear(); }
            if (ImGui::Button("Clear Last")) { this->vm.pop_back(); }
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());
        }
    }

    // Show the GUI?
    bool show_gui = true;
    // Some text fields for the gui
    std::string geom_text = "";
    // radius for spheres
    float thickness = 0.002f;
    // Colour for objects
    std::array<float, 3> clr = { 1.0f, 0.0f, 0.2f };
    // Do the mplot::VisualModels need to be re-built?
    bool needs_visualmodel_rebuild = true;

protected:
#if 0
    bool key_callback (int _key, int scancode, int action, int mods)
    {
        std::cout << "tripoints key_callback\n";
        ImGuiIO& io = ImGui::GetIO();
        // io.SetKeyEventNativeData (ImGuiKey key, int native_keycode, scancode);
        if (!io.WantCaptureKeyboard) {
            std::cout << "ImGui wants mathplot to process it\n";
            return mplot::VisualBase<>::key_callback (_key, scancode, action, mods);
        } else {
            std::cout << "ImGui wants it\n";
        }
        return false;
    }
#endif
    void key_callback_extra (int _key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (_key == mplot::key::escape && action == mplot::keyaction::press) { this->show_gui = this->show_gui ? false : true; }
        if (_key == mplot::key::h && action == mplot::keyaction::press) { std::cout << "Esc: Toggle GUI window\n"; }
    }
    void mouse_button_callback (int button, int action, int mods = 0)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent (button, (action > 0));
        if (!io.WantCaptureMouse) { mplot::VisualBase<>::mouse_button_callback (button, action, mods); }
    }
};

// Remove characters that aren't part of a string representation of an integer or floating point
// number or a comma separating elements
static constexpr std::string_view chars_num {CHARS_NUMERIC",.-e"};
void conditionAsNumeric (std::string& str)
{
    std::string::size_type ptr = std::string::npos;
    while ((ptr = str.find_last_not_of (chars_num, ptr)) != std::string::npos) {
        str = str.erase (ptr, 1);
        ptr--;
    }
}

void add_visualmodels (imgui_visual& v)
{
    bool have_coord = false;
    sm::vec<float, 3> coord;
    bool have_tri = false;
    sm::vec<sm::vec<float, 3>, 3> tri;
    bool have_vector = false;
    sm::vec<sm::vec<float, 3>, 2> vectr; // start, dirn

    // Process v.geom_text - is it three coords or one coord?
    std::vector<std::string> p1 = mplot::tools::stringToVector (v.geom_text, "),(");
    std::vector<std::string> p2;
    if (p1.size() == 1) {
        p2 = mplot::tools::stringToVector (p1[0], ",");
        // p2 should now have 3 elements
        if (p2.size() == 3) {
            for (int i = 0; i < 3; ++i) {
                conditionAsNumeric (p2[i]);
                coord[i] = std::atof (p2[i].c_str());
                have_coord = true;
            }
        }
    } else if (p1.size() == 2) {
        for (int i = 0; i < 2; ++i) {
            p2 = mplot::tools::stringToVector (p1[i], ",");
            for (int j = 0; j < 3; ++j) {
                conditionAsNumeric (p2[j]);
                vectr[i][j] = std::atof (p2[j].c_str());
            }
        }
        have_vector = true;

    } else if (p1.size() == 3) {
        for (int i = 0; i < 3; ++i) {
            p2 = mplot::tools::stringToVector (p1[i], ",");
            for (int j = 0; j < 3; ++j) {
                conditionAsNumeric (p2[j]);
                tri[i][j] = std::atof (p2[j].c_str());
            }
        }
        have_tri = true;
    }

    // Add TriVisual or SphereVisual here
    if (have_coord) {
        std::cout << "Draw sphere\n";
        auto sv = std::make_unique<mplot::SphereVisual<>>(coord, v.thickness, v.clr);
        v.bindmodel (sv);
        sv->setAlpha (0.5f);
        sv->finalize();
        v.addVisualModel (sv);
        v.setSceneTrans (-coord + sm::vec<>{0,0,-2});
    }
    if (have_vector) {
        std::cout << "Draw vector\n";
        auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(vectr[0]);
        v.bindmodel (vvm);
        vvm->thevec = vectr[1];
        vvm->vgoes = mplot::VectorGoes::FromOrigin;
        vvm->thickness = v.thickness * 0.5f; // A bit thinner than spheres
        vvm->arrowhead_prop = 0.1f;
        vvm->fixed_colour = true;
        vvm->single_colour = v.clr;
        vvm->setAlpha (0.5f);
        vvm->finalize();
        v.addVisualModel (vvm);
        v.setSceneTrans (-vectr[0] + sm::vec<>{0,0,-2});
    }
    if (have_tri) {
        std::cout << "Draw triangle\n";
        auto tv = std::make_unique<mplot::TriangleVisual<>> (sm::vec<>{}, tri[0], tri[1], tri[2], v.clr);
        v.bindmodel (tv);
        tv->finalize();
        v.addVisualModel (tv);
        v.setSceneTrans (-(tri[0] + tri[1] + tri[2]) / 3.0f + sm::vec<>{0,0,-2});
    }

    v.needs_visualmodel_rebuild = false;
}

int main()
{
    imgui_visual v(1536, 1536, "Geometry debug");
    v.lightingEffects (true);
    v.rotateAboutNearest (true);

    // Display until user closes window
    while (!v.readyToFinish()) {
        v.waitevents (0.018); // Wait or poll for events
        if (v.needs_visualmodel_rebuild == true) { add_visualmodels (v); }
        v.render();           // Render mathplot objects
        v.gui_draw();         // Render Dear ImGui frame(s)
        v.swapBuffers();      // Swap buffers
    }
}
