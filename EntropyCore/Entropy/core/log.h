#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#pragma warning(push, 0)
#include <spdlog\spdlog.h>
#include <spdlog\fmt\ostr.h>
#pragma warning(pop)

#include "utils.h"

namespace et
{
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& Logger() { return sLogger; }

	private:
		static std::shared_ptr<spdlog::logger> sLogger;
	};
}

template <typename ostream, glm::length_t L, typename T, glm::qualifier Q>
inline ostream& operator<<(ostream& stream, const glm::vec<L, T, Q>& vector)
{
	return stream << glm::to_string(vector);
}

template <typename ostream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline ostream& operator<<(ostream& stream, const glm::mat<C, R, T, Q>& matrix)
{
	return stream << glm::to_string(matrix);
}

template <typename ostream, typename T, glm::qualifier Q>
inline ostream& operator<<(ostream& stream, const glm::qua<T, Q>& quaternion)
{
	return stream << glm::to_string(quaternion);
}

#define ET_LOG_TRACE(...) ::et::Log::Logger()->trace(__VA_ARGS__)
#define ET_LOG_INFO(...) ::et::Log::Logger()->info(__VA_ARGS__)
#define ET_LOG_WARN(...) ::et::Log::Logger()->warn(__VA_ARGS__)
#define ET_LOG_ERROR(...) ::et::Log::Logger()->error(__VA_ARGS__)
#define ET_LOG_CRITICAL(...) ::et::Log::Logger()->critical(__VA_ARGS__)

#define ET_ASSERT_MSG_BREAK(check, ...) if (check) {} else { ET_LOG_ERROR(__VA_ARGS__); __debugbreak();}
#define ET_ASSERT_NO_MSG_BREAK(check) if (check) {} else { ET_LOG_ERROR("Assertion '{0}' failed at line: {1}, function: {2}, file: {3}", #check, __LINE__, __FUNCTION__, __FILE__); __debugbreak();}
#define ET_ASSERT_MSG(check, ...) if (check) {} else ET_LOG_ERROR(__VA_ARGS__)
#define ET_ASSERT_NO_MSG(check) if (check) {} else ET_LOG_ERROR("Assertion '{0}' failed at line: {1}, function: {2}, file: {3}", #check, __LINE__, __FUNCTION__, __FILE__)

template <bool> class TStaticAssert;
template <> class TStaticAssert<true> {};

#define ET_STATIC_ASSERT(expr) enum { assert_fail_##__LINE__ = sizeof(TStaticAssert<!!(expr)>) };

#ifdef ET_DEBUG
#define ET_DEBUG_ASSERT(check) ET_ASSERT_NO_MSG(check)
#define ET_DEBUG_ASSERT_BREAK(check) ET_ASSERT_NO_MSG_BREAK(check)
#else
#define ET_DEBUG_ASSERT(check)
#define ET_DEBUG_ASSERT_BREAK(check) 
#endif