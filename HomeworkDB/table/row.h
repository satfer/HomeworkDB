#pragma once

#include <cstdio>
#include <string>
#include <map>
#include "../include/HomeworkDB/status.h"



class Rows {
	// 指向数行的开头, 理解行里的内容(schema), 重载[]实现访问
public:
	Rows() = delete;
	Rows(Table); //只有某种初始化, 大概...
	~Rows();
private:
	std::map<std::string, std::string> schema;
};