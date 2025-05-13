// COMP710 GP Framework 2025
#ifndef __SPRITE_H_
#define __SPRITE_H_

// Forward Declarations:
class Renderer;
class Texture;

class Sprite
{
	// Member methods:
public:
	Sprite();
	~Sprite();

	bool Initialise(Texture& texture);
	bool InitialiseWithText(Texture& texture, const char* text, const char* fontname, int pointsize);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	int GetWidth() const;
	int GetHeight() const;

	int GetOriginalWidth() const { return m_width; }
	int GetOriginalHeight() const { return m_height; }

	void SetX(int x);
	int GetX() const;
	void SetY(int y);
	int GetY() const;

	void SetAngle(float angle);
	float GetAngle() const;

	// scaling methods
	void SetScale(float scaleX, float scaleY);
	void SetScaleX(float scaleX);
	void SetScaleY(float scaleY);
	float GetScaleX() const;
	float GetScaleY() const;

	void SetAlpha(float alpha);
	float GetAlpha() const;

	void SetRedTint(float value);
	float GetRedTint() const;
	void SetGreenTint(float value);
	float GetGreenTint() const;
	void SetBlueTint(float value);
	float GetBlueTint() const;

	void SetFlipHorizontal(bool flip);
	bool IsFlippedHorizontal() const;

protected:
	float Clamp(float minimum, float value, float maximum);

private:
	Sprite(const Sprite& sprite);
	Sprite& operator=(const Sprite& sprite);

	// Member data:
public:

protected:
	Texture* m_pTexture;
	int m_x;
	int m_y;

	float m_angle;
	int m_centerX;
	int m_centerY;

	int m_width;
	int m_height;

	float m_scaleX;
	float m_scaleY;
	float m_alpha;

	float m_tintRed;
	float m_tintGreen;
	float m_tintBlue;

	bool m_bFlipHorizontal;

private:
};

#endif // __SPRITE_H_