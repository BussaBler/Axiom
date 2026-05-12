#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace Axiom {
    struct ProfileData {
        std::string name;
        std::chrono::duration<double, std::micro> duration;
    };

    class Profiler {
      public:
        inline static void beginFrame() {
            lastFrameProfiles = std::move(activeProfiles);
            activeProfiles.clear();
            activeProfiles.reserve(lastFrameProfiles.size());
        }

        inline static void addProfile(const std::string& name, const std::chrono::duration<double, std::micro>& duration) {
            activeProfiles.push_back({name, duration});
        }

        inline static const std::vector<ProfileData>& getProfiles() { return lastFrameProfiles; }

      private:
        inline static std::vector<ProfileData> activeProfiles = {};
        inline static std::vector<ProfileData> lastFrameProfiles = {};
    };

    class ProfileScope {
      public:
        ProfileScope(const std::string& name) : name(name), startTime(std::chrono::high_resolution_clock::now()) {}
        ~ProfileScope() {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = endTime - startTime;
            Profiler::addProfile(name, duration);
        }

      private:
        std::string name;
        std::chrono::high_resolution_clock::time_point startTime;
    };

#if defined(AX_DEBUG)
#define AX_PROFILE_SCOPE(name) Axiom::ProfileScope profileScope##__LINE__(name)
#if defined(AX_PLATFORM_WINDOWS)
#define AX_PROFILE_FUNCTION() AX_PROFILE_SCOPE(__FUNCSIG__)
#else
#define AX_PROFILE_FUNCTION() AX_PROFILE_SCOPE(__PRETTY_FUNCTION__)
#endif
#else
#define AX_PROFILE_SCOPE(name)
#define AX_PROFILE_FUNCTION()
#endif
} // namespace Axiom