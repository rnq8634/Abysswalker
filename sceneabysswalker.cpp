// This inlcude
#include "SceneAbyssWalker.h"

// Local Include
#include "Player.h"
#include "EnemyType2.h"
#include "EnemyBat.h" 
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "LogManager.h" 
#include "AnimatedSprite.h" 
#include "SoundSystem.h"
#include "Game.h"
#include "Texture.h"
#include "XboxController.h"
#include "PlayerStats.h"
#include "Boss.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm>
#include <cstdio>

const std::string BGM_FIGHT_1_ID = "DMC4";
const std::string BGM_FIGHT_2_ID = "DMC3";
const char* BGM_FIGHT_1_FILEPATH = "assets/sounds/fightBGM_1.mp3";
const char* BGM_FIGHT_2_FILEPATH = "assets/sounds/fightBGM_2.mp3";

SceneAbyssWalker::SceneAbyssWalker()
    : m_pPlayer(nullptr)
    , m_pRenderer(nullptr)
    , m_playerChoseToQuit(false)
    , m_pWaveSystem(nullptr)
    , m_pEnemySpawner(nullptr)
    , m_pGameEndPrompt(nullptr)
    , m_pUpgradeMenu(nullptr)
    , m_pPlayerHUD(nullptr)
    , m_pCollisionSystem(nullptr)
    , m_pWaveCountTextTexture(nullptr)
    , m_pWaveCountTextSprite(nullptr)
    , m_pWaveTimerTextTexture(nullptr)
    , m_pWaveTimerTextSprite(nullptr)
    , m_lastWaveTimerStr("")
    , m_lastWaveCountStr("")
    , m_pmoonBackground(nullptr)
    , m_pCurrentBGMChannel(nullptr)
    , m_currentBGMState(CurrentPlayingBGM::NONE)
    , m_bInitialised(false)
    , m_pBoss(nullptr)
    , m_bBossHasSpawned(false)
{
}

SceneAbyssWalker::~SceneAbyssWalker()
{
    if (m_pCurrentBGMChannel)
    {
        SoundSystem::GetInstance().StopChannel(m_pCurrentBGMChannel);
        m_pCurrentBGMChannel = nullptr;
    }

    delete m_pBoss;
    m_pBoss = nullptr;

    delete m_pWaveCountTextTexture;
    m_pWaveCountTextTexture = nullptr;

    delete m_pWaveCountTextSprite;
    m_pWaveCountTextSprite = nullptr;

    delete m_pWaveTimerTextSprite;
    m_pWaveTimerTextSprite = nullptr;

    delete m_pWaveTimerTextTexture;
    m_pWaveTimerTextTexture = nullptr;

    delete m_pCollisionSystem;
    m_pCollisionSystem = nullptr;

    delete m_pPlayerHUD;
    m_pPlayerHUD = nullptr;

    delete m_pUpgradeMenu;
    m_pUpgradeMenu = nullptr;

    delete m_pEnemySpawner;
    m_pEnemySpawner = nullptr;

    delete m_pGameEndPrompt;
    m_pGameEndPrompt = nullptr;

    delete m_pWaveSystem;
    m_pWaveSystem = nullptr;

    delete m_pPlayer;
    m_pPlayer = nullptr;

    for (EnemyBat* enemyBat : m_enemyBats)
    {
        delete enemyBat;
    }
    m_enemyBats.clear();

    for (EnemyType2* enemyType2 : m_enemyType2)
    {
        delete enemyType2;
    }
    m_enemyType2.clear();

    // Background
    delete m_pmoonBackground; 
    m_pmoonBackground = nullptr;
}

void SceneAbyssWalker::fullBackground(Renderer& renderer)
{
    if (m_pmoonBackground) return;

    m_pmoonBackground = renderer.CreateSprite("assets/backgrounds/main_background.png");

    const int screenWidth = renderer.GetWidth();
    const int screenHeight = renderer.GetHeight();
    const int screenCenterX = screenWidth / 2;

    if (m_pmoonBackground)
    {
        float scaleX = static_cast<float>(screenWidth) / m_pmoonBackground->GetOriginalWidth();
        float scaleY = static_cast<float>(screenHeight) / m_pmoonBackground->GetOriginalHeight();

        m_pmoonBackground->SetX(screenCenterX);
        m_pmoonBackground->SetY(screenHeight / 2);
        m_pmoonBackground->SetScale(scaleX, scaleY);
        m_pmoonBackground->SetFlipHorizontal(true);
        m_pmoonBackground->SetAngle(180.0f);
    }
}

bool SceneAbyssWalker::Initialise(Renderer& renderer)
{
    m_pRenderer = &renderer;
    m_bInitialised = false;
    if (!m_pmoonBackground)
    {
        fullBackground(*m_pRenderer);
    }

    // Bats
    for (EnemyBat* enemyBat : m_enemyBats) delete enemyBat;
    m_enemyBats.clear();

    // Type 2
    for (EnemyType2* enemyType2 : m_enemyType2) delete enemyType2;
    m_enemyType2.clear();

    delete m_pBoss;
    m_pBoss = nullptr;
    m_bBossHasSpawned = false;

    // Load Player 
    if (m_pPlayer) 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::Initialise - Player exists, calling ResetForNewGame.");
        m_pPlayer->ResetForNewGame();
    }

    else 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::Initialise - Player is NULL, creating new.");
        m_pPlayer = new Player();
        if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer)) 
        {
            delete m_pPlayer; 
            m_pPlayer = nullptr;
            m_bInitialised = false;
            return false;
        }
    }

    if (!m_pUpgradeMenu) m_pUpgradeMenu = new UpgradeMenu(m_pRenderer, m_pPlayer, this);
    if (!m_pUpgradeMenu) { LogManager::GetInstance().Log("Failed to create/load UpgradeMenu!!"); m_bInitialised = false; return false; }

    if (!m_pGameEndPrompt) m_pGameEndPrompt = new GameEndPrompt(m_pRenderer, m_pPlayer, this);
    if (!m_pGameEndPrompt) { LogManager::GetInstance().Log("Failed to create/load Game End Prompt Menu!!"); m_bInitialised = false; return false; }

    if (!m_pWaveSystem) m_pWaveSystem = new WaveSystem(this, m_pPlayer);
    if (!m_pWaveSystem) { LogManager::GetInstance().Log("Failed to create/load WaveManager!!"); m_bInitialised = false; return false; }
    m_pWaveSystem->ResetForNewGame(); 

    if (!m_pEnemySpawner) m_pEnemySpawner = new EnemySpawner(this, m_pPlayer, m_pRenderer);
    if (!m_pEnemySpawner) { LogManager::GetInstance().Log("Failed to create/load EnemySpawner!!"); m_bInitialised = false; return false; }
    m_pEnemySpawner->Reset(); 

    if (!m_pPlayerHUD) m_pPlayerHUD = new PlayerHUD(m_pRenderer, m_pPlayer);
    if (!m_pPlayerHUD) { LogManager::GetInstance().Log("Failed to create/load playerHUD!!"); m_bInitialised = false; return false; }

    if (!m_pCollisionSystem) m_pCollisionSystem = new CollisionSystem();
    if (!m_pCollisionSystem) { LogManager::GetInstance().Log("Failed to create/load Collision System!!"); m_bInitialised = false; return false; }

    // Load BGMs (SoundSystem handles "already loaded" gracefully)
    SoundSystem& soundSys = SoundSystem::GetInstance();
    if (!soundSys.LoadSound(BGM_FIGHT_1_FILEPATH, BGM_FIGHT_1_ID, false, false))
    {
        LogManager::GetInstance().Log(("Failed to load BGM: " + std::string(BGM_FIGHT_1_FILEPATH)).c_str());
    }
    if (!soundSys.LoadSound(BGM_FIGHT_2_FILEPATH, BGM_FIGHT_2_ID, false, false))
    {
        LogManager::GetInstance().Log(("Failed to load BGM: " + std::string(BGM_FIGHT_2_FILEPATH)).c_str());
    }

    m_playerChoseToQuit = false;
    
    if (m_pUpgradeMenu) m_pUpgradeMenu->SetActive(false); 
    if (m_pGameEndPrompt) m_pGameEndPrompt->SetActive(false);

    StartBGM(); // BGM starts when scene starts

    LogManager::GetInstance().Log("SceneAbyssWalker::Initialise successful.");
    m_bInitialised = true;
    return true;
}

void SceneAbyssWalker::SpawnBoss()
{
    if (!m_pRenderer || !m_pPlayer)
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Cannot spawn boss.");
        return;
    }

    if (m_bBossHasSpawned)
    {
        if (m_pBoss && m_pBoss->IsAlive())
        {
            LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Boss already spawned and is alive.");
        }
        else
        {
            LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Boss was already spawned in this session.");
        }

        return;
    }

    if (m_pBoss)
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Previous boss instance exists, deleting it.");
        delete m_pBoss;
        m_pBoss = nullptr;
    }

    m_pBoss = new Boss();
    if (!m_pBoss)
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Failed to allocate Boss object.");
        return;
    }

    Vector2 bossSpawnPosition;
    bossSpawnPosition.x = static_cast<float>(m_pRenderer->GetWidth()) / 2.0f;
    bossSpawnPosition.y = Boss::kGroundLevel;

    if (m_pBoss->Initialise(*m_pRenderer, bossSpawnPosition))
    {
        m_pBoss->m_pTargetPlayer = m_pPlayer;
        m_pBoss->SetSceneReference(this);
        m_bBossHasSpawned = true;
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Boss spawned successfully.");
    }
    else
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnBoss - Boss->Initialise() failed.");
        delete m_pBoss;
        m_pBoss = nullptr;
        m_bBossHasSpawned = false;
    }
}

void SceneAbyssWalker::StartBGM()
{
    SoundSystem& soundSys = SoundSystem::GetInstance();
    if (m_pCurrentBGMChannel)
    {
        soundSys.StopChannel(m_pCurrentBGMChannel);
        m_pCurrentBGMChannel = nullptr;
    }

    // BGM 1
    m_pCurrentBGMChannel = soundSys.PlaySound(BGM_FIGHT_1_ID);
    if (m_pCurrentBGMChannel)
    {
        soundSys.SetChannelVolume(m_pCurrentBGMChannel, 0.5f);
        m_currentBGMState = CurrentPlayingBGM::BGM1;
    }
    else
    {
        m_currentBGMState = CurrentPlayingBGM::NONE;
        LogManager::GetInstance().Log("Failed to play BGM 1!!!");
    }
}

void SceneAbyssWalker::ProcessBGMTransition()
{
    if (m_currentBGMState == CurrentPlayingBGM::NONE && m_pCurrentBGMChannel) 
    {
        SoundSystem::GetInstance().StopChannel(m_pCurrentBGMChannel);
        m_pCurrentBGMChannel = nullptr;
    }

    if (!m_pCurrentBGMChannel)
    {
        if (m_currentBGMState != CurrentPlayingBGM::NONE)
        {
            LogManager::GetInstance().Log("BGM channel was null but state was not NONE. Attempting to restart BGM sequence.");
            StartBGM();
        }
        return;
    }

    bool isPlaying = false;
    FMOD_RESULT result = m_pCurrentBGMChannel->isPlaying(&isPlaying);

    if (result == FMOD_OK && !isPlaying)
    {
        SoundSystem& soundSys = SoundSystem::GetInstance();
        LogManager::GetInstance().Log(("Current BGM (State: " + std::to_string(static_cast<int>(m_currentBGMState)) + ") finished playing.").c_str());
        m_pCurrentBGMChannel = nullptr;

        if (m_currentBGMState == CurrentPlayingBGM::BGM1)
        {
            m_pCurrentBGMChannel = soundSys.PlaySound(BGM_FIGHT_2_ID);
            if (m_pCurrentBGMChannel)
            {
                soundSys.SetChannelVolume(m_pCurrentBGMChannel, 0.5f);
                m_currentBGMState = CurrentPlayingBGM::BGM2;
                LogManager::GetInstance().Log(("Transitioned to BGM 2 (ID: " + BGM_FIGHT_2_ID + ")").c_str());
            }
            else
            {
                m_currentBGMState = CurrentPlayingBGM::NONE;
                LogManager::GetInstance().Log(("Failed to play BGM 2 (ID: " + BGM_FIGHT_2_ID + ") after BGM 1 finished.").c_str());
            }
        }
        else if (m_currentBGMState == CurrentPlayingBGM::BGM2)
        {
            m_pCurrentBGMChannel = soundSys.PlaySound(BGM_FIGHT_1_ID);
            if (m_pCurrentBGMChannel)
            {
                soundSys.SetChannelVolume(m_pCurrentBGMChannel, 0.5f);
                m_currentBGMState = CurrentPlayingBGM::BGM1;
                LogManager::GetInstance().Log(("Looped back to BGM 1 (ID: " + BGM_FIGHT_1_ID + ")").c_str());
            }
            else
            {
                m_currentBGMState = CurrentPlayingBGM::NONE;
                LogManager::GetInstance().Log(("Failed to loop back to BGM 1 (ID: " + BGM_FIGHT_1_ID + ")").c_str());
            }
        }
        else
        {
            LogManager::GetInstance().Log("BGM finished but was in unexpected state. Restarting BGM sequence.");
            StartBGM();
        }
    }
    else if (result != FMOD_OK)
    {
        LogManager::GetInstance().Log(("FMOD Error checking BGM: " + std::to_string(result) + ". Stopping BGM.").c_str());
        SoundSystem::GetInstance().StopChannel(m_pCurrentBGMChannel);
        m_pCurrentBGMChannel = nullptr;
        m_currentBGMState = CurrentPlayingBGM::NONE;
    }
}

void SceneAbyssWalker::RestartGame()
{
    LogManager::GetInstance().Log("Restarting game...");
    // Clear existing enemies
    for (EnemyBat* enemyBat : m_enemyBats) delete enemyBat;
    m_enemyBats.clear();
    for (EnemyType2* enemyType2 : m_enemyType2) delete enemyType2;
    m_enemyType2.clear();

    // Delete boss so can spawn again for next instance
    delete m_pBoss;
    m_pBoss = nullptr;
    m_bBossHasSpawned = false;

    if (m_pWaveSystem) m_pWaveSystem->ResetForNewGame();
    if (m_pEnemySpawner) m_pEnemySpawner->Reset();

    if (m_pUpgradeMenu) m_pUpgradeMenu->SetActive(false);
    if (m_pGameEndPrompt) m_pGameEndPrompt->SetActive(false);

    if (m_pPlayer) m_pPlayer->ResetForNewGame();

    m_playerChoseToQuit = false;

    StartBGM();
}

void SceneAbyssWalker::Process(float deltaTime, InputSystem& inputSystem)
{
    if (!m_pPlayer || !m_pRenderer || !m_pWaveSystem) return;
    if (m_playerChoseToQuit)
    {
        Game::GetInstance().Quit();
        return;
    }

    ProcessBGMTransition();

    m_pWaveSystem->Process(deltaTime, inputSystem);
    WaveState currentWaveState = m_pWaveSystem->GetCurrentState();

    if (currentWaveState == WaveState::INTERMISSION)
    {
        UpdateUpgradeMenuUI(inputSystem);
    }
    
    if (currentWaveState == WaveState::GAME_END_PROMPT || currentWaveState == WaveState::GAME_WON)
    {
        UpdateGameEndPromptUI(inputSystem);
    }

    if (currentWaveState == WaveState::IN_WAVE ||
        currentWaveState == WaveState::INTERMISSION ||
        currentWaveState == WaveState::PRE_WAVE_DELAY)
    {
        // Player input processing
        const float moveSpeed = 150.0f;
        const float rollSpeed = 200.0f;
        bool isMoving = false;

        bool isPlayerAttemptingMove = false;
        PlayerState stateBeforePlayerInputAndProcess = PlayerState::IDLE;

        if (m_pPlayer)
        {
            stateBeforePlayerInputAndProcess = m_pPlayer->GetCurrentState();
        }

        XboxController* pController = nullptr;
        if (inputSystem.GetNumberOfControllersAttached() > 0)
        {
            pController = inputSystem.GetController(0);
        }

        if (m_pPlayer && m_pPlayer->IsAlive())
        {
            // Player Move Left
            if (inputSystem.GetKeyState(SDL_SCANCODE_A) == BS_HELD || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_LEFT) == BS_HELD))
            {
                m_pPlayer->MoveLeft(moveSpeed);
                isMoving = true;
            }

            // Player Move Right
            else if (inputSystem.GetKeyState(SDL_SCANCODE_D) == BS_HELD || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == BS_HELD))
            {
                m_pPlayer->MoveRight(moveSpeed);
                isMoving = true;
            }

            // Player Jump
            if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_A) == BS_PRESSED))
            {
                m_pPlayer->Jump();
            }

            // Player Attack
            if (inputSystem.GetKeyState(SDL_SCANCODE_J) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_X) == BS_PRESSED))
            {
                m_pPlayer->Attack();
                // add sound ^^^ like jump
            }

            // Player Roll
            if (inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_B) == BS_PRESSED))
            {
                m_pPlayer->Roll(rollSpeed);
                // add sound here
            }

            if (!isMoving &&
                m_pPlayer->IsAlive() &&
                m_pPlayer->GetCurrentState() != PlayerState::JUMPING &&
                m_pPlayer->GetCurrentState() != PlayerState::FALLING &&
                m_pPlayer->GetCurrentState() != PlayerState::TURNING &&
                m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
                m_pPlayer->GetCurrentState() != PlayerState::HURT &&
                m_pPlayer->GetCurrentState() != PlayerState::ATTACKING)
            {
                m_pPlayer->StopMoving();
            }
        }

        m_pPlayer->Process(deltaTime);

        bool processEnemies = (currentWaveState == WaveState::IN_WAVE);

        for (EnemyBat* enemyBat : m_enemyBats) 
        {
            if (enemyBat->IsAlive() && processEnemies) 
            {
                if (enemyBat->m_pTargetPlayer == nullptr && m_pPlayer) enemyBat->m_pTargetPlayer = m_pPlayer;
                enemyBat->Process(deltaTime);
            }
            else if (!enemyBat->IsAlive() && enemyBat->GetCurrentState() == EnemyBatState::DEATH) 
            {
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                if (sprite) enemyBat->UpdateSprite(sprite, deltaTime);
            }
        }

        // Similar loop for EnemyType2
        for (EnemyType2* enemyT2 : m_enemyType2) 
        {
            if (enemyT2->IsAlive() && processEnemies) 
            {
                if (enemyT2->m_pTargetPlayer == nullptr && m_pPlayer) enemyT2->m_pTargetPlayer = m_pPlayer;
                enemyT2->Process(deltaTime);
            }
            else if (!enemyT2->IsAlive() && enemyT2->GetCurrentState() == EnemyType2State::DEATH) 
            {
                AnimatedSprite* sprite = enemyT2->GetCurrentAnimatedSprite();
                if (sprite) enemyT2->UpdateSprite(sprite, deltaTime);
            }
        }

        // Boss
        if (m_pBoss && m_pBoss->IsAlive())
        {
            if (m_pBoss->m_pTargetPlayer == nullptr && m_pPlayer) m_pBoss->m_pTargetPlayer = m_pPlayer;
            m_pBoss->Process(deltaTime);

            if (!m_pBoss->IsAlive())
            {
                LogManager::GetInstance().Log("Is boss dead");
                m_pWaveSystem->NotifyBossKilled();
            }
        }
        else if (m_pBoss && !m_pBoss->IsAlive() && m_pBoss->GetCurrentState() == BossState::DEATH)
        {
            m_pBoss->UpdateSprite(m_pBoss->GetCurrentAnimatedSprite(), deltaTime);
        }

        bool isBossActive = (m_pBoss && m_pBoss->IsAlive() && m_bBossHasSpawned);
        if (m_pEnemySpawner && m_pPlayer->IsAlive() && !isBossActive && m_pWaveSystem->GetCurrentWaveNumber() < WaveSystem::MAX_WAVES && currentWaveState == WaveState::IN_WAVE)
        {
            m_pEnemySpawner->Update(deltaTime, m_enemyBats, m_enemyType2);
        }

        // Collisions
        if (m_pCollisionSystem && m_pPlayer && m_pPlayer->IsAlive())
        {
            m_pCollisionSystem->ProcessCollisions(m_pPlayer, m_enemyBats, m_enemyType2, m_pBoss);
        }
    }

    CleanupDead();
}

// Cleaning up
void SceneAbyssWalker::EndWaveEnemyCleanup()
{
    for (EnemyBat* bat : m_enemyBats)
    {
        if (bat->IsAlive())
        {
            bat->TransitionToState(EnemyBatState::DEATH);
            bat->SetDead();
        }
    }
    for (EnemyType2* t2 : m_enemyType2)
    {
        if (t2->IsAlive())
        {
            t2->TransitionToState(EnemyType2State::DEATH);
            t2->SetDead();
        }
    }
}

void SceneAbyssWalker::AddEnemy(EnemyBat* bat)
{
    if (bat) 
    {
        m_enemyBats.push_back(bat);
    }
}

void SceneAbyssWalker::AddEnemy(EnemyType2* type2)
{
    if (type2) 
    {
        m_enemyType2.push_back(type2);
    }
}

void SceneAbyssWalker::ClearUpgradeMenuUI() 
{
    if (m_pUpgradeMenu)
    {
        m_pUpgradeMenu->SetActive(false);
    }
}

void SceneAbyssWalker::SetupUpgradeMenuUI() 
{
    if (m_pUpgradeMenu)
    {
        m_pUpgradeMenu->SetActive(true);
    }
}

// Cheats
void SceneAbyssWalker::DebugSkipToLastWave()
{
    if (!m_pWaveSystem || !m_pPlayer)
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::DebugSkipToLastWave - WaveSystem or Player is null. Cannot Skip.");
        return;
    }

    LogManager::GetInstance().Log("SceneAbyssWalker::DebugSkipToLastWave - Initiating skip to last wave");

    EndWaveEnemyCleanup();
    CleanupDead();

    // Make sure no other existing boss is spawned by accident
    if (m_pBoss)
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::DebugSkipToLastWave has removed existing boss instance");
        delete m_pBoss;
        m_pBoss = nullptr;
    }
    m_bBossHasSpawned = false;

    // Reset UI in case they might glitch out
    if (m_pUpgradeMenu)
    {
        m_pUpgradeMenu->SetActive(false);
        LogManager::GetInstance().Log("DebugSkipToLastWave Upg menu deactivated");
    }

    if (m_pGameEndPrompt)
    {
        m_pGameEndPrompt->SetActive(false);
        LogManager::GetInstance().Log("DebugSkipToLastWave GameEnd menu been deactivated");
    }

    m_pWaveSystem->SetCurrentWaveNumber(WaveSystem::MAX_WAVES - 1);
    m_pWaveSystem->SetEnemiesKilledThisWave(0);
    m_pWaveSystem->ResetBossKilledFlag();

    LogManager::GetInstance().Log("DebugSkipLastWave has triggered boss wave!!");
    m_pWaveSystem->StartNewWave();
}

void SceneAbyssWalker::UpdateUpgradeMenuUI(InputSystem& inputSystem) 
{
    if (m_pUpgradeMenu && m_pUpgradeMenu->IsActive())
    {
        m_pUpgradeMenu->UpdateUI(inputSystem);
    }
}

void SceneAbyssWalker::DrawUpgradeMenu(Renderer& renderer) 
{
    if (m_pUpgradeMenu && m_pUpgradeMenu->IsActive())
    {
        m_pUpgradeMenu->Draw(renderer);
    }
}

// --- Game End Prompts (Custom UI) ---
void SceneAbyssWalker::ClearGameEndPromptUI() 
{
    if (m_pGameEndPrompt)
    {
        m_pGameEndPrompt->SetActive(false);
    }
}

void SceneAbyssWalker::SetupGameEndPromptUI(const std::string& titleMessage) 
{
    if (m_pGameEndPrompt)
    {
        m_pGameEndPrompt->SetActive(true, titleMessage);
    }
}

void SceneAbyssWalker::UpdateGameEndPromptUI(InputSystem& inputSystem) 
{
    if (m_pGameEndPrompt && m_pGameEndPrompt->IsActive())
    {
        m_pGameEndPrompt->UpdateUI(inputSystem);
    }
}

void SceneAbyssWalker::DrawEndGamePrompts(Renderer& renderer) 
{
    if (m_pGameEndPrompt && m_pGameEndPrompt->IsActive())
    {
        m_pGameEndPrompt->Draw(renderer);
    }
}

void SceneAbyssWalker::NotifyEnemyKilled()
{
    if (m_pWaveSystem)
    {
        m_pWaveSystem->NotifyEnemyKilled();
    }
}

void SceneAbyssWalker::CleanupDead()
{
    // Cleaning up the enemy bats
    WaveState currentWaveState = WaveState::PRE_WAVE_DELAY;

    if (m_pWaveSystem)
    {
        currentWaveState = m_pWaveSystem->GetCurrentState();
    }

    m_enemyBats.erase(std::remove_if(m_enemyBats.begin(), m_enemyBats.end(), [&](EnemyBat* enemyBat)
        {
            if (!enemyBat->IsAlive())
            {
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                bool animComplete = (sprite && !sprite->IsLooping() && sprite->IsAnimationComplete());
                //bool noSprite = !sprite;
                bool forceClean = (currentWaveState != WaveState::IN_WAVE && enemyBat->GetCurrentState() == EnemyBatState::DEATH);

                if (animComplete || forceClean)
                {
                    LogManager::GetInstance().Log("Cleaning up dead Bat.");
                    delete enemyBat;
                    return true;
                }
            }
            return false;
        }), m_enemyBats.end());

    // Cleaning up Type 2's
    m_enemyType2.erase(std::remove_if(m_enemyType2.begin(), m_enemyType2.end(),
        [&](EnemyType2* enemyT2) 
        {
            if (!enemyT2->IsAlive()) 
            {
                AnimatedSprite* sprite = enemyT2->GetCurrentAnimatedSprite();
                bool animComplete = (sprite && !sprite->IsLooping() && sprite->IsAnimationComplete());
                //bool noSprite = !sprite;
                bool forceClean = (currentWaveState != WaveState::IN_WAVE && enemyT2->GetCurrentState() == EnemyType2State::DEATH);

                if (animComplete || forceClean) 
                {
                    LogManager::GetInstance().Log("Cleaning up dead EnemyType2.");
                    delete enemyT2;
                    return true;
                }
            }
            return false;
        }), m_enemyType2.end());
}

void SceneAbyssWalker::Draw(Renderer& renderer)
{
    if (m_pmoonBackground) m_pmoonBackground->Draw(renderer);

    WaveState currentWaveState = WaveState::PRE_WAVE_DELAY;
    int currentWaveNum = 0;
    float waveTimer = 0.0f;

    if (m_pWaveSystem)
    {
        currentWaveState = m_pWaveSystem->GetCurrentState();
        currentWaveNum = m_pWaveSystem->GetCurrentWaveNumber();
        waveTimer = m_pWaveSystem->GetWaveTimer();
    }

    if (currentWaveState == WaveState::INTERMISSION)
    {
        DrawUpgradeMenu(renderer);
    }

    if (currentWaveState == WaveState::GAME_WON || currentWaveState == WaveState::GAME_END_PROMPT)
    {
        DrawEndGamePrompts(renderer);
    }

    // Draws player
    if (m_pPlayer)
    {
        m_pPlayer->Draw(renderer);
    }

    // Draws bats
    for (EnemyBat* enemyBat : m_enemyBats)
    {
        enemyBat->Draw(renderer);
    }

    // Draw EnemyType2
    for (EnemyType2* enemyT2 : m_enemyType2)
    {
        enemyT2->Draw(renderer);
    }

    // Draw the Boss
    if (m_pBoss)
    {
        if (m_pBoss->IsAlive() || (m_pBoss->GetCurrentState() == BossState::DEATH && m_pBoss->GetCurrentAnimatedSprite() && !m_pBoss->GetCurrentAnimatedSprite()->IsAnimationComplete()))
        {
            m_pBoss->Draw(renderer);
        }
    }

    // Player HUD
    if (m_pPlayerHUD)
    {
        m_pPlayerHUD->Draw();
    }

    // Wave timer
    if (currentWaveState == WaveState::IN_WAVE || currentWaveState == WaveState::PRE_WAVE_DELAY) 
    {
        char timerBuffer[64];

        if (currentWaveState == WaveState::PRE_WAVE_DELAY) 
        {
            if (currentWaveNum == 0) 
            {
                snprintf(timerBuffer, sizeof(timerBuffer), "Wave 1 Starting: %.0fs", std::max(0.0f, waveTimer));
            }
            else 
            {
                snprintf(timerBuffer, sizeof(timerBuffer), "Wave %d Starting: %.0fs", currentWaveNum + 1, std::max(0.0f, waveTimer));
            }
        }
        else if (currentWaveState == WaveState::IN_WAVE)
        {
            snprintf(timerBuffer, sizeof(timerBuffer), "Time Left: %.0fs", std::max(0.0f, waveTimer));
        }
        std::string timerStr(timerBuffer);

        if (m_lastWaveTimerStr != timerStr || !m_pWaveTimerTextSprite) 
        {
            delete m_pWaveTimerTextSprite; m_pWaveTimerTextSprite = nullptr;
            delete m_pWaveTimerTextTexture; m_pWaveTimerTextTexture = nullptr;

            m_pWaveTimerTextTexture = new Texture();
            m_pWaveTimerTextSprite = new Sprite();

            if (m_pWaveTimerTextSprite->InitialiseWithText(*m_pWaveTimerTextTexture, timerStr.c_str(), m_uiFontPath, m_uiFontSize)) 
            {
                m_lastWaveTimerStr = timerStr;
            }
        }

        if (m_pWaveTimerTextSprite) 
        {
            float panelWidth = static_cast<float>(m_pWaveTimerTextTexture->GetWidth()) + 20.0f;
            float panelHeight = 30.0f;
            float panelX = (renderer.GetWidth() / 2.0f) - (panelWidth / 2.0f);
            float panelY = 20.0f;
            renderer.DrawDebugRect(panelX, panelY, panelX + panelWidth, panelY + panelHeight, 30, 30, 30, 200);
            m_pWaveTimerTextSprite->SetX(static_cast<int>(panelX + panelWidth / 2.0f));
            m_pWaveTimerTextSprite->SetY(static_cast<int>(panelY + panelHeight / 2.0f));
            m_pWaveTimerTextSprite->Draw(renderer);
        }
    }

    else if (m_pWaveTimerTextSprite) 
    {
        delete m_pWaveTimerTextSprite; m_pWaveTimerTextSprite = nullptr;
        delete m_pWaveTimerTextTexture; m_pWaveTimerTextTexture = nullptr;
        m_lastWaveTimerStr = "";
    }

    // Wave count
    if (currentWaveState != WaveState::GAME_END_PROMPT && currentWaveState != WaveState::GAME_WON && m_pPlayerHUD) 
    {
        std::string countStr;
        if (currentWaveNum == 0 && currentWaveState == WaveState::PRE_WAVE_DELAY) {
            countStr = "Wave: 1";
        }

        else if (currentWaveState == WaveState::PRE_WAVE_DELAY && currentWaveNum > 0) 
        {
            countStr = "Wave: " + std::to_string(currentWaveNum + 1);
        }

        else if (currentWaveState == WaveState::IN_WAVE || currentWaveState == WaveState::INTERMISSION) 
        {
            countStr = "Wave: " + std::to_string(std::max(1, currentWaveNum));
        }

        if (!countStr.empty()) 
        {
            if (m_lastWaveCountStr != countStr || !m_pWaveCountTextSprite) 
            {
                delete m_pWaveCountTextSprite; m_pWaveCountTextSprite = nullptr;
                delete m_pWaveCountTextTexture; m_pWaveCountTextTexture = nullptr;
                m_pWaveCountTextTexture = new Texture();
                m_pWaveCountTextSprite = new Sprite();

                if (m_pWaveCountTextSprite->InitialiseWithText(*m_pWaveCountTextTexture, countStr.c_str(), m_uiFontPath, m_uiFontSize)) 
                {
                    m_lastWaveCountStr = countStr;
                }
            }

            if (m_pWaveCountTextSprite) 
            {
                float textWidth = static_cast<float>(m_pWaveCountTextTexture->GetWidth());
                float panelWidth = textWidth + 20.0f;
                float panelHeight = 30.0f;

                float groupStartX = m_pPlayerHUD->GetHealthBarStartX(renderer.GetWidth());
                float barsY = m_pPlayerHUD->GetBarsYPosition();

                float panelX = groupStartX - panelWidth - m_pPlayerHUD->GetBarSpacing();
                float panelY = barsY;

                renderer.DrawDebugRect(panelX, panelY, panelX + panelWidth, panelY + panelHeight, 30, 30, 30, 200);
                m_pWaveCountTextSprite->SetX(static_cast<int>(panelX + panelWidth / 2.0f));
                m_pWaveCountTextSprite->SetY(static_cast<int>(panelY + panelHeight / 2.0f));
                m_pWaveCountTextSprite->Draw(renderer);
            }
        }

        else if (m_pWaveCountTextSprite) 
        {
            delete m_pWaveCountTextSprite; m_pWaveCountTextSprite = nullptr;
            delete m_pWaveCountTextTexture; m_pWaveCountTextTexture = nullptr;
            m_lastWaveCountStr = "";
        }
    }

    else if (m_pWaveCountTextSprite) 
    {
        delete m_pWaveCountTextSprite; m_pWaveCountTextSprite = nullptr;
        delete m_pWaveCountTextTexture; m_pWaveCountTextTexture = nullptr;
        m_lastWaveCountStr = "";
    }

    if (m_pWaveSystem)
    {
        currentWaveState = m_pWaveSystem->GetCurrentState();
    }

    if (currentWaveState == WaveState::INTERMISSION)
    {
        if (m_pUpgradeMenu) m_pUpgradeMenu->Draw(renderer);
    }

    if (currentWaveState == WaveState::GAME_WON || currentWaveState == WaveState::GAME_END_PROMPT)
    {
        if (m_pGameEndPrompt) m_pGameEndPrompt->Draw(renderer);
    }
}

void SceneAbyssWalker::DebugDraw()
{
    if (ImGui::CollapsingHeader("Scene Debug"))
    {
        if (ImGui::Button("Force Spawn Boss"))
        {
            SpawnBoss();
        }

        if (m_pPlayer)
        {
            m_pPlayer->DebugDraw();
        }

        if (m_pBoss)
        {
            if (ImGui::CollapsingHeader("Boss Info"))
            {
                m_pBoss->DebugDraw();
                ImGui::Text("Boss Spawned: %s", m_bBossHasSpawned ? "Yes" : "No");
                ImGui::Text("Boss Alive: %s", m_pBoss->IsAlive() ? "Yes" : "No");
            }
        }

        if (ImGui::CollapsingHeader("Enemy Bat List"))
        {
            for (size_t i = 0; i < m_enemyBats.size(); ++i)
            {
                std::string enemyNodeId = "EnemyBat " + std::to_string(i);
                if (ImGui::TreeNode(enemyNodeId.c_str()))
                {
                    m_enemyBats[i]->DebugDraw();
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("EnemyType2 List"))
        {
            for (size_t i = 0; i < m_enemyType2.size(); ++i)
            {
                std::string enemyNodeId = "EnemyType2 " + std::to_string(i);
                if (ImGui::TreeNode(enemyNodeId.c_str()))
                {
                    m_enemyType2[i]->DebugDraw();
                    ImGui::TreePop();
                }
            }
        }
    }
}