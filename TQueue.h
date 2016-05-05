// ------------------------------------------------
// CTQueue Class By Craig Vella
// Templated Queues Using Data Structures
// Or any Other type of DataType
// 6 - 22 - 06
// ------------------------------------------------

#ifndef __CTQUEUE__
#define __CTQUEUE__

#ifndef NULL
	#define NULL 0
#endif

template <class DT> class CTQueue;

template <class DT>
class STACK_DATA
{
	friend class CTQueue<DT>;
private:
	DT data;
	STACK_DATA * next;
	STACK_DATA * prev;
};

template <class DT>
class CTQueue  
{
private:
	STACK_DATA<DT> * pTop;
	STACK_DATA<DT> * pTail;
	unsigned int iItemsInStack;
	unsigned int iMaxStackItems;
	bool bStackOverflowProtection;
public:
	CTQueue(bool bUseOverFlowProtection = false, int iMaxStackItems = 1024);
	virtual ~CTQueue(void) {FlushStack();};

	void SetOptions(bool bUseOverFlowProtection, int iMaxStackItems);
	void PushData(DT data);
	void SneekData(DT data);
	bool PopHeadData(DT& dataReturn);
	bool PeekHeadData(DT& dataReturn);
	bool DestroyHeadData(void);
	bool PopTailData(DT& dataReturn);
	bool PeekTailData(DT& dataReturn);
	bool DestroyTailData(void);
	void FlushStack(void);
	bool DataInQueue(void);
	unsigned int ItemsInStack(void){return iItemsInStack;};
};

template <class DT>
CTQueue<DT>::CTQueue(bool bUseOverFlowProtection, int iMaxStackItems) 
{
	pTop = NULL; 
	pTail = NULL;
	iItemsInStack = 0; 
	this->iMaxStackItems = iMaxStackItems;
	bStackOverflowProtection = bUseOverFlowProtection;
}

template <class DT>
bool CTQueue<DT>::DataInQueue(void)
{
	//if ((iItemsInStack > (iMaxStackItems - 1)) && bStackOverflowProtection)
		//FlushStack();
	return (pTop ? 1 : 0) && (pTail ? 1 : 0) ? 1 : 0;
}

template <class DT>
void CTQueue<DT>::FlushStack(void)
{
	STACK_DATA<DT> * sdHolder = NULL;
	while (DataInQueue())
	{
		sdHolder = pTop;
		pTop = pTop->next;
		if (sdHolder != NULL)
		{
			delete sdHolder;
			sdHolder = NULL;
		}
	}
	pTop = NULL;
	pTail = NULL;
}

template <class DT>
void CTQueue<DT>::PushData(DT data)
{
	STACK_DATA<DT> * newNode = new STACK_DATA<DT>;
	newNode->data = data;
	if (!DataInQueue())
	{
		newNode->next = newNode->prev = NULL;
		pTop = pTail = newNode;
		return;
	}
	newNode->next = pTop;
	newNode->prev = NULL;
	pTop->prev = newNode;
	pTop = newNode;
	if (bStackOverflowProtection)
		++iItemsInStack;
}

template <class DT>
void CTQueue<DT>::SneekData(DT data)
{
	if (!DataInQueue())
	{
		PushData(data);
		return;
	}
	STACK_DATA<DT> * newNode = new STACK_DATA<DT>;
	newNode->data = data;
	newNode->next = NULL;
	newNode->prev = pTail;
	pTail->next = newNode;
	pTail = newNode;
	if (bStackOverflowProtection)
		++iItemsInStack;
}

template <class DT>
bool CTQueue<DT>::PopHeadData(DT& dataReturn)
{
	if (!DataInQueue())
		return false;

	PeekHeadData(dataReturn);
	DestroyHeadData();

	return true;
}

template <class DT>
bool CTQueue<DT>::DestroyHeadData(void)
{
	if (!DataInQueue())
		return false;
	STACK_DATA<DT> * sdHolder = NULL;
	sdHolder = pTop;
	pTop = pTop->next;
	delete sdHolder;
	if (pTop != NULL)
		pTop->prev = NULL;
	if (bStackOverflowProtection)
		--iItemsInStack;
	return true;
}

template <class DT>
bool CTQueue<DT>::DestroyTailData(void)
{
	if (!DataInQueue())
		return false;
	STACK_DATA<DT> * sdHolder = NULL;
	sdHolder = pTail;
	pTail = pTail->prev;
	delete sdHolder;
	if (pTail != NULL)
		pTail->next = NULL;
	if (bStackOverflowProtection)
		--iItemsInStack;
	return true;
}

template <class DT>
bool CTQueue<DT>::PeekHeadData(DT& dataReturn)
{
	if (!DataInQueue())
		return false;
	dataReturn = pTop->data;
	return true;
}

template <class DT>
bool CTQueue<DT>::PopTailData(DT& dataReturn)
{
	if (!DataInQueue())
		return false;

	PeekTailData(dataReturn);
	DestroyTailData();
	return true;
}

template <class DT>
bool CTQueue<DT>::PeekTailData(DT& dataReturn)
{
	if (!DataInQueue())
		return false;
	dataReturn = pTail->data;
	return true;
}

template <class DT>
void CTQueue<DT>::SetOptions(bool bUseOverFlowProtection, int iMaxStackItems)
{
	this->iMaxStackItems = iMaxStackItems;
	bStackOverflowProtection = bUseOverFlowProtection;
	FlushStack();
}

#endif