// COMP710 GP Framework 2024
#ifndef __LOGMANAGER_H_
#define __LOGMANAGER_H_

class LogManager
{
	// Member methods:
public:
	static LogManager& GetInstance();
	static void DestroyInstance();
	void Log(const char* pcMessage);

protected:

private:
	LogManager();
	~LogManager();
	LogManager(const LogManager& logMnager);
	LogManager& operator=(const LogManager& logManager);

	// Member Data:
public:

protected:
	static LogManager* sm_pInstance;

private:
};

#endif // __LOGMANAGER_H_