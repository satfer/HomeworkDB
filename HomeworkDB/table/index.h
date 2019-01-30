#pragma once

#include <string>



// 虚基类基本只是为了方便用指针, 比如vector<Index *>
class Index {
public:
	virtual int le(void*) = 0; // less equal // 返回文件中的pos //一个再看看virtual  //void* 是指针地址, 然后强制转换 reinpreter_cast
	virtual int gt(void*) = 0; // more than
};