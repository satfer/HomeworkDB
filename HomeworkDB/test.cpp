#include <cstdlib>
#include "dbgen.h"
#include "table/table.h"
#include <ctime>
#include <random>


int trivialHash(int a) { return a; } // 啥都不做
unsigned short cat16Hash(int a) { return (unsigned short)(a & 0xFFFF); }
int cat24Hash(int a) { return a & 0x00FFFFF; }

int main() {
	if (0)
		dbgen();

	if (0) {
		Table nation("lineitem", "../instance/");
		NumberIndex<int> numberIndex(std::string("tbd"), nation.location_ + nation.name_ + std::string(".index"));
		int i = 23, j = 25;
		nation.print(numberIndex.eq(&i), numberIndex.eq(&j));
	

		std::vector<int> blockSizes{ 2,4,16,256 };
		//for (int blockSize : blockSizes) {
		//	ExtendibleHashIndex<int, unsigned short> exHash(blockSize, 16, cat16Hash);
		//	int i;
		//	for (i = 0; i < 11997996; ++i) {
		//		exHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // 不用内部的hash_算 是为了效率, 之后要把数据库写完的话, 这层肯定要封装的
		//	}
		//	i = 23;
		//	nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
		//	//记录内存
		//	//记录生成时间
		//	//记录查询时间
		//}
		for (int blockSize : blockSizes) {
			ExtendibleHashIndex<int, int> exHash(blockSize, 32, trivialHash);
			int i;
			for (i = 0; i < 11997996; ++i) {
				exHash.INSERT(i, trivialHash(i),numberIndex.eq(&i));
			}
			i = 23;
			nation.print(exHash.eq(&i)[0], exHash.eq(&j)[0]);
			//记录内存
			//记录生成时间
			//记录查询时间
		}

		printf("\nLinearHashIndex:\n");
		std::vector<int> blockSizes3{ 2,4 };
		std::vector<double> thresholds{ 0.6, 0.7, 0.8, 0.85 };
		for (int blockSize : blockSizes) {
			for (double threshold : thresholds) {
				LinearHashIndex<int, int> liHash(blockSize, threshold, trivialHash);
				int i;
				for (i = 0; i < 11997996; ++i) {
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
			for (i = 0; i < 11997996; ++i) {
				BPT.INSERT(i, numberIndex.eq(&i));
				//BPT.show();
			}
			i = 23;
			nation.print(BPT.eq(&i)[0], BPT.eq(&j)[0]);
			//记录内存
			//记录生成时间
			//记录查询时间
		}
	}

	if (0) {
		Table nation("customer", "../instance/");
		NumberIndex<int> numberIndex(std::string("tbd"), nation.location_ + nation.name_ + std::string(".index"));
		printf("NumberIndex 插入(大概): %ld ", clock());
		int i = 23, j = 25;
		const int maxTimes = 100000;
		const int maxN = 300000;
		clock_t startTime, endTime;
		std::default_random_engine e;
		std::uniform_int_distribution<unsigned> u(0, maxN - 1); // 
		

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			numberIndex.eq(&i);
		}
		endTime = clock();
		printf("NumberIndex 查找: %ld\n", endTime - startTime);

		printf("\nExtendibleHashIndex:\n");
		std::vector<int> blockSizes{ 2,4,16,256,2048,65536 };
		std::vector<int> useBits{ 26,27,28 };
		for (int blockSize : blockSizes) {
			for (int useBit : useBits) {
				printf("blockSize: %5d useBits: %d ", blockSize, useBit);
				ExtendibleHashIndex<int, int> exHash(blockSize, useBit, trivialHash);
				startTime = clock();
				for (i = 0; i < maxN; ++i) {
					exHash.INSERT(i, trivialHash(i), numberIndex.eq(&i));
				}
				endTime = clock();
				printf("插入: %5ld ", endTime - startTime);

				i = 23;
				startTime = clock();
				for (j = 0; j < maxTimes; ++j) {
					i = u(e);
					exHash.eq(&i);
				}
				endTime = clock();
				printf("查找: %ld\n", endTime - startTime);
			}
		}

		printf("\nLinearHashIndex:\n");
		std::vector<int> blockSizes3{ 2,4,8,16 };
		std::vector<double> thresholds{ 0.7, 0.8, 0.85,0.9,0.95 };
		for (int blockSize : blockSizes3) {
			for (double threshold : thresholds) {
				printf("blockSize: %2d threshold: %f ", blockSize, threshold);
				LinearHashIndex<int, int> liHash(blockSize, threshold, trivialHash);
				startTime = clock();
				for (i = 0; i < maxN; ++i) {
					liHash.INSERT(i, cat16Hash(i), numberIndex.eq(&i)); // 不用内部的hash_算 是为了效率, 之后要把数据库写完的话, 这层肯定要封装的
				}
				endTime = clock();
				printf("插入: %5ld ", endTime - startTime);
				i = 23; 
				startTime = clock();
				for (j = 0; j < maxTimes; ++j) {
					i = u(e);
					liHash.eq(&i);
				}
				endTime = clock();
				printf("查找: %ld\n", endTime - startTime);
			}
		}

		printf("\nBPlusTreeIndex:\n");
		std::vector<int> blockSizes4{ 4,16,256,340,512,1024,4096,65536 };
		for (int blockSize : blockSizes4) {
			printf("blockSize: %5d ", blockSize);
			BPlusTree<int> BPT(blockSize);
			startTime = clock();
			for (i = 0; i < maxN; ++i) {
				BPT.INSERT(i, numberIndex.eq(&i));
				//BPT.show();
			}
			endTime = clock();
			printf("插入: %5ld ", endTime - startTime);
			i = 23;
			startTime = clock();
			for (j = 0; j < maxTimes; ++j) {
				i = u(e);
				BPT.eq(&i);
			}
			endTime = clock();
			printf("查找: %ld\n", endTime - startTime);
		}
	}

	if (1) {
		Table lineitem("lineitem", "../instance/");
		//Table orders("orders", "../instance/");
		NumberIndex<int> numberIndex(std::string("tbd"), lineitem.location_ + lineitem.name_ + std::string(".index"));
		int i = 23, j = 25;
		const int maxTimes = 100000;
		const int maxN = 11997996;
		int timeLimit = 3600000;
		clock_t startTime, endTime;
		std::default_random_engine e;
		std::uniform_int_distribution<unsigned> u(0, maxN - 1); 


		printf("\nExtendibleHashIndex:\n");
		ExtendibleHashIndex<int, int> exHash(16, 24, trivialHash);
		startTime = clock();
		for (i = 0; i < maxN; ++i) {
			exHash.INSERT(i, trivialHash(i), numberIndex.eq(&i));
		}
		endTime = clock();
		printf("QUERY1: INSERT     : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			exHash.eq(&i);
		}
		endTime = clock();
		printf("QUERY2: EQUAL      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
			++i;
			exHash.eq(&i);
		}
		endTime = clock();
		printf("QUERY3: >= LIMIT 10: %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			exHash.eq(&i).size();
		}
		endTime = clock();
		printf("QUERY4: COUNT      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			for (; i < maxN; ++i) {
				exHash.eq(&i);
			}
			if (clock() - startTime > timeLimit) {
				printf("QUERY5: >=         : 超时\n");
				break;
			}
		}
		endTime = clock();
		if (endTime - startTime < timeLimit)
			printf("QUERY5: >=         : %ld\n", endTime - startTime);




		printf("\nLinearHashIndex:\n");
		LinearHashIndex<int, int> liHash(2, 0.8, trivialHash);
		startTime = clock();
		for (i = 0; i < maxN; ++i) {
			liHash.INSERT(i, trivialHash(i), numberIndex.eq(&i));
		}
		endTime = clock();
		printf("QUERY1: INSERT     : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			liHash.eq(&i);
		}
		endTime = clock();
		printf("QUERY2: EQUAL      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
			++i;
			liHash.eq(&i);
		}
		endTime = clock();
		printf("QUERY3: >= LIMIT 10: %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			liHash.eq(&i).size();
		}
		endTime = clock();
		printf("QUERY4: COUNT      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			for (; i < maxN; ++i) {
				liHash.eq(&i);
			}
			if (clock() - startTime > timeLimit) {
				printf("QUERY5: >=         : 超时\n");
				break;
			}
		}
		endTime = clock();
		if (endTime - startTime < timeLimit)
			printf("QUERY5: >=         : %ld\n", endTime - startTime);


		printf("\nBPlusTreeIndex:\n");
		BPlusTree<int> BPT(512);
		startTime = clock();
		for (i = 0; i < maxN; ++i) {
			BPT.INSERT(i, numberIndex.eq(&i));
		}
		endTime = clock();
		printf("QUERY1: INSERT     : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			BPT.eq(&i);
		}
		endTime = clock();
		printf("QUERY2: EQUAL      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			BPT.ge(i, 10);
		}
		endTime = clock();
		printf("QUERY3: >= LIMIT 10: %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < maxTimes; ++j) {
			i = u(e);
			BPT.eq(&i).size();
		}
		endTime = clock();
		printf("QUERY4: COUNT      : %ld\n", endTime - startTime);

		startTime = clock();
		for (j = 0; j < 1000; ++j) {
			i = u(e);
			BPT.ge(i);
		}
		endTime = clock();
		printf("QUERY5: >=         : %ld\n", endTime - startTime);
	}

	system("pause");
	return 0;
}