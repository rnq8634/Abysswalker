// COMMP710 GP Framework 2025
// This include:
#include "Sprite.h"

// Local includes:
#include "Renderer.h"
#include "Texture.h"
#include "LogManager.h"

// Lib includes
#include <cmath>

Sprite::Sprite()
	: m_pTexture(0)
	, m_x(0)
	, m_y(0)
	, m_width(0)
	, m_height(0)
	, m_angle(0.0f)
	, m_centerX(0)
	, m_centerY(0)
	, m_scaleX(1.0f)
	, m_scaleY(1.0f)
	, m_alpha(1.0f)
	, m_tintRed(1.0f)
	, m_tintGreen(1.0f)
	, m_tintBlue(1.0f)
	, m_bFlipHorizontal(false)
{
}

Sprite::~Sprite()
{
}

bool Sprite::Initialise(Texture& texture)
{
	m_pTexture = &texture;

	m_width = m_pTexture->GetWidth();
	m_height = m_pTexture->GetHeight();

	if (m_width == 0 || m_height == 0)
	{
		LogManager::GetInstance().Log("Sprite::Initialise has failed!!");
	}

	return true;
}

bool Sprite::InitialiseWithText(Texture& texture, const char* text, const char* fontname, int pointsize)
{
	texture.LoadTextTexture(text, fontname, pointsize);

	if (texture.GetWidth() == 0 || texture.GetHeight() == 0)
	{
		LogManager::GetInstance().Log("Sprite::InitialiseWithText has failed!");
	}

	return Initialise(texture);
}

void Sprite::Process(float deltaTime)
{
}

void Sprite::SetFlipHorizontal(bool flip)
{
	m_bFlipHorizontal = flip;
}

bool Sprite::IsFlippedHorizontal() const
{
	return m_bFlipHorizontal;
}

void Sprite::Draw(Renderer& renderer)
{
	if (m_pTexture && m_pTexture->GetWidth() > 0 && m_pTexture->GetHeight() > 0)
	{
		m_pTexture->SetActive();
		renderer.DrawSprite(*this, m_bFlipHorizontal);
	}
}

void Sprite::SetX(int x)
{
	m_x = x;
}

void Sprite::SetY(int y)
{
	m_y = y;
}

int Sprite::GetX() const
{
	return m_x;
}

int Sprite::GetY() const
{
	return m_y;
}

void Sprite::SetAngle(float angle)
{
	angle += 360;
	while (angle > 360.0f)
	{
		angle -= 360.0f;
	}

	m_angle = angle;
}

float Sprite::GetAngle() const
{
	return m_angle;
}

void Sprite::SetScale(float scaleX, float scaleY)
{
	m_scaleX = scaleX;
	m_scaleY = scaleY;
}

void Sprite::SetScaleX(float scaleX)
{
	m_scaleX = scaleX;
}

void Sprite::SetScaleY(float scaleY)
{
	m_scaleY = scaleY;
}

float Sprite::GetScaleX() const
{
	return m_scaleX;
}

float Sprite::GetScaleY() const
{
	return m_scaleY;
}

void Sprite::SetAlpha(float alpha)
{
	m_alpha = Clamp(0.0f, alpha, 1.0f);
}

float Sprite::GetAlpha() const
{
	return m_alpha;
}

int Sprite::GetWidth() const
{
	return static_cast<int>(ceilf(m_width * m_scaleX));
}

int Sprite::GetHeight() const
{
	return static_cast<int>(ceilf(m_height * m_scaleY));
}

float Sprite::Clamp(float minimum, float value, float maximum)
{
	if (value > maximum)
	{
		value = maximum;
	}
	else if (value < minimum)
	{
		value = minimum;
	}

	return value;
}

void Sprite::SetRedTint(float value)
{
	m_tintRed = Clamp(0.0f, value, 1.0f);
}

float Sprite::GetRedTint() const
{
	return m_tintRed;
}

void Sprite::SetGreenTint(float value)
{
	m_tintGreen = Clamp(0.0f, value, 1.0f);
}

float Sprite::GetGreenTint() const
{
	return m_tintGreen;
}

void Sprite::SetBlueTint(float value)
{
	m_tintBlue = Clamp(0.0f, value, 1.0f);
}

float Sprite::GetBlueTint() const
{
	return m_tintBlue;
}