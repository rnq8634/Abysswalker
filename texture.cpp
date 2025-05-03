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
	glDeleteTextures(1, &m_uiTextureId);
}

bool Texture::Initialize(const char* pcFilename)
{
	SDL_Surface* pSurface = IMG_Load(pcFilename);

	if (pSurface)
	{
		m_iWidth = pSurface->w;
		m_iHeight = pSurface->h;

		int bytesPerPixel = pSurface->format->BytesPerPixel;

		unsigned int format = 0;

		if (bytesPerPixel == 3)
		{
			format = GL_RGB;
		}
		else if (bytesPerPixel == 4)
		{
			format = GL_RGBA;
		}

		glGenTextures(1, &m_uiTextureId);
		glBindTexture(GL_TEXTURE_2D, m_uiTextureId);

		glTexImage2D(GL_TEXTURE_2D, 0, format, m_iWidth, m_iHeight, 0, format, GL_UNSIGNED_BYTE, pSurface->pixels);

		SDL_FreeSurface(pSurface);
		pSurface = 0;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		LogManager::GetInstance().Log("Texture failed to load!");
		assert(0);
		return false;
	}
	LoadSurfaceIntoTexture(pSurface);

	return true;
}

void Texture::SetActive()
{
	glBindTexture(GL_TEXTURE_2D, m_uiTextureId);
}

int Texture::GetWidth() const
{
	assert(m_iWidth);
	return (m_iWidth);
}

int Texture::GetHeight() const
{
	assert(m_iHeight);
	return (m_iHeight);
}

void
Texture::LoadTextTexture(const char* text, const char* fontname, int pointsize)
{
	// Init
	if (TTF_Init() == -1)
	{
		LogManager::GetInstance().Log("SDL_ttf failed to initialize!");
		return;
	}

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
	color.a = 100;

	SDL_Surface* pSurface = TTF_RenderText_Blended(pFont, text, color);

	if (!pSurface)
	{
		LogManager::GetInstance().Log("Failed to render text!");
		TTF_CloseFont(pFont);
		return;
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, pSurface->pitch / pSurface->format->BytesPerPixel);
	LoadSurfaceIntoTexture(pSurface);

	TTF_CloseFont(pFont);
	pFont = 0;
}

void
Texture::LoadSurfaceIntoTexture(SDL_Surface* pSurface)
{
	if (pSurface)
	{
		m_iWidth = pSurface->w;
		m_iHeight = pSurface->h;
		int bytesPerPixel = pSurface->format->BytesPerPixel;
		unsigned int format = 0;

		if (bytesPerPixel == 3)
		{
			format = GL_RGB;
		}
		else if (bytesPerPixel == 4)
		{
			format = GL_RGBA;
		}

		glGenTextures(1, &m_uiTextureId); 
		glBindTexture(GL_TEXTURE_2D, m_uiTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, m_iWidth, m_iHeight, 0, format, GL_UNSIGNED_BYTE, pSurface->pixels);
		
		SDL_FreeSurface(pSurface); 
		pSurface = 0;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}