#include <cstdlib>
#include "dbgen.h"
#include "table/table.h"



int trivialHash(int a) { return a; } // ɶ������
unsigned short cat16Hash(int a) { return (unsigned short)(a & 0xFFFF); }
int cat24Hash(int a) { return a & 0x00FFFFF; }

int main() {
	if (1)
		dbgen();

	if (1) {
		Table nation("lineitem", "../instance/");
		NumberIndex<int> numberIndex(std::string("tbd"), nation.location_ + nation.name_ + std::string(".index"));
		int i = 22, j = 24;
		nation.print(numberIndex.eq(&i));
	


		printf("%d\n", 1 >> 33);

		std::vector<int> blockSizes{ 2,4,16,256 };
		for (int blockSize : blockSizes) {
			ExtendibleHashIndex<int, unsigned short> exHash(blockSize, 16, cat16Hash);
			int i;
			for (i = 0; i <= 24; ++i) {
				exHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // �����ڲ���hash_�� ��Ϊ��Ч��, ֮��Ҫ�����ݿ�д��Ļ�, ���϶�Ҫ��װ��
			}
			i = 23;
			nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
			//��¼�ڴ�
			//��¼����ʱ��
			//��¼��ѯʱ��
		}
		//for (int blockSize : blockSizes) {
		//	ExtendibleHashIndex<int, int> exHash(blockSize, 24, cat24Hash);
		//	int i;
		//	for (i = 0; i <= 24; ++i) {
		//		exHash.INSERT(i, numberIndex.eq(&i));
		//	}
		//	i = 23;
		//	nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
		//	//��¼�ڴ�
		//	//��¼����ʱ��
		//	//��¼��ѯʱ��
		//}

		printf("\nLinearHashIndex:\n");
		std::vector<int> blockSizes3{ 2,4 };
		std::vector<double> thresholds{ 0.6, 0.7, 0.8, 0.85 };
		for (int blockSize : blockSizes) {
			for (double threshold : thresholds) {
				LinearHashIndex<int, unsigned short> liHash(blockSize, threshold, cat16Hash);
				int i;
				for (i = 0; i <= 24; ++i) {
					liHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // �����ڲ���hash_�� ��Ϊ��Ч��, ֮��Ҫ�����ݿ�д��Ļ�, ���϶�Ҫ��װ��
				}
				i = 23;
				nation.print(liHash.eq(&i)[0], liHash.eq(&j)[0]);
			}
		}

		printf("\nBPlusTreeIndex:\n");
		std::vector<int> blockSizes4{ 4,16,256,340,512 };
		for (int blockSize : blockSizes4) {
			BPlusTree<int> BPT(blockSize);
			int i;
			for (i = 0; i <= 24; ++i) {
				BPT.INSERT(i, numberIndex.eq(&i));
			}
			i = 23;
			nation.print(BPT.eq(&i)[0], BPT.eq(&j)[0]);
			//��¼�ڴ�
			//��¼����ʱ��
			//��¼��ѯʱ��
		}
	}

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