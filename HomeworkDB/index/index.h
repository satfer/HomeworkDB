#pragma once

#include <string>



class Index {
public:
	virtual int le(void*) = 0; // less equal // �����ļ��е�pos //һ���ٿ���virtual  //һ��������ô��le gt ����, ��Ϊ���ͻ��, ͳһ���ַ���?
	virtual int gt(void*) = 0; // more than
};