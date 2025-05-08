#ifndef __ANIMATEDSPRITE_H
#define __ANIMATEDSPRITE_H

// Local inc
#include "sprite.h"

// Lib includes
#include <functional> // for std::functional

// Forward dec
class Renderer;
class VertexArray;

// Class dec
class AnimatedSprite : public Sprite
{
	// Member methods
public:
	AnimatedSprite();
	~AnimatedSprite();

	bool Initialise(Texture& texture);
	void SetupFrames(int fixedFrameWidth, int fixedFrameHeight);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	void SetLooping(bool loop);
	void Animate();
	bool IsAnimating() const;
	void Restart();
	void SetFrameDuration(float seconds);
	int GetWidth() const;
	int GetHeight() const;
	void DebugDraw();

	void SetFlipHorizontal(bool flip);
	bool IsFlippedHorizontal() const;

	// Animation completion callback
	typedef std::function<void()> AnimationCompleteCallback;
	void SetAnimationCompleteCallback(AnimationCompleteCallback callback);
	bool IsAnimationComplete() const;

	bool IsLooping() const;

	void Pause();
	void Resume();
	bool IsPaused() const;
	void SetCurrentFrame(int frameIndex);
	int GetCurrentFrame() const;
	int GetTotalFrames() const;

protected:

private:
	AnimatedSprite(const AnimatedSprite& animatedsprite);
	AnimatedSprite& operator=(const AnimatedSprite& animatedsprite);

	// Member data
public:

protected:
	VertexArray* m_pVertexData;
	int m_iFrameWidth;
	int m_iFrameHeight;
	int m_iCurrentFrame;
	int m_iTotalFrames;
	float m_fTimeElapsed;
	float m_frameDuration;
	float totalTime;
	bool m_bAnimating;
	bool m_bLooping;
	bool m_bFlipHorizontal;
	bool m_bAnimationComplete;
	AnimationCompleteCallback m_animationCompleteCallback;

private:

};

#endif // __ANIMATEDSPRITE_H