#pragma once

#include <string>



// ��������ֻ��Ϊ�˷�����ָ��, ����vector<Index *>
class Index {
public:
	virtual int le(void*) = 0; // less equal // �����ļ��е�pos //һ���ٿ���virtual  //void* ��ָ���ַ, Ȼ��ǿ��ת�� reinpreter_cast
	virtual int gt(void*) = 0; // more than
};