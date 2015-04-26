#ifndef _H_LISTX_
#define _H_LISTX_

/*
** 库名		:	ListX
** 库描述	:	用于链表list操作的函数库
** 作者		:	calvin
** E-mail	:	
** QQ		:	
** 创建日期时间	:	2003/10/18
** 更新日期时间	:	2005/5/2
*/

#ifdef _DEFINE_FASTCGI_
#include "fcgi_stdio.h"
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** 宏名                 :       动态链接库 导入、导出修饰符
** 更新日志             :       2006/5/9        创建
*/

#ifdef _TYPE_COMPILER_MSC_
#ifndef _WINDLL_EXPORT
#define _WINDLL_EXPORT		_declspec(dllexport)
#endif
#else
#ifndef _WINDLL_EXPORT
#define _WINDLL_EXPORT		extern
#endif
#endif

#ifdef _TYPE_COMPILER_MSC_
#ifndef _WINDLL_IMPORT
#define _WINDLL_IMPORT		_declspec(dllimport)
#endif
#else
#ifndef _WINDLL_IMPORT
#define _WINDLL_IMPORT		extern
#endif
#endif


/*
** 类型名		:	BOOL
** 类型描述		:	用于真假值的类型
** 更新日志		:	2003/10/18	创建
*/

#ifndef _TYPEDEF_BOOL_
#define _TYPEDEF_BOOL_
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOLNULL
#define BOOLNULL -1
#endif

/*
** 类型描述	:	定义用于ListX函数库的结构
**				SList		:	链表结构
**				SListNode	:	链表结点结构
*/

typedef struct tagListNode
{
	long msize;
	void *member;
	BOOL (* FreeNodeMember)( void *pv );

	struct tagListNode *prev;
	struct tagListNode *next;
	
	/* 附加结构成员集 为iDB库 */
	struct tagListNode **_iDB_pplist ;
	short _iDB_action ;
	BOOL _iDB_enableflag ;
}
SList,SListNode;

/*
** 函数集描述	:	用于操作一个泛型的链表list的函数集
*/

_WINDLL_EXPORT SList *CreateList();
_WINDLL_EXPORT BOOL DestroyList( SList **list , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT SList *AddListNode( SList **list , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
#define InsertListNode	InsertListNodeBefore
_WINDLL_EXPORT SList *InsertListNodeBefore( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT SList *InsertListNodeAfter( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT SList *InsertListIndexNode( SList **list , int index , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT BOOL DeleteListNode( SList **list , SListNode **nodeList , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL DeleteListIndexNode( SList **list , int index , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL DeleteAllListNode( SList **list , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT SListNode *FindFirstListNode( SList *list );
_WINDLL_EXPORT SListNode *FindLastListNode( SList *list );
_WINDLL_EXPORT SListNode *FindPrevListNode( SListNode *nodeList );
_WINDLL_EXPORT SListNode *FindNextListNode( SListNode *nodeList );

_WINDLL_EXPORT BOOL IsFirstListNode( SListNode *nodeList );
_WINDLL_EXPORT BOOL IsLastListNode( SListNode *nodeList );
_WINDLL_EXPORT BOOL IsListEmpty( SList *list );

_WINDLL_EXPORT BOOL IsListNodeValid( SListNode *node );

_WINDLL_EXPORT SListNode *GetListIndexNode( SList *list , long position );
_WINDLL_EXPORT void *GetListIndexMember( SList *list , long position );
_WINDLL_EXPORT void *GetNodeMember( SListNode *nodeList );

_WINDLL_EXPORT long CountListNodes( SList *list );

_WINDLL_EXPORT long AccessList( SList *listHead , BOOL (* AccessListNodeProc)( void *member ) );

_WINDLL_EXPORT int SwapTwoListNodes( SListNode *pnode1 , SListNode *pnode2 );
_WINDLL_EXPORT int SortList( SList *plist , int (* SortListNodeProc)( void *pmember1 , void *pmember2 ) );

_WINDLL_EXPORT BOOL CopyList( SList **pplistSource , SList **pplistDest , BOOL (* CopyListNodeProc)( void *pmemberCopyFrom , long *plmsize , void **ppmemberNew ) );

_WINDLL_EXPORT BOOL JoinList( SList **pplistSource , SList **pplistAddition );
_WINDLL_EXPORT BOOL RuptureList( SList **pplistSource , SListNode *nodeRupture , SList **pplistNew );

_WINDLL_EXPORT BOOL DetachListNode( SList **pplistSource , SListNode *nodeDetach );
_WINDLL_EXPORT BOOL AttachListNodeAfter( SList **pplistSource , SListNode *node , SListNode *nodeAttach );
_WINDLL_EXPORT BOOL AttachListNodeBefore( SList **pplistSource , SListNode *node , SListNode *nodeAttach );

/*
** 函数集描述	:	栈式链表的函数集
*/

_WINDLL_EXPORT SListNode *PushStackList( SList **listHead , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL PopupStackList(SList **listHead , void **member );

/*
** 函数集描述	:	队列式链表的函数集
*/

_WINDLL_EXPORT SListNode *EnterQueueList( SList **listHead , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL LeaveQueueList( SList **listHead , void **member );

#ifdef __cplusplus
}
#endif

#endif
