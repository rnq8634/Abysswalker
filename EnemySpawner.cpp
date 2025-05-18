// This include
#include "EnemySpawner.h"

// Local include
#include "SceneAbyssWalker.h"
#include "Player.h"
#include "Renderer.h"
#include "EnemyBat.h"        
#include "EnemyType2.h"       
#include "LogManager.h"
#include "Vector2.h"    

// Lib includes
#include <cstdlib>           

EnemySpawner::EnemySpawner(SceneAbyssWalker* scene, Player* player, Renderer* renderer)
    : m_pScene(scene),
    m_pPlayer(player),
    m_pRenderer(renderer),
    m_batSpawnTimer(0.0f),
    m_type2SpawnTimer(0.0f)
{
    if (!m_pScene || !m_pPlayer || !m_pRenderer) 
    {
        LogManager::GetInstance().Log("EnemySpawner critical error: Null pointer in constructor.");
    }
    m_batSpawnTimer = m_batSpawnInterval;
    m_type2SpawnTimer = m_type2SpawnInterval;
}

EnemySpawner::~EnemySpawner()
{
}

void EnemySpawner::Reset()
{
    m_batSpawnTimer = m_batSpawnInterval;
    m_type2SpawnTimer = m_type2SpawnInterval;
}

void EnemySpawner::Update(float deltaTime,
    const std::vector<EnemyBat*>& currentBats,
    const std::vector<EnemyType2*>& currentType2s)
{
    if (!m_pRenderer || !m_pScene || !m_pPlayer) return;

    size_t totalCurrentEnemies = currentBats.size() + currentType2s.size();

    // Bats
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && currentBats.size() < static_cast<size_t>(m_maxBats)) 
    {
        m_batSpawnTimer += deltaTime;
        if (m_batSpawnTimer >= m_batSpawnInterval) 
        {
            m_batSpawnTimer = 0.0f;
            AttemptSpawn(EnemySpawnType::BAT, currentBats, currentType2s);
        }
    }

    // Type2
    totalCurrentEnemies = currentBats.size() + currentType2s.size();
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && currentType2s.size() < static_cast<size_t>(m_maxType2)) 
    {
        m_type2SpawnTimer += deltaTime;
        if (m_type2SpawnTimer >= m_type2SpawnInterval) 
        {
            m_type2SpawnTimer = 0.0f;
            AttemptSpawn(EnemySpawnType::TYPE2, currentBats, currentType2s);
        }
    }
}

void EnemySpawner::AttemptSpawn(EnemySpawnType type,
    const std::vector<EnemyBat*>& currentBats,
    const std::vector<EnemyType2*>& currentType2s)
{
    // Double check limits before actual spawn call
    size_t totalCurrentEnemies = currentBats.size() + currentType2s.size();
    if (totalCurrentEnemies >= static_cast<size_t>(m_maxEnemies)) return;

    bool spawnOnLeft = rand() % 2 == 0;

    if (type == EnemySpawnType::BAT && currentBats.size() < static_cast<size_t>(m_maxBats)) 
    {
        SpawnBat(spawnOnLeft);
    }
    else if (type == EnemySpawnType::TYPE2 && currentType2s.size() < static_cast<size_t>(m_maxType2)) 
    {
        SpawnType2(spawnOnLeft);
    }
}

void EnemySpawner::SpawnBat(bool spawnOnLeft)
{
    if (!m_pRenderer || !m_pPlayer || !m_pScene) return;

    EnemyBat* newEnemyBat = new EnemyBat();
    if (!newEnemyBat) {
        LogManager::GetInstance().Log("EnemySpawner: Failed to allocate Bat");
        return;
    }
   
    newEnemyBat->m_pTargetPlayer = m_pPlayer;
    newEnemyBat->SetSceneReference(m_pScene); 

    Vector2 spawnPos;
    const float spawnXOffset = 100.0f;
    spawnPos.x = spawnOnLeft ? -spawnXOffset : static_cast<float>(m_pRenderer->GetWidth()) + spawnXOffset;

    if (newEnemyBat->Initialise(*m_pRenderer, spawnPos)) 
    {
        m_pScene->AddEnemy(newEnemyBat);
    }
    else 
    {
        LogManager::GetInstance().Log("EnemySpawner: Failed to init Bat");
        delete newEnemyBat;
    }
}

void EnemySpawner::SpawnType2(bool spawnOnLeft)
{
    if (!m_pRenderer || !m_pPlayer || !m_pScene) return;

    EnemyType2* newEnemyType2 = new EnemyType2();
    if (!newEnemyType2) 
    {
        LogManager::GetInstance().Log("EnemySpawner: Failed to allocate Type2");
        return;
    }
    newEnemyType2->m_pTargetPlayer = m_pPlayer;
    newEnemyType2->SetSceneReference(m_pScene);

    Vector2 spawnPos;
    const float spawnXOffset = 100.0f;
    spawnPos.x = spawnOnLeft ? -spawnXOffset : static_cast<float>(m_pRenderer->GetWidth()) + spawnXOffset;

    if (newEnemyType2->Initialise(*m_pRenderer, spawnPos)) 
    {
        m_pScene->AddEnemy(newEnemyType2);
    }
    else 
    {
        LogManager::GetInstance().Log("EnemySpawner: Failed to init Type2");
        delete newEnemyType2;
    }
}