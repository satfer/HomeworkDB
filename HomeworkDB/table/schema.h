#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>



class Schema {
	// 属性->类型和空间大小
public:
	Schema()=delete; //如果把它delete掉, Table一开始就要初始化schema_
	// Schema(const std::vector<std::string> &);
	Schema(const std::string &file); // 文件名
	// 写join的时候应该会多某个构造函数, 写的时候再说吧
	~Schema()=default; // new 了一堆 TypeHandler, 记得处理 复制构造, =, 析构

	Schema(const Schema &) = delete;
	Schema & operator=(const Schema &) = delete;
	// Schema & operator=(Schema &&a) { type_ = a.type_; handler_ = a.handler_; }

	// const std::string & operator[] (const std::string &attr) { return schema_[attr]; }
	//const std::string & getType(const std::string &attr) { return attrType_[attr]; }
	//const TypeHandler * getHandler(const std::string &attr) { return handler_[attr]; }
//private: // 设计模式qtm
	std::vector<std::string> attrs_;
	std::vector<std::string> types_;
	std::map<std::string, std::string> attrType_; // attr->type
};

//// 类型处理 机. 抽象基类基本只是为了方便用指针, 比如vector<TypeHandler *>
//class TypeHandler {
//public:
//	TypeHandler() = default;
//	~TypeHandler() = default;
//	virtual void read(FILE*);
//	virtual void write(FILE*);
//	// 即使是虚函数, 重写也要求返回类型相同或协同(为子类), 只能把读写的东西放在参数里了
//protected:
//	size_t typeSize_ = 0;
//	void* temp_ = nullptr;
//};
//
//// 子类考虑memcpy之类的东西
//class INTEGER: public TypeHandler {
//public:
//	INTEGER() = default;
//	~INTEGER();
//	void toWrite(void*);

