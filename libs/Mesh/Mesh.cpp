#include "Mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/packing.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/component_wise.hpp"
#include "glm/gtx/string_cast.hpp"
#include <array>
#include <limits>

Mesh::Mesh()
{
    m_geometries.clear();
    m_elementBufferBB = INVALID_GL_VALUE;
    m_normalBufferBB  = INVALID_GL_VALUE;
    m_vertexBufferBB  = INVALID_GL_VALUE;
    m_uvBufferBB      = INVALID_GL_VALUE;
    m_vaoBB           = INVALID_GL_VALUE;
    m_meshes.clear();
    m_modelMatrix   = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 10));
    m_maxResolution = 20.f;
    m_window        = nullptr;
    m_moveDirection = glm::vec3(0.0f);
}

Mesh::Mesh(std::string pathToFile)
{
    m_path = pathToFile;
    m_geometries.clear();
    m_elementBufferBB = INVALID_GL_VALUE;
    m_normalBufferBB  = INVALID_GL_VALUE;
    m_vertexBufferBB  = INVALID_GL_VALUE;
    m_uvBufferBB      = INVALID_GL_VALUE;
    m_vaoBB           = INVALID_GL_VALUE;
    m_meshes.clear();
    loadObj(pathToFile);
    m_modelMatrix   = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 10));
    m_maxResolution = 20.f;
    m_window        = nullptr;
    m_moveDirection = glm::vec3(0.0f);
}

Mesh::Mesh(GLFWwindow* window, std::string pathToFile)
{
    m_path = pathToFile;
    m_geometries.clear();
    m_elementBufferBB = INVALID_GL_VALUE;
    m_normalBufferBB  = INVALID_GL_VALUE;
    m_vertexBufferBB  = INVALID_GL_VALUE;
    m_uvBufferBB      = INVALID_GL_VALUE;
    m_vaoBB           = INVALID_GL_VALUE;
    m_meshes.clear();
    loadObj(pathToFile);
    m_modelMatrix   = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 10));
    m_window        = window;
    m_maxResolution = 20.f;
    m_moveDirection = glm::vec3(0.0f);
}

Mesh::~Mesh() {}

glm::mat4 Mesh::getMod() 
{ 
	return m_modelMatrix; 
}
glm::vec3 Mesh::getVoxelResolution() { return m_voxelResolution; }

std::unordered_map<std::string, int> Mesh::getBB() { return bbCoordinates; }

void Mesh::calculateBoundingBox()
{
    int xmin = std::numeric_limits<int>::max();
    int ymin = std::numeric_limits<int>::max();
    int zmin = std::numeric_limits<int>::max();
    int xmax = std::numeric_limits<int>::min();
    int ymax = std::numeric_limits<int>::min();
    int zmax = std::numeric_limits<int>::min();

    for(auto i : m_meshes)
    {
        for(unsigned int vCount = 0; vCount < i->mNumVertices; vCount++)
        {
            if(i->mVertices[vCount].x < xmin)
                xmin = i->mVertices[vCount].x;
            if(i->mVertices[vCount].y < ymin)
                ymin = i->mVertices[vCount].y;
            if(i->mVertices[vCount].z < zmin)
                zmin = i->mVertices[vCount].z;
            if(i->mVertices[vCount].x > xmax)
                xmax = i->mVertices[vCount].x;
            if(i->mVertices[vCount].y > ymax)
                ymax = i->mVertices[vCount].y;
            if(i->mVertices[vCount].z > zmax)
                zmax = i->mVertices[vCount].z;
        }
    }
    bbCoordinates.insert({"xmin", xmin - 100});
    bbCoordinates.insert({"xmax", xmax + 100});
    bbCoordinates.insert({"ymin", ymin - 100});
    bbCoordinates.insert({"ymax", ymax + 100});
    bbCoordinates.insert({"zmin", zmin - 100});
    bbCoordinates.insert({"zmax", zmax + 100});

    GLfloat vertices[] = {xmin, ymax, zmax, xmin, ymin, zmax, xmax, ymin, zmax, xmax, ymax, zmax, xmin, ymax, zmin, xmin, ymin, zmin,
                          xmin, ymin, zmax, xmin, ymax, zmax, xmax, ymax, zmax, xmax, ymin, zmax, xmax, ymin, zmin, xmax, ymax, zmin,
                          xmin, ymax, zmin, xmin, ymax, zmax, xmax, ymax, zmax, xmax, ymax, zmin, xmin, ymin, zmax, xmin, ymin, zmin,
                          xmax, ymin, zmin, xmax, ymin, zmax, xmax, ymax, zmin, xmax, ymin, zmin, xmin, ymin, zmin, xmin, ymax, zmin};

    GLfloat normals[] = {0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
                         -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
                         0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,
                         0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f};

    GLfloat texCoords[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                           0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                           0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    std::vector<glm::vec4> bbvertices;
    std::vector<glm::vec3> bbnormals;
    std::vector<glm::vec2> bbuvs;
    bbvertices.clear();
    bbnormals.clear();
    bbuvs.clear();

    for(int i = 0; i < 24; i++)
    {
        bbvertices.push_back(glm::vec4(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2], 1.0f));
        bbnormals.push_back(glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]));
        bbuvs.push_back(glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]));
    }
    std::vector<unsigned int> bbIndices;
    bbIndices.clear();
    for(int i = 0; i < 6; i++)
    {
        bbIndices.push_back(i * 4 + 0);
        bbIndices.push_back(i * 4 + 1);
        bbIndices.push_back(i * 4 + 2);
        bbIndices.push_back(i * 4 + 2);
        bbIndices.push_back(i * 4 + 3);
        bbIndices.push_back(i * 4 + 0);
    }

    glGenBuffers(1, &m_elementBufferBB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferBB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bbIndices.size() * sizeof(unsigned int), &bbIndices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_vertexBufferBB);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferBB);
    glBufferData(GL_ARRAY_BUFFER, bbvertices.size() * sizeof(glm::vec4), &bbvertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_uvBufferBB);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferBB);
    glBufferData(GL_ARRAY_BUFFER, bbuvs.size() * sizeof(glm::vec2), &bbuvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_normalBufferBB);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferBB);
    glBufferData(GL_ARRAY_BUFFER, bbnormals.size() * sizeof(glm::vec3), &bbnormals[0], GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_vaoBB);

    glBindVertexArray(m_vaoBB);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferBB);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferBB);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferBB);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferBB);

    glBindVertexArray(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

glm::uvec3 Mesh::getCubicVoxelResolution(unsigned int maxResolution)
{
    auto result        = glm::uvec3(0);
    m_bbSize           = glm::vec3(bbCoordinates.at("xmax") - bbCoordinates.at("xmin"),
                         bbCoordinates.at("ymax") - bbCoordinates.at("ymin"),
                         bbCoordinates.at("zmax") - bbCoordinates.at("zmin"));
    const float maxVal = glm::compMax(m_bbSize);
    if(maxVal == m_bbSize.x)
    {
        result.x = maxResolution;
        result.y = static_cast<unsigned int>(glm::round((m_bbSize.y / m_bbSize.x) * maxResolution));
        result.z = static_cast<unsigned int>(glm::round((m_bbSize.z / m_bbSize.x) * maxResolution));
    }
    else if(maxVal == m_bbSize.y)
    {
        result.y = maxResolution;
        result.x = static_cast<unsigned int>(glm::round((m_bbSize.x / m_bbSize.y) * maxResolution));
        result.z = static_cast<unsigned int>(glm::round((m_bbSize.z / m_bbSize.y) * maxResolution));
    }
    else if(maxVal == m_bbSize.z)
    {
        result.z = maxResolution;
        result.y = static_cast<unsigned int>(glm::round((m_bbSize.y / m_bbSize.z) * maxResolution));
        result.x = static_cast<unsigned int>(glm::round((m_bbSize.x / m_bbSize.z) * maxResolution));
    }
    return result;
}

glm::vec3 Mesh::createDistanceField(unsigned int maxResolution)
{
    m_maxResolution   = maxResolution;
    m_voxelResolution = getCubicVoxelResolution(m_maxResolution);
    m_vDim            = m_bbSize / (glm::vec3(m_voxelResolution));

    grid.init(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    grid.gen3DTexture(m_voxelResolution.x, m_voxelResolution.y, m_voxelResolution.z);

    /*
     * BRUTE GPU
     */
    std::vector<glm::vec4>    allVertices = m_geometries[m_geometries.size() - 1].mVertices;
    std::vector<glm::vec3>    allNormals  = m_geometries[m_geometries.size() - 1].mNormals;
    std::vector<unsigned int> allIndices  = m_geometries[m_geometries.size() - 1].mIndices;

    for(int i = 0; i < m_geometries.size() - 1; i++)
    {
        allVertices.insert(allVertices.end(), m_geometries[i].mVertices.begin(), m_geometries[i].mVertices.end());
        allNormals.insert(allNormals.end(), m_geometries[i].mNormals.begin(), m_geometries[i].mNormals.end());
        allIndices.insert(allIndices.end(), m_geometries[i].mIndices.begin(), m_geometries[i].mIndices.end());
    }
    GLuint indexBuff, vertexBuff, normalBuff;
    glGenBuffers(1, &indexBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int), &allIndices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuff);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuff);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(glm::vec4), &allVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuff);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuff);
    glBufferData(GL_ARRAY_BUFFER, allNormals.size() * sizeof(glm::vec3), &allNormals[0], GL_STATIC_DRAW);

    ComputeShader bruteGenShader;
    bruteGenShader.initFromFiles(CLOUDS_SHADERS_PATH "/generateDistanceField.comp");
    bruteGenShader.use();

    bruteGenShader.setuiVec3("res", m_voxelResolution);
    bruteGenShader.setVec3("bmin", glm::vec3(bbCoordinates.at("xmin"), bbCoordinates.at("ymin"), bbCoordinates.at("zmin")));
    bruteGenShader.setVec3("bmax", glm::vec3(bbCoordinates.at("xmax"), bbCoordinates.at("ymax"), bbCoordinates.at("zmax")));
    bruteGenShader.setInt("signedDf", 1);

    // bind buffers
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, vertexBuff, 0, allVertices.size() * sizeof(glm::vec4));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, normalBuff, 0, allNormals.size() * sizeof(glm::vec3));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, indexBuff, 0, allIndices.size() * sizeof(unsigned int));

    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x / 4.0f)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.y / 4.0f)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.z / 4.0f)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    gradientGrid.init(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    gradientGrid.gen3DTexture(m_voxelResolution.x, m_voxelResolution.y, m_voxelResolution.z);
    ComputeShader gradients;
    gradients.initFromFiles(CLOUDS_SHADERS_PATH "/computeGradients.comp");
    gradients.use();
    gradients.setSampler3D("sdfTexture", grid.getTextureID(), 8);
    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.y)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.z)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint gradientID = gradientGrid.getTextureID();

    Texture smoothGrid(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    smoothGrid.gen3DTexture(m_voxelResolution.x, m_voxelResolution.y, m_voxelResolution.z);
    ComputeShader smooth;
    smooth.initFromFiles(CLOUDS_SHADERS_PATH "/smoothSDF.comp");
    smooth.use();
    smooth.setSampler3D("sdfTexture", gradientID, 8);
    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.y)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.z)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint smoothSDFId = smoothGrid.getTextureID();

    windGrid.init(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    windGrid.gen3DTexture(m_voxelResolution.x, m_voxelResolution.y, m_voxelResolution.z);
    ComputeShader wind;
    wind.initFromFiles(CLOUDS_SHADERS_PATH "/windDirection.comp");
    wind.use();
    wind.setSampler3D("sdfTexture", smoothSDFId, 8);
    wind.setFloat("random", rand() % 360);
    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.y)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.z)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint windID = windGrid.getTextureID();

	Texture turbulence(CLOUDS_RESOURCES_PATH "/WavyTurbulence.bmp", 2, 3);
    flowGrid.init(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    flowGrid.gen3DTexture(m_voxelResolution.x, m_voxelResolution.y, m_voxelResolution.z);
    ComputeShader flow;
    flow.initFromFiles(CLOUDS_SHADERS_PATH "/computeFlowMap.comp");
    flow.use();
    flow.setSampler3D("sdfTexture", smoothSDFId, 8);
    flow.setSampler3D("windTexture", windID, 9);
    flow.setSampler2D("turbulence", turbulence.getTextureID(), 10);
    flow.setBool("firstRun", true);

    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x)),
                                       static_cast<GLint>(std::ceil(m_voxelResolution.y)),
                                       static_cast<GLint>(std::ceil(m_voxelResolution.z)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    flow.use();
    flow.setSampler3D("sdfTexture", smoothSDFId, 8);
    flow.setSampler3D("windTexture", windID, 9);
    flow.setSampler2D("turbulence", turbulence.getTextureID(), 10);
    flow.setBool("firstRun", false);

    glDispatchCompute(static_cast<GLint>(std::ceil(m_voxelResolution.x)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.y)),
                      static_cast<GLint>(std::ceil(m_voxelResolution.z)));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    GLuint flowID = flowGrid.getTextureID();
    return glm::vec3(windID, flowID, smoothSDFId);

}

std::vector<aiMesh*> Mesh::getMeshes() { return m_meshes; }

std::string Mesh::getPath() { return m_path; }

void Mesh::setModelMat(glm::mat4 modelMat) { m_modelMatrix = modelMat; }

void Mesh::update()
{
    m_moveDirection -= m_moveDirection * 0.1f;

    if(glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_modelMatrix   = glm::translate(m_modelMatrix, glm::vec3(0.0, 10.0, 0.0));
        m_moveDirection = glm::vec3(0.0, 1.0, 0.0);
    }
    if(glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(-10.0, 0.0, 0.0));
        m_moveDirection += 0.2f * glm::vec3(-1.0, 0.0, 0.0);
        m_moveDirection.z = 0;
        m_moveDirection.x = glm::max(-1.f, m_moveDirection.x);
    }
    if(glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_modelMatrix   = glm::translate(m_modelMatrix, glm::vec3(0.0, -10.0, 0.0));
        m_moveDirection = glm::vec3(0.0, -1.0, 0.0);
    }
    if(glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(10.0, 0.0, 0.0));
        m_moveDirection += 0.2f * glm::vec3(1.0, 0.0, 0.0);
        m_moveDirection.z = 0;
        m_moveDirection.x = glm::min(1.f, m_moveDirection.x);
    }
    if(glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        m_modelMatrix   = glm::translate(m_modelMatrix, glm::vec3(0.0, 0.0, -10.0));
        m_moveDirection += 0.2f * glm::vec3(0.0, 0.0, -1.0);
        m_moveDirection.x = 0;
        m_moveDirection.z = glm::max(-1.f, m_moveDirection.z);
    }
    if(glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_modelMatrix   = glm::translate(m_modelMatrix, glm::vec3(0.0, 0.0, 10.0));
        m_moveDirection += 0.2f * glm::vec3(0.0, 0.0, 1.0);
        m_moveDirection.x = 0;
        m_moveDirection.z = glm::min(1.f, m_moveDirection.z);
    }
}

glm::vec3 Mesh::getMoveDirection() { return m_moveDirection; }
void      Mesh::Geometry::createBuffers()
{
    glGenBuffers(1, &m_elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(glm::vec4), &mVertices[0], GL_STATIC_DRAW);

    if(mUVs.size() > 0)
    {
        glGenBuffers(1, &m_uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, mUVs.size() * sizeof(glm::vec2), &mUVs[0], GL_STATIC_DRAW);
    }
    glGenBuffers(1, &m_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(glm::vec3), &mNormals[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBuffer);

    glBindVertexArray(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Mesh::Geometry::render()
{
    glBindVertexArray(m_vao);

    glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::loadObj(std::string path)
{
    m_path = path;
    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(path, aiProcess_GenSmoothNormals | aiProcess_Triangulate);

    if(scene)
    {
        std::cout << "SUCCESS: Loaded Model from Path: \"" << path << "\"" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Loading failed from Path: \"" << path << "\"" << std::endl;
    }
    std::cout << scene->mNumMeshes << std::endl;

    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        Geometry meshGeom;
        aiMesh*  mesh = scene->mMeshes[i];
        m_meshes.push_back(mesh);

        // load geometry information of the current mesh
        for(unsigned int vCount = 0; vCount < mesh->mNumVertices; vCount++)
        {
            // vertices
            aiVector3D v = mesh->mVertices[vCount];
            meshGeom.mVertices.push_back(glm::vec4(v.x, v.y, v.z, 1.0f));

            // normals
            if(mesh->HasNormals())
            {
                v = mesh->mNormals[vCount];
                meshGeom.mNormals.push_back(glm::vec3(v.x, v.y, v.z));
            }

            // texture coordinates
            if(mesh->HasTextureCoords(0))
            {
                v = mesh->mTextureCoords[0][vCount];
                meshGeom.mUVs.push_back(glm::vec2(v.x, v.y));
            }
        }

        for(unsigned int fCount = 0; fCount < mesh->mNumFaces; fCount++)
        {
            aiFace f = mesh->mFaces[fCount];
            // index numbers
            for(unsigned int iCount = 0; iCount < f.mNumIndices; iCount++)
            {
                meshGeom.mIndices.push_back(f.mIndices[iCount]);
            }
        }
        meshGeom.createBuffers();
        m_geometries.push_back(meshGeom);
    }
    calculateBoundingBox();
}

void Mesh::init(GLFWwindow* window, std::string pathToFile)
{
    m_elementBufferBB = INVALID_GL_VALUE;
    m_normalBufferBB  = INVALID_GL_VALUE;
    m_vertexBufferBB  = INVALID_GL_VALUE;
    m_uvBufferBB      = INVALID_GL_VALUE;
    m_vaoBB           = INVALID_GL_VALUE;
    m_meshes.clear();
    m_geometries.clear();
    bbCoordinates.clear();
    m_window = window;
    loadObj(pathToFile);
}

void Mesh::render()
{
    for(auto geom : m_geometries)
    {
        geom.render();
    }
}

void Mesh::renderBB()
{
    glBindVertexArray(m_vaoBB);

    glDrawElements(GL_LINE_STRIP, 36, GL_UNSIGNED_INT, 0);
}
