#ifndef VERSION_H
#define VERSION_H

#include <QString>
#include "defines.h"

struct Version {
	int major;
	int minor;
	int patch;

	Version()
		: major(VERSION_MAJOR)
		, minor(VERSION_MINOR)
		, patch(VERSION_PATCH)
	{}
	Version(int major, int minor, int patch) 
		: major(major)
		, minor(minor)
		, patch(patch)
	{}

	QString to_string() const {return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);}
	
	bool operator<(const Version &other) const {
		if (major == other.major) {
			if (minor == other.minor) {
				return patch < other.patch;
			} else {
				return minor < other.minor;
			}
		} else {
			return major < other.major;
		}
	}
};

#endif