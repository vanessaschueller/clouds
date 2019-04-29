#pragma once

#include <Includes/Defs.h>

class FBO {
public:
  FBO(int width, int height, int numColorTextures = 1,
      bool depthTexture = false, bool stencilTexture = false);
  ~FBO();
  void create(int width, int height, int numColorTextures, bool depthTexture,
              bool stencilTexture);
  void reset();
  void resize(int width, int height);
  void bind() const;
  void unbind() const;
  GLuint getColorTexture(unsigned int index);
  GLuint getDepthTexture() const;

private:
  GLuint createTexture() const;
  int m_width, m_height; //!< the size of each texture object
  std::vector<GLuint>
      m_colorTextures; //!< List containing all OpenGL color textures
  GLuint m_depthTexture, m_stencilTexture;
  GLuint m_frameBufferHandle; //!< The actual OpenGL FBO id
};
