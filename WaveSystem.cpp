// This icnludes
#include "WaveSystem.h"

// Local includes
#include "SceneAbyssWalker.h"
#include "Player.h"
#include "InputSystem.h"
#include "LogManager.h"

// Lib includes
#include <string>

const float WaveSystem::PRE_WAVE_DELAY_DURATION = 3.0f;
const float WaveSystem::WAVE_DURATION = 60.0f;
const float WaveSystem::BOSS_WAVE_DURATION = 180.0f;
const float WaveSystem::INTERMISSION_DURATION = 30.0f;
const int WaveSystem::KILLS_TO_END_WAVE_EARLY = 10;
const int WaveSystem::MAX_WAVES = 10;

WaveSystem::WaveSystem(SceneAbyssWalker* scene, Player* player)
    : m_pScene(scene)
    , m_pPlayer(player)
    , m_currentWaveState(WaveState::PRE_WAVE_DELAY)
    , m_currentWaveNumber(0)
    , m_waveTimer(0.0f)
    , m_intermissionTimer(0.0f)
    , m_enemiesKilledThisWave(0)
    , m_bBossKilled(false)
{
    // Ensuring if valid
    if (!m_pScene) 
    {
        LogManager::GetInstance().Log("WaveSystem critical error: Scene pointer is null.");
    }
    if (!m_pPlayer) 
    {
        LogManager::GetInstance().Log("WaveSystem critical error: Player pointer is null.");
    }
}

WaveSystem::~WaveSystem()
{
}

void WaveSystem::Initialise()
{
    // Initial setup when a new game/scene starts
    m_currentWaveNumber = 0;
    m_enemiesKilledThisWave = 0;
    m_waveTimer = PRE_WAVE_DELAY_DURATION;
    m_intermissionTimer = INTERMISSION_DURATION;
    m_currentWaveState = WaveState::PRE_WAVE_DELAY;
    m_bBossKilled = false;
}

void WaveSystem::ResetForNewGame()
{
    Initialise();
}

void WaveSystem::Process(float deltaTime, InputSystem& inputSystem)
{
    if (!m_pPlayer || !m_pScene) return;

    if (m_currentWaveState == WaveState::GAME_WON || m_currentWaveState == WaveState::GAME_END_PROMPT)
    {
        return;
    }

    switch (m_currentWaveState)
    {
    case WaveState::PRE_WAVE_DELAY:
        ProcessPreWaveDelay(deltaTime);
        break;

    case WaveState::IN_WAVE:
        ProcessInWave(deltaTime);
        break;

    case WaveState::INTERMISSION:
        ProcessIntermission(deltaTime, inputSystem);
        break;

    case WaveState::GAME_WON:
    case WaveState::GAME_END_PROMPT:

        break;
    }
}

void WaveSystem::ProcessPreWaveDelay(float deltaTime)
{
    m_waveTimer -= deltaTime;
    if (m_waveTimer <= 0)
    {
        StartNewWave();
    }
}

void WaveSystem::ProcessInWave(float deltaTime)
{
    if (!m_pPlayer->IsAlive())
    {
        //if (m_pScene) m_pScene->EndWaveEnemyCleanup();
        TransitionToState(WaveState::GAME_END_PROMPT);
        if (m_pScene) m_pScene->SetupGameEndPromptUI("YOU DIED!");
        return;
    }

    if (m_currentWaveState == WaveState::GAME_WON)
    {
        return;
    }

    m_waveTimer -= deltaTime;
    bool isBossWave = (m_currentWaveNumber == MAX_WAVES);

    if (isBossWave)
    {
        // If the player runs out of time AND does not kill the boss
        if (m_waveTimer <= 0)
        {
            if (!m_bBossKilled)
            {
                // Player loses by default
                if (m_pScene) m_pScene->EndWaveEnemyCleanup();
                TransitionToState(WaveState::GAME_END_PROMPT);
                if (m_pScene) m_pScene->SetupGameEndPromptUI("The Night has consumed you...");

                return;
            }
        }
    }
    else
    {
        if (m_waveTimer <= 0 || m_enemiesKilledThisWave >= KILLS_TO_END_WAVE_EARLY)
        {
            ProcessEndOfWaveLogic();
        }
    }
}

void WaveSystem::ProcessEndOfWaveLogic()
{
    if (m_currentWaveNumber >= MAX_WAVES)
    {
        if (m_currentWaveNumber == MAX_WAVES)
        {
            if (m_bBossKilled && m_currentWaveState != WaveState::GAME_WON)
            {
                TransitionToState(WaveState::GAME_WON);

                if (m_pScene)
                {
                    m_pScene->SetupGameEndPromptUI("YOU HAVE SURVIVED THE NIGHT!");
                }
            }
            else if (!m_bBossKilled && m_currentWaveState != WaveState::GAME_END_PROMPT && m_waveTimer <= 0)
            {
                if (m_pScene) m_pScene->EndWaveEnemyCleanup();
                TransitionToState(WaveState::GAME_END_PROMPT);
                if (m_pScene) m_pScene->SetupGameEndPromptUI("The Night has consumed your soul...");
            }
        }
        return;
    }

    if (m_pScene) m_pScene->EndWaveEnemyCleanup(); 
    LogManager::GetInstance().Log(("Wave " + std::to_string(m_currentWaveNumber) + " ended. Kills: " + std::to_string(m_enemiesKilledThisWave)).c_str());

    StartIntermission();
}

void WaveSystem::ProcessIntermission(float deltaTime, InputSystem& inputSystem)
{
    m_intermissionTimer -= deltaTime;
    if (m_intermissionTimer <= 0)
    {
        EndIntermission();
    }
}

void WaveSystem::EndIntermission()
{
    LogManager::GetInstance().Log("Intermission has ended.");
    m_pScene->ClearUpgradeMenuUI();
    ResetWaveTimerForPreWave();
    TransitionToState(WaveState::PRE_WAVE_DELAY);
}


void WaveSystem::StartNewWave()
{
    m_currentWaveNumber++;
    m_enemiesKilledThisWave = 0;
    m_waveTimer = WAVE_DURATION;

    if (m_pScene && m_currentWaveNumber == MAX_WAVES)
    {
        LogManager::GetInstance().Log(("Starting Boss Wave: " + std::to_string(m_currentWaveNumber)).c_str());
        m_pScene->SpawnBoss();
        m_waveTimer = BOSS_WAVE_DURATION;
    }
    else
    {
        LogManager::GetInstance().Log(("Starting Wave " + std::to_string(m_currentWaveNumber)).c_str());
    }

    TransitionToState(WaveState::IN_WAVE);
}

void WaveSystem::StartIntermission()
{
    ResetIntermissionTimer();
    TransitionToState(WaveState::INTERMISSION);
    m_pScene->SetupUpgradeMenuUI();
    LogManager::GetInstance().Log("Starting Intermission.");
}

void WaveSystem::NotifyEnemyKilled()
{
    if (m_currentWaveState == WaveState::IN_WAVE)
    {
        m_enemiesKilledThisWave++;
    }
}

void WaveSystem::NotifyBossKilled()
{
    if (m_currentWaveState == WaveState::GAME_WON)
    {
        return;
    }

    m_bBossKilled = true;
    if (m_pScene)
    {
        m_pScene->EndWaveEnemyCleanup();
    }

    TransitionToState(WaveState::GAME_WON);
    if (m_pScene)
    {
        m_pScene->SetupGameEndPromptUI("YOU HAVE SURVIVED THE NIGHT!");
    }
}

void WaveSystem::SetCurrentWaveNumber(int number)
{
    m_currentWaveNumber = number;
    LogManager::GetInstance().Log(("CHEAT: Wave number set to " + std::to_string(m_currentWaveNumber)).c_str());
}

void WaveSystem::TransitionToState(WaveState newState)
{
    if (m_currentWaveState != newState) // Only log/act if state actually changes
    {
        m_currentWaveState = newState;
    }
}

void WaveSystem::SetEnemiesKilledThisWave(int count) // Or WaveManager::
{
    m_enemiesKilledThisWave = count;
    if (m_enemiesKilledThisWave < 0) 
    {
        m_enemiesKilledThisWave = 0; // Basic validation
    }
}

void WaveSystem::ResetWaveTimerForPreWave()
{
    m_waveTimer = PRE_WAVE_DELAY_DURATION;
}

void WaveSystem::ResetIntermissionTimer()
{
    m_intermissionTimer = INTERMISSION_DURATION;
}