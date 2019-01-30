#pragma once

#include <string>
#include <vector>


// sql����ģ��
// ��ʱֻΪcreate������
// ��ʱֻ�� һ��SQL���� �հ׷�, ����: ȫ��ȥ��
// ����CREATE��䲻����NOT NULL ֮���
// �ֶ����������()ȥ��
// �� CREATE TABLE ȥ����www
std::vector<std::string> split(std::string &s) {
	std::vector<std::string> words;

	std::string::size_type pos1, pos2 = 0;
	while (isEat(s[pos2])) ++pos2;
	while (pos2 < s.size()) {
		pos1 = pos2;
		while (!isEat(s[pos2])) ++pos2;
		words.push_back(s.substr(pos1, pos2 - pos1));
		while (isEat(s[pos2])) ++pos2;
	}

	// !!! �� CREATE TABLE ȥ����www
	if (words.size() >= 2) {
		words.erase(words.begin(), words.begin() + 2);
		printf("�� CREATE TABLE ȥ����www\n");
	}

	return words;
}

inline bool isEat(char c) {
	return c == '(' || c == ')' || c == '\n' || c == '\t' || c == ' ' || c == ';';
}