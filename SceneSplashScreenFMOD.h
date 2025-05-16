#ifndef __SCENESPLASHSCREENFMOD_H__
#define __SCENESPLASHSCREENFMOD_H__

// Local includes
#include "Scene.h"
#include "Vector2.h"

// Forward declarations
class Renderer;
class InputSystem;
class Sprite;
class Texture;

class SceneSplashScreenFMOD : public Scene
{
	// Member methods
public:
	SceneSplashScreenFMOD();
	virtual ~SceneSplashScreenFMOD();

	virtual bool Initialise(Renderer& renderer) override;
	virtual void Process(float deltaTme, InputSystem& inputSystem) override;
	virtual void Draw(Renderer& renderer) override;
	virtual void DebugDraw() override;

	bool IsFinished() const;

	// Member data
private:
	Sprite* m_pFMODLogoSprite;
	Texture* m_pFMODLogoTexture;

	float m_fDisplayTime;
	float m_fFadeTime;
	float m_fTimer;
	float m_fAlpha;

	enum class FMODSplashState
	{
		FADING_IN,
		DISPLAYING,
		FADING_OUT,
		FINISHED
	};

	FMODSplashState m_eCurrentState;
};

#endif // !__SCENESPLASHSCREENFMOD_H__
