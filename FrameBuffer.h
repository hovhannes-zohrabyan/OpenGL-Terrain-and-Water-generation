#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <glew.h>

class FrameBuffer
{
  public:
	FrameBuffer(int width, int height);
//	~FrameBuffer();
	void Bind(int width, int height);
	void Unbind();
	void BindTexture(int slot);

  private:
	unsigned int m_frameBufferID;
	unsigned int m_textureID;
	unsigned int m_textureDepthID;
	int InitTextureAttachment(int width, int height);
	int InitDepthTextureAttachment(int width, int height);
	int InitRenderBufferAttachment(int width, int height);
};
#endif