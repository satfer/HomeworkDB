#pragma once

#include <string>
#include <vector>


// sql解析模块
// 暂时只为create语句服务
// 暂时只把 一行SQL语句的 空白符, 最后的: 全部去掉
// 而且CREATE语句不能由NOT NULL 之类的
// 手动把最外面的()去掉
// 把 CREATE TABLE 去掉了www
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

	// !!! 把 CREATE TABLE 去掉了www
	if (words.size() >= 2) {
		words.erase(words.begin(), words.begin() + 2);
		printf("把 CREATE TABLE 去掉了www\n");
	}

	return words;
}

inline bool isEat(char c) {
	return c == '(' || c == ')' || c == '\n' || c == '\t' || c == ' ' || c == ';';
}