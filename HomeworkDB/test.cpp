#include <cstdlib>
#include "dbgen.h"

int main() {
	dbgen();
	system("pause");
	return 0;
	// tt.INSERT(xx.tbl)
}

// Table::INSERT(FILE* fp) // 暂时封装一下, 不然连type_, handler_都访问不了
// 打开tbl
// while readline
//   for attr: attrs // attrs暂时还没存
//     if (type_[attr] == "..") // 一个长if搞定算了, 这样handler_暂时只负责读?
//   每行的地址要存, (主索引

// 单表SELECT, 返回行数和列数
// 看看join再想多表SELECT