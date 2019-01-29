#pragma once

#include <string>



class Index {
public:
	virtual int le(void*) = 0; // less equal // 返回文件中的pos //一个再看看virtual  //一个想想怎么给le gt 传参, 因为类型会变, 统一用字符串?
	virtual int gt(void*) = 0; // more than
};