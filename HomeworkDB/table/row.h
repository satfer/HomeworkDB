#pragma once

#include <cstdio>
#include <string>
#include <map>
#include "../include/HomeworkDB/status.h"



class Rows {
	// ָ�����еĿ�ͷ, ������������(schema), ����[]ʵ�ַ���
public:
	Rows() = delete;
	Rows(Table); //ֻ��ĳ�ֳ�ʼ��, ���...
	~Rows();
private:
	std::map<std::string, std::string> schema;
};