#ifndef DEBUG_HH_
#define DEBUG_HH_

#ifdef DEBUG_APPLICATION

#define LOG_SERVICE_NAME OSException log = OSException(__PRETTY_FUNCTION__);
#define LOG_SERVICE_PARAM1(p1) CharSeq LogStringParam1; LogStringParam1 << #p1 << ": " << CharSeq(p1); log.addPath(LogStringParam1.c_str());
#define LOG_SERVICE_PARAM2(p2) CharSeq LogStringParam2; LogStringParam2 << #p2 << ": " << CharSeq(p2); log.addPath(LogStringParam2.c_str());
#define LOG_SERVICE_PARAM3(p3) CharSeq LogStringParam3; LogStringParam3 << #p3 << ": " << CharSeq(p3); log.addPath(LogStringParam3.c_str());
#define LOG_SERVICE_PARAM4(p4) CharSeq LogStringParam4; LogStringParam4 << #p4 << ": " << CharSeq(p4); log.addPath(LogStringParam4.c_str());
#define LOG_SERVICE_PARAM5(p5) CharSeq LogStringParam5; LogStringParam5 << #p5 << ": " << CharSeq(p5); log.addPath(LogStringParam5.c_str());
#define LOG_SERVICE_RESULT(r) CharSeq LogStringResult; LogStringResult << "RESULT: " << CharSeq(r);
#define LOG_SERVICE log.printLoggingMessage(ASAAC_LOG_MESSAGE_TYPE_APPLICATION);

#define LOG_SERVICE0(r) LOG_SERVICE_NAME;LOG_SERVICE_RESULT(r); LOG_SERVICE;
#define LOG_SERVICE1(p1, r) LOG_SERVICE_NAME;LOG_SERVICE_PARAM1(p1);LOG_SERVICE_RESULT(r); LOG_SERVICE;
#define LOG_SERVICE2(p1, p2, r) LOG_SERVICE_NAME;LOG_SERVICE_PARAM1(p1);LOG_SERVICE_PARAM2(p2);LOG_SERVICE_RESULT(r); LOG_SERVICE;
#define LOG_SERVICE3(p1, p2, p3, r) LOG_SERVICE_NAME;LOG_SERVICE_PARAM1(p1);LOG_SERVICE_PARAM2(p2);LOG_SERVICE_PARAM3(p3);LOG_SERVICE_RESULT(r); LOG_SERVICE;
#define LOG_SERVICE4(p1, p2, p3, p4, r) LOG_SERVICE_NAME;LOG_SERVICE_PARAM1(p1);LOG_SERVICE_PARAM2(p2);LOG_SERVICE_PARAM3(p3);LOG_SERVICE_PARAM4(p4);LOG_SERVICE_RESULT(r); LOG_SERVICE;
#define LOG_SERVICE5(p1, p2, p3, p4, p5, r) LOG_SERVICE_NAME;LOG_SERVICE_PARAM1(p1);LOG_SERVICE_PARAM2(p2);LOG_SERVICE_PARAM3(p3);LOG_SERVICE_PARAM4(p4);LOG_SERVICE_PARAM5(p5);LOG_SERVICE_RESULT(r); LOG_SERVICE;


#else

#define LOG_SERVICE0(r)
#define LOG_SERVICE1(p1, r)
#define LOG_SERVICE2(p1, p2, r)
#define LOG_SERVICE3(p1, p2, p3, r)
#define LOG_SERVICE4(p1, p2, p3, p4, r)
#define LOG_SERVICE5(p1, p2, p3, p4, p5, r)

#endif

void logService();

#endif /*DEBUG_HH_*/
