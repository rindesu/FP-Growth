// FP-growth.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <queue>
#include <math.h>
using namespace std;

#define ItemPerRec 8//每项记录中的产品数（项数）
#define MaxProductNum 20//一共有多少种不同的产品（项）

double min_sup;//支持度阈值，取值范围(0,1)
ofstream fout("result.txt");//保存结果（频繁项集）的文件

//挖掘FPtree时用到
bool flag[ItemPerRec];
int multi[ItemPerRec];

class FreItem{//产品（项）
public:
	string pro;//产品名称
	int fre;//频数
	FreItem() {fre = 0;};
};

class node{//FPtree结点
public:
	friend class FPTree;
	string pro;//结点中的产品名称
	int ct;//结点中的产品计数
	node * parent;//指向父结点的指针，用于向上寻找条件模式基
	node * firstchild;//指向第一个子结点的指针
	node * sibling ;//指向右侧第一个兄弟结点的指针
	node * next;//指向FPtree中下一个相同产品的结点
	node(){parent = NULL; firstchild = NULL;sibling = NULL; next = NULL; ct = 1;};
};

class HeadTable:public FreItem{//项头表中的项
public:
	friend class FPTree;
	node * nodelist;//相同产品结点链的链头
	HeadTable() {nodelist = NULL; fre = 0;};
};

class DBRecord{//数据库记录/频繁项集（使用的是同一种结构）
public:
	int f_num;//数据库记录中频繁项的个数/频繁项集中频繁项的个数
	int sup;//频繁项集的支持度
	FreItem f[ItemPerRec];//记录/项集中的项的数组，长度为f_num
	DBRecord() {f_num = 0;sup = 1;};
};
queue<DBRecord> q;

class FPTree{
public:
	HeadTable h[MaxProductNum];//表头项
	int f_num;//表头项中的项数
	node * root; //根结点
	void DeleteTree(node * t);//由析构函数调用
	FPTree() {root = NULL; f_num = 0;};
	~FPTree() {DeleteTree(root);};
	void InsertTree(DBRecord &r);//插入一条记录，由BuildTree函数调用
	void BuildTree();
	void PrintTree();
};


void FPTree::DeleteTree(node * t){
	if (t != NULL){
		DeleteTree(t->sibling);
		DeleteTree(t->firstchild);
		delete t;
	}
}
void FPTree::InsertTree(DBRecord &r) {//插入一条记录，由BuildTree函数调用
	int pos = 0, flag = 0, i, j;
	if (pos >= r.f_num)//若整个记录的所有项已经插入完毕，则退出
		return;
	node * pointer = NULL;
	node * child = NULL;
	node * tmp = NULL;
	pointer = root;
	while (pointer->firstchild != NULL) {
		flag = 4;
		if (pointer->firstchild->pro == r.f[pos].pro) {//若当前结点的最左子结点的产品名称与待插入的记录中的当前项相同，则继续找其最左子结点的最左子结点
			pointer->firstchild->ct += r.sup;
			pointer = pointer->firstchild;
			if (pos + 1 >= r.f_num) {//若整个记录的所有项已经插入完毕
				flag = 1;
				break;
			}
			pos++;
			flag = 5;
			continue;
		}
		child = pointer->firstchild;
		while (child->sibling != NULL) {//否则，找当前结点的其他子结点
			flag = 3;//说明不是独子
			if (child->sibling->pro == r.f[pos].pro) {//若当前结点的某一非最左子结点的产品名称与待插入的记录中的当前项相同，则把这个结点的指针赋给当前结点，继续寻找
				child->sibling->ct += r.sup;
				pointer = child->sibling;
				if (pos + 1 >= r.f_num) {//若整个记录的所有项已经插入完毕
					flag = 1;
					break;
				}
				pos++;
				flag = 2;
				break;
			}
			child = child->sibling;
		}
		if (flag == 1 || flag == 3 || flag == 4)
			break;
		if (flag == 2)
			continue;
	}
	if (flag == 3 || flag == 0 || flag == 4 || flag == 2 || flag == 5) {//此时，需要创建新结点（记录中仍有一些项未插入，因为没有在树中找到完全相同的路径）
		tmp = new node;
		tmp->pro = r.f[pos].pro;
		tmp->ct = r.sup;
		tmp->parent = pointer;
		if (flag == 3 || flag == 4) {
			child->sibling = tmp;
		}
		else {
			pointer->firstchild = tmp;
		}

		for (i = 0; i < f_num; i++) {//相同产品的结点需要用链连起来
			if (h[i].pro == tmp->pro) {
				if (h[i].nodelist == NULL)
					h[i].nodelist = tmp;
				else {
					pointer = h[i].nodelist;
					while (pointer->next != NULL) {
						pointer = pointer->next;
					}
					pointer->next = tmp;
				}
				break;
			}
		}

		pos++;
		for (i = pos; i < r.f_num; i++) {//继续创建新结点
			pointer = tmp;
			tmp = new node;
			tmp->pro = r.f[i].pro;
			tmp->ct = r.sup;
			pointer->firstchild = tmp;
			tmp->parent = pointer;

			for (j = 0; j < f_num; j++) {//相同产品的结点需要用链连起来
				if (h[j].pro == tmp->pro) {
					if (h[j].nodelist == NULL)
						h[j].nodelist = tmp;
					else {
						pointer = h[j].nodelist;
						while (pointer->next != NULL) {
							pointer = pointer->next;
						}
						pointer->next = tmp;
					}
					break;
				}
			}
		}
	}
}

int compare_h(const void * f1, const void * f2) {//用于按支持度计数从大到小，对表头项中的项排序
	HeadTable * ff1 = (HeadTable *)f1, * ff2 = (HeadTable *)f2;
	int a = (*ff2).fre - (*ff1).fre;
	if (a == 0){//若支持度计数相同，则根据字典序排序
		if ((*ff1).pro > (*ff2).pro)
			return 1;
		return -1;
    }
	return a;
} 
int compare_f(const void * f1, const void * f2) {//用于按支持度计数从大到小，对数据库记录中的项排序
	FreItem * ff1 = (FreItem *)f1, * ff2 = (FreItem *)f2;
	int a = (*ff2).fre - (*ff1).fre;
	if (a == 0) {//若支持度计数相同，则根据字典序排序
		if ((*ff1).pro > (*ff2).pro)
			return 1;
		return -1;
	}
	return a;
} 

void FPTree::BuildTree() {//用队列q中的记录来建立FPtree
	int i, j, k, flag, DBRecord_num = q.size();
	DBRecord dbr;
	root = new node;
	for (i = 0; i < DBRecord_num; i++) {//获得所有的项的支持度计数
		dbr = q.front();
		q.pop();
		q.push(dbr);

		for (j = 0; j < dbr.f_num; j++) {
			flag = 0;
			for (k = 0; k < f_num; k++) {
				if (dbr.f[j].pro == h[k].pro) {
					flag = 1;
					h[k].fre += dbr.sup;
					break;
				}
			}
			if (flag == 0) {
				h[f_num].pro = dbr.f[j].pro;
				h[f_num++].fre += dbr.sup;
			}
		}
	}
	qsort(h, f_num, sizeof(h[0]), compare_h);//对表头项中的所有的项按照支持度从大到小的顺序排序
	for (i = f_num - 1; i >= 0; i--) {//根据支持度阈值，获得频繁项
		if ((double)h[i].fre >= min_sup)
			break;	
	}
	f_num = i + 1;//频繁项的数量
	for (i = 0; i < DBRecord_num; i++) {//对每个记录中的项按照项头表中的顺序进行排序，并获得每个记录中的频繁项
		dbr = q.front();
		q.pop();
		for (j = 0; j < ItemPerRec; j++) {
			dbr.f[j].fre = 0;
		}
		for (j = 0; j < dbr.f_num; j++) {
			for (k = 0; k < f_num; k++) {
				if (dbr.f[j].pro == h[k].pro) {
					dbr.f[j].fre = h[k].fre;
					break;
				}
			}
		}
		qsort(dbr.f, dbr.f_num, sizeof(dbr.f[0]), compare_f);
		for (j = dbr.f_num - 1; j >= 0; j--) {
			if ((double)dbr.f[j].fre >= min_sup)
				break;
		}
		dbr.f_num = j + 1;
		q.push(dbr);
	}
	while (!q.empty()) {//将记录逐个插入到FPtree中
		dbr = q.front();
		q.pop();
		InsertTree(dbr);
	}
}

void FPTree::PrintTree() {//广度优先遍历输出FPtree，用于调试
	if (root == NULL)
		return;
	cout << "print tree" << endl;
	node * pointer = root;
	pointer = pointer->firstchild;
	queue<node *> q_node;
	q_node.push(pointer);
	while (!q_node.empty()) {
		pointer = q_node.front();
		q_node.pop();
		while (pointer != NULL) {
			cout << pointer->pro << "," << pointer->ct << endl;
			if (pointer->firstchild != NULL)
				q_node.push(pointer->firstchild);
			if (pointer->sibling != NULL) {
				pointer = pointer->sibling;
				continue;
			}
			break;
		}
	}
	cout << endl;
}

void combination(DBRecord & dbr, DBRecord & prefix) {//若FPtree只包含单个路径P（保存在dbr中），则直接对P中的结点进行组合，并和模式前缀prefix拼接
	DBRecord tmp;
	int i, j, minval, num, s = int(pow(2.0, dbr.f_num));//2^dbr.f_num种组合情况
	for (i = 1; i < s; i++) {
		num = i;
		for (j = 0; j < ItemPerRec; j++) {
			flag[j] = false;
		}
		for (j = ItemPerRec - 1; j >= 0; j--) {
			if (num >= multi[j]) {
				flag[j] = true;
				if (num == 0)
					break;
				num = num - multi[j];
			}
		}
		tmp.f_num = 0;
		minval = 999999;
		for (j = 0; j < dbr.f_num; j++) {
			if (flag[j] == true) {
				tmp.f[tmp.f_num++].pro = dbr.f[j].pro;
				if (minval > dbr.f[j].fre)
					minval = dbr.f[j].fre;
			}
		}
		tmp.sup = minval;
		for (j = tmp.f_num - 1; j >= 0; j--) {//和模式前缀拼接
			tmp.f[j + prefix.f_num].pro = tmp.f[j].pro;
		}
		for (j = 0; j < prefix.f_num; j++) {
			tmp.f[j].pro = prefix.f[j].pro;
		}
		tmp.f_num += prefix.f_num;
		
		//写出到文件中
		for (j = 0; j < tmp.f_num; j++) {
			fout << tmp.f[j].pro << " ";
		}
		fout << tmp.sup << endl;
	}
}

void FP_growth(DBRecord &dbr) {//挖掘FPtree，参数为模式前缀dbr
	int i, j;
	FPTree t;
	t.BuildTree();//用队列q中的记录来建立FPtree
	//t.PrintTree();
	if (t.root == NULL)
		return;
	int flag = 0;
	int minval;
	DBRecord path, tmp;
	node * pointer = NULL, * cur = NULL;
	pointer = t.root->firstchild;
	minval = 999999;
	while (pointer != NULL) {//判断树是否只包含单一路径（若任一结点有兄弟结点，则非单一路径）
		path.f[path.f_num].pro = pointer->pro;
		path.f[path.f_num++].fre = pointer->ct; 
		if (minval > pointer->ct)
			minval = pointer->ct;
		if (pointer->sibling != NULL) {
			flag = 1;
			break;
		}
		pointer = pointer->firstchild;
	}
	if (flag == 0) {//若是单一路径
		path.sup = minval;
		//则直接对路径path上的点枚举所有组合情况
		combination(path, dbr);
	}
	else {//若不是单一路径
		for (i = t.f_num - 1; i >= 0; i--) {//对项头表中的每一项a，产生模式：b=前缀∪a
			tmp = dbr;
			tmp.f[tmp.f_num++].pro = t.h[i].pro;
			tmp.sup = t.h[i].fre;
			for (j = 0; j < tmp.f_num; j++) {
				fout << tmp.f[j].pro << " ";
			}
			fout << tmp.sup << endl;
			
			//获得b的条件模式基，并作为新的数据库记录，递归建立FPtree并挖掘
			cur = t.h[i].nodelist;
			int ct = 0;
			while (cur != NULL) {
				path.f_num = 0;
				pointer = cur->parent;
				while (pointer != NULL) {
					path.f[path.f_num++].pro = pointer->pro;
					pointer = pointer->parent;
				}
				path.sup = cur->ct;
				path.f_num--;
				if (path.f_num > 0)
					q.push(path);
				ct++;
				cur = cur->next;
			}
			if (!q.empty()) {
				FP_growth(tmp);
			}
		}
	}
}
int main()
{
	int i, tmp11, tmp12, tmp21, tmp22, flag;
	string tmp;
	DBRecord dbr;
	cout << "请输入数据文件（*.txt），并确保该文件在本程序所在的文件夹中" << endl;
	char name[15];
	cin >> name;
	cout << "请输入最小支持度阈值（小数）并以回车键结束" << endl;
	cin >> min_sup;
	ifstream fin(name);
	
	if (!fin) {
        cout << "Open Error!" << endl;
        return -1;
    }
	for (i = 0; i < ItemPerRec; i++) {
		multi[i] = int(pow(2.0, i));
	}
	//读入数据
	fin >> tmp11 >> tmp12;
	fin >> dbr.f[0].pro;
	dbr.f_num = 1;
	while (fin >> tmp21 >> tmp22) {
		fin >> tmp;
		if (tmp21 == tmp11) {
			flag = 0;
			for (i = 0; i < dbr.f_num; i++) {
				if (dbr.f[i].pro == tmp) {
					flag = 1;
					break;
				}
			}
			if (flag == 0)
				dbr.f[dbr.f_num++].pro = tmp;
		}
		else {
			q.push(dbr);
			dbr.f_num = 1;
			dbr.f[0].pro = tmp;
			tmp11 = tmp21;
		}
	}
	fout << "频繁项集      支持度计数" << endl;
	q.push(dbr);
	dbr.f_num = 0;
	dbr.sup = 0;
	for (i = 0; i < ItemPerRec; i++) {
		dbr.f[i].fre = 0;
		dbr.f[i].pro = "";
	}
	min_sup = min_sup * q.size();//将支持度阈值转化为最小支持度
	FP_growth(dbr);//调用函数挖掘频繁项集
	fin.close();
	fout.close();
	cout << "结果已保存在result.txt中，并存放在本程序所在的文件夹中" << endl;
	return 0;
}

