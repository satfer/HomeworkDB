#include <cstdlib>
#include "dbgen.h"
#include "table/table.h"



int trivialHash(int a) { return a; } // 啥都不做
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
				exHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // 不用内部的hash_算 是为了效率, 之后要把数据库写完的话, 这层肯定要封装的
			}
			i = 23;
			nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
			//记录内存
			//记录生成时间
			//记录查询时间
		}
		//for (int blockSize : blockSizes) {
		//	ExtendibleHashIndex<int, int> exHash(blockSize, 24, cat24Hash);
		//	int i;
		//	for (i = 0; i <= 24; ++i) {
		//		exHash.INSERT(i, numberIndex.eq(&i));
		//	}
		//	i = 23;
		//	nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
		//	//记录内存
		//	//记录生成时间
		//	//记录查询时间
		//}

		printf("\nLinearHashIndex:\n");
		std::vector<int> blockSizes3{ 2,4 };
		std::vector<double> thresholds{ 0.6, 0.7, 0.8, 0.85 };
		for (int blockSize : blockSizes) {
			for (double threshold : thresholds) {
				LinearHashIndex<int, unsigned short> liHash(blockSize, threshold, cat16Hash);
				int i;
				for (i = 0; i <= 24; ++i) {
					liHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // 不用内部的hash_算 是为了效率, 之后要把数据库写完的话, 这层肯定要封装的
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
			//记录内存
			//记录生成时间
			//记录查询时间
		}
	}

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