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
	Table(const std::string &, const std::string &); //������.db���ļ���ʼ��
	// Table(const std::vector<std::string> &); //��"CREATE TABLE"�����ʼ��
	~Table() = default;

	Table(const Table &) = delete;
	Table & operator=(const Table &) = delete;

	// start һ��Ҫ��һ�еĿ�ʼ. ����end(����)
	void print(const fpos_t &start, const fpos_t &end = LLONG_MAX);

	// Status CREATE();
	// Status INSERT();
	// Status SELECT();
	// Status UPDATE();
	// Status DELETE(); // delete��c++����,�ɴ�ȫ����д

	//Status setMainIndex();
	//Status addIndex(const std::string &, const std::string &); // (Attr, indexType)
	//Status removeIndex();
//private: //���ģʽqtm
	std::string name_;
	std::string location_;
	FILE* fp_;
	Schema schema_;
	MainIndex *mainIndex_;
	std::vector<Index*> indexes_;
};
