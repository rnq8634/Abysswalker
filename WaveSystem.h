#ifndef __WAVESYSTEM_H__
#define __WAVESYSTEM_H__

// Lib includes
#include <string>

// Forward declarations
class SceneAbyssWalker;
class Player;
class InputSystem;
class Renderer;

enum class WaveState
{
    PRE_WAVE_DELAY,
    IN_WAVE,
    INTERMISSION,
    GAME_WON,
    GAME_END_PROMPT
};

class WaveSystem
{
    // Member methods
public:
    WaveSystem(SceneAbyssWalker* scene, Player* player); // Scene provides context
    ~WaveSystem();

    void Initialise();
    void Process(float deltaTime, InputSystem& inputSystem);
    void ResetForNewGame();

    void NotifyEnemyKilled();
    void NotifyBossKilled();

    // Getters for abysswalker
    WaveState GetCurrentState() const { return m_currentWaveState; }
    int GetCurrentWaveNumber() const { return m_currentWaveNumber; }
    int GetEnemiesKilledThisWave() const { return m_enemiesKilledThisWave; }
    float GetWaveTimer() const { return m_waveTimer; }
    float GetIntermissionTimer() const { return m_intermissionTimer; }
    bool IsMaxWavesReached() const { return m_currentWaveNumber > MAX_WAVES;  }
    bool IsBossKilled() const { return m_bBossKilled; }

    // Methods to be called by SceneAbyssWalker
    void EndIntermission();
    void TransitionToState(WaveState newState);
    void ResetWaveTimerForPreWave();
    void ResetIntermissionTimer();
    void SetEnemiesKilledThisWave(int count);

private:
    void StartNewWave();
    void ProcessEndOfWaveLogic(); // Handles logic after wave timer or kills are met
    void StartIntermission();

    // Internal state-specific processing
    void ProcessPreWaveDelay(float deltaTime);
    void ProcessInWave(float deltaTime);
    void ProcessIntermission(float deltaTime, InputSystem& inputSystem);

    // Member data
public:
    static const float PRE_WAVE_DELAY_DURATION;
    static const float WAVE_DURATION;
    static const float INTERMISSION_DURATION;
    static const int KILLS_TO_END_WAVE_EARLY;
    static const int MAX_WAVES; // Don't forget to set this to 10!
    static const float BOSS_WAVE_DURATION;

private:
    SceneAbyssWalker* m_pScene;  
    Player* m_pPlayer;

    WaveState m_currentWaveState;
    int m_currentWaveNumber;
    float m_waveTimer;
    float m_intermissionTimer;
    int m_enemiesKilledThisWave;
    bool m_bBossKilled;
};

#endif // !__WAVESYSTEM_H__