#include "oal.h"

const char *oal_signal_description( const int Signal )
{
	static const  char* DescriptionArray[29] = {
		"[SIGABRT] Process abort signal.",
		"[SIGALRM] Alarm clock.",
		"[SIGBUS] Access to an undefined portion of a memory object.",
		"[SIGCHLD] Child process terminated, stopped, [Option Start] or continued. [Option End]",
		"[SIGCONT] Continue executing, if stopped.",
		"[SIGFPE] Erroneous arithmetic operation.",
		"[SIGHUP] Hangup.",
		"[SIGILL] Illegal instruction.",
		"[SIGINT] Terminal interrupt signal.",
		"[SIGKILL] Kill (cannot be caught or ignored).",
		"[SIGPIPE] Write on a pipe with no one to read it.",
		"[SIGQUIT] Terminal quit signal.",
		"[SIGSEGV] Invalid memory reference.",
		"[SIGSTOP] Stop executing (cannot be caught or ignored).",
		"[SIGTERM] Termination signal.",
		"[SIGTTSP] Terminal stop signal.",
		"[SIGTTIN] Background process attempting read.",
		"[SIGTTOU] Background process attempting write.",
		"[SIGUSR1] User-defined signal 1.",
		"[SIGUSR2] User-defined signal 2.",
		"[SIGPOLL] Pollable event.",
		"[SIGPROF] Profiling timer expired.",
		"[SIGSYS] Bad system call.",
		"[SIGTRAP] Trace/breakpoint trap. [Option End]",
		"[SIGURG] High bandwidth data is available at a socket.",
		"[SIGVTALRM] Virtual timer expired.",
		"[SIGXCPU] CPU time limit exceeded.",
		"[SIGXFSZ] File size limit exceeded.",
		"UNKNOWN SIGNAL"
	};

	switch (Signal)
	{
		case SIGABRT:   return DescriptionArray[ 0]; break;
		case SIGALRM:   return DescriptionArray[ 1]; break;
		case SIGBUS:    return DescriptionArray[ 2]; break;
		case SIGCHLD:   return DescriptionArray[ 3]; break;
		case SIGCONT:   return DescriptionArray[ 4]; break;
		case SIGFPE:    return DescriptionArray[ 5]; break;
		case SIGHUP:    return DescriptionArray[ 6]; break;
		case SIGILL:    return DescriptionArray[ 7]; break;
		case SIGINT:    return DescriptionArray[ 8]; break;
		case SIGKILL:   return DescriptionArray[ 9]; break;
		case SIGPIPE:   return DescriptionArray[10]; break;
		case SIGQUIT:   return DescriptionArray[11]; break;
		case SIGSEGV:   return DescriptionArray[12]; break;
		case SIGSTOP:   return DescriptionArray[13]; break;
		case SIGTERM:   return DescriptionArray[14]; break;
		case SIGTSTP:   return DescriptionArray[15]; break;
		case SIGTTIN:   return DescriptionArray[16]; break;
		case SIGTTOU:   return DescriptionArray[17]; break;
		case SIGUSR1:   return DescriptionArray[18]; break;
		case SIGUSR2:   return DescriptionArray[19]; break;
		case SIGPOLL:   return DescriptionArray[20]; break;
		case SIGPROF:   return DescriptionArray[21]; break;
		case SIGSYS:    return DescriptionArray[22]; break;
		case SIGTRAP:   return DescriptionArray[23]; break;
		case SIGURG:    return DescriptionArray[24]; break;
		case SIGVTALRM: return DescriptionArray[25]; break;
		case SIGXCPU:   return DescriptionArray[26]; break;
		case SIGXFSZ:   return DescriptionArray[27]; break;
		default:        return DescriptionArray[28]; break;
	}
}
