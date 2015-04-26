/*
** 库名		:	ListX
** 库描述	:	用于链表list操作的函数库
** 作者		:	calvin
** E-mail	:	
** QQ		:	
** 创建日期时间	:	2003/10/18
** 更新日期时间	:	2005/5/2
*/

#include "ListX.h"

/*
** 函数名		:	CreateList
** 函数描述		:	创建一个泛型的链表list
** 输入参数说明	:	无
** 返回值		:	返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SList *CreateList()
{
	return NULL;
}

/*
** 函数名		:	DestroyList
** 函数描述		:	销毁一个泛型的链表list
** 输入参数说明	:	SList **list				链表的首地址
**					void (* DeleteNodeMember)()	删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL DestroyList( SList **list , BOOL (* DeleteNodeMember)( void *pv ) )
{
	if( (*list) == NULL )
		return TRUE;
	
	if( DeleteAllListNode( list , DeleteNodeMember ) == FALSE )
		return FALSE;

	return TRUE;
}

/*
** 函数名		:	AddListNode
** 函数描述		:	添加一个结点到链表的末尾
** 输入参数说明	:	SList **list							链表的首地址
**					void *member							单个结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *AddListNode( SList **list , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *node,*nodeNew;

	if( *list == NULL )
	{
		nodeNew=(SListNode *)malloc( sizeof(SListNode) );
		if( nodeNew == NULL )
			return NULL;

		nodeNew->member			= member ;
		nodeNew->msize			= msize ;
		nodeNew->FreeNodeMember	= FreeNodeMember ;

		nodeNew->prev = NULL ;
		nodeNew->next = NULL ;

		*list = nodeNew ;

		return nodeNew;
	}
	else
	{
		node = *list ;
		while( node->next != NULL )
			node = node->next ;

		nodeNew=(SListNode *)malloc( sizeof( SListNode ) );
		if( nodeNew == NULL )
			return NULL;

		nodeNew->member			= member ;
		nodeNew->msize			= msize ;
		nodeNew->FreeNodeMember	= FreeNodeMember ;

		nodeNew->prev = node;
		nodeNew->next = NULL;

		node->next = nodeNew;

		return nodeNew;
	}
}

/*
** 函数名		:	InsertListNodeBefore
** 函数描述		:	插入一个结点到当前链表结点前
** 输入参数说明	:	SList **list							链表首指针
**					SList **nodeList						当前链表结点
**					void *member							单个结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *InsertListNodeBefore( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *nodeNew=NULL;
	
	if( (*list) == NULL )
	{
		nodeNew = AddListNode( list , member , msize , FreeNodeMember ) ;
		if( nodeNew == NULL )
			return NULL;
		
		(*nodeList) = (*list) ;
		
		return nodeNew;
	}
	else
	{
		if( (*nodeList) == NULL )
			(*nodeList) = (*list) ;
	}
	
	nodeNew=(SListNode *)malloc( sizeof( SListNode ) );
	if( nodeNew == NULL )
		return NULL;

	nodeNew->member			= member ;
	nodeNew->msize			= msize ;
	nodeNew->FreeNodeMember	= FreeNodeMember ;
	
	if( (*nodeList)->prev == NULL )
	{
		nodeNew->prev = NULL ;
		
		nodeNew->next = (*nodeList) ;
		(*nodeList)->prev = nodeNew ;
		
		(*list) = nodeNew ;
		(*nodeList) = nodeNew ;
	}
	else
	{
		(*nodeList)->prev->next = nodeNew ;
		nodeNew->prev = (*nodeList)->prev ;
		
		nodeNew->next = (*nodeList) ;
		(*nodeList)->prev = nodeNew ;
		
		(*nodeList) = nodeNew ;
	}
	
	return nodeNew;
}

/*
** 函数名		:	InsertListNodeAfter
** 函数描述		:	插入一个结点到当前链表结点后
** 输入参数说明	:	SList **list							链表首指针
**					SList **nodeList						当前链表结点
**					void *member							单个结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *InsertListNodeAfter( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *nodeNew=NULL;
	
	if( (*list) == NULL )
	{
		nodeNew = AddListNode( list , member , msize , FreeNodeMember ) ;
		if( nodeNew == NULL )
			return NULL;
		
		(*nodeList) = (*list) ;
		
		return nodeNew;
	}
	else
	{
		if( (*nodeList) == NULL )
			(*nodeList) = (*list) ;
	}
	
	nodeNew=(SListNode *)malloc( sizeof( SListNode ) );
	if( nodeNew == NULL )
		return NULL;

	nodeNew->member			= member ;
	nodeNew->msize			= msize ;
	nodeNew->FreeNodeMember	= FreeNodeMember ;

	if( (*nodeList)->next == NULL )
	{
		nodeNew->next = NULL ;
	}
	else
	{
		(*nodeList)->next->prev = nodeNew ;
		nodeNew->next = (*nodeList)->next ;
	}
	
	nodeNew->prev = (*nodeList) ;
	(*nodeList)->next = nodeNew ;

	(*nodeList) = nodeNew ;

	return nodeNew;
}

/*
** 函数名		:	InsertListIndexNode
** 函数描述		:	插入一个结点到链表的第index个结点前
** 输入参数说明	:	SList **list							链表的首地址
**					int index								链表的第index个结点
**					void *member							单个结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *InsertListIndexNode( SList **list , int index , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *node=NULL;

	if( ( *list ) == NULL )
		return AddListNode( list , member , msize , FreeNodeMember );

	node = GetListIndexNode( ( *list ) , index ) ;

	if( node == NULL )
		return NULL;

	if( InsertListNode( list , &node , member , msize , FreeNodeMember ) == NULL )
		return NULL;

	if( index == 1 )
		( *list ) = ( *list )->prev;

	return node;
}

/*
** 函数名		:	DeleteListNode
** 函数描述		:	删除当前结点
** 输入参数说明	:	SListHead **list						链表首结点
**					SListNode **node						链表当前结点
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
**					2004/3/3	修改 当链表当前结点是首结点时，会删除第一个结点，丢失第二个结点的BUG
*/

BOOL DeleteListNode( SList **list , SListNode **node , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *nodeDelete = NULL ;
	
	if( (*list) == NULL )
		return FALSE;
	
	if( (*node) == NULL )
		return FALSE;
	
	if( (*node)->prev == NULL )
	{
		if( (*node)->next == NULL ) /* 只有一个结点 */
		{
			nodeDelete = (*node) ;
			
			if( FreeNodeMember != NULL )
			{
				FreeNodeMember( nodeDelete->member );
			}
			else if( nodeDelete->FreeNodeMember != NULL )
			{
				nodeDelete->FreeNodeMember( nodeDelete->member );
			}
			
			free( nodeDelete );
			(*list) = NULL ;
			(*node) = NULL ;
		}
		else /* if( (*nodeList)->next != NULL ) */ /* 第一个结点 */
		{
			nodeDelete = (*node) ;
			
			if( (*list) == (*node) )
			{
				(*node) = (*node)->next ;
				(*node)->prev = NULL ;
				
				if( FreeNodeMember != NULL )
				{
					FreeNodeMember( nodeDelete->member );
				}
				else if( nodeDelete->FreeNodeMember != NULL )
				{
					nodeDelete->FreeNodeMember( nodeDelete->member );
				}
				
				free( nodeDelete );
				(*list) = (*node) ;
			}
			else
			{
				(*node) = (*node)->next ;
				(*node)->prev = NULL ;
				
				if( FreeNodeMember != NULL )
					FreeNodeMember( nodeDelete->member );
				else if( nodeDelete->FreeNodeMember != NULL )
					nodeDelete->FreeNodeMember( nodeDelete->member );
				
				free( nodeDelete );
			}
		}
	}
	else /* if( (*nodeList)->prev != NULL ) */
	{
		if( (*node)->next == NULL ) /* 最后一个结点 */
		{
			nodeDelete = (*node) ;
	
			(*node) = (*node)->prev ;
			(*node)->next = NULL ;
			
			if( FreeNodeMember != NULL )
				FreeNodeMember( nodeDelete->member );
			else if( nodeDelete->FreeNodeMember != NULL )
				nodeDelete->FreeNodeMember( nodeDelete->member );
			
			free( nodeDelete );
		}
		else /* if( (*nodeList)->next != NULL ) */ /* 中间的结点 */
		{
			nodeDelete = (*node) ;
			
			(*node)->prev->next = (*node)->next ;
			(*node)->next->prev = (*node)->prev ;
			
			(*node) = (*node)->next ;
			
			if( FreeNodeMember != NULL )
				FreeNodeMember( nodeDelete->member );
			else if( nodeDelete->FreeNodeMember != NULL )
				nodeDelete->FreeNodeMember( nodeDelete->member );
			
			free( nodeDelete );
		}
	}
	
	return TRUE;
}

/*
** 函数名		:	DeleteListIndexNode
** 函数描述		:	删除链表的第index个结点
** 输入参数说明	:	SList **list							链表首结点
**					int index								第index个结点
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL DeleteListIndexNode( SListNode **list , int index , BOOL (* FreeNodeMember)( void *pv ) )
{
	SListNode *node = NULL ;

	if( (*list) == NULL )
		return FALSE;
	
	node = GetListIndexNode( (*list) , index );
	if( node == NULL )
		return FALSE;

	return DeleteListNode( list , &node , FreeNodeMember );
}

/*
** 函数名		:	DeleteAllListNode
** 函数描述		:	删除链表所有结点
** 输入参数说明	:	SList **list							链表首结点
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL DeleteAllListNode( SList **list , BOOL (* FreeNodeMember)( void *pv ) )
{
	if( (*list) == NULL )
		return TRUE;
	
	while( DeleteListNode( list , list , FreeNodeMember ) == TRUE );
	
	(*list) = NULL ;
	
	return TRUE;
}

/*
** 函数名		:	FindFirstListNode
** 函数描述		:	定位链表的第一个结点
** 输入参数说明	:	SList *list			链表首结点
** 返回值		:	成功，返回 第一个结点地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SListNode *FindFirstListNode( SList *list )
{
	return list;
}

/*
** 函数名		:	FindLastListNode
** 函数描述		:	定位链表的第一个结点
** 输入参数说明	:	SList *list			链表首结点
** 返回值		:	成功，返回 第一个结点地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SListNode *FindLastListNode( SList *list )
{
	SListNode *node=NULL;
	
	node = FindFirstListNode( list ) ;
	while( FindNextListNode( node ) != NULL )
		node = FindNextListNode( node ) ;
	
	return node;
}

/*
** 函数名		:	FindPrevListNode
** 函数描述		:	定位链表的上一个结点
** 输入参数说明	:	SListNode *nodeList			当前链表结点
** 返回值		:	成功，返回 上一个结点地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SListNode *FindPrevListNode( SListNode *nodeList )
{
	if( nodeList == NULL )
		return NULL;
		
	if( nodeList->prev == NULL )
		return NULL;
		
	return nodeList->prev;
}

/*
** 函数名		:	FindPrevListNode
** 函数描述		:	定位链表的下一个结点
** 输入参数说明	:	SListNode *nodeList			当前链表结点
** 返回值		:	成功，返回 下一个结点地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SListNode *FindNextListNode( SListNode *nodeList )
{
	if( nodeList == NULL )
		return NULL;
		
	return nodeList->next;
}

/*
** 函数名		:	IsFirstListNode
** 函数描述		:	判断是否是链表的第一个结点
** 输入参数说明	:	SListNode *nodeList			当前链表结点
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL IsFirstListNode( SListNode *nodeList )
{
	if( nodeList->prev == NULL )
		return TRUE;
	else
		return FALSE;
}

/*
** 函数名		:	IsFirstListNode
** 函数描述		:	判断是否是链表的最后一个结点
** 输入参数说明	:	SListNode *nodeList			当前链表结点
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL IsLastListNode( SListNode *nodeList )
{
	if( nodeList->next == NULL )
		return TRUE;
	else
		return FALSE;
}

/*
** 函数名		:	IsListEmpty
** 函数描述		:	判断是否是空链表
** 输入参数说明	:	SList *list			链表首结点
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2004/2/18	创建
*/

BOOL IsListEmpty( SList *list )
{
	if( list == NULL )
		return TRUE;
	else
		return FALSE;
}

/*
** 函数名		:	IsListNodeValid
** 函数描述		:	判断当前结点是否有效
** 输入参数说明	:	SList *list			链表首结点
** 返回值		:	有效，返回 TRUE
**					无效，返回 FALSE
** 更新日志		:	2004/2/18	创建
*/

BOOL IsListNodeValid( SListNode *node )
{
	if( node == NULL )
		return FALSE;
	else
		return TRUE;
}

/*
** 函数名		:	GetListIndexNode
** 函数描述		:	得到第position个链表结点
** 输入参数说明	:	SList *list			链表首结点
**					long position		位置（从1开始）
** 返回值		:	成功，返回 结点地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

SListNode *GetListIndexNode( SList *list , long position )
{
	SListNode *node;
	long i=0;

	if(position<1)
		return NULL;

	if( position == 1 )
		return list;
	
	position--;
	
	node = list ;
	for( i=0 ; i<position ; i++ )
	{
		if( node == NULL )
			return NULL;

		node = node->next ;
	}

	return node;
}

/*
** 函数名		:	GetListIndexNode
** 函数描述		:	得到第position个链表结点成员
** 输入参数说明	:	SList *list			链表首结点
**					long position		位置（从1开始）
** 返回值		:	成功，返回 结点成员地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

void *GetListIndexMember( SList *list , long position )
{
	SListNode *node = NULL ;

	node = GetListIndexNode( list , position );

	if( node == NULL )
		return NULL;
	else
		return GetNodeMember( node );
}

/*
** 函数名		:	GetNodeMember
** 函数描述		:	得到当前结点的成员
** 输入参数说明	:	SListNode *nodeList		当前链表结点
** 返回值		:	成功，返回 结点成员地址
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

void *GetNodeMember( SListNode *nodeList )
{
	if( nodeList == NULL )
		return NULL;
	else
		return nodeList->member;
}

/*
** 函数名		:	CountListNodes
** 函数描述		:	得到敛表结点数量
** 输入参数说明	:	SList *list			链表首结点
** 返回值		:	成功，返回 结点数量
**					失败，返回 NULL
** 更新日志		:	2003/10/18	创建
*/

long CountListNodes( SList *list )
{
	int count;
	SListNode *node=NULL;
		
	if( list == NULL )
		return 0;
	
	count = 0 ;
	node = list ;
	while( node != NULL )
	{
		count++;
		
		node = node->next ;
	}
	
	return count;
}

long AccessList( SList *listHead , BOOL (* AccessListNodeProc)( void *member ) )
{
	SListNode *node=NULL;
	BOOL bReturnValue;
	long lIndex;
	
	if( listHead == NULL )
		return -1;
	
	node = listHead ;
	lIndex = 1 ;
	while( node != NULL )
	{
		bReturnValue = AccessListNodeProc( node->member ) ;
		if( bReturnValue != TRUE )
			return lIndex;
		
		node = node->next ;
		lIndex++;
	}
	
	return -1;
}

int SwapTwoListNodes( SListNode *pnode1 , SListNode *pnode2 )
{
	SListNode nodeBackup ;
	
	if( pnode1 == NULL || pnode2 == NULL )
		return -1;
	
	nodeBackup.msize		= pnode1->msize ;
	nodeBackup.member		= pnode1->member ;
	nodeBackup.FreeNodeMember	= pnode1->FreeNodeMember ;
	
	pnode1->msize			= pnode2->msize ;
	pnode1->member			= pnode2->member ;
	pnode1->FreeNodeMember		= pnode2->FreeNodeMember ;
	
	pnode2->msize			= nodeBackup.msize ;
	pnode2->member			= nodeBackup.member ;
	pnode2->FreeNodeMember		= nodeBackup.FreeNodeMember ;
	
	return 0;
}

int SortList( SList *plist , int (* SortListNodeProc)( void *pmember1 , void *pmember2 ) )
{
	SListNode *pnode1 = NULL ;
	SListNode *pnode2 = NULL ;
	SListNode *pnodeSelected = NULL ;
	int iret ;
	
	if( plist == NULL )
		return -1;
	
	pnode1 = plist ;
	while( pnode1->next != NULL )
	{
		pnodeSelected = pnode1 ;
		pnode2 = pnode1 ;
		while( pnode2 != NULL )
		{
			iret = SortListNodeProc( pnodeSelected->member , pnode2->member ) ;
			if( iret > 0 )
				pnodeSelected = pnode2 ;
			
			pnode2 = pnode2->next ;
		}
		if( pnode1 != pnodeSelected )
		{
			iret = SwapTwoListNodes( pnode1 , pnodeSelected ) ;
			if( iret < 0 )
				return -2;
		}
		
		pnode1 = pnode1->next ;
	}
	
	return 0;
}

BOOL CopyList( SList **pplistSource , SList **pplistDest , BOOL (* CopyListNodeProc)( void *pmemberCopyFrom , long *plmsize , void **ppmemberNew ) )
{
	SListNode *pnode = NULL ;
	void *pmember = NULL ;
	long lmsize ;
	SListNode *pnodeCopy = NULL ;
	BOOL bret ;
	
	pnode = (*pplistSource) ;
	while( pnode != NULL )
	{
		if( CopyListNodeProc == NULL )
		{
			pmember = malloc( pnode->msize ) ;
			if( pmember == NULL )
				return FALSE;
			
			memcpy( pmember , pnode->member , pnode->msize );
		}
		else
		{
			lmsize = pnode->msize ;
			bret = CopyListNodeProc( pnode->member , &lmsize , &pmember ) ;
			if( bret != TRUE )
				return FALSE;
		}
		
		pnodeCopy = AddListNode( pplistDest , pmember , lmsize , pnode->FreeNodeMember ) ;
		if( pnodeCopy == NULL )
		{
			if( CopyListNodeProc == NULL )
			{
				free( pmember );
			}
			else
			{
				pnode->FreeNodeMember( pmember );
			}
			
			return FALSE;
		}
		
		pnode = pnode->next ;
	}
	
	return TRUE;
}

BOOL JoinList( SList **pplistSource , SList **pplistAddition )
{
	if( pplistSource == NULL || pplistAddition == NULL )
	{
		return FALSE;
	}
	
	if( (*pplistSource) == NULL && (*pplistAddition) == NULL )
	{
		return TRUE;
	}
	else if( (*pplistSource) == NULL )
	{
		(*pplistSource) = (*pplistAddition) ;
		
		return TRUE;
	}
	else if( (*pplistAddition) == NULL )
	{
		return TRUE;
	}
	else
	{
		SListNode *node = FindLastListNode( *pplistSource ) ;
		
		node->next = (*pplistAddition) ;
		(*pplistAddition)->prev = node ;
		
		return TRUE;
	}
}

BOOL RuptureList( SList **pplistSource , SListNode *nodeRupture , SList **pplistNew )
{
	if( nodeRupture == NULL )
		return FALSE ;
	
	if( nodeRupture->prev == NULL )
	{
		(*pplistSource) = NULL ;
		
		(*pplistNew) = nodeRupture ;
	}
	else
	{
		nodeRupture->prev->next = NULL ;
		
		nodeRupture->prev = NULL ;
		
		(*pplistNew) = nodeRupture ;
	}
	
	return TRUE;
}

BOOL DetachListNode( SList **pplistSource , SListNode *nodeDetach )
{
	if( nodeDetach == NULL )
		return FALSE ;
	
	if( nodeDetach->prev == NULL && nodeDetach->next == NULL )
	{
		(*pplistSource) = NULL ;
	}
	else if( nodeDetach->prev == NULL )
	{
		(*pplistSource) = nodeDetach->next ;
		nodeDetach->next->prev = NULL ;
		
		nodeDetach->next = NULL ;
	}
	else if( nodeDetach->next == NULL )
	{
		nodeDetach->prev->next = NULL ;
		
		nodeDetach->prev = NULL ;
	}
	else
	{
		nodeDetach->prev->next = nodeDetach->next ;
		nodeDetach->next->prev = nodeDetach->prev ;
	}
	
	return TRUE;
}

BOOL AttachListNodeAfter( SList **pplistSource , SListNode *node , SListNode *nodeAttach )
{
	if( node == nodeAttach )
		return TRUE;
	
	if( node->next == NULL )
	{
		node->next = nodeAttach ;
		nodeAttach->prev = node ;
	}
	else
	{
		nodeAttach->prev = node ;
		nodeAttach->next = node->next ;
		node->next->prev = nodeAttach ;
		node->next = nodeAttach ;
	}
	
	return TRUE;
}

BOOL AttachListNodeBefore( SList **pplistSource , SListNode *node , SListNode *nodeAttach )
{
	if( node == nodeAttach )
		return TRUE;
	
	if( node->prev == NULL )
	{
		node->prev = nodeAttach ;
		nodeAttach->next = node ;
		if( (*pplistSource) == node )
			(*pplistSource) = nodeAttach ;
	}
	else
	{
		node->prev->next = nodeAttach ;
		node->prev = nodeAttach ;
		nodeAttach->next = node ;
		nodeAttach->prev = node->prev ;
	}
	
	return TRUE;
}

/*
** 函数名		:	PushListStack
** 函数描述		:	压入结点到栈式链表中
** 输入参数说明	:	SList *list								链表首结点
**					long max_len							栈式链表允许的最大结点数
**					void *member							结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *PushStackList( SList **list , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	if( CountListNodes( *list ) >= max_len )
		return FALSE;
	
	return AddListNode( list , member , msize , FreeNodeMember );
}

/*
** 函数名		:	PopListStack
** 函数描述		:	从栈式链表中弹出结点
** 输入参数说明	:	SList **list			链表首结点
**					void **member			用于存放结点成员
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL PopupStackList( SList **list , void **member )
{
	SListNode *node=NULL;
	
	if( (*list) == NULL )
		return FALSE;
	
	node=GetListIndexNode( *list , CountListNodes(*list) );
	
	if( member != NULL )
		(*member) = node->member ;
	
	if( node->prev == NULL )
	{
		free( node );
		(*list) = NULL ;
	}
	else
	{
		node->prev->next=NULL;
		free(node);
	}
	
	return TRUE;
}

/*
** 函数名		:	EnterListQueue
** 函数描述		:	对队列式链表中加入结点
** 输入参数说明	:	SList *list								链表首结点
**					long max_len							队列链表允许的最大结点数
**					void *member							结点成员
**					long msize								结点成员的大小
**					BOOL (* FreeNodeMember)( void *pv )		删除该结点成员回调函数
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

SListNode *EnterQueueList( SList **list , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) )
{
	if( CountListNodes( *list ) >= max_len && max_len != 0 )
		return FALSE;
	
	return AddListNode( list , member , msize , FreeNodeMember );
}

/*
** 函数名		:	LeaveQueueList
** 函数描述		:	从队列式链表中拿出结点
** 输入参数说明	:	SList **list			链表首结点
**					void **member			用于存放结点成员
** 返回值		:	成功，返回 TRUE
**					失败，返回 FALSE
** 更新日志		:	2003/10/18	创建
*/

BOOL LeaveQueueList( SList **list , void **member )
{
	SListNode *nodeDelete=NULL;
	
	if( (*list) == NULL )
		return FALSE;
	
	(*member) = (*list)->member ;
	
	nodeDelete = (*list) ;
	if( (*list)->next == NULL )
	{
		(*list) = NULL ;
		free( nodeDelete );
	}
	else
	{
		(*list) = (*list)->next ;
		free( nodeDelete );
		(*list)->prev = NULL ;
	}
	
	return TRUE;
}

