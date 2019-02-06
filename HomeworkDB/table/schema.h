#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>



class Schema {
	// ����->���ͺͿռ��С
public:
	Schema(); //��Ȼ�����delete��, ����Table::Table(const std::string &s)û��һ��ʼ�ͳ�ʼ��schema_����Ҫ����Schema::Schema()
	Schema(const std::vector<std::string> &);
	// дjoin��ʱ��Ӧ�û��ĳ�����캯��, д��ʱ����˵��
	~Schema(); // new ��һ�� TypeHandler, �ǵô��� ���ƹ���, =, ����

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

// ���ʹ��� ��. ����������ֻ��Ϊ�˷�����ָ��, ����vector<TypeHandler *>
class TypeHandler {
public:
	TypeHandler() = default;
	~TypeHandler() = default;
	virtual void read(FILE*);
	virtual void write(FILE*);
	// ��ʹ���麯��, ��дҲҪ�󷵻�������ͬ��Эͬ(Ϊ����), ֻ�ܰѶ�д�Ķ������ڲ�������
protected:
	size_t typeSize_ = 0;
	void* temp_ = nullptr;
};

// ���࿼��memcpy֮��Ķ���
class INTEGER: public TypeHandler {
public:
	INTEGER() = default;
	~INTEGER();
	void toWrite(void*);
};

// ����,һ��CHAR(20)�൱��SQL���Ե�CHAR(20)�Ĵ����
// CHAR char20 = CHAR(20) ֱ�۵���Ҫд=����ֵ����...������û̫��, д��CHAR char20(20)
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


