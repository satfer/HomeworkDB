#include <cstdlib>
#include "dbgen.h"

int main() {
	dbgen();
	system("pause");
	return 0;
	// tt.INSERT(xx.tbl)
}

// Table::INSERT(FILE* fp) // ��ʱ��װһ��, ��Ȼ��type_, handler_�����ʲ���
// ��tbl
// while readline
//   for attr: attrs // attrs��ʱ��û��
//     if (type_[attr] == "..") // һ����if�㶨����, ����handler_��ʱֻ�����?
//   ÿ�еĵ�ַҪ��, (������

// ����SELECT, ��������������
// ����join������SELECT