#include <Includes/Defs.h>
#include <Mesh/Mesh.h>
#include <Noise/NoiseGenerator.h>
#include <Scene/Scene.h>
#include <Shader/ComputeShader.h>
#include <Shader/FBO.h>
#include <Shader/ScreenFillingQuad.h>
#include <Texture/Texture.h>
#include <tinyfile/tinyfiledialogs.h>
#include <unordered_map>

int width  = 1280;
int height = 720;

/*
 * CAM & SCENE
 */
float                                  fogAmount = 0.2;
glm::vec4                              clearColor(0.0);
GLFWwindow*                            window;
std::unique_ptr<Camera>                camera = nullptr;
std::unordered_map<std::string, bool>  states;
std::unordered_map<std::string, float> values;

/*
 * SAMPLING
 */
int maxSample = 98;
int minSample = 32;
int stepCount = 4;

/*
 * CLOUD PARAMS
 */
float cloudSpeed     = 10.0f;
float coverageM      = 0.4094f;
float coverageH      = 0.5f;
float amountErosion  = 0.537f;
float stratus        = 1.0f;
float stratuscumulus = 0.0f;
float cumulus        = 0.0f;
float absorption     = 0.0f;
bool  editClouds     = false;
bool  userDefined    = false;
bool  useWind        = false;
bool  pause          = false;
bool  highRes        = true;
bool  powder         = true;
bool  beersLaw       = true;
bool  scattering     = true;
bool  smooth         = true;

/*
 * TEXTURES
 */
Texture weather;
Texture worl;
Texture perl;
Texture cirrusCloud;
Texture cirrusCloud2;
Texture cirrusCloud3;
Texture turbulence;
Texture rock;
Texture rockNormal;
GLuint  flowID = 0;
GLuint  windID = 0;
GLuint  sdfID  = 0;

/*
 * LIGHT
 */
bool      lensFlare        = false;
bool      addSunGlare      = false;
bool      editGodRays      = false;
bool      editScattering   = false;
bool      userDefinedSky   = false;
bool      godrays          = true;
float     silverIntensity  = 0.7f;
float     silverSpread     = 1.2f;
float     eccentricity     = 0.1f;
float     betaM            = 0.0f;
float     sunpowerMie      = 0.025f;
float     sunpowerRayleigh = 0.4f;
float     angle            = 0.0f;
float     sunColor[3]      = {0.97f, 0.83f, 0.75f};
glm::vec3 lightDir(0.f, 1.f, 0.f);
glm::mat3 rotation(1.0f);

/*
 * MESH
 */
Mesh                                 mesh;
glm::vec3                            voxelRes;
std::unordered_map<std::string, int> bb;
bool                                 showBB = false;

/*
 * POST PROCESSING
 */
float ppDecay      = 1.0f;
float ppDensity    = 1.0f;
float ppWeight     = 0.19f;
float ppExposure   = 0.05f;
int   ppNumSamples = 64;

/*
 * DEBUG
 */
int    vectorLength = 1;
double iTime;
double lastTime      = glfwGetTime();
bool   showNormals   = false;
bool   onlyFirstPass = false;
bool   nocutscene    = false;
bool   densheight    = true;
bool   applyWeather  = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////

void charCallback(GLFWwindow* window, unsigned int key)
{
    switch(key)
    {
    case '1':
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case '2':
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    }
}

void resizeCallback(GLFWwindow* window, int w, int h)
{
    camera->setWidthHeight(w, h);
    camera->setViewport(0, 0, w, h);
    glViewport(0, 0, w, h);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(ImGui::GetIO().WantCaptureMouse)
    {
        return ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
    else
    {
        float fov = camera->getFOV();
        camera->setFOV(fov - yoffset);
    }
}

void initGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport /
                                                          // Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void renderGui(float deltaTime, glm::vec3 print)
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
    opt_flags |= ImGuiDockNodeFlags_PassthruDockspace;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport*   viewport     = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    if(opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Menu", NULL, window_flags);

    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
    }
    ImGui::End();

    ImGui::Begin("CLOUD MENU", NULL, 0);
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Load scene"))
            {
                nlohmann::json j;
                const char*    filterPatterns[1] = {"*.json"};
                const char*    file              = tinyfd_openFileDialog("Open scene...", nullptr, 1, filterPatterns, "Scene Files", false);
                bool           fail              = file == nullptr;
                if(!fail)
                {
                    try
                    {
                        Scene scene(file);
                        j = scene.loadFile(file);
                        // VALUES
                        coverageM     = j["coverageM"];
                        coverageH     = j["coverageH"];
                        amountErosion = j["erosion"];
                        angle         = j["angle"];
                        fogAmount     = j["fogAmount"];
                        absorption    = j["absorption"];
                        float r       = j["sunRed"];
                        float g       = j["sunGreen"];
                        float b       = j["sunBlue"];
                        sunColor[0]   = r;
                        sunColor[1]   = g;
                        sunColor[2]   = b;

                        // BOOLS
                        addSunGlare    = j["sunGlare"];
                        editGodRays    = j["editGodRays"];
                        editClouds     = j["editClouds"];
                        highRes        = j["highResolution"];
                        powder         = j["powder"];
                        beersLaw       = j["beersLaw"];
                        scattering     = j["scattering"];
                        smooth         = j["smooth"];
                        userDefined    = j["userDefinedCloudType"];
                        useWind        = j["useWind"];
                        showNormals    = j["showNormals"];
                        userDefinedSky = j["userDefinedSkyParams"];
                        godrays        = j["godrays"];
                        lensFlare      = j["lensFlare"];
                        editScattering = j["editScattering"];
                        nocutscene     = j["nocutscene"];
						

                        // VALUES IF DEFINED
                        if(editGodRays)
                        {
                            ppDecay    = j["decay"];
                            ppDensity  = j["density"];
                            ppWeight   = j["weight"];
                            ppExposure = j["exposure"];
                        }
                        if(userDefined)
                        {
                            stratus        = j["stratus"];
                            stratuscumulus = j["stratuscumulus"];
                            cumulus        = j["cumulus"];
                        }
                        if(useWind)
                        {
                            cloudSpeed = j["windSpeed"];
                            pause      = j["pause"];
                        }

                        if(userDefinedSky)
                        {
                            betaM            = j["betaM"];
                            sunpowerMie      = j["sunpowerMie"];
                            sunpowerRayleigh = j["sunpowerRayleigh"];
                        }

                        if(editScattering)
                        {
                            silverIntensity = j["silverIntensity"];
                            silverSpread    = j["silverSpread"];
                            eccentricity    = j["eccentricity"];
                        }

                        std::string meshFile = j["filePath"];
                        std::string fullPath = CLOUDS_RESOURCES_PATH "/" + meshFile;
                        mesh.init(window, fullPath);
                        bb = mesh.getBB();
                        mesh.setModelMat(glm::mat4(static_cast<float>(j["modelMatrix"][0]),
                                                   static_cast<float>(j["modelMatrix"][1]),
                                                   static_cast<float>(j["modelMatrix"][2]),
                                                   static_cast<float>(j["modelMatrix"][3]),
                                                   static_cast<float>(j["modelMatrix"][4]),
                                                   static_cast<float>(j["modelMatrix"][5]),
                                                   static_cast<float>(j["modelMatrix"][6]),
                                                   static_cast<float>(j["modelMatrix"][7]),
                                                   static_cast<float>(j["modelMatrix"][8]),
                                                   static_cast<float>(j["modelMatrix"][9]),
                                                   static_cast<float>(j["modelMatrix"][10]),
                                                   static_cast<float>(j["modelMatrix"][11]),
                                                   static_cast<float>(j["modelMatrix"][12]),
                                                   static_cast<float>(j["modelMatrix"][13]),
                                                   static_cast<float>(j["modelMatrix"][14]),
                                                   static_cast<float>(j["modelMatrix"][15])));


						
                        auto ids = mesh.createDistanceField(100);
                        windID   = ids[0];
                        flowID   = ids[1];
                        sdfID    = ids[2];
                        voxelRes = mesh.getVoxelResolution();
                    }
                    catch(std::exception&)
                    {
                        fail = true;
                    }
                }
                if(fail)
                {
                    tinyfd_notifyPopup("Open scene", "Please specify a valid file!", "error");
                }
            }
            if(ImGui::MenuItem("Save scene"))
            {
				//VALUES
                values.clear();
                values.insert({"coverageM", coverageM});
                values.insert({"coverageH", coverageH});
                values.insert({"erosion", amountErosion});
                values.insert({"angle", angle});                
				values.insert({"fogAmount", fogAmount});
                values.insert({"absorption", absorption});
                values.insert({"sunRed", sunColor[0]});
                values.insert({"sunGreen", sunColor[1]});
                values.insert({"sunBlue", sunColor[2]});

				//BOOLS
                states.clear();                
				states.insert({"sunGlare", addSunGlare});
                states.insert({"editGodRays", editGodRays});
                states.insert({"editClouds", editClouds});
                states.insert({"highResolution", highRes});
                states.insert({"powder", powder});
                states.insert({"beersLaw", beersLaw});
                states.insert({"scattering", scattering});
                states.insert({"smooth", smooth});
                states.insert({"userDefinedCloudType", userDefined});
                states.insert({"useWind", useWind});                
				states.insert({"showNormals", showNormals});
                states.insert({"userDefinedSkyParams", userDefinedSky});
                states.insert({"godrays", godrays});
                states.insert({"lensFlare", lensFlare});
                states.insert({"editScattering", editScattering});
                states.insert({"nocutscene", nocutscene});

				//VALUES IF DEFINED                
				if(editGodRays)
                {
                    values.insert({"decay", ppDecay});
                    values.insert({"density", ppDensity});
                    values.insert({"weight", ppWeight});
                    values.insert({"exposure", ppExposure});
                }
                if(userDefined)
                {
                    values.insert({"stratus", stratus});
                    values.insert({"stratuscumulus", stratuscumulus});
                    values.insert({"cumulus", cumulus});
                }

                if(useWind)
                {
                    values.insert({"windSpeed", cloudSpeed});
                    states.insert({"pause", pause});
                }
                if(userDefinedSky)
                {
                    values.insert({"betaM", betaM});
                    values.insert({"sunpowerMie", sunpowerMie});
                    values.insert({"sunpowerRayleigh", sunpowerRayleigh});
                }
                if(editScattering)
                {
                    values.insert({"silverIntensity", silverIntensity});
                    values.insert({"silverSpread", silverSpread});
                    values.insert({"eccentricity", eccentricity});
				}


                const char* filterPatterns[1] = {"*.json"};
                const char* file              = tinyfd_saveFileDialog("Save scene...", CLOUDS_RESOURCES_PATH, 1, filterPatterns, "Scene Files");

                Scene scene(file);
                scene.update("test", *camera, mesh, states, values);
                scene.saveToFile(file);
            }
            if(ImGui::MenuItem("Load mesh"))
            {
                const char* filterPatterns[5] = {"*.obj", "*.fbx", "*.dae", "*.stl", "*.ply"};
                const char* file              = tinyfd_openFileDialog("Open mesh...", nullptr, 5, filterPatterns, "Mesh Files", false);
                bool        fail              = file == nullptr;
                if(!fail)
                {
                    try
                    {
                        mesh.init(window, file);
                        bb       = mesh.getBB();
                        auto ids = mesh.createDistanceField(100);
                        windID   = ids[0];
                        flowID   = ids[1];
                        sdfID    = ids[2];
                        voxelRes = mesh.getVoxelResolution();
                    }
                    catch(std::exception&)
                    {
                        fail = true;
                    }
                }
                if(fail)
                {
                    tinyfd_notifyPopup("Open mesh", "Please specify a valid file!", "error");
                }
            }
            if(ImGui::MenuItem("Load weather map"))
            {
                const char* filterPatterns[1] = {"*.bmp"};
                const char* file              = tinyfd_openFileDialog("Open weather file...", nullptr, 1, filterPatterns, "Weather Map Files", false);
                bool        fail              = file == nullptr;
                if(!fail)
                {
                    try
                    {
                        weather.load(file, 2, 3);
                    }
                    catch(std::exception&)
                    {
                        fail = true;
                    }
                }
                if(fail)
                {
                    tinyfd_notifyPopup("Open mesh", "Please specify a valid file!", "error");
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Value("FPS: ", 1 / static_cast<float>(deltaTime));
    ImGui::PlotVar("RenderTime", static_cast<float>(deltaTime) * 1000);
    ImGui::Value("Width: ", width);
    ImGui::Value("Height: ", height);
    ImGui::Separator();
    ImGui::Checkbox("NoCutScene", &nocutscene);
    ImGui::Checkbox("DensHeight", &densheight);
    ImGui::Checkbox("Weather", &applyWeather);

    // if(ImGui::Button("1", ImVec2(100, 20)))
    //{
    //    maxSample = 98;
    //    minSample = 32;
    //}
    // if(ImGui::Button("2", ImVec2(100, 20)))
    //{
    //    maxSample = 128;
    //    minSample = 64;
    //}
    // if(ImGui::Button("3", ImVec2(100, 20)))
    //{
    //    maxSample = 256;
    //    minSample = 128;
    //}
    // if(ImGui::Button("Scene1", ImVec2(100, 20)))
    //{
    //    coverageM = 0.2759;
    //}
    // if(ImGui::Button("Scene2", ImVec2(100, 20)))
    //{
    //    coverageM = 0.5103;
    //}
    // if(ImGui::Button("Scene345", ImVec2(100, 20)))
    //{
    //    coverageM = 0.3931;
    //}
    ImGui::Separator();
    ImGui::Text("CLOUD OPTIONS");
    ImGui::Checkbox("Smooth", &smooth);
    if(ImGui::CollapsingHeader("LightingStages"))
    {
        ImGui::Checkbox("BeersLaw", &beersLaw);
        ImGui::Checkbox("Powder", &powder);
        ImGui::Checkbox("Scattering", &scattering);
        ImGui::Checkbox("Godrays", &godrays);
    }
    editClouds = false;
    if(ImGui::CollapsingHeader("EditClouds") || editClouds)
    {
        editClouds = true;
        ImGui::SliderFloat("CoverageMediumClouds", &coverageM, 0.0, 1.0, "%.4f");
        ImGui::SliderFloat("CoverageHighClouds", &coverageH, 0.0, 1.0, "%.1f");
        ImGui::SliderFloat("Erosion", &amountErosion, 0.0, 5.0, "%.3f");
        ImGui::SliderFloat("Absorption", &absorption, 0.0, 10.0, "%.1f");
    }
    userDefined = false;
    if(ImGui::CollapsingHeader("UserDefinedCloudType") || userDefined)
    {
        userDefined = true;
        ImGui::SliderFloat("Stratus", &stratus, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("StratusCumulus", &stratuscumulus, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("Cumulus", &cumulus, 0.0, 1.0, "%.2f");
    }
    ImGui::Separator();
    ImGui::Text("LIGHT OPTIONS");
    ImGui::Checkbox("SunGlare", &addSunGlare);
    ImGui::Checkbox("LensFlare", &lensFlare);
    ImGui::SliderFloat("Angle", &angle, 0, 360, "%.0f");
    editScattering = false;
    if(ImGui::CollapsingHeader("EditScattering") || editScattering)
    {
        editScattering = true;
        ImGui::SliderFloat("Eccentricity", &eccentricity, -1, 1, "%.2f");
        ImGui::SliderFloat("SilverIntensity", &silverIntensity, 0, 1, "%.1f");
        ImGui::SliderFloat("SilverSpread", &silverSpread, 0, 2, "%.1f");
    }
    editGodRays = false;
    if(ImGui::CollapsingHeader("EditGodRays") || editGodRays)
    {
        editGodRays = true;
        ImGui::SliderFloat("Decay", &ppDecay, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("Density", &ppDensity, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("Weight", &ppWeight, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("Exposure", &ppExposure, 0.0, 1.0, "%.2f");
    }
    ImGui::Separator();
    ImGui::Text("ATMOSPHERE OPTIONS");
    ImGui::SliderFloat("FogAmount", &fogAmount, 0.0, 1.0, "%.2f");
    if(ImGui::CollapsingHeader("Wind"))
    {
        ImGui::Checkbox("TurnOnWind", &useWind);
        ImGui::Checkbox("PauseWind", &pause);
        if(ImGui::Button("ResetTime", ImVec2(100, 20)))
        {
            lastTime = 0.0;
        }
        ImGui::SliderFloat("WindSpeed", &cloudSpeed, 0.0, 200.0, "%.0f");
    }
    userDefinedSky = false;
    if(ImGui::CollapsingHeader("UserDefinedSkyParams") || userDefinedSky)
    {
        userDefinedSky = true;
        ImGui::SliderFloat("BetaM", &betaM, 0.0, 200.0, "%.2f");
        ImGui::SliderFloat("SunpowerMie", &sunpowerMie, 0.0, 1.0, "%.2f");
        ImGui::SliderFloat("SunpowerRayleigh", &sunpowerRayleigh, 0.0, 1.0, "%.2f");
        ImGui::ColorEdit3("Suncolor", sunColor);
    }
    ImGui::Separator();
    ImGui::Text("DEBUG");
    ImGui::Checkbox("ShowBB", &showBB);
    ImGui::Checkbox("OnlyFirstPass", &onlyFirstPass);
    ImGui::SliderInt("MaxSamples", &maxSample, 32, 256);
    ImGui::SliderInt("MinSamples", &minSample, 32, 256);
    showNormals = false;
    if(ImGui::CollapsingHeader("ShowNormals") || showNormals)
    {
        showNormals = true;
        ImGui::SliderInt("VectorLength", &vectorLength, 0, 10);
    }
    ImGui::Separator();

    ImGui::End();
    ImGui::Begin("TEXTURES");
    ImGui::Text("WEATHER MAP");
    ImGui::Image(ImTextureID(weather.getTextureID()), ImVec2(100, 100));
    ImGui::Text("CLOUD TEXTURES");
    ImGui::Image(ImTextureID(cirrusCloud.getTextureID()), ImVec2(100, 100));
    ImGui::Image(ImTextureID(cirrusCloud2.getTextureID()), ImVec2(100, 100));
    ImGui::Image(ImTextureID(cirrusCloud3.getTextureID()), ImVec2(100, 100));
    ImGui::Text("OBJECT TEXTURES");
    ImGui::Image(ImTextureID(rock.getTextureID()), ImVec2(100, 100));
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void endGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int main()
{
    /*
     *Window definitions
     */
    glfwInit();

    window = glfwCreateWindow(width, height, "Realtime Clouds", nullptr, nullptr);
    glfwSetWindowPos(window, 100, 50);
    glfwMakeContextCurrent(window);
    glfwSetCharCallback(window, charCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);

    glfwSwapInterval(0); // vsync
    glewInit();
    glClearColor(1, 1, 1, 0);
    glEnable(GL_DEPTH_TEST);

    initGui();

    /*
     *Camera definitions
     */
    camera = std::make_unique<Camera>(window, width, height);
    camera->setCamPos(&glm::vec3(0.f, 0, 0.f));
    camera->setLookAt(&glm::vec3(0.f, 0, 0), &glm::vec3(0.f, 0, 10));
    camera->setUpvector(&glm::vec3(0.f, 1.f, 0.f));
    camera->setWidthHeight(width, height);

    /*
     *Load meshes
     */
    mesh.init(window, CLOUDS_RESOURCES_PATH "/sphereHighRes.obj");
    bb       = mesh.getBB();
    auto ids = mesh.createDistanceField(200);
    windID   = ids[0];
    flowID   = ids[1];
    sdfID    = ids[2];
    voxelRes = mesh.getVoxelResolution();
    /*
     *Shader definitions
     */
    Shader debugShader;
    debugShader.initFromFiles(CLOUDS_SHADERS_PATH "/Debug.vert", CLOUDS_SHADERS_PATH "/Debug.geom", CLOUDS_SHADERS_PATH "/Debug.frag");

    Shader simpleShader;
    simpleShader.initFromFiles(CLOUDS_SHADERS_PATH "/Color.vert", CLOUDS_SHADERS_PATH "/Color.frag");
    /*
     *Compute noises
     */
    NoiseGenerator noise;
    // noise.generateNoises();

    weather.load(CLOUDS_RESOURCES_PATH "/weather.bmp", 2, 3);
    worl.load(CLOUDS_RESOURCES_PATH "/worlnoise.bmp", 3, 3);
    perl.load(CLOUDS_RESOURCES_PATH "/perlworlnoise.tga", 3, 4);
    cirrusCloud.load(CLOUDS_RESOURCES_PATH "/cirrus.bmp", 2, 3);
    cirrusCloud2.load(CLOUDS_RESOURCES_PATH "/cirrus2.bmp", 2, 3);
    cirrusCloud3.load(CLOUDS_RESOURCES_PATH "/cirrus3.bmp", 2, 3);
    turbulence.load(CLOUDS_RESOURCES_PATH "/WavyTurbulence.bmp", 2, 3);
    rock.load(CLOUDS_RESOURCES_PATH "/mountain.bmp", 2, 3);
    rockNormal.load(CLOUDS_RESOURCES_PATH "/NormalMap.bmp", 2, 3);
    Texture blueNoise(CLOUDS_RESOURCES_PATH "/BlueNoise.bmp", 2, 3);
    /*
     * initialize FBO and ScreenFillingQuad
     */
    FBO               fbo(width, height, 1, true, false);
    ScreenFillingQuad quad(CLOUDS_SHADERS_PATH "/clouds.frag");
    Shader            quadShader = quad.getShader();

    FBO               cloudFbo(width, height, 4, true, false);
    ScreenFillingQuad post(CLOUDS_SHADERS_PATH "/postProcessing.frag");
    Shader            postShader = post.getShader();

    FBO               smoothFbo(width, height, 1, true, false);
    ScreenFillingQuad smoothQuad(CLOUDS_SHADERS_PATH "/smoothClouds.frag");
    Shader            smoothShader = smoothQuad.getShader();
    /*
     *Render Loop
     */
    iTime = glfwGetTime();
    states.clear();

    bool  initialised = false;
    int   nbFrames    = 0;
    int   currentWidth;
    int   currentHeight;
    float previousAngle, currentAngle;

    while(!glfwWindowShouldClose(window))
    {
        glfwGetWindowSize(window, &currentWidth, &currentHeight);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::vec3 bbmin     = glm::vec3(bb.at("xmin"), bb.at("ymin"), bb.at("zmin"));
        glm::vec3 bbmax     = glm::vec3(bb.at("xmax"), bb.at("ymax"), bb.at("zmax"));
        auto      dbgVec    = glm::vec3(mesh.getMod() * glm::vec4(bbmax, 1.0));
        double    deltaTime = glfwGetTime() - iTime;
        iTime               = glfwGetTime();
        double currentTime  = glfwGetTime();
        nbFrames++;
        previousAngle = angle;
        if(currentTime - lastTime >= 0.05)
        {
            if(!pause)
                lastTime += 0.05;
        }
        camera->update(deltaTime);

        mesh.update();

        rotation = glm::mat3(cos(glm::radians(angle)), -sin(glm::radians(angle)), 0, sin(glm::radians(angle)), cos(glm::radians(angle)), 0, 0, 0, 1);

        if(showNormals)
        {
            debugShader.use();
            debugShader.setInt("vectorLength", vectorLength);
            debugShader.setMat4("viewMatrix", camera->getView());
            debugShader.setMat4("projectionMatrix", camera->getProj());
            debugShader.setMat4("modelMatrix", mesh.getMod());

            mesh.render();
        }

        else
        {
            fbo.bind();
            if(currentWidth != width || currentHeight != height)
            {
                fbo.resize(currentWidth, currentHeight);
            }
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            simpleShader.use();
            simpleShader.setBool("simpleColor", false);
            simpleShader.setVec2("iResolution", glm::vec2(width, height));
            simpleShader.setVec3("lightPos", rotation * lightDir * 149600000.f);
            simpleShader.setVec3("camPos", camera->getCamPos());
            simpleShader.setMat4("view", camera->getView());
            simpleShader.setMat4("projection", camera->getProj());
            simpleShader.setMat4("model", mesh.getMod());
            simpleShader.setSampler2D("colorTexture", rock.getTextureID(), 0);
            simpleShader.setSampler2D("normalTexture", rockNormal.getTextureID(), 1);
            mesh.render();

            if(showBB)
            {
                simpleShader.use();
                simpleShader.setBool("simpleColor", true);
                simpleShader.setMat4("viewMatrix", camera->getView());
                simpleShader.setMat4("projectionMatrix", camera->getProj());
                simpleShader.setMat4("modelMatrix", mesh.getMod());

                mesh.renderBB();
            }
            fbo.unbind();

            cloudFbo.bind();
            if(currentWidth != width || currentHeight != height)
            {
                cloudFbo.resize(currentWidth, currentHeight);
            }
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            quadShader.use();
            quadShader.setBool("nocutscene", nocutscene);
            quadShader.setBool("onlyFirstPass", onlyFirstPass);
            quadShader.setBool("lensFlare", lensFlare);
            quadShader.setBool("densHeight", densheight);
            quadShader.setBool("weather", applyWeather);
            quadShader.setInt("userCloudType", userDefined);
            quadShader.setInt("highRes", highRes);
            quadShader.setInt("useWind", useWind);
            quadShader.setInt("usePowder", powder);
            quadShader.setInt("useBeerLaw", beersLaw);
            quadShader.setInt("useScattering", scattering);
            quadShader.setInt("addSunGlare", addSunGlare);
            quadShader.setInt("maxSample", maxSample);
            quadShader.setInt("minSample", minSample);
            quadShader.setInt("stepCount", stepCount);
            quadShader.setFloat("iTime", lastTime);
            quadShader.setFloat("coverageMultiplier", coverageM);
            quadShader.setFloat("coverageMultiplierHigh", coverageH * 3);
            quadShader.setFloat("erosionMultiplier", amountErosion);
            quadShader.setFloat("cloudSpeed", cloudSpeed * 10);
            quadShader.setFloat("fogAmount", fogAmount);
            quadShader.setFloat("strat", stratus);
            quadShader.setFloat("stratcum", stratuscumulus);
            quadShader.setFloat("cum", cumulus);
            quadShader.setFloat("betaM", betaM);
            quadShader.setFloat("sunpowerMie", sunpowerMie);
            quadShader.setFloat("sunpowerRayleigh", sunpowerRayleigh);
            quadShader.setFloat("silverIntensity", silverIntensity);
            quadShader.setFloat("silverSpread", silverSpread);
            quadShader.setFloat("eccentricity", eccentricity);
            quadShader.setFloat("absorption", absorption);
            quadShader.setVec2("iResolution", glm::vec2(width, height));
            quadShader.setVec3("lightDir", rotation * lightDir);
            quadShader.setVec3("bbmin", glm::vec3(mesh.getMod() * glm::vec4(bbmin, 1.0)));
            quadShader.setVec3("bbmax", glm::vec3(mesh.getMod() * glm::vec4(bbmax, 1.0)));
            quadShader.setVec3("voxelRes", voxelRes);
            quadShader.setVec3("camPos", camera->getCamPos());
            quadShader.setVec3("sunColor", glm::vec3(sunColor[0], sunColor[1], sunColor[2]));
            quadShader.setVec3("moveDirection", mesh.getMoveDirection());
            quadShader.setVec3("windDirection", glm::normalize(glm::vec3(1, 0, 0)));
            quadShader.setMat4("invView", glm::inverse(camera->getView()));
            quadShader.setMat4("invProj", glm::inverse(camera->getProj()));
            quadShader.setMat4("viewMatrix", camera->getView());
            quadShader.setMat4("projMatrix", camera->getProj());
            quadShader.setSampler2D("highClouds", cirrusCloud.getTextureID(), 8);
            quadShader.setSampler2D("highClouds2", cirrusCloud2.getTextureID(), 10);
            quadShader.setSampler2D("highClouds3", cirrusCloud3.getTextureID(), 11);
            quadShader.setSampler2D("turbulence", turbulence.getTextureID(), 9);
            quadShader.setSampler2D("weatherMap", weather.getTextureID(), 2);
            quadShader.setSampler2D("depthTexture", fbo.getDepthTexture(), 3);
            quadShader.setSampler2D("colorTexture", fbo.getColorTexture(0), 4);
            quadShader.setSampler2D("blueNoise", blueNoise.getTextureID(), 12);
            quadShader.setSampler3D("sdfTexture", sdfID, 5);
            quadShader.setSampler3D("cloudBase", perl.getTextureID(), 0);
            quadShader.setSampler3D("erodeCloud", worl.getTextureID(), 1);
            quadShader.setSampler3D("flowTexture", flowID, 6);

            quad.drawQuad();
            cloudFbo.unbind();

            smoothFbo.bind();
            if(currentWidth != width || currentHeight != height)
            {
                smoothFbo.resize(currentWidth, currentHeight);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            postShader.use();
            postShader.setBool("showGodrays", godrays);
            postShader.setInt("numSamples", ppNumSamples);
            postShader.setFloat("density", ppDensity);
            postShader.setFloat("weight", ppWeight);
            postShader.setFloat("decay", ppDecay);
            postShader.setFloat("exposure", ppExposure);
            postShader.setVec2("resolution", glm::vec2(width, height));
            postShader.setVec3("sunPositionWorld", (rotation * lightDir));
            postShader.setVec3("cameraPosition", camera->getCamPos());
            postShader.setSampler2D("cloudOutputTexture", cloudFbo.getColorTexture(0), 0);
            postShader.setSampler2D("alphaTexture", cloudFbo.getColorTexture(2), 1);
            postShader.setMat4("view", camera->getView());
            postShader.setMat4("projection", camera->getProj());
            postShader.setFloat("angle", angle);

            post.drawQuad();

            smoothFbo.unbind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            smoothShader.use();
            smoothShader.setBool("smoothClouds", smooth);
            smoothShader.setVec2("resolution", glm::vec2(width, height));
            smoothShader.setSampler2D("cloudColorTexture", smoothFbo.getColorTexture(0), 0);
            smoothShader.setSampler2D("cloudDepthTexture", cloudFbo.getColorTexture(1), 1);
            smoothShader.setSampler2D("currentDensityMap", cloudFbo.getColorTexture(3), 2);
            smoothQuad.drawQuad();
        }

        initialised = true;

        renderGui(deltaTime, dbgVec);
        width  = currentWidth;
        height = currentHeight;
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    endGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
