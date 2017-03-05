#ifndef SRC_LOGPROCESS_H_
#define SRC_LOGPROCESS_H_

#include <stdarg.h>
#include <rutil/ThreadIf.hxx>
#include <rutil/Logger.hxx>
//#include "AppCommon.h"
#ifndef RESIPROCATE_SUBSYSTEM
#define RESIPROCATE_SUBSYSTEM Subsystem::SIP
#endif

#define DEFAULT_LOG_FILE_SIZE	(10240000 * 3)

using namespace resip;

class LogProcess: public ThreadIf {
public:
	LogProcess();
	virtual ~LogProcess();

	void InitLog(const char* szAppName, int nIndex = 0);
	void Run();
	void CheckDirectory(const char *path, ...);

private:
	bool m_QueueOpened;
	bool m_bIsActive;
	Data m_strAppName;
	int m_iIndex;
	char m_strLogFile[256 + 1];
	char m_strProgramPath[256 + 1];

private:
	void thread();	// ThreadIf::

	void ThreadCleanUp();
	static void ThreadCleanUp(void *lpData);
};

#endif /* SRC_LOGPROCESS_H_ */
