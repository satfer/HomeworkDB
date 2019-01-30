#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>



class Schema {
	// ����->���ͺͿռ��С
public:
	Schema() = default; //��Ȼ�����delete��, ����Table::Table(const std::string &s)û��һ��ʼ�ͳ�ʼ��schema_����Ҫ����Schema::Schema()
	Schema(const std::vector<std::string> &);
	// дjoin��ʱ��Ӧ�û��ĳ�����캯��, д��ʱ����˵��
	~Schema() = default;

	Schema(const Schema &) = delete;
	Schema & operator=(const Schema &) = delete;
	Schema & operator=(const Schema &&a) { type_ = a.type_; handler_ = a.handler_; }

	// const std::string & operator[] (const std::string &attr) { return schema_[attr]; }
	const std::string & getType(const std::string &attr) { return type_[attr]; }
	const TypeHandler * getHandler (const std::string &attr) { return handler_[attr]; }
private:
	std::map<std::string, std::string> type_; // attr->type
	std::map<std::string, TypeHandler *> handler_; // attr->TypeHandler
};

// ���ʹ��� ��. ����������ֻ��Ϊ�˷�����ָ��, ����vector<TypeHandler *>
class TypeHandler {
	TypeHandler() = default;
	~TypeHandler() = default;
	virtual void read() = 0;
	virtual void toWrite() = 0; // ��ʹ���麯��, ��дҲҪ�󷵻�������ͬ��Эͬ(Ϊ����), ֻ�ܰѶ�д�Ķ������ڲ�������
};

class INTEGER: public TypeHandler {
	INTEGER();
	void read();
	void toWrite();
};

// ����,һ��CHAR(20)�൱��SQL���Ե�CHAR(20)�Ĵ�����
// CHAR char20 = CHAR(20) ֱ�۵���Ҫд=����ֵ����...������û̫��, д��CHAR char20(20)
class CHAR : public TypeHandler {
public:
	CHAR(int n);
	CHAR(const std::string &);
	void read();
	void toWrite();
};

class VARCHAR : public TypeHandler {
public:
	VARCHAR(int n);
	VARCHAR(const std::string &);
	void read();
	void toWrite();
};


