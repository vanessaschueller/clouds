#include <Scene/Scene.h>

Scene::Scene(std::string name) { m_name = name; }

Scene::~Scene() {}

void Scene::update(std::string name, Camera cam, Mesh mesh, std::unordered_map<std::string, bool> states,
                   std::unordered_map<std::string, float> values)
{
    m_name   = name;
    m_cam    = cam;
    m_mesh   = mesh;
    m_states = states;
    m_values = values;
}

void Scene::saveToFile(std::string filePath)
{
    nlohmann::json json;
    json["camPos"]     = {m_cam.getCamPos().x, m_cam.getCamPos().y, m_cam.getCamPos().z};
    json["speed"]      = m_cam.getSpeed();
    json["theta"]      = m_cam.getTheta();
    json["phi"]        = m_cam.getPhi();
    json["direction"]  = {m_cam.getDirection().x, m_cam.getDirection().y, m_cam.getDirection().z};
    json["up"]         = {m_cam.getUp().x, m_cam.getUp().y, m_cam.getUp().z};
    json["ratio"]      = m_cam.getRatio();
    json["viewMatrix"] = {
            m_cam.getView()[0][0],
            m_cam.getView()[0][1],
            m_cam.getView()[0][2],
            m_cam.getView()[0][3],
            m_cam.getView()[1][0],
            m_cam.getView()[1][1],
            m_cam.getView()[1][2],
            m_cam.getView()[1][3],
            m_cam.getView()[2][0],
            m_cam.getView()[2][1],
            m_cam.getView()[2][2],
            m_cam.getView()[2][3],
            m_cam.getView()[3][0],
            m_cam.getView()[3][1],
            m_cam.getView()[3][2],
            m_cam.getView()[3][3],
    };
    json["projectionMatrix"] = {
            m_cam.getProj()[0][0],
            m_cam.getProj()[0][1],
            m_cam.getProj()[0][2],
            m_cam.getProj()[0][3],
            m_cam.getProj()[1][0],
            m_cam.getProj()[1][1],
            m_cam.getProj()[1][2],
            m_cam.getProj()[1][3],
            m_cam.getProj()[2][0],
            m_cam.getProj()[2][1],
            m_cam.getProj()[2][2],
            m_cam.getProj()[2][3],
            m_cam.getProj()[3][0],
            m_cam.getProj()[3][1],
            m_cam.getProj()[3][2],
            m_cam.getProj()[3][3],
    };
    auto path           = m_mesh.getPath();
    auto marker         = path.find_last_of("/\\");
    auto fileEnding     = path.substr(marker + 1);
    json["filePath"]    = fileEnding;

    json["modelMatrix"] = {
            m_mesh.getMod()[0][0],
            m_mesh.getMod()[0][1],
            m_mesh.getMod()[0][2],
            m_mesh.getMod()[0][3],
            m_mesh.getMod()[1][0],
            m_mesh.getMod()[1][1],
            m_mesh.getMod()[1][2],
            m_mesh.getMod()[1][3],
            m_mesh.getMod()[2][0],
            m_mesh.getMod()[2][1],
            m_mesh.getMod()[2][2],
            m_mesh.getMod()[2][3],
            m_mesh.getMod()[3][0],
            m_mesh.getMod()[3][1],
            m_mesh.getMod()[3][2],
            m_mesh.getMod()[3][3],
    };

    for(auto pair : m_states)
    {
        json[pair.first] = pair.second;
    }
    for(auto pair : m_values)
    {
        json[pair.first] = pair.second;
    }

    std::ofstream file(filePath+".json");
    file << json;
}

nlohmann::json Scene::loadFile(std::string filePath)
{
    std::ifstream  i(filePath);
    nlohmann::json j;
    i >> j;

    return j;
}
