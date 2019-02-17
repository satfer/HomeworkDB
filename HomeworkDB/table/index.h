#pragma once

#include <string>
#include <vector>
#include <set>
#include <cassert>

#define MainIndex SortedIndex



// ��������ֻ��Ϊ�˷�����ָ��, ����vector<Index *>

// ������������, ֻ��eq
class Index {
public:
	virtual fpos_t eq(void *) = 0; // equal // �����е���ʼλ��
	virtual void show() = 0;
};

// �������Ķ������������
class SortedIndex : virtual public Index{
public:
	virtual fpos_t eq(void *) = 0; // equal
	virtual fpos_t ge(void *, int n = 1) = 0; // greater equal >=��ǰn��
	virtual fpos_t lt(void *) = 0; // less than <�ĵ�һ��
	virtual void show() = 0;
};

template <typename T>
class NumberIndex : public Index {
public:
	NumberIndex() = delete;
	NumberIndex(const std::string &keyAttr, const std::string &file) : keyAttr_(keyAttr) {
		FILE *fpTemp;
		if (typeid(T) == typeid(std::string))
			throw("Not support. ��֧�ְ��ַ�����������Ϊ����.index�ļ�");
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
	} // ��.index�ļ���
	// NumberIndex(const std::string &keyAttr, FILE *fpDB)
	~NumberIndex() = default;

	fpos_t eq(void *a) {
		// ���д�ɶ��ַ�, NumberIndex���Դ�����˳������
		// ��ôд, ����֮��дɾ���� �����׳����� (³����̫��
		assert(typeid(T) == typeid(int));
		return positions_[*(T *)a]; // *reinpreter_cast<T*>a ���벻��...???
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

template <typename T, typename hashType> // ���hash���type, ������int��, ������ֻ����24λ
class Block { // ����չɢ�б� �� ��
// Ҫô����ǰ��, Ҫô������class Block; vs�ı���������Block�ĵط� ȱ������˵���� - �ٶ�Ϊ int��ע��: C++ ��֧��Ĭ�� int
public:
	Block() = delete;
	Block(int size) : size_(size), count_(0), j_(1) { positions_ = new fpos_t[size]; keys_ = new hashType[size]; rawKeys_ = new T[size]; }
	Block(const Block &) = delete;
	Block & operator=(const Block &) = delete;
	~Block() { delete[] positions_; delete[] keys_; delete rawKeys_; }

	bool fullBool() { return count_ == size_; }
	void INSERT(T rawKey, hashType key, fpos_t fpos) { // ע��ͬ����(T, hashType, fpos_t), index��INSERT��(T key, hashType hkey, fpos_t fpos), ����Block����(T rawKey, hashType key, fpos_t fpos), ���ͺ�˳��һ��, �β�����һ��, ���е����׻���, �����ø���,
		// ���fullBool()ʱ���� INSERT ���ڴ�й©, �������֤.
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
	int size_; // ��Ĵ�С
	int count_;
	int j_;
	T *rawKeys_;
	hashType *keys_; // ����ԭkey, Ͱ���鷭����ʱ�򷽱�
	fpos_t *positions_; // �п�ʼ��λ��
};

template <typename T, typename hashType> // ǰһ���Ǽ�������, ��һ���Ǽ�hash��Ĵ�������, useBit_�����õ�λ
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
		//������ȥdelete�Ѿ�delete�Ķ���, Ȼ�󱨴�
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
		int ikey = hkey >> useBit_ - i_; // hkey >> (32 - i_); ��Ϥ�����ȼ�
		if (bucket_[ikey]->fullBool()) {
			if (bucket_[ikey]->getJ() < i_) { // �����
				Block<T, hashType>* tempBlock[3]; // 1 2 �ǲ�ɵ��¿�, 2�ݴ�bucket_[ikey], ���°�2ָ��Ŀ�ɾ��
				int preJ = bucket_[ikey]->getJ();
				tempBlock[0] = new Block<T, hashType>(blockSize_);
				tempBlock[1] = new Block<T, hashType>(blockSize_);
				tempBlock[0]->setJ(preJ + 1);
				tempBlock[1]->setJ(preJ + 1);
				for (int k = 0; k < blockSize_; ++k) {
					// bucket_[ikey]�ĵ�k��, ��key��j+1λΪ0�ķŽ�tempBlock[0], Ϊ1�ķŽ�tempBlock[1]
					// unsigned short����>> ��<<���Զ����int ??? ����... ��>>=����?
					tempBlock[(bucket_[ikey]->getKeys()[k] << preJ >> useBit_  - 1) & 1]->INSERT(bucket_[ikey]->getRawKeys()[k], bucket_[ikey]->getKeys()[k], bucket_[ikey]->getPositions()[k]); // ��Ϥ�����ȼ�
				}
				// ��Ӱ��Ĳ���ֹ��������, ������������
				// delete bucket_[ikey]; // �ͷ�bucket_[ikey >> 1 << 1] �� bucket_[ikey >> 1 << 1 & 1] ָ���ͬһ����
				//bucket_[ikey >> 1 << 1] = tempBlock[0];
				//bucket_[(ikey >> 1 << 1)+1] = tempBlock[1];
				tempBlock[2] = bucket_[ikey];
				int from = bucket_[ikey]->getKeys()[0] >> useBit_ - preJ << i_ - preJ; //bucket_[ikey]������,����bucket_[hkey]->getKeys()[0]û����
				int to = (bucket_[ikey]->getKeys()[0] >> useBit_ - preJ) + 1 << i_ - preJ;
				for (int k = from; k < to; ++k) { // ��Ӱ��Ŀ���ǰpreJλ��ͬ��iλ����ָ��ԭbucket_[ikey]��
					if (bucket_[k] == tempBlock[2])
						bucket_[k] = tempBlock[(k << preJ >> i_ - 1) & 1]; // k��i_λ��(������), �����preJ+1λ
				}
				delete tempBlock[2]; //bucket_[ikey]�ᱻ�޸�, �ʼָ��Ŀ�ᱻɾ��, ��tempBlock[2]��һ��
			}
			else { // bucket_[ikey]->j_ == i_ Ͱ��������
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
				// ������INSERT�ݹ��if-if �����
			}
			// ��Ϊ����Ҫ���Ͱ��������, ˳���õݹ����ѭ��
			INSERT(key, hkey, fpos);
		}
		else { // �ݹ����
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
	int useBit_; // ��int�浽Block��, ������ֻ��24λ, ��hash_���������׵�
	Block<T, hashType> **bucket_; // Ͱ: ���ָ�������
	hashType (*hash_)(T);
};



template <typename T, typename hashType> // ���hash���type, ������int��, ������ֻ����24λ
class Block2 { // ����ɢ�б� �� ��
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
	int size_; // ��Ĵ�С
	int count_;
	T *rawKeys_;
	hashType *keys_; // ����ԭkey, Ͱ���鷭����ʱ�򷽱�
	fpos_t *positions_; // �п�ʼ��λ��

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
			if (n_ > low_[i_] + 1) { // n_>2**i_ ע���ʱ��i_λ���ѷ�����ȷ, ����i_����i_-1λ����ȷ
				++i_;
				if (i_ > sizeof(hashType) * 8)
					throw("i_ overflow.");
			}
			bucket_.push_back(new Block2<T, hashType>(blockSize_));
			// �µĿ�ı����n_-1, ��i_λ��, ��һλ��1, Ҫ�ѱ�ŵ�һλ1��0��Ͱ ����
			Block2<T, hashType> *pBlock = bucket_[(n_ - 1) & low_[i_ - 1]]; // ָ��ԭ��, ԭ��֮���ɾ��
			Block2<T, hashType> *tempBlock = new Block2<T, hashType>(blockSize_); // ���0��ͷ���¿�, д��������ԭͰԭ�ط��Ѽ�
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
		0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF }; // ����Ŀǰ���index���֧��32λ��hash���ֵ
};




