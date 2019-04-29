#include <Shader/FBO.h>

FBO::FBO(int width, int height, int numColorTextures, bool depthTexture,
         bool stencilTexture) {
  create(width, height, numColorTextures, depthTexture, stencilTexture);
}

FBO::~FBO() { reset(); }

void FBO::create(int width, int height, int numColorTextures, bool depthTexture,
                 bool stencilTexture) {
  reset();

  m_width = width;
  m_height = height;

  // generate fbo
  glGenFramebuffers(1, &m_frameBufferHandle);
  glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);

  // generate color textures
  // std::vector<GLenum> drawBuffers; Problem: funktioniert durch .data() nur in
  // neuer Umgebung...
  GLenum *drawBuffers = new GLenum[numColorTextures];

  for (int i = 0; i < numColorTextures; i++) {
    GLuint colorTextureHandle = createTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0 + m_colorTextures.size(),
                           GL_TEXTURE_2D, colorTextureHandle, 0);
    m_colorTextures.push_back(colorTextureHandle);
    // drawBuffers.push_back(GL_COLOR_ATTACHMENT0+i);
    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
  }
  // use color textures in fbo
  // glDrawBuffers(numColorTextures, drawBuffers.data());
  glDrawBuffers(numColorTextures, drawBuffers);

  if (depthTexture) {
    //// generate depth texture
    m_depthTexture = createTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           m_depthTexture, 0);
    // The depth buffer
    //GLuint depthrenderbuffer;
    //glGenRenderbuffers(1, &depthrenderbuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    

  }

  if (stencilTexture) {
    // generate stencil texture
    m_stencilTexture = createTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_width, m_height, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                           GL_TEXTURE_2D, m_stencilTexture, 0);
  }

  // Any errors while generating fbo ?
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR: Unable to create FBO." << std::endl;
    reset();
  }

  // cleanup
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::reset() {
  // delete all textures
  for (unsigned int i = 0; i < m_colorTextures.size(); i++) {
    glDeleteTextures(1, &m_colorTextures[i]);
  }
  m_colorTextures.clear();
  if (m_depthTexture != INVALID_GL_VALUE)
    glDeleteTextures(1, &m_depthTexture);
  if (m_stencilTexture != INVALID_GL_VALUE)
    glDeleteTextures(1, &m_stencilTexture);
  if (m_frameBufferHandle != INVALID_GL_VALUE)
    glDeleteFramebuffers(1, &m_frameBufferHandle);

  // reset values
  m_depthTexture = INVALID_GL_VALUE;
  m_stencilTexture = INVALID_GL_VALUE;
  m_frameBufferHandle = INVALID_GL_VALUE;
}

void FBO::resize(int width, int height) {
  int numColorTextures = m_colorTextures.size();
  bool depthTexture = false;
  if (m_depthTexture != INVALID_GL_VALUE)
    depthTexture = true;
  bool stencilTexture = false;
  if (m_stencilTexture != INVALID_GL_VALUE)
    stencilTexture = true;

  reset();
  create(width, height, numColorTextures, depthTexture, stencilTexture);
}

void FBO::bind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    glViewport(0, 0, m_width, m_height);
}

void FBO::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

GLuint FBO::getColorTexture(unsigned int index) {
  // does color buffer with given index exist ?
  if (index < m_colorTextures.size())
    return m_colorTextures[index];
  return 0;
}

GLuint FBO::getDepthTexture() const {
  // does depth buffer exist ?
  if (m_depthTexture == INVALID_GL_VALUE)
    return 0;
  return m_depthTexture;
}

GLuint FBO::createTexture() const {
  // generate fresh texture in OpenGL
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  return textureID;
}
