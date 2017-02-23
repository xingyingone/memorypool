#pragma once
#include<iostream>
#include<vector>
#include<string>
using namespace std;

template<class T>
class ObjectPool    //�����
{
	struct BlockNode
	{
		void* _memory;    //�ڴ��
		BlockNode* _next;   //ָ����һ������ָ�롣
		size_t _objNum;   //�ڴ�����ĸ�����

		BlockNode(size_t objNum)
			:_next(NULL)
			, _objNum(objNum)
		{
			_memory = malloc(objNum*_itemSize);
		}
	};
public:
	ObjectPool(size_t initNum = 32, size_t maxNum = 100000)
		:_maxNum(maxNum)
		, _useIn(0)
		, _lastDelete(NULL)
	{
		_first = _last = new BlockNode(initNum);
	}
	~ObjectPool()
	{
		_Destory();
		_lastDelete = NULL;
		_first = NULL;
		_last = NULL;
		_maxNum = 0;
		_useIn = 0;
	}
	T* New()   //�����ڴ�
	{
		if (_lastDelete)   //����ʹ�û��������ڴ����
		{
			T* obj = _lastDelete;
			_lastDelete = *((T**)_lastDelete);   //ȡ��_lastDelete��ָ��������ݣ���lastDeleteָ����һ�黹�������ڴ����
			return new(obj)T();//��λnew���ʽ�Ѵ˶����ڴ��ʼ���󷵻�
		}
		if (_useIn >= _last->_objNum)   //��Ҫ�����µ��ڴ��
		{
			size_t objNum = GetobjNum(_last->_objNum);
			_last->_next = new BlockNode(objNum);
			_last = _last->_next;
			_useIn = 0;
		}
		T* obj1 = new((T*)((char*)_last->_memory + _useIn*_itemSize))T();
		_useIn++;
		return  obj1;
	}
	static size_t InitItemSize()
	{
		if (sizeof(T) < sizeof(void*))
		{
			return sizeof(void*);
		}
		else
		{
			return sizeof(T);
		}
	}

	void Delete(T* ptr)
	{
		if (ptr)
		{
			*(T**)ptr = _lastDelete;  //��lastDelete��ֵ�Ž��ͷŵ��ڴ浥Ԫ
			_lastDelete = ptr;    //����lastDeleteָ���ͷŵ��ڴ����
		}

	}
protected:
	size_t GetobjNum(size_t oldNum)
	{
		size_t objNum = 2 * oldNum;
		if (objNum > _maxNum)
		{
			return _maxNum;
		}
		else
		{
			return objNum;
		}
	}
	void _Destory()
	{
		BlockNode* cur = _first;
		while (cur)
		{
			BlockNode* del = cur;
			cur = cur->_next;
			free(del->_memory);
			del->_objNum = 0;
			delete del;
		}
	}
protected:
	static size_t _itemSize;    //ÿ������Ĵ�С���������Ķ�����ܱ�һ��ָ��С����ʱ��Ҫ����һ��ָ��Ĵ�С��
	size_t _maxNum;   //�ڴ���������
	size_t _useIn;    //��ǰʹ�õ��ڼ���������
	T* _lastDelete;  //ָ�򻹻������ڴ����
	BlockNode* _first; //�����ͷ���
	BlockNode* _last;   //�����β���
};

template<class T>
size_t ObjectPool<T>::_itemSize = ObjectPool<T>::InitItemSize();

void TestObjectPool()
{
	vector<string*> v;

	ObjectPool<string> pool;
	for (size_t i = 0; i < 32; ++i)
	{
		v.push_back(pool.New());
		printf("Pool New [%d]: %p\n", i, v.back());
	}

	while (!v.empty())
	{
		pool.Delete(v.back());
		v.pop_back();
	}

	for (size_t i = 0; i < 32; ++i)
	{
		v.push_back(pool.New());
		printf("Pool New [%d]: %p\n", i, v.back());
	}

	v.push_back(pool.New());
}

#include <Windows.h>

// ��Ե�ǰ���ڴ����ؽ��м򵥵����ܲ���
void TestObjectPoolOP()
{
	size_t begin, end;
	vector<string*> v;
	const size_t N = 1000000;
	v.reserve(N);
	int t;
	cout << "pool new/delete===============================" << endl;
	// ���������ͷ�5��
	begin = GetTickCount();
	ObjectPool<string> pool;
	for (t = 0; t < 5; ++t)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v.push_back(pool.New());
		}

		while (!v.empty())
		{
			pool.Delete(v.back());
			v.pop_back();
		}
	}


	end = GetTickCount();
	cout << "Pool:" << end - begin << endl;

	cout << "new/delete===============================" << endl;
	begin = GetTickCount();

	for (t = 0; t < 5; ++t)
	{
		for (size_t i = 0; i < N; ++i)
		{
			v.push_back(new string);
		}

		while (!v.empty())
		{
			delete v.back();
			v.pop_back();
		}
	}
	
	end = GetTickCount();
	cout << "new/delete:" << end - begin << endl;
}
