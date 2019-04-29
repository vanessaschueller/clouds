#pragma once

#include <Buffer/SSBO.h>
#include <Includes/Defs.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <unordered_map>

class Mesh
{
    struct Geometry
    {
        void createBuffers();

        std::vector<unsigned int> mIndices;
        std::vector<glm::vec4>    mVertices;
        std::vector<glm::vec2>    mUVs;
        std::vector<glm::vec3>    mNormals;

        GLuint m_elementBuffer;
        GLuint m_vertexBuffer;
        GLuint m_normalBuffer;
        GLuint m_uvBuffer;
        GLuint m_vao;

        void render();
    };

public:
    Mesh();
    Mesh(std::string pathToFile);
    Mesh(GLFWwindow* window, std::string pathToFile);
    ~Mesh();

    void init(GLFWwindow* window, std::string pathToFile);
    void render();
    void renderBB();

    void      update();
    glm::mat4 getMod();
    glm::vec3 getVoxelResolution();
    glm::vec3 getMoveDirection();

    std::unordered_map<std::string, int> getBB();

    void   calculateBoundingBox();
    glm::vec3 createDistanceField(unsigned int maxResolution);

    std::vector<aiMesh*> getMeshes();
    std::string          getPath();
    void                 setModelMat(glm::mat4 modelMat);

private:
    void loadObj(std::string path);

	std::string                          m_path;
    std::unordered_map<std::string, int> bbCoordinates;
    std::vector<Geometry>                m_geometries;

    GLuint m_elementBufferBB;
    GLuint m_vertexBufferBB;
    GLuint m_normalBufferBB;
    GLuint m_uvBufferBB;
    GLuint m_vaoBB;

    glm::vec3  m_bbSize;
    glm::uvec3 m_voxelResolution;

    glm::mat4 m_modelMatrix;

    glm::vec3 m_moveDirection;
    int       m_maxResolution;
    glm::vec3 m_vDim;

    glm::uvec3 getCubicVoxelResolution(unsigned int maxResolution);
    void       voxelizeMesh();

    std::vector<aiMesh*> m_meshes;
    GLFWwindow*          m_window;
    Texture              grid;
    Texture              gradientGrid;
    Texture              smoothGrid;
    Texture              windGrid;
    Texture              flowGrid;
};