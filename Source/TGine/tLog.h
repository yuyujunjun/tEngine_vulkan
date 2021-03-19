#pragma once
#include<iostream>
namespace tEngine {
	enum class LogLevel
	{
		Verbose = 0,
		Debug = 1,
		Information = 2,
		Warning = 3,
		Error = 4,
		Critical = 5,
		Performance = 6,
		None = 100,
	};

	void LogStyle(const std::string& s, LogLevel level);
	template<typename Attribute>
	void LOGI(Attribute args) {
		std::stringstream stream;
		stream << args;
		LogStyle(stream.str(), LogLevel::Information);
	}
	template<typename Attribute, typename ...Args>
	void LOGI(Attribute value, Args... args) {
		LOGI(value);
		LOGI(args...);
	}

	template<typename Attribute>
	void LOG(LogLevel level, Attribute info) {
		std::stringstream stream;
		stream << info;
		LogStyle(stream.str(), level);

	}
	template<typename Attribute, typename ...Args>
	void LOG(LogLevel level, Attribute info, Args... args) {
		LOG(level, info);
		LOG(args...)
	}
	template<typename ...Args>
	void LOGD(LogLevel level, Args... args) {
#ifndef NDEBUG
		LOG(level, args...);
#endif
	}
}