#include "Path.hpp"

#include <iostream>

std::string Path::extractFileName(std::string const& path)
{
	auto lastSlashPos = path.find_last_of("\\/");
	auto lastDotPos = path.find_last_of(".");

	if (lastSlashPos == std::string::npos)
	{
		lastSlashPos = 0u;
	}
	else
	{
		lastSlashPos += 1;
	}
	if (lastDotPos == std::string::npos)
	{
		lastDotPos = path.size();
	}

	auto const extractSize = lastDotPos - lastSlashPos;

	return path.substr(lastSlashPos, extractSize);
}

std::string Path::extractFileNameWithExtension(std::string const& path)
{
	auto lastSlashPos = path.find_last_of("\\/");

	if (lastSlashPos == std::string::npos)
	{
		lastSlashPos = 0u;
	}
	else
	{
		lastSlashPos += 1;
	}
	return path.substr(lastSlashPos);
}

std::string Path::extractExtension(std::string const& path)
{
	auto lastDotPos = path.find_last_of(".");

	if (lastDotPos != std::string::npos)
	{
		lastDotPos += 1;
	}
	else
	{
		lastDotPos = path.size();
	}
	return path.substr(lastDotPos);
}
