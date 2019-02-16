#include "table.h"

Table::Table(const std::string &name, const std::string &location = "/") : 
	name_(name), location_(location), schema_(location_ + name_ + std::string(".schema"))
{
	fopen_s(&fp_, (location+name+std::string(".db")).c_str(), "r+");
}

//Table::Table(const std::vector<std::string> &words) {
//	// 假设输入是 SQL语句调用了SQL.h的split后的vector<string>
//	// 例如CREATE TABLE REGION R_REGIONKEY INTEGER R_NAME CHAR(25) R_COMMENT VARCHAR(152)
//	name_ = words[0];
//	fopen_s(&fp_, name_.c_str(), "w"); // 文件路径用配置接管比较好控制?
//	schema_ = Schema(words);
//
//	for (auto word : words) {
//		fprintf(fp_, word.c_str());
//		fprintf(fp_, " ");
//	}
//	fprintf(fp_, ";");
//
//	fseek(fp_, 4095, SEEK_SET);
//	fprintf(fp_, " ");
//}

void Table::print(const fpos_t &start, const fpos_t &end) {
	if (fsetpos(fp_, &start))
		throw("Table::read setpos failed");
	else {
		// 用的string, 效率没那么高;
		// 追求效率的话, 应该是new char[lenth], 用flag控制第一次进是new, 之后直接用
		fpos_t fpos = start;
		int tempInt;
		char tempChar;
		double tempDouble;
		std::string tempString;
		char tempDate[11];
		int lenth, i;
		while (1) {
			for (auto type : schema_.types_) {
				if (type == "INTEGER") {
					fread(&tempInt, sizeof(int), 1, fp_);
					fpos += sizeof(int);
				}
				else if (type == "DOUBLE") {
					fread(&tempDouble, sizeof(double), 1, fp_);
					fpos += sizeof(double);
				}
				else if (type.substr(0,4) == "CHAR") {
					lenth = std::stoi(type.substr(5, type.find(')') - 5));
					if (lenth == 1) {
						fread(&tempChar, sizeof(char), 1, fp_);
						fpos += sizeof(char)*lenth;
					}
					else {
						tempString.clear();
						for (i = 0; i < lenth; ++i) {
							fread(&tempChar, sizeof(char), 1, fp_);
							if (tempChar == '\0')
								break;
							else
								tempString.append(1, tempChar);
						}
						fpos += sizeof(char) * (lenth + 1);
						fsetpos(fp_, &fpos);
					}
				}
				else if (type.substr(0, 7) == "VARCHAR") {
					lenth = std::stoi(type.substr(8, type.find(')') - 8));
					tempString.clear();
					for (i = 0; i < lenth; ++i) {
						fread(&tempChar, sizeof(char), 1, fp_);
						if (tempChar == '\0')
							break;
						else
							tempString.append(1, tempChar);
					}
					fpos += sizeof(char) * (i + 1);
					fsetpos(fp_, &fpos);
				}
				else if (type == "DATE") {
					fread(tempDate, sizeof(char), 11, fp_);
					fpos += sizeof(char)*11;
				}

				// 到end或文件结束, 退出.
				if (fpos >= end || feof(fp_))
					return;

				if (type == "INTEGER")
					printf("%d ", tempInt);
				else if (type == "DOUBLE")
					printf("%f ", tempDouble);
				else if (type.substr(0, 4) == "CHAR") {
					// lenth = std::stoi(type.substr(5, type.find(')') - 5)); // lenth已经算过
					if (lenth == 1)
						printf("%c ", tempChar);
					else
						printf("%s ", tempString.c_str());
				}
				else if (type.substr(0, 7) == "VARCHAR")
					printf("%s ", tempString.c_str());
				else if (type == "DATE")
					printf("%s ", tempDate);
			} // for (type)
			printf("\n");
		} // while (1)
	}
}
