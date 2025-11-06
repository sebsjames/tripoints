/*
 * Visualize some graphs as an example use of sebsjames/mathplot
 */
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

// In this example, we extend mplot::Visual to add the ability to display a Dear ImGui frame in our window
struct imgui_visual final : public mplot::Visual<>
{
    // In our constructor, we carry out the ImGui setup
    imgui_visual (int width, int height, const std::string& title) : mplot::Visual<> (width, height, title)
    {
        // Additional Dear ImGui setup
        this->setContext(); // Set the OpenGL context before Dear ImGui initialization
        this->renderSwapsBuffers (false); // With Dear ImGui, we manually swapBuffers(), so set this false
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        ImGui_ImplGlfw_InitForOpenGL (this->window, true);
        ImGui_ImplOpenGL3_Init();

        // Because mplot::Visual provides fonts, we can load them into Dear ImGui. This
        // program will have created /tmp/DejaVuSans.ttf as it first loads.
        constexpr int size_in_pixels = 24;
        io.Fonts->AddFontFromFileTTF ("/tmp/DejaVuSans.ttf", size_in_pixels);
    }

    // Draw the GUI frame for your Visual. This frame then updates the state stored in
    // your visual and you can update your graphs/visualizations accordingly
    void gui_draw()
    {
        if (this->show_gui == true) {
            // The Dear ImGui scheme seems to be to rebuild the frame on each rendering
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Our Options window
            ImGui::Begin("Options (Esc to toggle)");
            if (ImGui::SliderFloat("Thickness", &this->thickness, 0.0f, 0.3f)) { }
            if (ImGui::ColorEdit3("Colour", this->clr.data())) {
            }
            static char buf1[512] = "";
            if (ImGui::InputText("Coords/Tri", buf1, IM_ARRAYSIZE(buf1))) {
                this->geom_text = std::string (buf1);
            }

            if (ImGui::Button("Draw")) { needs_visualmodel_rebuild = true; }
            if (ImGui::Button("Clear")) { this->vm.clear(); }
            ImGui::End();

            // Now render
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());
        }
    }

    /*
     * Public members - state for the GUI
     */

    // Show the GUI?
    bool show_gui = true;

    // Some text fields for the gui
    std::string geom_text = "";

    float thickness = 0.15f;

    std::array<float, 3> clr = { 1.0f, 0.0f, 0.2f };

    // Do the mplot::VisualModels need to be re-built?
    bool needs_visualmodel_rebuild = true;

protected:
    // This is the mathplot method for binding keys
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        // Use escape to toggle the GUI
        if (key == mplot::key::escape && action == mplot::keyaction::press) { this->show_gui = this->show_gui ? false : true; }

        if (key == mplot::key::h && action == mplot::keyaction::press) {
            std::cout << "ImGui extra help:\n";
            std::cout << "Esc: Toggle GUI window\n";
        }
    }
    // We have to overload the mouse_button_callback, so that mouse press events that
    // occur on the ImGui frame are discarded
    void mouse_button_callback (int button, int action, int mods = 0)
    {
        ImGuiIO& io = ImGui::GetIO();
        // Forward to ImGui
        io.AddMouseButtonEvent (button, (action > 0));
        // If ImGui not focussed, do usual mathplot callback
        if (!io.WantCaptureMouse) {
            mplot::VisualBase<>::mouse_button_callback (button, action, mods);
        }
    }
};

static constexpr std::string_view chars_num {CHARS_NUMERIC",."};
void conditionAsNumeric (std::string& str)
{
    std::string::size_type ptr = std::string::npos;
    while ((ptr = str.find_last_not_of (chars_num, ptr)) != std::string::npos) {
        str = str.erase (ptr, 1);//str[ptr] = '_'; // Replacement character
        ptr--;
    }
}

void add_visualmodels (imgui_visual& v)
{
    bool have_coord = false;
    sm::vec<float, 3> coord;
    bool have_tri = false;
    sm::vec<sm::vec<float, 3>, 3> tri;

    // Process v.geom_text - is it three coords or one coord?
    std::vector<std::string> p1 = mplot::tools::stringToVector (v.geom_text, "),(");
    std::vector<std::string> p2;
    if (p1.size() == 1) {
        std::cout << "1. 1p1[0] = " << p1[0] << "\n";
        p2 = mplot::tools::stringToVector (p1[0], ",");
        // p2 should now have 3 elements
        if (p2.size() == 3) {
            for (int i = 0; i < 3; ++i) {
                conditionAsNumeric (p2[i]);
                coord[i] = std::atof (p2[i].c_str());
                have_coord = true;
            }
        }
    } else if (p1.size() == 3) {
        std::cout << "3: " << p1[0] << "; " << p1[1] << "; " << p1[2] << "\n";
        for (int i = 0; i < 3; ++i) {
            p2 = mplot::tools::stringToVector (p1[i], ",");
            for (int j = 0; j < 3; ++j) {
                conditionAsNumeric (p2[j]);
                tri[i][j] = std::atof (p2[j].c_str());
                std::cout << "tri["<<i<<"]["<<j<<"] = " << tri[i][j]
                          << " (from " << p2[j] << ")\n";
            }
        }
        have_tri = true;
    } else {
        std::cout << "else\n";
    }

    // Add TriVisual or SphereVisual here
    if (have_coord) {
        std::cout << "Draw sphere\n";
        auto sv = std::make_unique<mplot::SphereVisual<>>(coord, v.thickness, v.clr);
        v.bindmodel (sv);
        sv->finalize();
        v.addVisualModel (sv);
    }
    if (have_tri) {
        std::cout << "Draw tri\n";
        auto tv = std::make_unique<mplot::TriangleVisual<>> (sm::vec<>{}, tri[0], tri[1], tri[2], v.clr);
        v.bindmodel (tv);
        tv->finalize();
        v.addVisualModel (tv);
    }

    v.needs_visualmodel_rebuild = false;
}

int main()
{
    imgui_visual v(1536, 1536, "Triangles and points");
    v.lightingEffects (true);

    // Display until user closes window
    while (!v.readyToFinish()) {
        v.waitevents (0.018); // Wait or poll for events
        if (v.needs_visualmodel_rebuild == true) { add_visualmodels (v); }
        v.render();           // Render mathplot objects
        v.gui_draw();         // Render Dear ImGui frame(s)
        v.swapBuffers();      // Swap buffers
    }
}
