#include "utils.h"

namespace utils
{
	
std::vector<std::string> split(std::string s, char d)
{
	std::vector<std::string> r;
	
	std::size_t p0 = 0;
	std::size_t p1 = s.find_first_of(d);
	if (p1 == std::string::npos)
	{
		r.emplace_back(s);
		return r;
	}
		
	while (p1 != std::string::npos)
	{
		r.emplace_back(s.substr(p0, p1 - p0));
		p0 = p1 + 1;
		p1 = s.find_first_of(d, p0);
	}
	r.emplace_back(s.substr(p0));
	
	return r;
}
	
} // namespace utils
