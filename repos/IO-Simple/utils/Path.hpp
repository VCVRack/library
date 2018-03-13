#if!defined PATH_HPP
#define PATH_HPP
#include <string>

class Path
{
public:
	static std::string extractFileName(std::string const& path);
	static std::string extractFileNameWithExtension(std::string const& path);
	static std::string extractExtension(std::string const& path);
};

#endif
