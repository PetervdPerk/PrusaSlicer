#include "Platform.hpp"

#include <boost/log/trivial.hpp>
#include <boost/filesystem/operations.hpp>

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>
#endif

namespace Slic3r {

static auto s_platform 		  = Platform::Uninitialized;
static auto s_platform_flavor = PlatformFlavor::Uninitialized;

void detect_platform()
{
#if defined(_WIN32)
    BOOST_LOG_TRIVIAL(info) << "Platform: Windows";
	s_platform 		  = Platform::Windows;
	s_platform_flavor = PlatformFlavor::Generic;
#elif defined(__APPLE__)
    BOOST_LOG_TRIVIAL(info) << "Platform: OSX";
	s_platform 		  = Platform::OSX;
	s_platform_flavor = PlatformFlavor::GenericOSX;
    {
        cpu_type_t type;
        size_t     size = sizeof(type);
        if (sysctlbyname("hw.cputype", &type, &size, NULL, 0) == 0) {
            if (type == CPU_TYPE_X86_64) {
                int proc_translated = 0;
                size                = sizeof(proc_translated);
                if (sysctlbyname("sysctl.proc_translated", &proc_translated, &size, NULL, 0) == 0) {
                    if (proc_translated == 1) {
                        s_platform_flavor = PlatformFlavor::OSXOnArm64;
                        BOOST_LOG_TRIVIAL(info) << "Platform flavor: arm64";
                    } else {
                        s_platform_flavor = PlatformFlavor::OSXOnX86_64;
                        BOOST_LOG_TRIVIAL(info) << "Platform flavor: x86_64";
                    }
                }
            } else if (type == CPU_TYPE_ARM64) {
                s_platform_flavor = PlatformFlavor::OSXOnArm64;
                BOOST_LOG_TRIVIAL(info) << "Platform flavor: arm64";
            }
        }
    }
#elif defined(__linux__)
    BOOST_LOG_TRIVIAL(info) << "Platform: Linux";
	s_platform 		  = Platform::Linux;
	s_platform_flavor = PlatformFlavor::GenericLinux;
	// Test for Chromium.
	{
		FILE *f = ::fopen("/proc/version", "rt");
		if (f) {
			char buf[4096];
			// Read the 1st line.
			if (::fgets(buf, 4096, f)) {
				if (strstr(buf, "Chromium OS") != nullptr) {
					s_platform_flavor = PlatformFlavor::LinuxOnChromium;
				    BOOST_LOG_TRIVIAL(info) << "Platform flavor: LinuxOnChromium";
				} else if (strstr(buf, "microsoft") != nullptr || strstr(buf, "Microsoft") != nullptr) {
					if (boost::filesystem::exists("/run/WSL") && getenv("WSL_INTEROP") != nullptr) {
						BOOST_LOG_TRIVIAL(info) << "Platform flavor: WSL2";
						s_platform_flavor = PlatformFlavor::WSL2;
					} else {
						BOOST_LOG_TRIVIAL(info) << "Platform flavor: WSL";
						s_platform_flavor = PlatformFlavor::WSL;
					}
				}
			}
			::fclose(f);
		}
	}
#elif defined(__OpenBSD__)
    BOOST_LOG_TRIVIAL(info) << "Platform: OpenBSD";
	s_platform 		  = Platform::BSDUnix;
	s_platform_flavor = PlatformFlavor::OpenBSD;
#else
	// This should not happen.
    BOOST_LOG_TRIVIAL(info) << "Platform: Unknown";
	static_assert(false, "Unknown platform detected");
	s_platform 		  = Platform::Unknown;
	s_platform_flavor = PlatformFlavor::Unknown;
#endif
}

Platform platform()
{
	return s_platform;
}

PlatformFlavor platform_flavor()
{
	return s_platform_flavor;
}

} // namespace Slic3r
