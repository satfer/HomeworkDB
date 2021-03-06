#pragma once

#include <string>
#include <vector>
#include <set>
#include <cassert>
#include <iostream>

#define MainIndex SortedIndex



// 虚基类基本只是为了方便用指针, 比如vector<Index *>

// 索引可能无序, 只有eq
class Index {
public:
	virtual std::vector<fpos_t> eq(void *) = 0; // equal // 返回行的起始位置
	//virtual void show() = 0;
};

// 主索引的东西总是有序的
class SortedIndex : virtual public Index{
public:
	virtual std::vector<fpos_t> eq(void *) = 0; // equal
	virtual fpos_t ge(void *, int n = 1) = 0; // greater equal >=的前n个
	virtual fpos_t lt(void *) = 0; // less than <的第一个
	virtual void show() = 0;
};

template <typename T>
class NumberIndex{ //: public Index {
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
	int count_; // 记录的数量
	int j_; // 使用的位数
	T *rawKeys_; // 储存原key
	hashType *keys_; // 储存hash值, 桶数组翻倍的时候方便
	fpos_t *positions_; // 行开始的位置
};

template <typename T, typename hashType> // 前一个是键的类型, 后一个是键hash后的储存类型, useBit_限制用的位
class ExtendibleHashIndex : public Index {
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

		// 这析构效率极低, 别析构了(反正内存够
		/*std::set< Block<T, hashType>*> haveBeenDelete;
		for (int k = 0; k < size_; ++k) {
			if (haveBeenDelete.count(bucket_[k]) == 0) {
				delete bucket_[k];
				haveBeenDelete.insert(bucket_[k]);
			}
		}*/
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
	int i_; // 使用的位数
	unsigned int size_; // size_ == 2 ** i_
	int blockSize_; // 块的大小
	int useBit_; // 用int存到Block里, 但可能只用24位, 和hash_函数是配套的
	Block<T, hashType> **bucket_; // 桶: 块的指针的数组
	hashType (*hash_)(T); // hash函数
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
	int count_; // 记录的数量
	T *rawKeys_; // 储存原key
	hashType *keys_; // 储存hash值
	fpos_t *positions_; // 行开始的位置

	Block2 *next_; // 链表形式的溢出块
};

template <typename T, typename hashType>
class LinearHashIndex : public Index {
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
	int i_; // 使用的位数
	int n_; // 桶的总数
	int r_; // 记录的总数
	int blockSize_; // 块的大小
	double threshold_; // r/n的阈值
	hashType (*hash_)(T); // 散列函数
	std::vector<Block2<T, hashType>*> bucket_; // 桶数组
	unsigned int low_[33]{ 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095,
		0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
		0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
		0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
		0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
		0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF }; // 所以目前这个index最大支持32位的hash后的值
};





template <typename T>
class BPlusTreeLeaf;


template <typename T>
class BPlusTreeNode {
public:
	BPlusTreeNode() = delete;
	BPlusTreeNode(int n) : size_(n), count_(0) {// n个查找键, n+1个指针
		keys_ = new T[n + 1]; //多的位置是为了 要溢出->分裂时方便
	}
	~BPlusTreeNode() { delete[]keys_; }
	BPlusTreeNode(const BPlusTreeNode &) = delete;
	BPlusTreeNode & operator=(const BPlusTreeNode &) = delete;

	virtual std::vector<fpos_t> find(const T &key, int limit = 1) = 0;
	virtual void INSERT(const T &key, const fpos_t &fpos, T &newkey, BPlusTreeNode<T> *&newp) = 0; // 后两个参数 相当于返回值, 用于递归分裂
	virtual void show() = 0;

	virtual BPlusTreeNode ** getChildren() { throw("应该调用重载的函数."); }
	virtual fpos_t * getPositions() { throw("应该调用重载的函数."); } //重载virtual可以让指向子类的父类指针访问子类的成员
	virtual BPlusTreeLeaf<T> * getNext() { throw("应该调用重载的函数."); }
	virtual void setNext(BPlusTreeLeaf<T> *) { throw("应该调用重载的函数."); }

	//protect:
	int size_; // 键的最大数量
	int count_; // 已使用的键的数量
	T *keys_; // 键数组
};

template <typename T>
class BPlusTreeLeaf : public BPlusTreeNode<T> {
public:
	BPlusTreeLeaf() = delete;
	BPlusTreeLeaf(int n) : BPlusTreeNode<T>(n), next_(nullptr) {
		positions_ = new fpos_t[n + 1];//多的位置是为了 要溢出->分裂时方便
	}
	~BPlusTreeLeaf() { delete[]positions_; }
	BPlusTreeLeaf(const BPlusTreeLeaf &) = delete;
	BPlusTreeLeaf & operator=(const BPlusTreeLeaf &) = delete;

	std::vector<fpos_t> find(const T &key, int limit = 1) {
		std::vector<fpos_t> ans;
		// 二分法
		int low = 0, high = this->count_ - 1, mid = (low + high) / 2; // 标号含low含high [low, high]
		while (high >= low) {
			mid = (low + high) / 2;
			if (this->keys_[mid] == key)
				break;
			else if (key < this->keys_[mid])
				high = mid - 1;
			else
				low = mid + 1;
		}
		if (high < low)
			return ans; //空的

		// 可重复b+树 鸽了
		// mid是找到的key, 找最前面的keys_[mid] == key
		//while (this->keys_[mid] == key)
		//	--mid;
		//++mid; 
		// 可能重复到下一个块去了
		//BPlusTreeLeaf *pLeaf = this;
		//while (pLeaf) {
		//	for (int i = (pLeaf == this ? mid : 0); i < pLeaf->count_; ++i) {
		//		if (pLeaf->keys_[i] > key)
		//			return ans; //带break效果
		//		ans.push_back(pLeaf->positions_[i]);
		//	}
		//	pLeaf = pLeaf->next_;
		//}
		//return ans; // 查最后一个的时候刚好会出循环且无返回

		BPlusTreeLeaf *pLeaf = this;
		int j = limit;
		while (pLeaf) {
			for (int i = (pLeaf == this ? mid : 0); i < pLeaf->count_; ++i) {
				ans.push_back(pLeaf->positions_[i]);
				--j;
				if (j == 0)
					return ans;
			}
			pLeaf = pLeaf->next_;
		}
		return ans; // 防
	}

	void INSERT(const T &key, const fpos_t &fpos, T &newkey, BPlusTreeNode<T> *&newp) {
		int low = 0, high = this->count_ - 1, mid = (low + high) / 2, k; // 标号含low含high [low, high]
		while (high >= low) {
			mid = (low + high) / 2;
			if (this->keys_[mid] == key)
				throw("暂时不支持可重复B+树.");
			else if (key < this->keys_[mid])
				high = mid - 1;
			else
				low = mid + 1;
		}
		// 最后一次循环一定是 high==low 或者 high==low+1,  退出循环high==low-1, key在(keys_[high],keys_[low])里, keys_[-1]视为负无穷, keys_[conut_]视为正无穷
		for (k = this->count_ - 1; k >= low && k >= 0; --k) {
			this->keys_[k + 1] = this->keys_[k];
			positions_[k + 1] = positions_[k];
		}
		this->keys_[low] = key;
		positions_[low] = fpos;
		++this->count_;

		// 自动兼容初始情况
		if (this->count_ > this->size_) {
			newp = new BPlusTreeLeaf<T>(this->size_);
			newp->setNext(this->next_);
			this->next_ = (BPlusTreeLeaf<T>*)newp;
			
			int newc = this->count_ / 2;
			newp->count_ = this->count_ - newc;
			this->count_ = newc;
			memcpy(newp->keys_, this->keys_ + newc, sizeof(T) * newp->count_);
			memcpy(newp->getPositions(), this->positions_ + newc, sizeof(fpos_t) * newp->count_);

			newkey = newp->keys_[0]; // 可重复b+树 这里要调整
		}
		else {
			newkey = -1;
			newp = nullptr;
		}
	}

	void show() {
		std::cout << "size:" << this->size_ << " count:" << this->count_ << "\n";
		int k;
		for (k = 0; k < this->count_; ++k)
			printf("%9d", this->keys_[k]);
		std::cout << "\n";
		for (k = 0; k < this->count_; ++k)
			printf("%9lld", this->positions_[k]);
		std::cout << "\n\n";
	}

	fpos_t * getPositions() { return positions_; } //重载virtual可以让指向子类的父类指针访问子类的成员
	BPlusTreeLeaf * getNext() { return next_; }
	void setNext(BPlusTreeLeaf * next) { next_ = next; }
	//private:
	fpos_t *positions_; // 文件位置数组
	BPlusTreeLeaf *next_; // 指向下一个叶子节点
};

template <typename T>
class BPlusTreeNotLeaf : public BPlusTreeNode<T> {
public:
	BPlusTreeNotLeaf() = delete;
	BPlusTreeNotLeaf(int n) : BPlusTreeNode<T>(n) {
		children_ = new BPlusTreeNode<T>*[n+2];//多的位置是为了 要溢出->分裂时方便
	}
	~BPlusTreeNotLeaf() { delete[]children_; }
	BPlusTreeNotLeaf(const BPlusTreeNotLeaf &) = delete;
	BPlusTreeNotLeaf & operator=(const BPlusTreeNotLeaf &) = delete;

	std::vector<fpos_t> find(const T &key, int limit = 1) {
		int low = 0, high = this->count_ - 1, mid = (low + high) / 2; // 标号含low含high [low, high]
		while (high >= low) {
			mid = (low + high) / 2;
			if (this->keys_[mid] == key)
				return children_[mid + 1]->find(key, limit);
			else if (key < this->keys_[mid])
				high = mid - 1;
			else
				low = mid + 1;
		}
		// 最后一次循环一定是 high==low 或者 high==low+1,  退出循环high==low-1, key在(keys_[high],keys_[low])里, keys_[-1]视为负无穷, keys_[conut_]视为正无穷
		return children_[low]->find(key, limit);
	}

	void INSERT(const T &key, const fpos_t &fpos, T &newkey, BPlusTreeNode<T> *&newp) {
		int low = 0, high = this->count_ - 1, mid = (low + high) / 2, k; // 标号含low含high [low, high]
		while (high >= low) {
			mid = (low + high) / 2;
			if (this->keys_[mid] == key)
				return children_[mid + 1]->INSERT(key, fpos, newkey, newp);
			else if (key < this->keys_[mid])
				high = mid - 1;
			else
				low = mid + 1;
		}
		// 最后一次循环一定是 high==low 或者 high==low+1,  退出循环high==low-1, key在(keys_[high],keys_[low])里, keys_[-1]视为负无穷, keys_[conut_]视为正无穷
		children_[low]->INSERT(key, fpos, newkey, newp);

		if (newp) {
			for (k = this->count_ - 1; k >= low && k >= 0; --k) {
				this->keys_[k + 1] = this->keys_[k];
				children_[k + 2] = children_[k + 1];
			}
			this->keys_[low] = newkey;
			children_[low + 1] = newp;
			++this->count_;

			// 记得调最开始情况的兼容!!!
			if (this->count_ > this->size_) {
				newp = new BPlusTreeNotLeaf<T>(this->size_);

				int newc = this->count_ / 2;
				newp->count_ = this->count_ - newc - 1;
				this->count_ = newc;
				memcpy(newp->keys_, this->keys_ + newc + 1, sizeof(T) * newp->count_);
				memcpy(newp->getChildren(), this->children_ + newc + 1, sizeof(BPlusTreeNode<T> *) * (newp->count_ + 1));

				newkey = this->keys_[newc]; // 可重复b+树 这里要调整
			}
			else {
				newkey = -1;
				newp = nullptr;
			}
		}
	}

	void show() {
		std::cout << "size:" << this->size_ << " count:" << this->count_ << "\n";
		int k;
		for (k = 0; k < this->count_; ++k)
			printf("%9d", this->keys_[k]);
		std::cout << "\n";
		for (k = 0; k < this->count_ + 1; ++k)
			this->children_[k]->show();
		std::cout << "\n\n";
	}

	BPlusTreeNode<T> ** getChildren() { return children_; }
	//private:
	BPlusTreeNode<T> **children_; // 指向子节点
};

template <typename T>
class BPlusTree : public Index {
public:
	BPlusTree() = delete;
	BPlusTree(int n) {
		assert(n >= 3);
		root_ = new BPlusTreeLeaf<T>(n);
		/*root_ = new BPlusTreeNotLeaf<T>(n);
		root_.rootBool_ = true;
		root_.children_[0] = new BPlusTreeLeaf<T>(n);
		root_.children_[1] = new BPlusTreeLeaf<T>(n);*/
	}
	~BPlusTree() { ; } //!!
	BPlusTree(const BPlusTree &) = delete;
	BPlusTree & operator=(const BPlusTree &) = delete;


	std::vector<fpos_t> eq(void *a) {
		T b = *(T*)a;

		return root_->find(b);
	}

	std::vector<fpos_t> ge(T a, int limit = INT_MAX) {

		return root_->find(a, limit);
	}

	void INSERT(const T &key, const fpos_t &fpos) {
		T newkey = -1;
		BPlusTreeNode<T> *newp = nullptr;
		root_->INSERT(key, fpos, newkey, newp);
		if (newp) {
			BPlusTreeNotLeaf<T> *tempRoot = new BPlusTreeNotLeaf<T>(root_->size_);
			tempRoot->count_ = 1;
			tempRoot->keys_[0] = newkey;
			tempRoot->children_[0] = root_;
			tempRoot->children_[1] = newp;

			root_ = tempRoot;
		}
	}

	void show() {
		root_->show();
	}

	//private:
	BPlusTreeNode<T> *root_;
};