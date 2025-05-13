// COMP710 GP Framework 2024
#ifndef __TEXTURE_H_
#define __TEXTURE_H_

#include <SDL_image.h>

class Texture
{
	// Member methods:
public:
	Texture();
	~Texture();

	bool Initialise(const char* pcFilename);

	void SetActive();

	int GetWidth() const;
	int GetHeight() const;

	void LoadTextTexture(const char* text, const char* fontname, int pointsize);
	void LoadSurfaceIntoTexture(SDL_Surface* pSurface);

protected:

private:
	Texture(const Texture& texture);
	Texture& operator=(const Texture& texture);

	// Member data:
public:

protected:
	unsigned int m_uiTextureId;
	int m_iWidth;
	int m_iHeight;

private:

};

#endif // __TEXTURE_H_