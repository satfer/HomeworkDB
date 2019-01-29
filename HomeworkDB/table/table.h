#pragma once

#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include "../include/HomeworkDB/status.h"
#include "../index/index.h"



class Table {
public:
	Table();
	Table(const std::string &);
	~Table();

	Status CREATE();
	Status INSERT();
	Status SELECT();
	Status UPDATE();
	Status DELETE(); // delete��c++����,�ɴ�ȫ����д

	Status addIndex(const std::string &, const std::string &); // (Attr, indexType)
	Status removeIndex();
private:
	FILE* fp;
	std::map<std::string, std::string> schema;
	std::vector<Index*> indexes;
};
