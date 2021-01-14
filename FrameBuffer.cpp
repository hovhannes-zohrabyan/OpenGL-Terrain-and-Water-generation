#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <stb_image.h>


#include <fstream>
#include <sstream>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>

#include "FrameBuffer.h";


FrameBuffer::FrameBuffer(int width, int height) {

	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_frameBufferID = frameBuffer;

	//FrameBuffer::Bind(width, height);

	m_textureID = InitTextureAttachment(width, height);
	m_textureDepthID = InitDepthTextureAttachment(width, height);
}

void FrameBuffer::BindTexture(int slot)
{
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

int FrameBuffer::InitTextureAttachment(int width, int height) {

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

	return texture;

}

int FrameBuffer::InitDepthTextureAttachment(int width, int height) {

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

	return texture;

}

int FrameBuffer::InitRenderBufferAttachment(int width, int height) {
	unsigned int renderBuffer;
	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	return renderBuffer;
}

void FrameBuffer::Bind(int width, int height) {

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
	glViewport(0, 0, width, height);

}

void FrameBuffer::Unbind() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}