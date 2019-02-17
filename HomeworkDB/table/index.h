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

template <typename T, typename hashType> // 存的hash后的type, 比如用int存, 但可能只用了24位
class Block { // 可扩展散列表 的 块
// 要么放在前面, 要么声明下class Block; vs的报错是在用Block的地方 缺少类型说明符 - 假定为 int。注意: C++ 不支持默认 int
public:
	Block() = delete;
	Block(int size) : size_(size), count_(0), j_(1) { positions_ = new fpos_t[size]; keys_ = new hashType[size]; rawKeys_ = new T[size]; }
	Block(const Block &) = delete;
	Block & operator=(const Block &) = delete;
	~Block() { delete[] positions_; delete[] keys_; delete rawKeys_; }

	bool fullBool() { return count_ == size_; }
	void INSERT(T rawKey, hashType key, fpos_t fpos) { // 注意同样是(T, hashType, fpos_t), index的INSERT是(T key, hashType hkey, fpos_t fpos), 这里Block的是(T rawKey, hashType key, fpos_t fpos), 类型和顺序一样, 形参名不一样, 是有点容易混淆, 但懒得改了,
		// 如果fullBool()时调用 INSERT 会内存泄漏, 上面程序保证.
		keys_[count_] = key;
		positions_[count_] = fpos;
		rawKeys_[count_] = rawKey;
		++count_;
	}
	int getJ() { return j_; }
	void setJ(int j) { j_ = j; }
	//int getCount() { return count_; }
	T * getRawKeys() { return rawKeys_; }
	hashType * getKeys() { return keys_; }
	fpos_t * getPositions() { return positions_; }

	std::vector<fpos_t> eq(T a) {
		std::vector<fpos_t> ans;
		for (int k = 0; k < count_; ++k) {
			if (a == rawKeys_[k])
				ans.push_back(positions_[k]);
		}
		return ans;
	}
private:
	int size_; // 块的大小
	int count_;
	int j_;
	T *rawKeys_;
	hashType *keys_; // 储存原key, 桶数组翻倍的时候方便
	fpos_t *positions_; // 行开始的位置
};

template <typename T, typename hashType> // 前一个是键的类型, 后一个是键hash后的储存类型, useBit_限制用的位
class ExtendibleHashIndex { //: public Index {
public:
	ExtendibleHashIndex(int blockSize, int useBit, hashType (*hash)(T)) {
		assert(typeid(T) == typeid(int));
		i_ = 1;
		size_ = 2;
		assert(blockSize > 0);
		blockSize_ = blockSize;
		useBit_ = useBit;
		bucket_ = new Block<T, hashType>* [size_];
		bucket_[0] = new Block<T, hashType>(blockSize);
		bucket_[1] = new Block<T, hashType>(blockSize);
		hash_ = hash;
	}
	~ExtendibleHashIndex() {
		//for (int k = 0; k < size_; ++k)
		//	delete bucket_[k];
		//这样会去delete已经delete的东西, 然后报错
		std::set< Block<T, hashType>*> haveBeenDelete;
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

	void INSERT(T key, hashType hkey, fpos_t fpos) {
		assert(typeid(T) == typeid(int));
		//hashType hkey = hash_(key);
		int ikey = hkey >> useBit_ - i_; // hkey >> (32 - i_); 熟悉下优先级
		if (bucket_[ikey]->fullBool()) {
			if (bucket_[ikey]->getJ() < i_) { // 块分裂
				Block<T, hashType>* tempBlock[3]; // 1 2 是拆成的新块, 2暂存bucket_[ikey], 完事把2指向的块删了
				int preJ = bucket_[ikey]->getJ();
				tempBlock[0] = new Block<T, hashType>(blockSize_);
				tempBlock[1] = new Block<T, hashType>(blockSize_);
				tempBlock[0]->setJ(preJ + 1);
				tempBlock[1]->setJ(preJ + 1);
				for (int k = 0; k < blockSize_; ++k) {
					// bucket_[ikey]的第k个, 其key第j+1位为0的放进tempBlock[0], 为1的放进tempBlock[1]
					// unsigned short用了>> 和<<回自动变成int ??? 绝了... 但>>=不变?
					tempBlock[(bucket_[ikey]->getKeys()[k] << preJ >> useBit_  - 1) & 1]->INSERT(bucket_[ikey]->getRawKeys()[k], bucket_[ikey]->getKeys()[k], bucket_[ikey]->getPositions()[k]); // 熟悉下优先级
				}
				// 受影响的并不止这两个块, 书上描述有误
				// delete bucket_[ikey]; // 释放bucket_[ikey >> 1 << 1] 和 bucket_[ikey >> 1 << 1 & 1] 指向的同一个块
				//bucket_[ikey >> 1 << 1] = tempBlock[0];
				//bucket_[(ikey >> 1 << 1)+1] = tempBlock[1];
				tempBlock[2] = bucket_[ikey];
				int from = bucket_[ikey]->getKeys()[0] >> useBit_ - preJ << i_ - preJ; //bucket_[ikey]是满的,所以bucket_[hkey]->getKeys()[0]没问题
				int to = (bucket_[ikey]->getKeys()[0] >> useBit_ - preJ) + 1 << i_ - preJ;
				for (int k = from; k < to; ++k) { // 受影响的块是前preJ位相同的i位数且指向原bucket_[ikey]的
					if (bucket_[k] == tempBlock[2])
						bucket_[k] = tempBlock[(k << preJ >> i_ - 1) & 1]; // k是i_位数(二进制), 求其第preJ+1位
				}
				delete tempBlock[2]; //bucket_[ikey]会被修改, 最开始指向的块会被删除, 用tempBlock[2]存一下
			}
			else { // bucket_[ikey]->j_ == i_ 桶容量翻倍
				++i_;
				if (i_ > useBit_)
					throw("Overflow.");
				size_ *= 2;
				Block<T, hashType>** tempBucket = new Block<T, hashType>*[size_];
				for (int k = 0; k < size_; ++k) {
					tempBucket[k] = bucket_[k >> 1];
				}
				delete[] bucket_;
				bucket_ = tempBucket;
				// 接下来INSERT递归进if-if 块分裂
			}
			// 因为可能要多次桶容量翻倍, 顺手用递归代替循环
			INSERT(key, hkey, fpos);
		}
		else { // 递归出口
			bucket_[ikey]->INSERT(key, hkey, fpos);
		}
	}

	std::vector<fpos_t> eq(void *a) {
		assert(typeid(T) == typeid(int));
		int ia = *((int *)a);
		hashType hkey = hash_(ia);
		return bucket_[hkey >> useBit_ - i_]->eq(ia);
	}

private:
	int i_;
	int size_; // size_ == 2 ** i_
	int blockSize_;
	int useBit_; // 用int存到Block里, 但可能只用24位, 和hash_函数是配套的
	Block<T, hashType> **bucket_; // 桶: 块的指针的数组
	hashType (*hash_)(T);
};



template <typename T, typename hashType> // 存的hash后的type, 比如用int存, 但可能只用了24位
class Block2 { // 线性散列表 的 块
public:
	Block2() = delete;
	Block2(int size) : size_(size), count_(0), next_(nullptr) { positions_ = new fpos_t[size]; keys_ = new hashType[size]; rawKeys_ = new T[size]; }
	Block2(const Block2 &) = delete;
	Block2 & operator=(const Block2 &) = delete;
	~Block2() { delete[] positions_; delete[] keys_; delete[] rawKeys_; if (next_) delete next_; }

	bool fullBool() { return count_ == size_; }
	void INSERT(T rawKey, hashType key, fpos_t fpos) {
		if (!fullBool()) {
			rawKeys_[count_] = rawKey;
			keys_[count_] = key;
			positions_[count_] = fpos;
			++count_;
		}
		else {
			next_ = new Block2(size_);
			next_->INSERT(rawKey, key, fpos);
		}
	}
	int getCount() { return count_; }
	T * getRawKeys() { return rawKeys_; }
	hashType * getKeys() { return keys_; }
	fpos_t * getPositions() { return positions_; }
	Block2 * getNext() { return next_; }

	std::vector<fpos_t> eq(T a) {
		std::vector<fpos_t> ans;
		if (next_) {
			std::vector<fpos_t> ans = next_->eq(a);
		}
		for (int k = 0; k < count_; ++k) {
			if (a == rawKeys_[k])
				ans.push_back(positions_[k]);
		}
		return ans;
	}
private:
	int size_; // 块的大小
	int count_;
	T *rawKeys_;
	hashType *keys_; // 储存原key, 桶数组翻倍的时候方便
	fpos_t *positions_; // 行开始的位置

	Block2 *next_;
};

template <typename T, typename hashType>
class LinearHashIndex {
public:
	LinearHashIndex() = delete;
	LinearHashIndex(int blockSize, double threshold, hashType(*hash)(int)) : i_(1), n_(2), r_(0), blockSize_(blockSize), threshold_(threshold), hash_(hash) {
		bucket_.push_back(new Block2<T, hashType>(blockSize)); // [0]
		bucket_.push_back(new Block2<T, hashType>(blockSize)); // [1]
	}
	~LinearHashIndex() {
		for (int k = 0; k < n_; ++k) {
			delete bucket_[k];
		}
	}
	LinearHashIndex(const LinearHashIndex &) = delete;
	LinearHashIndex & operator=(const LinearHashIndex &) = delete;

	void INSERT(T key, hashType hkey, fpos_t fpos) {
		assert(typeid(T) == typeid(int));
		//hashType hkey = hash_(key);
		int ikey = hkey & low_[i_];
		if (ikey < n_) {
			bucket_[ikey]->INSERT(key, hkey, fpos);
		}
		else {
			bucket_[ikey & low_[i_ - 1]]->INSERT(key, hkey, fpos);
		}
		++r_;
		if (double(r_) / double(n_) > threshold_) {
			++n_;
			if (n_ > low_[i_] + 1) { // n_>2**i_ 注意此时第i_位都已分裂正确, 对新i_就是i_-1位都正确
				++i_;
				if (i_ > sizeof(hashType) * 8)
					throw("i_ overflow.");
			}
			bucket_.push_back(new Block2<T, hashType>(blockSize_));
			// 新的块的标号是n_-1, 是i_位数, 第一位是1, 要把标号第一位1变0的桶 分裂
			Block2<T, hashType> *pBlock = bucket_[(n_ - 1) & low_[i_ - 1]]; // 指向原块, 原块之后会删除
			Block2<T, hashType> *tempBlock = new Block2<T, hashType>(blockSize_); // 标号0开头的新块, 写起来比在原桶原地分裂简单
			while (pBlock) {
				for (int k = 0; k < blockSize_; ++k) {
					if ((pBlock->getKeys()[k] & low_[i_]) >> i_ - 1) {
						bucket_[n_ - 1]->INSERT(pBlock->getRawKeys()[k], pBlock->getKeys()[k], pBlock->getPositions()[k]);
					}
					else {
						tempBlock->INSERT(pBlock->getRawKeys()[k], pBlock->getKeys()[k], pBlock->getPositions()[k]);
					}
				}
				pBlock = pBlock->getNext();
			}
			delete bucket_[(n_ - 1) & low_[i_ - 1]];
			bucket_[(n_ - 1) & low_[i_ - 1]] = tempBlock;
		}
	}

	std::vector<fpos_t> eq(void *a) {
		assert(typeid(T) == typeid(int));
		Block2<T, hashType> *pBlock;
		int ia = *((int *)a);
		hashType hkey = hash_(ia);
		int ikey = hkey & low_[i_];
		if (ikey < n_) {
			pBlock = bucket_[ikey];
		}
		else {
			pBlock = bucket_[ikey & low_[i_ - 1]];
		}
		
		return pBlock->eq(ia);
	}
private:
	int i_;
	int n_;
	int r_;
	int blockSize_;
	double threshold_;
	hashType (*hash_)(T);
	std::vector<Block2<T, hashType>*> bucket_;
	unsigned int low_[33]{ 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095,
		0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
		0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
		0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
		0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
		0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF }; // 所以目前这个index最大支持32位的hash后的值
};




