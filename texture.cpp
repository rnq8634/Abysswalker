// COMP710 GP Framework 2024

// This include:
#include "texture.h"

// Local includes:
#include "logmanager.h"

// Library include:
#include <SDL_image.h>
#include <cassert>
#include <glew.h>
#include <SDL_ttf.h>


Texture::Texture()
	: m_uiTextureId(0)
	, m_iHeight(0)
	, m_iWidth(0)
{
}

Texture::~Texture()
{
	if (m_uiTextureId != 0)
	{
		glDeleteTextures(1, &m_uiTextureId);
		m_uiTextureId = 0;
	}
}

bool Texture::Initialise(const char* pcFilename)
{
	SDL_Surface* pSurface = IMG_Load(pcFilename);

	if (pSurface)
	{
		GLint previousUnpackAlignment;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
		GLint previousUnpackRowLength;
		glGetIntegerv(GL_UNPACK_ROW_LENGTH, &previousUnpackRowLength);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, pSurface->pitch / pSurface->format->BytesPerPixel);

		m_iWidth = pSurface->w;
		m_iHeight = pSurface->h;

		GLenum internalGlFormat = 0;
		GLenum surfacePixelGlFormat = 0;
		int bytesPerPixel = pSurface->format->BytesPerPixel;

		if (bytesPerPixel == 4)
		{
			internalGlFormat = GL_RGBA;

			if (pSurface->format->Rmask == 0x000000ff)
			{
				surfacePixelGlFormat = GL_RGBA;
			}
			else if (pSurface->format->Bmask == 0x000000ff)
			{
				surfacePixelGlFormat = GL_BGRA;
			}
			else
			{
				surfacePixelGlFormat = GL_RGBA; // Fallback
			}
		}
		else if (bytesPerPixel == 3)
		{
			internalGlFormat = GL_RGB;
			if (pSurface->format->Rmask == 0x0000ff)
			{
				surfacePixelGlFormat = GL_RGB;
			}
			else if (pSurface->format->Bmask == 0x0000ff)
			{
				surfacePixelGlFormat = GL_BGR;
			}
			else
			{
				surfacePixelGlFormat = GL_RGB;
			}
		}
		else
		{
			SDL_FreeSurface(pSurface);
			glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, previousUnpackRowLength);
			return false;
		}

		glGenTextures(1, &m_uiTextureId);
		glBindTexture(GL_TEXTURE_2D, m_uiTextureId);

		glTexImage2D(GL_TEXTURE_2D, 0, internalGlFormat, m_iWidth, m_iHeight, 0, surfacePixelGlFormat, GL_UNSIGNED_BYTE, pSurface->pixels);

		SDL_FreeSurface(pSurface);
		pSurface = nullptr;

		glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, previousUnpackRowLength);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // NOTE: Must be GL_NEAREST otherwise the pixels will mess up
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		LogManager::GetInstance().Log("Texture failed to load!");
		assert(0);
		return false;
	}

	return true;
}

void Texture::SetActive()
{
	glBindTexture(GL_TEXTURE_2D, m_uiTextureId);
}

int Texture::GetWidth() const
{
	return (m_iWidth);
}

int Texture::GetHeight() const
{
	return (m_iHeight);
}

void
Texture::LoadTextTexture(const char* text, const char* fontname, int pointsize)
{
	// Open font
	TTF_Font* pFont = TTF_OpenFont(fontname, pointsize);
	if (!pFont)
	{
		LogManager::GetInstance().Log("Failed to load font!");
		return;
	}

	SDL_Color color;
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 255;

	SDL_Surface* pSurface = TTF_RenderText_Blended(pFont, text, color);

	if (!pSurface)
	{
		LogManager::GetInstance().Log("Failed to render text!");
		TTF_CloseFont(pFont);
		return;
	}

	GLint previousUnpackAlignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
	GLint previousUnpackRowLength;
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &previousUnpackRowLength);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TTF surfaces are byte-aligned
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pSurface->pitch / pSurface->format->BytesPerPixel);

	LoadSurfaceIntoTexture(pSurface);

	glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, previousUnpackRowLength);

	TTF_CloseFont(pFont);
	pFont = 0;
}

void
Texture::LoadSurfaceIntoTexture(SDL_Surface* pSurface)
{
	if (pSurface)
	{
		if (m_uiTextureId != 0)
		{
			glDeleteTextures(1, &m_uiTextureId);
			m_uiTextureId = 0;
		}

		m_iWidth = pSurface->w;
		m_iHeight = pSurface->h;

		GLenum internalGlFormat = GL_RGBA;
		GLenum surfacePixelGlFormat = GL_RGBA;
		int bytesPerPixel = pSurface->format->BytesPerPixel;

		if (pSurface->format->BytesPerPixel == 4)
		{
			internalGlFormat = GL_RGBA;
			surfacePixelGlFormat = GL_BGRA;
			surfacePixelGlFormat = GL_RGBA;
		}
		else if (pSurface->format->BytesPerPixel == 3)
		{
			internalGlFormat = GL_RGB;
			if (pSurface->format->Rmask == 0x0000ff) surfacePixelGlFormat = GL_RGB;
			else surfacePixelGlFormat = GL_BGR;
			if (pSurface->format->Rmask == 0xff0000) surfacePixelGlFormat = GL_RGB;
			else surfacePixelGlFormat = GL_BGR;
		}
		else
		{
			SDL_FreeSurface(pSurface);
			return;
		}

		glGenTextures(1, &m_uiTextureId); 
		glBindTexture(GL_TEXTURE_2D, m_uiTextureId);

		glTexImage2D(GL_TEXTURE_2D, 0, internalGlFormat, m_iWidth, m_iHeight, 0, surfacePixelGlFormat, GL_UNSIGNED_BYTE, pSurface->pixels);
		
		SDL_FreeSurface(pSurface); 
		pSurface = 0;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}