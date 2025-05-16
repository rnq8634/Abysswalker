#ifndef __SCENESPLASHSCREENAUT_H__
#define __SCENESPLASHSCREENAUT_H__

// Local includes
#include "Scene.h"
#include "Vector2.h"

// Forward declarations
class Renderer;
class InputSystem;
class Sprite;
class Texture;

class SceneSplashScreenAUT : public Scene
{
	// Member methods
public:
	SceneSplashScreenAUT();
	virtual ~SceneSplashScreenAUT();

	virtual bool Initialise(Renderer& renderer) override;
	virtual void Process(float deltaTme, InputSystem& inputSystem) override;
	virtual void Draw(Renderer& renderer) override;
	virtual void DebugDraw() override;

	bool IsFinished() const;

	// Member data
private:
	Sprite* m_pAUTLogoSprite;
	Texture* m_pAUTLogoTexture;

	float m_fDisplayTime;
	float m_fFadeTime;
	float m_fTimer;
	float m_fAlpha;

	enum class AUTSplashState
	{
		FADING_IN,
		DISPLAYING,
		FADING_OUT,
		FINISHED
	};

	AUTSplashState m_eCurrentState;
};

#endif // !__SCENESPLASHSCREENAUT_H__
