#include "schema.h"

//Schema::Schema(const std::vector<std::string> & words)
//{
//	int i = 0;
//	TypeHandler *THp;
//	for (i = 1; i < words.size(); i += 2) {
//		type_[words[i]] = words[i + 1];
//		if (words[i + 1] == "INTEGER")
//			THp = new INTEGER();
//		else if (words[i + 1].find("VARCHAR")) {
//			THp = new VARCHAR(stoi(words[i + 1].substr(8, words[i + 1].size() - 9)));
//		}
//		else if (words[i + 1].find("CHAR")) {
//			THp = new VARCHAR(stoi(words[i + 1].substr(5, words[i + 1].size() - 6)));
//		}
//		else if (words[i + 1].find("REAL")) {
//			THp = new REAL();
//		}
//		else if (words[i + 1].find("TIMESTAMP")) {
//			THp = new TIMESTAMP();
//		}
//		handler_[words[i]] = THp;
//	}
//}

//void TypeHandler::read(FILE* fp) {
//	if (typeSize_ == 0 || temp_ == nullptr)
//		throw("mark1");
//	fread(temp_, typeSize_, 1, fp);
//}
//
//void TypeHandler::write(FILE *fp)
//{
//	if (typeSize_ == 0 || temp_ == nullptr)
//		throw("mark2");
//	fwrite(temp_, typeSize_, 1, fp);
//}

Schema::Schema(const std::string &file) {
	std::string attr, type;
	std::ifstream of(file.c_str());
	of >> attr;
	while (!of.eof()) {
		of >> type;

		attrs_.push_back(attr);
		types_.push_back(type);
		attrType_[attr] = type;

		of >> attr;
	}
}
