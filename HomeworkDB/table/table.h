#pragma once

#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <climits>
#include "../include/HomeworkDB/status.h"
#include "schema.h"
#include "index.h"



class Table {
public:
	Table() = delete;
	Table(const std::string &, const std::string &); //可以由.db等文件初始化
	// Table(const std::vector<std::string> &); //由"CREATE TABLE"命令初始化
	~Table() = default;

	Table(const Table &) = delete;
	Table & operator=(const Table &) = delete;

	// start 一定要是一行的开始. 读到end(不含)
	void print(const fpos_t &start, const fpos_t &end = LLONG_MAX);

	// Status CREATE();
	// Status INSERT();
	// Status SELECT();
	// Status UPDATE();
	// Status DELETE(); // delete被c++用了,干脆全部大写

	//Status setMainIndex();
	//Status addIndex(const std::string &, const std::string &); // (Attr, indexType)
	//Status removeIndex();
//private: //设计模式qtm
	std::string name_;
	std::string location_;
	FILE* fp_;
	Schema schema_;
	MainIndex *mainIndex_;
	std::vector<Index*> indexes_;
};
