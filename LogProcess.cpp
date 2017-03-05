#include <sys/stat.h>
#include "LogProcess.h"

// 테스트를 위한 코드변경

LogProcess::LogProcess() {
	m_bIsActive = false;
	m_QueueOpened = true;
	m_iIndex = 0;
}

LogProcess::~LogProcess() {
	shutdown();

	for (int n = 0; n < 20; n++)
	{	// 최대 2초 대기
		if (m_QueueOpened == false)
			break;	// 정상 종료
		usleep(100000);	// 1000000 = 1sec, 100000 = 0.1sec
	}

	if (mId)
	{
		pthread_cancel(mId);	// 강제 종료
	}

	join();
}

void LogProcess::InitLog(const char* szAppName, int nIndex) {
	m_strAppName = szAppName;
	m_strAppName = m_strAppName.uppercase();
	m_iIndex = nIndex;

	strcpy(m_strProgramPath, getenv("SKYCOM_HOME"));
	if (strlen(m_strProgramPath) <= 0)
	{
		strcpy(m_strProgramPath, "/usr/local/skycom/");
	}

	struct timeval theTime;
	gettimeofday(&theTime, NULL);

	struct tm curDateTime;
	localtime_r(&theTime.tv_sec, &curDateTime);
	CheckDirectory("%s/var/log/%04d%02d%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1, curDateTime.tm_mday);

	sprintf(m_strLogFile, "%s/var/log/%04d%02d%02d/%s%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1, curDateTime.tm_mday,
			m_strAppName.c_str(), m_iIndex);

	unsigned int maxFileSize = DEFAULT_LOG_FILE_SIZE;

	char* szLogFileSize = getenv("SKYCOM_LOG_FILE_SIZE");
	if (szLogFileSize != NULL)
		maxFileSize = atoi(szLogFileSize);

	Log::setMaxByteCount(maxFileSize);
	Log::initialize(Log::File, Log::Info, szAppName, m_strLogFile);
}

void LogProcess::Run() {
	m_QueueOpened = true;
	run();
}

void LogProcess::CheckDirectory(const char *path, ...) {
	va_list args;
	va_start(args, path);

	// Guess we need no more than 1024 bytes.
	int n, size = 256;
	char *p;

	p = (char*) malloc(size);

	while (p != NULL)
	{
		// Try to print in the allocated space.
		n = vsnprintf(p, size, path, args);

		// If that worked, return the string.
		if (n > -1 && n < size)
		{
			break;
		}
		// Else try again with more space.
		if (n > -1)
			size = n + 1; // precisely what is needed
		else
			size *= 2;  // twice the old size

		if ((p = (char*) realloc(p, size)) == NULL)
		{
			break;
		}
	}

	if (p != NULL)
	{
		if (access(p, F_OK) < 0)
		{
			if (mkdir(p, 0655) < 0)
			{
				ErrLog(<< "create Failed[%s]" << p);
			}
		}

		free(p);
	}

	va_end(args);
}

void LogProcess::ThreadCleanUp(void *lpData) {
	LogProcess* pLogProcess = (LogProcess*) lpData;

	pLogProcess->ThreadCleanUp();
}

void LogProcess::ThreadCleanUp() {
	m_QueueOpened = false;
}

void LogProcess::thread() {
//	detach();	// detach를 하면 mId가 0이 됨 (pthread_cancel이 않됨)

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	pthread_cleanup_push(ThreadCleanUp, (void*)this);

		struct timeval theTime;
		gettimeofday(&theTime, NULL);

		//char PROGRAM_PATH      getenv("SKYCOM_HOME")
		struct tm curDateTime;
		localtime_r(&theTime.tv_sec, &curDateTime);
		int today = curDateTime.tm_mday;
		CheckDirectory("%s/var/log/%04d%02d%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1, curDateTime.tm_mday);

		sprintf(m_strLogFile, "%s/var/log/%04d%02d%02d/%s%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1, curDateTime.tm_mday,
				m_strAppName.c_str(), m_iIndex);

		while (isShutdown() == false)
		{
			gettimeofday(&theTime, NULL);
			localtime_r(&theTime.tv_sec, &curDateTime);
			if (curDateTime.tm_mday != today)
			{
				today = curDateTime.tm_mday;
				CheckDirectory("%s/var/log/%04d%02d%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1, curDateTime.tm_mday);

				sprintf(m_strLogFile, "%s/var/log/%04d%02d%02d/%s%02d", m_strProgramPath, 1900 + curDateTime.tm_year, curDateTime.tm_mon + 1,
						curDateTime.tm_mday, m_strAppName.c_str(), m_iIndex);
				Log::initialize(Log::File, Log::Info, m_strAppName, m_strLogFile);
			}
			usleep(10000);
		}

		pthread_exit(0);

		pthread_cleanup_pop(0);	// pthread_exit()해야 호출됨
}

