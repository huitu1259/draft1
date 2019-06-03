#pragma once

#include<stack>
#include<vector>
#include<string>
#include<stdarg.h>
#include<iostream>
#include<unordered_set>
#include<unordered_map>

using namespace std;

typedef vector<string>*(*pf_pvectorSTR__pvoid)(void*);
typedef vector<string>*(*pf_pvectorSTR__pvoid_int)(void*, int);
typedef vector<string>*(*pf_pvectorSTR__pvoid_string)(void*, string);
typedef unordered_set<string>*(*pf_pusetSTR__pvoid_string)(void*, string);


struct Item
{
	string raw;								//原产生式,不带点的
	vector<string>* prod_NOT_DELETE;		//从文法中获取产生式右部的引用,不能删除!
	int point;								//点的位置,处于prod->at(point)的左边
};


//  分析表的内容
struct Operation
{
	int type;					//操作类型
								/*
								0 acc,表示结束
								1 移进
								2 规约
								3 错误 - 跳过符号
								999 goto (不会访问到,因为它只会出现在规约之后)
								*/

	int operate;
};

class LRParser_2
{
private:
	void* grammar;
	vector<string>* _grammar_prodList;					//产生式列表
	pf_pvectorSTR__pvoid_int _grammar_getProd;			//输入索引获取产生式右部
	pf_pusetSTR__pvoid_string _grammar_getFirst;		//First函数
	pf_pusetSTR__pvoid_string _grammar_getFollow;		//Follow函数

	bool enabled;
	string extendProd;									//拓展文法的起始符
	vector<string>* extendProd_Vector;					//拓展文法的向量
	
	vector<string>* Errmsg;								//错误信息
	/*以下是分析表的action或goto*/
	unordered_map<int, unordered_map<string, Operation*>*>* action_stat_input__act;

	/*在一个分析器中,可能存在很多项目集.
	ItemSets存储这个分析器中的所有项目集In
	所有常驻的项目集都在这里,并从这里回收
	但是非常驻的项目集需要自行处理(例如closure的参数)
	我们还需要定义一些映射,使这些项目集能用于其他的运算

	*/
	vector<unordered_set<string>*>* ItemSets;


	unordered_map<string, Item*>* map_item2Item;			//某项目字符串与项目类的映射

	unordered_map<string, int>* map_gotoFunc_In;			//goto函数的结果会前往的项目集

	//closure函数
	/*
	每运行一次closure函数,就会生成一个新的项目集,因此I0之后的项目集编号与运行closure的顺序有关
	*/
	unordered_set<string>* closure(unordered_set<string>& Items);

	//goto函数
	/*goto函数需要调用closure,因此它应该返回一个closure创建的实例
	同时,goto可能会到达已存在的项目集,但是这个问题的处理应该交给closure函数
	它可能会返回一个旧的指针.
	*/
	unordered_set<string>* _goto(int In, string input);

	/*checkExist_In_SISF
	它将会确认标准项目集族中是否存在与引用相同的项目集
	如果存在,会返回!!!In的索引!!!
	如果不存在,会返回-1;
	*/
	int checkExist_In_SISF(unordered_set<string>& NewItemSet);

	string getOneItem(string raw,vector<string>& rawProd, int point);

	/*最终生成标准项目族的递归函数*/
	void LR0_build(unordered_set<string>&TokenList, int In);

	void parser_build();

public:
	LRParser_2();
	~LRParser_2();

	bool isEnabled();

	// init函数:初始化LR分析器
	// 参数:文法的实例, 以及这个实例的以下函数指针:
	//		获取产生式列表
	//		获取指定索引位置的产生式
	//		该文法的First函数
	//		该文法的Follow函数
	bool init(void* grammar,
		pf_pvectorSTR__pvoid function_getAllProd,
		pf_pvectorSTR__pvoid_int _grammar_getProd,
		pf_pusetSTR__pvoid_string _grammar_getFirst,
		pf_pusetSTR__pvoid_string _grammar_getFollow);


	/*一些常见的错误类型:
		符号匹配
			可能的错误如下:
				在不接收右括号的状态下接受了右括号
					原因:括号不匹配,多输入了一个括号
					操作:忽视这个右括号,继续向后分析

				在不接收左括号的状态下接受了左括号
					原因: Symbol_next中不存在左括号:
						上一个标记后必须接运算符.-----因此不该接受左括号的情况应该由Symbol_next定义

				想要接收右括号,但是已到达表达式最后


		多余的标记
			计算方法:参数为必须跟随的标记列表
				例如 id后除了id和(都可以跟随
					那么如果输入了id 要么缺少运算符,要么多输入了一个id
					如果输入了左括号,要么是缺少运算符,要么是误输入了左括号
		
					但是一般无法确认缺少的是什么运算符,因此采取的操作是忽略



	*/
	
	// 添加可忽视的错误符号->未匹配的右括号,或加号等运算符
	void add_ignorableErr(string symbol,string comment);

	// parse函数:开始分析
	// 参数:输出流,输入记号流的引用(vector<string>)
	// 输入:记号流
	// 输出:向输出流输出语法分析所用的产生式,以及最后的分析结果或错误信息
	void parse(vector<pair<string,string>>& tokens,ostream& output);
};

