#pragma once

#include <Includes/Defs.h>
#include <nlohmann/json.hpp>
#include <Mesh/Mesh.h>
class Scene
{
public:
    Scene(std::string name);
    ~Scene();

    void update(std::string name, Camera cam, Mesh mesh, std::unordered_map<std::string, bool> states,
                std::unordered_map<std::string, float> values);
    void saveToFile(std::string filePath);
    nlohmann::json loadFile(std::string filePath);

private:
    std::string m_name;
    Camera                                m_cam;
    Mesh m_mesh;
    std::unordered_map<std::string, bool> m_states;
    std::unordered_map<std::string, float> m_values;
};
