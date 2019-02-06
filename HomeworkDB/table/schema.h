#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>



class Schema {
	// 属性->类型和空间大小
public:
	Schema(); //虽然想把它delete掉, 但是Table::Table(const std::string &s)没法一开始就初始化schema_导致要调用Schema::Schema()
	Schema(const std::vector<std::string> &);
	// 写join的时候应该会多某个构造函数, 写的时候再说吧
	~Schema(); // new 了一堆 TypeHandler, 记得处理 复制构造, =, 析构

	Schema(const Schema &) = delete;
	Schema & operator=(const Schema &) = delete;
	Schema & operator=(Schema &&a) { type_ = a.type_; handler_ = a.handler_; }

	// const std::string & operator[] (const std::string &attr) { return schema_[attr]; }
	const std::string & getType(const std::string &attr) { return type_[attr]; }
	const TypeHandler * getHandler(const std::string &attr) { return handler_[attr]; }
private:
	std::map<std::string, std::string> type_; // attr->type
	std::map<std::string, TypeHandler *> handler_; // attr->TypeHandler
};

// 类型处理 机. 抽象基类基本只是为了方便用指针, 比如vector<TypeHandler *>
class TypeHandler {
public:
	TypeHandler() = default;
	~TypeHandler() = default;
	virtual void read(FILE*);
	virtual void write(FILE*);
	// 即使是虚函数, 重写也要求返回类型相同或协同(为子类), 只能把读写的东西放在参数里了
protected:
	size_t typeSize_ = 0;
	void* temp_ = nullptr;
};

// 子类考虑memcpy之类的东西
class INTEGER: public TypeHandler {
public:
	INTEGER() = default;
	~INTEGER();
	void toWrite(void*);
};

// 比如,一个CHAR(20)类当成SQL语言的CHAR(20)的处理机
// CHAR char20 = CHAR(20) 直观但是要写=的右值引用...懒而且没太懂, 写成CHAR char20(20)
class CHAR: public TypeHandler {
public:
	CHAR(int n);
	void read();
	void write();
};

class VARCHAR: public TypeHandler {
public:
	VARCHAR(int n);
	void read();
	void write();
};

class REAL: public TypeHandler {
public:
	REAL();
	void read();
	void write();
};

class TIMESTAMP : public TypeHandler {
public:
	TIMESTAMP();
	void read();
	void write();
};


