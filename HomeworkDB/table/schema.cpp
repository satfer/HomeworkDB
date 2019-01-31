#include "schema.h"

Schema::Schema(const std::vector<std::string> & words)
{
	int i = 0;
	TypeHandler *THp;
	for (i = 1; i < words.size(); i += 2) {
		type_[words[i]] = words[i + 1];
		if (words[i + 1] == "INTEGER")
			THp = new INTEGER();
		else if (words[i + 1] == "CHAR(") //???
		handler_[words[i]] = THp;
	}
}
