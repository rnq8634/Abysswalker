// This include
#include "animatedsprite.h"

// Local include
#include "renderer.h"
#include "vertexarray.h"
#include "texture.h"
#include "imgui/imgui.h"

// Lib includes
#include <cassert>

AnimatedSprite::AnimatedSprite()
	: m_pVertexData(0)
	, m_iFrameWidth(0)
	, m_iFrameHeight(0)
	, m_iCurrentFrame(0)
	, m_iTotalFrames(0)
	, m_fTimeElapsed(0.0f)
	, m_bAnimating(false)
	, m_bLooping(false)
	, m_frameDuration(1.0f)
	, totalTime(0.0f)
	, m_bFlipHorizontal(false)
	, m_bAnimationComplete(false)
	, m_animationCompleteCallback(nullptr)
{

}

AnimatedSprite::~AnimatedSprite()
{
	delete m_pVertexData;
	m_pVertexData = 0;
}

bool
AnimatedSprite::Initialise(Texture& texture)
{
	return Sprite::Initialise(texture);
}

int
AnimatedSprite::GetWidth() const
{
	return static_cast<int>(ceilf(m_iFrameWidth * m_scaleX));
}

int
AnimatedSprite::GetHeight() const
{
	return static_cast<int>(ceilf(m_iFrameHeight * m_scaleY));
}

bool
AnimatedSprite::IsAnimating() const
{
	return m_bAnimating;
}

void
AnimatedSprite::SetupFrames(int fixedFrameWidth, int fixedFrameHeight)
{
	m_iFrameWidth = fixedFrameWidth;
	m_iFrameHeight = fixedFrameHeight;

	const int textureWidth = m_pTexture->GetWidth();
	const int textureHeight = m_pTexture->GetHeight();
	const int totalFramesWide = textureWidth / fixedFrameWidth;
	const int totalFramesHigh = textureHeight / fixedFrameHeight;

	const int stride = 5;
	const float uFrameWidth = 1.0f / totalFramesWide;
	const float vFrameHeight = 1.0f / totalFramesHigh;
	m_iTotalFrames = totalFramesWide * totalFramesHigh;

	const int vertsPerSprite = 4;
	const int numVertices = vertsPerSprite * (m_iTotalFrames);

	float* vertices = new float[numVertices * stride];

	for (int h = 0; h < totalFramesHigh; ++h)
	{
		for (int w = 0; w < totalFramesWide; ++w)
		{
			float uOffset = (w * uFrameWidth);
			float vOffset = (h * vFrameHeight);

			float quad[] =
			{
				-0.5f, 0.5f, 0.0f,		   0.0f + uOffset, vOffset + vFrameHeight,
				 0.5f, 0.5f, 0.0f,	uFrameWidth + uOffset, vOffset + vFrameHeight,
				 0.5f, -0.5f, 0.0f, uFrameWidth + uOffset, vOffset,
				-0.5f, -0.5f, 0.0f,		   0.0f + uOffset, vOffset
			};

			const int floatPerSprite = stride * vertsPerSprite;

			for (int j = 0; j < floatPerSprite; ++j)
			{
				int index = ((w * floatPerSprite) + j) + (h * (floatPerSprite * totalFramesWide));

				vertices[index] = quad[j];
			}
		}
	}

	const int totalIndices = 6 * m_iTotalFrames;
	unsigned int* allIndices = new unsigned int[totalIndices];

	unsigned int i = 0;
	for (int k = 0; k < m_iTotalFrames; ++k)
	{
		unsigned int indices[] = { 0 + i, 1 + i, 2 + i, 2 + i, 3 + i, 0 + i };

		for (int j = 0; j < 6; ++j)
		{
			int index = (k * 6) + j;

			allIndices[index] = indices[j];
		}

		i += 4;
	}

	m_pVertexData = new VertexArray(vertices, numVertices, allIndices, totalIndices);

	delete vertices;
	vertices = 0;

	delete allIndices;
	allIndices = 0;
}

void
AnimatedSprite::Process(float deltaTime)
{
	totalTime += deltaTime;

	if (m_bAnimating)
	{
		m_fTimeElapsed += deltaTime; // Count how long the current frame has been shown

		if (m_fTimeElapsed > m_frameDuration) // Swap to the next frame
		{
			++m_iCurrentFrame;

			if (m_iCurrentFrame >= m_iTotalFrames)
			{
				if (m_bLooping) // Loop at the end of animation?
				{
					Restart();
				}
				else
				{
					m_iCurrentFrame = m_iTotalFrames - 1;
					m_bAnimating = false;
					m_bAnimationComplete = true;

					if (m_animationCompleteCallback)
					{
						m_animationCompleteCallback();
					}
				}
			}
			m_fTimeElapsed = 0.0f; // Reset this counter at the completion of each animated frame
		}
	}
}

void
AnimatedSprite::SetFlipHorizontal(bool flip)
{
	m_bFlipHorizontal = flip;
}

bool
AnimatedSprite::IsFlippedHorizontal() const
{
	return m_bFlipHorizontal;
}

void
AnimatedSprite::Draw(Renderer& renderer)
{
	assert(m_pVertexData);
	m_pTexture->SetActive();
	m_pVertexData->SetActive(); // Set the vertex data required for this animated sprite before drawing it
	renderer.DrawAnimatedSprite(*this, m_iCurrentFrame, m_bFlipHorizontal); // Draw the current frame
}

void
AnimatedSprite::Animate()
{
	m_bAnimating = true;
	m_bAnimationComplete = false; // Reset animation complete state
}

void
AnimatedSprite::SetFrameDuration(float seconds)
{
	m_frameDuration = seconds;
}

void
AnimatedSprite::SetLooping(bool loop)
{
	m_bLooping = loop;
}

void
AnimatedSprite::Restart()
{
	m_iCurrentFrame = 0;
	m_fTimeElapsed = 0.0f;
	m_bAnimationComplete = false;
}

void
AnimatedSprite::SetAnimationCompleteCallback(AnimationCompleteCallback callBack)
{
	m_animationCompleteCallback = callBack;
}

bool
AnimatedSprite::IsAnimationComplete() const
{
	return m_bAnimationComplete;
}

bool
AnimatedSprite::IsLooping() const
{
	return m_bLooping;
}

void
AnimatedSprite::DebugDraw()
{
	ImGui::SliderInt("Frame ", &m_iCurrentFrame, 0, m_iTotalFrames - 1);
}