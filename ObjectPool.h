#pragma once
#include<iostream>
#include<vector>
#include<string>
using namespace std;

template<class T>
class ObjectPool    //对象池
{
	struct BlockNode
	{
		void* _memory;    //内存块
		BlockNode* _next;   //指向下一个结点的指针。
		size_t _objNum;   //内存块对象的个数。

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
	T* New()   //开辟内存
	{
		if (_lastDelete)   //优先使用还回来的内存对象
		{
			T* obj = _lastDelete;
			_lastDelete = *((T**)_lastDelete);   //取出_lastDelete所指对象的内容，让lastDelete指向下一块还回来的内存对象
			return new(obj)T();//定位new表达式把此对象内存初始化后返回
		}
		if (_useIn >= _last->_objNum)   //需要开辟新的内存块
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
			*(T**)ptr = _lastDelete;  //把lastDelete的值放进释放的内存单元
			_lastDelete = ptr;    //再让lastDelete指向释放的内存对象。
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
	static size_t _itemSize;    //每个对象的大小，传进来的对象可能比一个指针小，这时需要给它一个指针的大小。
	size_t _maxNum;   //内存块的最大个数
	size_t _useIn;    //当前使用到第几个对象了
	T* _lastDelete;  //指向还回来的内存对象。
	BlockNode* _first; //链表的头结点
	BlockNode* _last;   //链表的尾结点
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

// 针对当前的内存对象池进行简单的性能测试
void TestObjectPoolOP()
{
	size_t begin, end;
	vector<string*> v;
	const size_t N = 1000000;
	v.reserve(N);
	int t;
	cout << "pool new/delete===============================" << endl;
	// 反复申请释放5次
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
