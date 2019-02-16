#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>



class Schema {
	// ����->���ͺͿռ��С
public:
	Schema()=delete; //�������delete��, Tableһ��ʼ��Ҫ��ʼ��schema_
	// Schema(const std::vector<std::string> &);
	Schema(const std::string &file); // �ļ���
	// дjoin��ʱ��Ӧ�û��ĳ�����캯��, д��ʱ����˵��
	~Schema()=default; // new ��һ�� TypeHandler, �ǵô��� ���ƹ���, =, ����

	Schema(const Schema &) = delete;
	Schema & operator=(const Schema &) = delete;
	// Schema & operator=(Schema &&a) { type_ = a.type_; handler_ = a.handler_; }

	// const std::string & operator[] (const std::string &attr) { return schema_[attr]; }
	//const std::string & getType(const std::string &attr) { return attrType_[attr]; }
	//const TypeHandler * getHandler(const std::string &attr) { return handler_[attr]; }
//private: // ���ģʽqtm
	std::vector<std::string> attrs_;
	std::vector<std::string> types_;
	std::map<std::string, std::string> attrType_; // attr->type
};

//// ���ʹ��� ��. ����������ֻ��Ϊ�˷�����ָ��, ����vector<TypeHandler *>
//class TypeHandler {
//public:
//	TypeHandler() = default;
//	~TypeHandler() = default;
//	virtual void read(FILE*);
//	virtual void write(FILE*);
//	// ��ʹ���麯��, ��дҲҪ�󷵻�������ͬ��Эͬ(Ϊ����), ֻ�ܰѶ�д�Ķ������ڲ�������
//protected:
//	size_t typeSize_ = 0;
//	void* temp_ = nullptr;
//};
//
//// ���࿼��memcpy֮��Ķ���
//class INTEGER: public TypeHandler {
//public:
//	INTEGER() = default;
//	~INTEGER();
//	void toWrite(void*);

