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

#define ItemPerRec 8//ÿ���¼�еĲ�Ʒ����������
#define MaxProductNum 20//һ���ж����ֲ�ͬ�Ĳ�Ʒ���

double min_sup;//֧�ֶ���ֵ��ȡֵ��Χ(0,1)
ofstream fout("result.txt");//��������Ƶ��������ļ�

//�ھ�FPtreeʱ�õ�
bool flag[ItemPerRec];
int multi[ItemPerRec];

class FreItem{//��Ʒ���
public:
	string pro;//��Ʒ����
	int fre;//Ƶ��
	FreItem() {fre = 0;};
};

class node{//FPtree���
public:
	friend class FPTree;
	string pro;//����еĲ�Ʒ����
	int ct;//����еĲ�Ʒ����
	node * parent;//ָ�򸸽���ָ�룬��������Ѱ������ģʽ��
	node * firstchild;//ָ���һ���ӽ���ָ��
	node * sibling ;//ָ���Ҳ��һ���ֵܽ���ָ��
	node * next;//ָ��FPtree����һ����ͬ��Ʒ�Ľ��
	node(){parent = NULL; firstchild = NULL;sibling = NULL; next = NULL; ct = 1;};
};

class HeadTable:public FreItem{//��ͷ���е���
public:
	friend class FPTree;
	node * nodelist;//��ͬ��Ʒ���������ͷ
	HeadTable() {nodelist = NULL; fre = 0;};
};

class DBRecord{//���ݿ��¼/Ƶ�����ʹ�õ���ͬһ�ֽṹ��
public:
	int f_num;//���ݿ��¼��Ƶ����ĸ���/Ƶ�����Ƶ����ĸ���
	int sup;//Ƶ�����֧�ֶ�
	FreItem f[ItemPerRec];//��¼/��е�������飬����Ϊf_num
	DBRecord() {f_num = 0;sup = 1;};
};
queue<DBRecord> q;

class FPTree{
public:
	HeadTable h[MaxProductNum];//��ͷ��
	int f_num;//��ͷ���е�����
	node * root; //�����
	void DeleteTree(node * t);//��������������
	FPTree() {root = NULL; f_num = 0;};
	~FPTree() {DeleteTree(root);};
	void InsertTree(DBRecord &r);//����һ����¼����BuildTree��������
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
void FPTree::InsertTree(DBRecord &r) {//����һ����¼����BuildTree��������
	int pos = 0, flag = 0, i, j;
	if (pos >= r.f_num)//��������¼���������Ѿ�������ϣ����˳�
		return;
	node * pointer = NULL;
	node * child = NULL;
	node * tmp = NULL;
	pointer = root;
	while (pointer->firstchild != NULL) {
		flag = 4;
		if (pointer->firstchild->pro == r.f[pos].pro) {//����ǰ���������ӽ��Ĳ�Ʒ�����������ļ�¼�еĵ�ǰ����ͬ����������������ӽ��������ӽ��
			pointer->firstchild->ct += r.sup;
			pointer = pointer->firstchild;
			if (pos + 1 >= r.f_num) {//��������¼���������Ѿ��������
				flag = 1;
				break;
			}
			pos++;
			flag = 5;
			continue;
		}
		child = pointer->firstchild;
		while (child->sibling != NULL) {//�����ҵ�ǰ���������ӽ��
			flag = 3;//˵�����Ƕ���
			if (child->sibling->pro == r.f[pos].pro) {//����ǰ����ĳһ�������ӽ��Ĳ�Ʒ�����������ļ�¼�еĵ�ǰ����ͬ������������ָ�븳����ǰ��㣬����Ѱ��
				child->sibling->ct += r.sup;
				pointer = child->sibling;
				if (pos + 1 >= r.f_num) {//��������¼���������Ѿ��������
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
	if (flag == 3 || flag == 0 || flag == 4 || flag == 2 || flag == 5) {//��ʱ����Ҫ�����½�㣨��¼������һЩ��δ���룬��Ϊû���������ҵ���ȫ��ͬ��·����
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

		for (i = 0; i < f_num; i++) {//��ͬ��Ʒ�Ľ����Ҫ����������
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
		for (i = pos; i < r.f_num; i++) {//���������½��
			pointer = tmp;
			tmp = new node;
			tmp->pro = r.f[i].pro;
			tmp->ct = r.sup;
			pointer->firstchild = tmp;
			tmp->parent = pointer;

			for (j = 0; j < f_num; j++) {//��ͬ��Ʒ�Ľ����Ҫ����������
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

int compare_h(const void * f1, const void * f2) {//���ڰ�֧�ֶȼ����Ӵ�С���Ա�ͷ���е�������
	HeadTable * ff1 = (HeadTable *)f1, * ff2 = (HeadTable *)f2;
	int a = (*ff2).fre - (*ff1).fre;
	if (a == 0){//��֧�ֶȼ�����ͬ��������ֵ�������
		if ((*ff1).pro > (*ff2).pro)
			return 1;
		return -1;
    }
	return a;
} 
int compare_f(const void * f1, const void * f2) {//���ڰ�֧�ֶȼ����Ӵ�С�������ݿ��¼�е�������
	FreItem * ff1 = (FreItem *)f1, * ff2 = (FreItem *)f2;
	int a = (*ff2).fre - (*ff1).fre;
	if (a == 0) {//��֧�ֶȼ�����ͬ��������ֵ�������
		if ((*ff1).pro > (*ff2).pro)
			return 1;
		return -1;
	}
	return a;
} 

void FPTree::BuildTree() {//�ö���q�еļ�¼������FPtree
	int i, j, k, flag, DBRecord_num = q.size();
	DBRecord dbr;
	root = new node;
	for (i = 0; i < DBRecord_num; i++) {//������е����֧�ֶȼ���
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
	qsort(h, f_num, sizeof(h[0]), compare_h);//�Ա�ͷ���е����е����֧�ֶȴӴ�С��˳������
	for (i = f_num - 1; i >= 0; i--) {//����֧�ֶ���ֵ�����Ƶ����
		if ((double)h[i].fre >= min_sup)
			break;	
	}
	f_num = i + 1;//Ƶ���������
	for (i = 0; i < DBRecord_num; i++) {//��ÿ����¼�е������ͷ���е�˳��������򣬲����ÿ����¼�е�Ƶ����
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
	while (!q.empty()) {//����¼������뵽FPtree��
		dbr = q.front();
		q.pop();
		InsertTree(dbr);
	}
}

void FPTree::PrintTree() {//������ȱ������FPtree�����ڵ���
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

void combination(DBRecord & dbr, DBRecord & prefix) {//��FPtreeֻ��������·��P��������dbr�У�����ֱ�Ӷ�P�еĽ�������ϣ�����ģʽǰ׺prefixƴ��
	DBRecord tmp;
	int i, j, minval, num, s = int(pow(2.0, dbr.f_num));//2^dbr.f_num��������
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
		for (j = tmp.f_num - 1; j >= 0; j--) {//��ģʽǰ׺ƴ��
			tmp.f[j + prefix.f_num].pro = tmp.f[j].pro;
		}
		for (j = 0; j < prefix.f_num; j++) {
			tmp.f[j].pro = prefix.f[j].pro;
		}
		tmp.f_num += prefix.f_num;
		
		//д�����ļ���
		for (j = 0; j < tmp.f_num; j++) {
			fout << tmp.f[j].pro << " ";
		}
		fout << tmp.sup << endl;
	}
}

void FP_growth(DBRecord &dbr) {//�ھ�FPtree������Ϊģʽǰ׺dbr
	int i, j;
	FPTree t;
	t.BuildTree();//�ö���q�еļ�¼������FPtree
	//t.PrintTree();
	if (t.root == NULL)
		return;
	int flag = 0;
	int minval;
	DBRecord path, tmp;
	node * pointer = NULL, * cur = NULL;
	pointer = t.root->firstchild;
	minval = 999999;
	while (pointer != NULL) {//�ж����Ƿ�ֻ������һ·��������һ������ֵܽ�㣬��ǵ�һ·����
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
	if (flag == 0) {//���ǵ�һ·��
		path.sup = minval;
		//��ֱ�Ӷ�·��path�ϵĵ�ö������������
		combination(path, dbr);
	}
	else {//�����ǵ�һ·��
		for (i = t.f_num - 1; i >= 0; i--) {//����ͷ���е�ÿһ��a������ģʽ��b=ǰ׺��a
			tmp = dbr;
			tmp.f[tmp.f_num++].pro = t.h[i].pro;
			tmp.sup = t.h[i].fre;
			for (j = 0; j < tmp.f_num; j++) {
				fout << tmp.f[j].pro << " ";
			}
			fout << tmp.sup << endl;
			
			//���b������ģʽ��������Ϊ�µ����ݿ��¼���ݹ齨��FPtree���ھ�
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
	cout << "�����������ļ���*.txt������ȷ�����ļ��ڱ��������ڵ��ļ�����" << endl;
	char name[15];
	cin >> name;
	cout << "��������С֧�ֶ���ֵ��С�������Իس�������" << endl;
	cin >> min_sup;
	ifstream fin(name);
	
	if (!fin) {
        cout << "Open Error!" << endl;
        return -1;
    }
	for (i = 0; i < ItemPerRec; i++) {
		multi[i] = int(pow(2.0, i));
	}
	//��������
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
	fout << "Ƶ���      ֧�ֶȼ���" << endl;
	q.push(dbr);
	dbr.f_num = 0;
	dbr.sup = 0;
	for (i = 0; i < ItemPerRec; i++) {
		dbr.f[i].fre = 0;
		dbr.f[i].pro = "";
	}
	min_sup = min_sup * q.size();//��֧�ֶ���ֵת��Ϊ��С֧�ֶ�
	FP_growth(dbr);//���ú����ھ�Ƶ���
	fin.close();
	fout.close();
	cout << "����ѱ�����result.txt�У�������ڱ��������ڵ��ļ�����" << endl;
	return 0;
}

