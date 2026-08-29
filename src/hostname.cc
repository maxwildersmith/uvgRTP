#include "hostname.hh"

#include "debug.hh"

#ifdef _WIN32
//#include <windows.h>
//#include <winbase.h>
#else
#include <unistd.h>
#include <cstring>
#include <limits.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#define NAME_MAXLEN 512

std::string uvgrtp::hostname::get_hostname()
{
#ifdef _WIN32
    char buffer[NAME_MAXLEN];
    DWORD bufCharCount = NAME_MAXLEN;

    if (!GetComputerName((TCHAR *)buffer, &bufCharCount))
        log_platform_error("GetComputerName() failed");

    return std::string(buffer);
#else
    char hostname[NAME_MAXLEN];

    if (gethostname(hostname, NAME_MAXLEN) != 0) {
        UVG_LOG_ERROR("%s", strerror(errno));
        return "";
    }

    return std::string(hostname);
#endif
}

std::string uvgrtp::hostname::get_username()
{
#ifdef _WIN32
    char buffer[NAME_MAXLEN];
    DWORD bufCharCount = NAME_MAXLEN;

    if (!GetUserName((TCHAR *)buffer, &bufCharCount)) {
        log_platform_error("GetUserName() failed");
        return "";
    }

    return std::string(buffer);
#else
    /* getlogin first: it names the actual login session, which is the most
     * accurate answer when there is one.
     *
     * It fails with ENXIO ("No such device or address") whenever the process
     * has no controlling terminal -- a daemon, a container, a systemd unit, a
     * roslaunch-started node. That is an entirely ordinary way to run, so the
     * old code logged
     *
     *     [uvgRTP][ERROR][::get_username] No such device or address
     *
     * on every such startup, for a condition the caller already handles:
     * generate_cname() substitutes a random string for an empty result. An
     * ERROR nobody can act on and nothing is wrong with trains people to
     * ignore the log.
     *
     * So fall through to the password database, which is keyed on the
     * effective uid and needs no terminal, and only then to the environment.
     * A real username also makes the RTCP CNAME meaningful instead of random,
     * which is the point of putting it there. */
#if defined(ANDROID) && __ANDROID_MIN_SDK_VERSION__ < 28
    {
        const char* login = getlogin();
        if (login != nullptr && *login != '\0')
            return std::string(login);
    }
#else
    {
        char username[NAME_MAXLEN];
        if (getlogin_r(username, NAME_MAXLEN) == 0 && username[0] != '\0')
            return std::string(username);
    }
#endif

    /* getpwuid_r, not getpwuid: this runs from context construction and there
     * is nothing stopping two sessions being created concurrently. */
    {
        struct passwd pwd;
        struct passwd* result = nullptr;
        char buffer[NAME_MAXLEN];
        if (getpwuid_r(geteuid(), &pwd, buffer, sizeof(buffer), &result) == 0 &&
            result != nullptr && result->pw_name != nullptr &&
            result->pw_name[0] != '\0') {
            return std::string(result->pw_name);
        }
    }

    for (const char* var : {"USER", "LOGNAME"}) {
        const char* value = getenv(var);
        if (value != nullptr && *value != '\0')
            return std::string(value);
    }

    /* Debug, not error: generate_cname() copes, and by here we have simply
     * failed to discover a nicety. */
    UVG_LOG_DEBUG("could not determine a username; CNAME will use a random string");
    return "";
#endif
}
