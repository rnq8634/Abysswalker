// COMP710 GP Framework 2025

// This include:
#include "renderer.h"

// Local includes:
#include "texturemanager.h"
#include "logmanager.h"
#include "shader.h"
#include "vertexarray.h"
#include "sprite.h"
#include "matrix4.h"
#include "animatedsprite.h"
#include "texture.h"

// IMGUI INCLUDES
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"


// Library includes:
#include <SDL.h>
#include <SDL_image.h>
#include <glew.h>
#include <cassert>
#include <cmath>

Renderer::Renderer()
	: m_pTextureManager(0)
	, m_pSpriteShader(0)
	, m_pSpriteVertexData(0)
	, m_glContext(0)
	, m_iWidth(0)
	, m_iHeight(0)
	, m_fClearRed(0.0f)
	, m_fClearGreen(0.0f)
	, m_fClearBlue(0.0f)
	, m_pWindow(nullptr)
	, m_whitePixelTextureID(0)
{

}

Renderer::~Renderer()
{
	// IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	delete m_pSpriteShader;
	m_pSpriteShader = 0;

	delete m_pSpriteVertexData;
	m_pSpriteVertexData = 0;

	delete m_pTextureManager;
	m_pTextureManager = 0;

	if (m_whitePixelTextureID != 0)
	{
		glDeleteTextures(1, &m_whitePixelTextureID);
		m_whitePixelTextureID = 0;
	}

	SDL_DestroyWindow(m_pWindow);
	IMG_Quit();
	SDL_Quit();
}

bool Renderer::Initialize(bool windowed, int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LogSdlError();
		return false;
	}

	if (!windowed)
	{
		// Go fullscreen, with current resolution!
		int numDisplays = SDL_GetNumVideoDisplays();
		SDL_DisplayMode* currentDisplayMode = new SDL_DisplayMode[numDisplays];

		for (int k = 0; k < numDisplays; ++k)
		{
			int result = SDL_GetCurrentDisplayMode(k, &currentDisplayMode[k]);
		}

		// Use the widest display?
		int widest = 0;
		int andItsHeight = 0;

		for (int k = 0; k < numDisplays; ++k)
		{
			if (currentDisplayMode[k].w > widest)
			{
				widest = currentDisplayMode[k].w;
				andItsHeight = currentDisplayMode[k].h;
			}
		}

		delete[] currentDisplayMode;
		currentDisplayMode = 0;

		width = widest;
		height = andItsHeight;
	}

	bool initialized = InitializeOpenGL(width, height);

	SetFullscreen(!windowed);

	if (initialized)
	{
		m_pTextureManager = new TextureManager();
		assert(m_pTextureManager);
		initialized = m_pTextureManager->Initialize();
	}

	// IMGUI
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(m_pWindow, m_glContext);
	ImGui_ImplOpenGL3_Init();

	return initialized;
}

bool Renderer::InitializeOpenGL(int screenWidth, int screenHeight)
{
	m_iWidth = screenWidth;
	m_iHeight = screenHeight;

	m_pWindow = SDL_CreateWindow("COMP710 Game Framework 2025", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8); // Default was 8
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); // Default was 8
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8); // Default was 8
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // Default was 8

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	m_glContext = SDL_GL_CreateContext(m_pWindow);

	GLenum glewResult = glewInit();

	if (glewResult != GLEW_OK)
	{
		return false;
	}

	glGenTextures(1, &m_whitePixelTextureID);
	glBindTexture(GL_TEXTURE_2D, m_whitePixelTextureID);
	unsigned char whiteData[4] = { 255, 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Disable VSYN
	SDL_GL_SetSwapInterval(0);

	bool shadersLoaded = SetupSpriteShader();

	return shadersLoaded;
}

void Renderer::Clear()
{
	glClearColor(m_fClearRed, m_fClearGreen, m_fClearBlue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// IMGUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void Renderer::Present()
{
	// IMGUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(m_pWindow);
}

void Renderer::SetFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		SDL_SetWindowFullscreen(m_pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALWAYS_ON_TOP);
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
		SDL_SetWindowSize(m_pWindow, m_iWidth, m_iHeight);
	}
	else
	{
		SDL_SetWindowFullscreen(m_pWindow, 0);
	}
}

void Renderer::SetClearColor(unsigned char r, unsigned char g, unsigned char b)
{
	m_fClearRed = r / 255.0f; // Default was 255.0f
	m_fClearGreen = g / 255.0f; // Default was 255.0f
	m_fClearBlue = b / 255.0f; // Default was 255.0f
}

void Renderer::GetClearColor(unsigned char& r, unsigned char& g, unsigned char& b)
{
	r = static_cast<unsigned char>(m_fClearRed * 255.0f);
	g = static_cast<unsigned char>(m_fClearGreen * 255.0f);
	b = static_cast<unsigned char>(m_fClearBlue * 255.0f);
}

int Renderer::GetWidth() const
{
	return m_iWidth;
}

int Renderer::GetHeight() const
{
	return m_iHeight;
}

// To create sprites!! (Static sprites)
Sprite* Renderer::CreateSprite(const char* pcFilename)
{
	assert(m_pTextureManager);

	Texture* pTexture = m_pTextureManager->GetTexture(pcFilename);

	Sprite* pSprite = new Sprite();
	if (!pSprite->Initialise(*pTexture))
	{
		LogManager::GetInstance().Log("Sprite Failed to Create!");
	}

	return (pSprite);
}

void Renderer::LogSdlError()
{
	LogManager::GetInstance().Log(SDL_GetError());
}

// Sprites
bool Renderer::SetupSpriteShader()
{
	m_pSpriteShader = new Shader();

	bool loaded = m_pSpriteShader->Load("shader/sprite.vert", "shader/sprite.frag");

	m_pSpriteShader->SetActive();

	float vertices[] =
	{
		-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, // Top Left
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top Right
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // Bottom Right 
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f // Bottom Left
	};

	unsigned int indices[] = { 0,1,2,2,3,0 };

	m_pSpriteVertexData = new VertexArray(vertices, 4, indices, 6);

	return loaded;
}

void Renderer::DrawSprite(Sprite& sprite, bool flipHorizontal)
{
	m_pSpriteShader->SetActive();
	m_pSpriteVertexData->SetActive();

	float angleInDegrees = sprite.GetAngle();

	float sizeX = static_cast<float>(sprite.GetWidth());
	float sizeY = static_cast<float>(sprite.GetHeight());

	const float PI = 3.14159f;
	float angleInRadians = (angleInDegrees * PI) / 180.0f;

	Matrix4 world;
	SetIdentity(world);

	// Handle horizontal flipping
	if (flipHorizontal)
	{
		world.m[0][0] = -cosf(angleInRadians) * (sizeX); // Flip horizontally by negating X scale
		world.m[0][1] = sinf(angleInRadians) * (sizeX);  // Also need to flip this component
		world.m[1][0] = -sinf(angleInRadians) * (sizeY); // Flip this component too
		world.m[1][1] = cosf(angleInRadians) * (sizeY);  // Keep this the same
	}
	else
	{
		world.m[0][0] = cosf(angleInRadians) * (sizeX);   // Normal X scale
		world.m[0][1] = -sinf(angleInRadians) * (sizeX);  // Normal rotation component
		world.m[1][0] = sinf(angleInRadians) * (sizeY);   // Normal rotation component
		world.m[1][1] = cosf(angleInRadians) * (sizeY);   // Normal Y scale
	}

	world.m[3][0] = static_cast<float>(sprite.GetX());
	world.m[3][1] = static_cast<float>(sprite.GetY());

	m_pSpriteShader->SetMatrixUniform("uWorldTransform", world);

	Matrix4 orthoViewProj;
	CreateOrthoProjection(orthoViewProj, static_cast<float>(m_iWidth), static_cast<float>(m_iHeight));

	m_pSpriteShader->SetVector4Uniform("color", sprite.GetRedTint(), sprite.GetGreenTint(), sprite.GetBlueTint(), sprite.GetAlpha());

	m_pSpriteShader->SetMatrixUniform("uViewProj", orthoViewProj);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// To create ANIMATED SPRITES
AnimatedSprite*
Renderer::CreateAnimatedSprite(const char* pcFilename)
{
	assert(m_pTextureManager);

	Texture* pTexture = m_pTextureManager->GetTexture(pcFilename);

	AnimatedSprite* pSprite = new AnimatedSprite();
	if (!pSprite->Initialise(*pTexture))
	{
		LogManager::GetInstance().Log("AnimatedSprite failed to create!");
	}

	return pSprite;
}

// -----------------------------------------------------------Draw Animated Sprites-------------------------------------------------

void
Renderer::DrawAnimatedSprite(AnimatedSprite& sprite, int frame, bool flipHorizontal)
{
	m_pSpriteShader->SetActive();

	float angleInDegrees = sprite.GetAngle();
	float sizeX = static_cast<float>(sprite.GetWidth());
	float sizeY = static_cast<float>(sprite.GetHeight());

	const float PI = 3.14159f;
	float angleInRadians = (angleInDegrees * PI) / 180.0f;

	Matrix4 world;
	SetIdentity(world);

	// Handle horizontal flipping
	if (flipHorizontal)
	{
		world.m[0][0] = -cosf(angleInRadians) * (sizeX); // Flip horizontally by negating X scale
		world.m[0][1] = sinf(angleInRadians) * (sizeX);  // Also need to flip this component
		world.m[1][0] = -sinf(angleInRadians) * (sizeY); // Flip this component too
		world.m[1][1] = cosf(angleInRadians) * (sizeY);  // Keep this the same
	}
	else
	{
		world.m[0][0] = cosf(angleInRadians) * (sizeX);   // Normal X scale
		world.m[0][1] = -sinf(angleInRadians) * (sizeX);  // Normal rotation component
		world.m[1][0] = sinf(angleInRadians) * (sizeY);   // Normal rotation component
		world.m[1][1] = cosf(angleInRadians) * (sizeY);   // Normal Y scale
	}

	world.m[3][0] = static_cast<float>(sprite.GetX());
	world.m[3][1] = static_cast<float>(sprite.GetY());

	m_pSpriteShader->SetMatrixUniform("uWorldTransform", world);

	Matrix4 orthoViewProj;
	CreateOrthoProjection(orthoViewProj, static_cast<float>(m_iWidth), static_cast<float>(m_iHeight));

	m_pSpriteShader->SetVector4Uniform("color"
		, sprite.GetRedTint()
		, sprite.GetGreenTint()
		, sprite.GetBlueTint()
		, sprite.GetAlpha());
	
	m_pSpriteShader->SetMatrixUniform("uViewProj", orthoViewProj); 
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)((frame * 6) * sizeof(GLuint)));
}

// ------------------------------------------------------------To Create Static Texts--------------------------------------------

void
Renderer::CreateStaticText(const char* pText, int pointsize)
{
	Texture* pTexture = new Texture();
	pTexture->LoadTextTexture(pText, "ADDTEXTUREFONTHERE.ttf", pointsize);
	m_pTextureManager->AddTexture(pText, pTexture);
}

void Renderer::DrawDebugRect(float x1, float y1, float x2, float y2, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	GLint previous_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);

	GLint last_active_texture_unit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture_unit); // Save current active texture unit
	glActiveTexture(GL_TEXTURE0);

	GLint last_texture_bound_on_unit0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_bound_on_unit0); // Save texture currently bound to unit 0
	glBindTexture(GL_TEXTURE_2D, m_whitePixelTextureID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pSpriteShader->SetActive();

	Matrix4 orthoViewProj;
	CreateOrthoProjection(orthoViewProj, static_cast<float>(m_iWidth), static_cast<float>(m_iHeight));
	m_pSpriteShader->SetMatrixUniform("uViewProj", orthoViewProj);

	Matrix4 world;
	SetIdentity(world);

	float rectWidth = x2 - x1;
	float rectHeight = y2 - y1;

	world.m[0][0] = rectWidth;
	world.m[1][1] = rectHeight;
	world.m[3][0] = x1 + rectWidth / 2.0f;
	world.m[3][1] = y1 + rectHeight / 2.0f;
	m_pSpriteShader->SetMatrixUniform("uWorldTransform", world);

	// Set color
	m_pSpriteShader->SetVector4Uniform("color", r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

	// Draw using the sprite vertex data
	m_pSpriteVertexData->SetActive();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, last_texture_bound_on_unit0); // Restore previously bound texture to unit 0
	glActiveTexture(last_active_texture_unit);

	// Restore previous shader
	glUseProgram(previous_program);
}