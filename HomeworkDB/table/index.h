#pragma once

#include <string>
#include <vector>
#include <set>
#include <cassert>

#define MainIndex SortedIndex



// 虚基类基本只是为了方便用指针, 比如vector<Index *>

// 索引可能无序, 只有eq
class Index {
public:
	virtual fpos_t eq(void *) = 0; // equal // 返回行的起始位置
	virtual void show() = 0;
};

// 主索引的东西总是有序的
class SortedIndex : virtual public Index{
public:
	virtual fpos_t eq(void *) = 0; // equal
	virtual fpos_t ge(void *, int n = 1) = 0; // greater equal >=的前n个
	virtual fpos_t lt(void *) = 0; // less than <的第一个
	virtual void show() = 0;
};

template <typename T>
class NumberIndex : public Index {
public:
	NumberIndex() = delete;
	NumberIndex(const std::string &keyAttr, const std::string &file) : keyAttr_(keyAttr) {
		FILE *fpTemp;
		if (typeid(T) == typeid(std::string))
			throw("Not support. 不支持把字符串的索引作为存入.index文件");
		else if (fopen_s(&fpTemp, file.c_str(), "rb"))
			throw("Open file failed.");
		else {
			T dataTemp;
			fpos_t fpos;
			fread(&dataTemp, sizeof(T), 1, fpTemp);
			while (!feof(fpTemp)) {
				fread(&fpos, sizeof(fpos_t), 1, fpTemp);
				data_.push_back(dataTemp);
				positions_.push_back(fpos);
				fread(&dataTemp, sizeof(T), 1, fpTemp);
			}
		}
	} // 从.index文件打开
	// NumberIndex(const std::string &keyAttr, FILE *fpDB)
	~NumberIndex() = default;

	fpos_t eq(void *a) {
		// 如果写成二分法, NumberIndex可以存数的顺序索引
		// 这么写, 可能之后写删除行 都容易出问题 (鲁棒性太差
		assert(typeid(T) == typeid(int));
		return positions_[*(T *)a]; // *reinpreter_cast<T*>a 编译不过...???
	}

	void show() {
		printf("Index Type: %s, on %s", indexType_.c_str(), keyAttr_.c_str());
	}
private:
	const std::string indexType_{ "SeqIndex" };
	std::string keyAttr_;
	// bool isVARCHAR;
	// int typeSize_;

	std::vector<T> data_;
	std::vector<fpos_t> positions_;
};

template <typename hashType> // 存的hash后的type
class Block { // 可扩展散列表 线性散列表 通用
// 要么放在前面, 要么声明下class Block; vs的报错是在用Block的地方 缺少类型说明符 - 假定为 int。注意: C++ 不支持默认 int
public:
	Block() = delete;
	Block(int size) : size_(size), count_(0), j_(1) { positions_ = new fpos_t[size]; keys_ = new hashType[size]; }
	Block(const Block &) = delete;
	Block & operator=(const Block &) = delete;
	~Block() { delete[] positions_; delete[] keys_; }

	bool fullBool() { return count_ == size_; }
	void INSERT(hashType key, fpos_t fpos) {
		// 如果fullBool()时调用 INSERT 会内存泄漏, 上面程序保证.
		keys_[count_] = key;
		positions_[count_] = fpos;
		++count_;
	}
	int getJ() { return j_; }
	void setJ(int j) { j_ = j; }
	int getCount() { return count_; }
	hashType * getKeys() { return keys_; }
	fpos_t * getPositions() { return positions_; }
private:
	int size_; // 块的大小
	int count_;
	int j_;
	hashType *keys_; // 储存原key, 桶数组翻倍的时候方便
	fpos_t *positions_; // 行开始的位置
};

template <typename T, typename hashType> // 前一个是键的类型, 后一个是键hash后的类型
class ExtendibleHashIndex { //: public Index {
public:
	ExtendibleHashIndex(int blockSize, hashType (*hash)(int)) {
		assert(typeid(T) == typeid(int));
		i_ = 1;
		size_ = 2;
		assert(blockSize > 0);
		blockSize_ = blockSize;
		bucket_ = new Block<hashType>* [size_];
		bucket_[0] = new Block<hashType>(blockSize);
		bucket_[1] = new Block<hashType>(blockSize);
		hash_ = hash;
	}
	~ExtendibleHashIndex() {
		//for (int k = 0; k < size_; ++k)
		//	delete bucket_[k];
		//这样会去delete已经delete的东西, 然后报错
		std::set< Block<hashType>*> haveBeenDelete;
		for (int k = 0; k < size_; ++k) {
			if (haveBeenDelete.count(bucket_[k]) == 0) {
				delete bucket_[k];
				haveBeenDelete.insert(bucket_[k]);
			}
		}
		delete[] bucket_;
	}
	ExtendibleHashIndex(const ExtendibleHashIndex &) = delete;
	ExtendibleHashIndex & operator= (const ExtendibleHashIndex &) = delete;

	void INSERT(int key, fpos_t fpos) {
		assert(typeid(T) == typeid(int));
		hashType hkey = hash_(key);
		hkey >>= sizeof(hashType) * 8 - i_; // hkey >> (32 - i_); 熟悉下优先级
		if (bucket_[hkey]->fullBool()) {
			if (bucket_[hkey]->getJ() < i_) { // 块分裂
				Block<hashType>* tempBlock[3]; // 1 2 是拆成的新块, 2暂存bucket_[hkey], 完事把2指向的块删了
				int preJ = bucket_[hkey]->getJ();
				tempBlock[0] = new Block<hashType>(blockSize_);
				tempBlock[1] = new Block<hashType>(blockSize_);
				tempBlock[0]->setJ(preJ + 1);
				tempBlock[1]->setJ(preJ + 1);
				for (int k = 0; k < blockSize_; ++k) {
					// bucket_[hkey]的第k个, 其key第j+1位为0的放进tempBlock[0], 为1的放进tempBlock[1]
					// unsigned short用了>> 和<<回自动变成int ??? 绝了... 但>>=不变?
					tempBlock[(bucket_[hkey]->getKeys()[k] << preJ >> sizeof(hashType) * 8  - 1) & 1]->INSERT(bucket_[hkey]->getKeys()[k], bucket_[hkey]->getPositions()[k]); // 熟悉下优先级
				}
				// 受影响的并不止这两个块, 书上描述有误
				// delete bucket_[hkey]; // 释放bucket_[hkey >> 1 << 1] 和 bucket_[hkey >> 1 << 1 & 1] 指向的同一个块
				//bucket_[hkey >> 1 << 1] = tempBlock[0];
				//bucket_[(hkey >> 1 << 1)+1] = tempBlock[1];
				tempBlock[2] = bucket_[hkey];
				int from = bucket_[hkey]->getKeys()[0] >> sizeof(hashType) * 8 - preJ << i_ - preJ; //bucket_[hkey]是满的,所以bucket_[hkey]->getKeys()[0]没问题
				int to = (bucket_[hkey]->getKeys()[0] >> sizeof(hashType) * 8 - preJ) + 1 << i_ - preJ;
				for (int k = from; k < to; ++k) { // 受影响的块是前preJ位相同的i位数且指向原bucket_[hkey]的
					if (bucket_[k] == tempBlock[2])
						bucket_[k] = tempBlock[(k << preJ >> i_ - 1) & 1]; // k是i_位数(二进制), 求其第preJ+1位
				}
				delete tempBlock[2]; //bucket_[hkey]会被修改, 最开始指向的块会被删除, 用tempBlock[2]存一下
			}
			else { // bucket_[hkey]->j_ == i_ 桶容量翻倍
				++i_;
				size_ *= 2;
				Block<hashType>** tempBucket = new Block<hashType>*[size_];
				for (int k = 0; k < size_; ++k) {
					tempBucket[k] = bucket_[k >> 1];
				}
				delete[] bucket_;
				bucket_ = tempBucket;
				// 接下来INSERT递归进if-if 块分裂
			}
			// 因为可能要多次桶容量翻倍, 顺手用递归代替循环
			INSERT(key, fpos);
		}
		else { // 递归出口
			bucket_[hkey]->INSERT(key, fpos);
		}
	}

	fpos_t eq(void *a) {
		assert(typeid(T) == typeid(int));
		int ia = *((int *)a);
		Block<hashType> *tempBlock = bucket_[hash_(ia) >> sizeof(hashType) * 8 - i_];
		for (int k = 0; k < tempBlock->getCount(); ++k) {
			if (ia == tempBlock->getKeys()[k])
				return tempBlock->getPositions()[k];
		}
		return -1; //!!!
	}

private:
	int i_;
	int size_; // size_ == 2 ** i_
	int blockSize_;
	Block<hashType> **bucket_; // 桶: 块的指针的数组
	hashType (*hash_)(int);
};

// trivalHash int 爆内存
//template <typename T>
//class ExtendibleHashIndex { //: public Index {
//public:
//	ExtendibleHashIndex(int blockSize, int(*hash)(int)) {
//		assert(typeid(T) == typeid(int));
//		i_ = 0;
//		size_ = 1;
//		blockSize_ = blockSize;
//		bucket_ = new Block*[size_];
//		bucket_[0] = new Block(blockSize);
//		hash_ = hash;
//	}
//	~ExtendibleHashIndex() {
//		for (int k = 0; k < size_; ++k) {
//			delete bucket_[k];
//		}
//		delete[] bucket_;
//	}
//	ExtendibleHashIndex(const ExtendibleHashIndex &) = delete;
//	ExtendibleHashIndex & operator= (const ExtendibleHashIndex &) = delete;
//
//	void INSERT(int key, fpos_t fpos) {
//		assert(typeid(T) == typeid(int));
//		int hkey = hash_(key);
//		hkey = (i_ == 0 ? 0 : hkey >> 32 - i_); // hkey >> (32 - i_); 熟悉下优先级
//		if (bucket_[hkey]->fullBool()) {
//			if (bucket_[hkey]->getJ() < i_) { // 块分裂
//				Block* tempBlock[2];
//				tempBlock[0] = new Block(blockSize_);
//				tempBlock[1] = new Block(blockSize_);
//				tempBlock[0]->setJ(bucket_[hkey]->getJ() + 1);
//				tempBlock[1]->setJ(bucket_[hkey]->getJ() + 1);
//				for (int k = 0; k < blockSize_; ++k) {
//					tempBlock[bucket_[hkey]->getKeys()[k] >> 32 - bucket_[hkey]->getJ() - 1 & 1]->INSERT(key, fpos); // 熟悉下优先级
//				}
//				delete bucket_[hkey]; // 释放bucket_[hkey >> 1 << 1] 和 bucket_[hkey >> 1 << 1 & 1] 指向的同一个块
//				// tempBlock[2] = bucket_[hkey];
//				bucket_[hkey >> 1 << 1] = tempBlock[0];
//				bucket_[hkey >> 1 << 1 | 1] = tempBlock[1];
//				// delete tempBlock[2]; //要这么写要写operator=吗, 讲道理是指针 应该不用
//			}
//			else { // bucket_[hkey]->j_ == i_ 桶容量翻倍
//				++i_;
//				size_ *= 2;
//				Block** tempBucket = new Block*[size_];
//				for (int k = 0; k < size_; ++k) {
//					tempBucket[k] = bucket_[k >> 1];
//				}
//				delete[] bucket_;
//				bucket_ = tempBucket;
//				// 接下来INSERT递归进if-if 块分裂
//			}
//			// 因为可能要多次桶容量翻倍, 顺手用递归代替循环
//			INSERT(key, fpos);
//		}
//		else { // 递归出口
//			bucket_[hkey]->INSERT(key, fpos);
//		}
//	}
//
//	fpos_t eq(void *a) {
//		assert(typeid(T) == typeid(int));
//		int ia = *((int *)a);
//		Block *tempBlock = bucket_[hash_(ia) >> 32 - i_];
//		for (int k = 0; k < tempBlock->getCount(); ++k) {
//			if (ia == tempBlock->getKeys()[k])
//				return tempBlock->getPositions()[k];
//		}
//		return -1; //!!!
//	}
//
//private:
//	int i_;
//	int size_; // size_ == 2 ** i_
//	int blockSize_;
//	Block **bucket_; // 桶: 块的指针的数组
//	int(*hash_)(int);
//};