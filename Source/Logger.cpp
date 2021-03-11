#include"Core.h"
#include"termcolor/termcolor.hpp"
namespace tEngine {
	 std::stringstream Logger::stream;
	 void Logger::LogStyle(const std::string& stream,LogLevel level) {
		 std::stringstream ss;
		 switch (level) {
		 case LogLevel::Error:
			 std::cout << termcolor::bright_red << stream;
			 break;
		 case LogLevel::Debug:
			 std::cout << termcolor::bright_yellow << stream; break;
		 case LogLevel::Performance:
			 std::cout << termcolor::blue << stream; break;
		 default:
			 std::cout << stream; break;
			}
		 std::cout <<"\n"<< termcolor::reset;
		 
	 }
}