#ifndef DEBUG_H
#define DEBUG_H

#include "loguru.hpp"
#include <string>

#define gError(...)	 LOG_F(1, __VA_ARGS__) // NOLINT
#define gWarn(...)	 LOG_F(1, __VA_ARGS__) // NOLINT
#define gInfo(...)	 LOG_F(2, __VA_ARGS__) // NOLINT
#define gDebug(...)	 LOG_F(3, __VA_ARGS__) // NOLINT
#define gLog(...)	 LOG_F(4, __VA_ARGS__) // NOLINT
#define gLogV(...)	 LOG_F(5, __VA_ARGS__) // NOLINT
#define gLogVV(...)	 LOG_F(6, __VA_ARGS__) // NOLINT
#define gLogVVV(...) LOG_F(7, __VA_ARGS__) // NOLINT

#define gErrorS()  LOG_S(1)
#define gWarnS()   LOG_S(1)
#define gInfoS()   LOG_S(2)
#define gDebugS()  LOG_S(3)
#define gLogS()	   LOG_S(4)
#define gLogVS()   LOG_S(5)
#define gLogVVS()  LOG_S(6)
#define gLogVVVS() LOG_S(7)

extern void debugEnableUDPLogging(std::string target = "127.0.0.1", int16_t port = 14788);

#endif // DEBUG_H
