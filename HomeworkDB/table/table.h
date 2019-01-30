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
	Table(const std::string &); //�������Ѵ��ڵı��ʼ��
	Table(const std::vector<std::string> &); //��"CREATE TABLE"�����ʼ��
	~Table();

	Table(const Table &) = delete;
	Table & operator=(const Table &) = delete;

	// Status CREATE();
	Status INSERT();
	Status SELECT();
	Status UPDATE();
	Status DELETE(); // delete��c++����,�ɴ�ȫ����д

	Status addIndex(const std::string &, const std::string &); // (Attr, indexType)
	Status removeIndex();
private:
	std::string name_;
	FILE* fp_;
	Schema schema_;
	std::vector<Index*> indexes_;
};
