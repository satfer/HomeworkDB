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

template <typename hashType> // ���hash���type
class Block { // ����չɢ�б� ����ɢ�б� ͨ��
// Ҫô����ǰ��, Ҫô������class Block; vs�ı���������Block�ĵط� ȱ������˵���� - �ٶ�Ϊ int��ע��: C++ ��֧��Ĭ�� int
public:
	Block() = delete;
	Block(int size) : size_(size), count_(0), j_(1) { positions_ = new fpos_t[size]; keys_ = new hashType[size]; }
	Block(const Block &) = delete;
	Block & operator=(const Block &) = delete;
	~Block() { delete[] positions_; delete[] keys_; }

	bool fullBool() { return count_ == size_; }
	void INSERT(hashType key, fpos_t fpos) {
		// ���fullBool()ʱ���� INSERT ���ڴ�й©, �������֤.
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
	int size_; // ��Ĵ�С
	int count_;
	int j_;
	hashType *keys_; // ����ԭkey, Ͱ���鷭����ʱ�򷽱�
	fpos_t *positions_; // �п�ʼ��λ��
};

template <typename T, typename hashType> // ǰһ���Ǽ�������, ��һ���Ǽ�hash�������
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
		//������ȥdelete�Ѿ�delete�Ķ���, Ȼ�󱨴�
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
		hkey >>= sizeof(hashType) * 8 - i_; // hkey >> (32 - i_); ��Ϥ�����ȼ�
		if (bucket_[hkey]->fullBool()) {
			if (bucket_[hkey]->getJ() < i_) { // �����
				Block<hashType>* tempBlock[3]; // 1 2 �ǲ�ɵ��¿�, 2�ݴ�bucket_[hkey], ���°�2ָ��Ŀ�ɾ��
				int preJ = bucket_[hkey]->getJ();
				tempBlock[0] = new Block<hashType>(blockSize_);
				tempBlock[1] = new Block<hashType>(blockSize_);
				tempBlock[0]->setJ(preJ + 1);
				tempBlock[1]->setJ(preJ + 1);
				for (int k = 0; k < blockSize_; ++k) {
					// bucket_[hkey]�ĵ�k��, ��key��j+1λΪ0�ķŽ�tempBlock[0], Ϊ1�ķŽ�tempBlock[1]
					// unsigned short����>> ��<<���Զ����int ??? ����... ��>>=����?
					tempBlock[(bucket_[hkey]->getKeys()[k] << preJ >> sizeof(hashType) * 8  - 1) & 1]->INSERT(bucket_[hkey]->getKeys()[k], bucket_[hkey]->getPositions()[k]); // ��Ϥ�����ȼ�
				}
				// ��Ӱ��Ĳ���ֹ��������, ������������
				// delete bucket_[hkey]; // �ͷ�bucket_[hkey >> 1 << 1] �� bucket_[hkey >> 1 << 1 & 1] ָ���ͬһ����
				//bucket_[hkey >> 1 << 1] = tempBlock[0];
				//bucket_[(hkey >> 1 << 1)+1] = tempBlock[1];
				tempBlock[2] = bucket_[hkey];
				int from = bucket_[hkey]->getKeys()[0] >> sizeof(hashType) * 8 - preJ << i_ - preJ; //bucket_[hkey]������,����bucket_[hkey]->getKeys()[0]û����
				int to = (bucket_[hkey]->getKeys()[0] >> sizeof(hashType) * 8 - preJ) + 1 << i_ - preJ;
				for (int k = from; k < to; ++k) { // ��Ӱ��Ŀ���ǰpreJλ��ͬ��iλ����ָ��ԭbucket_[hkey]��
					if (bucket_[k] == tempBlock[2])
						bucket_[k] = tempBlock[(k << preJ >> i_ - 1) & 1]; // k��i_λ��(������), �����preJ+1λ
				}
				delete tempBlock[2]; //bucket_[hkey]�ᱻ�޸�, �ʼָ��Ŀ�ᱻɾ��, ��tempBlock[2]��һ��
			}
			else { // bucket_[hkey]->j_ == i_ Ͱ��������
				++i_;
				size_ *= 2;
				Block<hashType>** tempBucket = new Block<hashType>*[size_];
				for (int k = 0; k < size_; ++k) {
					tempBucket[k] = bucket_[k >> 1];
				}
				delete[] bucket_;
				bucket_ = tempBucket;
				// ������INSERT�ݹ��if-if �����
			}
			// ��Ϊ����Ҫ���Ͱ��������, ˳���õݹ����ѭ��
			INSERT(key, fpos);
		}
		else { // �ݹ����
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
	Block<hashType> **bucket_; // Ͱ: ���ָ�������
	hashType (*hash_)(int);
};

// trivalHash int ���ڴ�
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
//		hkey = (i_ == 0 ? 0 : hkey >> 32 - i_); // hkey >> (32 - i_); ��Ϥ�����ȼ�
//		if (bucket_[hkey]->fullBool()) {
//			if (bucket_[hkey]->getJ() < i_) { // �����
//				Block* tempBlock[2];
//				tempBlock[0] = new Block(blockSize_);
//				tempBlock[1] = new Block(blockSize_);
//				tempBlock[0]->setJ(bucket_[hkey]->getJ() + 1);
//				tempBlock[1]->setJ(bucket_[hkey]->getJ() + 1);
//				for (int k = 0; k < blockSize_; ++k) {
//					tempBlock[bucket_[hkey]->getKeys()[k] >> 32 - bucket_[hkey]->getJ() - 1 & 1]->INSERT(key, fpos); // ��Ϥ�����ȼ�
//				}
//				delete bucket_[hkey]; // �ͷ�bucket_[hkey >> 1 << 1] �� bucket_[hkey >> 1 << 1 & 1] ָ���ͬһ����
//				// tempBlock[2] = bucket_[hkey];
//				bucket_[hkey >> 1 << 1] = tempBlock[0];
//				bucket_[hkey >> 1 << 1 | 1] = tempBlock[1];
//				// delete tempBlock[2]; //Ҫ��ôдҪдoperator=��, ��������ָ�� Ӧ�ò���
//			}
//			else { // bucket_[hkey]->j_ == i_ Ͱ��������
//				++i_;
//				size_ *= 2;
//				Block** tempBucket = new Block*[size_];
//				for (int k = 0; k < size_; ++k) {
//					tempBucket[k] = bucket_[k >> 1];
//				}
//				delete[] bucket_;
//				bucket_ = tempBucket;
//				// ������INSERT�ݹ��if-if �����
//			}
//			// ��Ϊ����Ҫ���Ͱ��������, ˳���õݹ����ѭ��
//			INSERT(key, fpos);
//		}
//		else { // �ݹ����
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
//	Block **bucket_; // Ͱ: ���ָ�������
//	int(*hash_)(int);
//};