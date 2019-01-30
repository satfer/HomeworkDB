#include "table.h"

Table::Table(const std::string &s) {
	name_ = s;
	fopen_s(&fp_, name_.c_str(), "r+");

	std::vector<std::string> words;
	char temp[50];
	fscanf(fp_, "%s", temp);
	while (temp[0] != ';') {
		words.push_back(temp);
		fscanf(fp_, "%s", temp);
	}
	schema_ = Schema(words);

	fseek(fp_, 4096, SEEK_SET);
}

Table::Table(const std::vector<std::string> &words) {
	// 假设输入是 SQL语句调用了SQL.h的split后的vector<string>
	// 例如CREATE TABLE REGION R_REGIONKEY INTEGER R_NAME CHAR(25) R_COMMENT VARCHAR(152)
	name_ = words[0];
	fopen_s(&fp_, name_.c_str(), "w"); // 文件路径用配置接管比较好控制?
	schema_ = Schema(words);

	for (auto word : words) {
		fprintf(fp_, word.c_str());
		fprintf(fp_, " ");
	}
	fprintf(fp_, ";");

	fseek(fp_, 4095, SEEK_SET);
	fprintf(fp_, " ");
}


