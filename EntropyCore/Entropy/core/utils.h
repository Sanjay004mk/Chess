#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <utility>

namespace et
{
	template <typename T>
	using Ref = std::shared_ptr<T>;
	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using WeakRef = std::weak_ptr<T>;

	template <typename T>
	using Scope = std::unique_ptr<T>;
	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}

namespace et
{
	inline std::string get_file_name(std::string_view file_name)
	{
		std::string name(file_name);
		size_t last_dot = name.find_last_of('.');
		name = name.substr(0, last_dot);
		size_t last_slash = name.find_last_of("/\\");
		last_slash = last_slash == std::string::npos ? 0 : last_slash + 1;
		name = name.substr(last_slash);
		return name;
	}

	inline 	void trim(std::string& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) {return !std::isspace(c); }));
		str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), str.end());
	}
}
