#pragma once

#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include "../include/HomeworkDB/status.h"
#include "schema.h"
#include "index.h"



class Table {
public:
	Table() = delete;
	Table(const std::string &); //可以由已存在的表初始化
	Table(const std::vector<std::string> &); //由"CREATE TABLE"命令初始化
	~Table();

	Table(const Table &) = delete;
	Table & operator=(const Table &) = delete;

	// Status CREATE();
	Status INSERT();
	Status SELECT();
	Status UPDATE();
	Status DELETE(); // delete被c++用了,干脆全部大写

	Status addIndex(const std::string &, const std::string &); // (Attr, indexType)
	Status removeIndex();
private:
	std::string name_;
	FILE* fp_;
	Schema schema_;
	std::vector<Index*> indexes_;
};
