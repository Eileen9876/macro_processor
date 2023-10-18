#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include <vector>

#define filename "input/input05.txt"

using namespace std;

//----- 初始值設定 -----//
bool EXPANDING;
int def_point = 0;
string instr, symbol, opcode, operand;
string label = "AA";

//----- 建立TAB -----//
vector <string> DEFTAB;
map <string, pair<int, int> > NAMTAB;
map <string, vector<string> > ARGTAB;
map <string, int> op_map;

//----- 開檔 -----// 
ifstream in(filename);
ofstream out("EXPAND.txt");

int str2int(string str);
template <class T> T expression(T type);
void instr_proc(map<string, int>& var, string arg[]);

void PROCESSLINE();
void GETLINE(int line = 0);
void DEFINE();
void EXPAND();

int main()
{
	//----- 運算子設定 -----//	
	op_map["EQ"] = 1;
	op_map["NE"] = 2;
	op_map["GE"] = 3;
	op_map["GT"] = 4;
	op_map["LE"] = 5;
	op_map["LT"] = 6;
	op_map["+"] = 7;
	op_map["-"] = 8;
	
	//----- 輸入檔處理 -----//
	EXPANDING = false;
	do{
		GETLINE();
		PROCESSLINE();
	}while(opcode != "END");
	
	//----- 關檔 -----//
	in.close();
	out.close();
	
	//----- 寫入DEFTAB FILE -----//
	out.open("DEFTAB.txt");
	for(int i=0; i<DEFTAB.size(); i++) out << DEFTAB[i] << endl;
	out.close();
	
	//----- 寫入NAMTAB FILE -----//
	out.open("NAMTAB.txt");
	for(map<string, pair<int, int> >::iterator i=NAMTAB.begin(); i!=NAMTAB.end(); i++)
	{
		string name = i->first;
		out << name << ", " << NAMTAB[name].first << ", " << NAMTAB[name].second << endl; 	
	} 
	out.close();
} 

void PROCESSLINE()
{
	if(NAMTAB.find(opcode) != NAMTAB.end()) EXPAND();
	else if(opcode == "MACRO") DEFINE();
	else out << instr << endl;
}

void GETLINE(int line)
{
	//----- 取得下一行指令 -----// 
	if(EXPANDING) instr = DEFTAB[line];
	else getline(in, instr);
	
	//----- 指令處理 -----// 
	symbol="", opcode="", operand="";
	int cnt=0;
	for(int i=0; i<instr.length(); i++)
	{
		if(instr[i] == '\t') cnt++;
		else if(cnt == 0) symbol += instr[i];
		else if(cnt == 1) opcode += instr[i];
		else if(cnt == 2) operand += instr[i];
	}
}

void DEFINE()
{
	cout << "define" << endl;
	//----- 儲存巨集資訊 -----//
	string macro_name = symbol;
	int start_point = def_point;
	
	//----- 儲存引數名稱和默認值 -----//
	vector <string> arg_nam; //儲存引數名稱 
	bool set_arg = true;
	string tmp = "";
	for(int i=0; i<operand.length(); i++)
	{
		if(operand[i]=='=')
		{
			arg_nam.push_back(tmp);
			set_arg = false;
			tmp = "";
		}
		else if(operand[i]==',')
		{
			if(set_arg)
			{
				arg_nam.push_back(tmp);
				ARGTAB[macro_name].push_back("");
			}
			else ARGTAB[macro_name].push_back(tmp);
			set_arg = true;
			tmp = "";
		}
		else tmp += operand[i]; 
	}
	if(set_arg) //儲存最後一個值 
	{
		arg_nam.push_back(tmp);
		ARGTAB[macro_name].push_back("");
	}
	else ARGTAB[macro_name].push_back(tmp);
	
	//----- 寫入DEFTAB -----//
	string def = symbol + '\t' + arg_nam[0];
	for(int i=1; i<arg_nam.size(); i++) def += "," + arg_nam[i];
	DEFTAB.push_back(def);
	def_point++; //行數增加 
	
	//----- 定義處理 -----//
	int level = 1;
	while(level > 0)
	{
		GETLINE();
		
		//----- 略過註解 -----//
		if(instr[0] == '.') continue;
		
		//----- 索引替代引數 -----//
		string opd_tmp = "";
		for(int i=0; i<operand.length(); i++)
		{
			if(operand[i]!='&')
			{
				opd_tmp += operand[i];
				continue;
			}
			//----- 引數名 -----// 
			tmp = "&";
			i++;
			while(i<operand.length())
			{
				if(operand[i]>='A'&&operand[i]<='Z') tmp+=operand[i++];
				else break; 
			}
			i--;
			//----- 取代 -----// 
			bool find = false;
			for(int j=0; j<arg_nam.size(); j++)
				if(tmp == arg_nam[j]) 
				{
					opd_tmp += "?" + to_string(j+1);
					find = true;
				}
			if(!find) opd_tmp += tmp;
		}
		
		//----- 寫入DEFTAB -----//
		def = symbol + '\t' + opcode + '\t' + opd_tmp;
		DEFTAB.push_back(def);
		def_point++; //行數增加 
		
		if(opcode == "MACRO") level+=1;
		else if(opcode == "MEND") level-=1;
	}
	
	//----- 將巨集資訊寫入NAMTAB -----//
	NAMTAB[macro_name] = make_pair(start_point, def_point-1);
	
	cout << "end define" << endl;
}

void EXPAND()
{
	cout << "expand" << endl;
	EXPANDING = true;
	
	//----- 初始引數值 -----//
	int arg_size = ARGTAB[opcode].size();
	string arg[arg_size+1]; //從1開始儲存
	for(int i=0; i<arg_size; i++) arg[i+1] = ARGTAB[opcode][i];
	
	//----- 填入引數值 -----//
	string tmp = "";
	int cnt = 1;
	for(int i=0; i<operand.length(); i++)
	{
		if(operand[i]==',') 
		{
			arg[cnt++] = tmp;
			tmp = "";
		} 
		else tmp += operand[i];
	}
	arg[cnt] = tmp;
	
	//----- 寫入ARGTAB FILE -----//
	ofstream f("ARGTAB.txt");
	for(int i=1; i<arg_size+1; i++) f << arg[i] << endl;
	f.close();
	
	//----- 將巨集調用以註解方式寫入output file -----//
	out << "." << instr << endl;
	
	//----- 巨集處理 -----//
	map <string, int> var; //儲存內部參數 
	int beg=NAMTAB[opcode].first, end=NAMTAB[opcode].second;
	for(int i=beg+1; i<end; i++)
	{
		GETLINE(i);
		
		//----- 指令處理(引數、參數等) -----//	
		instr_proc(var, arg);
		
		 
		if(opcode == "SET") continue; //巨集內部語法 跳過
		
		//----- 非IF、WHILE處理 -----//
		if(opcode != "IF" && opcode != "WHILE")
		{
			out << instr << endl;
			continue;
		}
		
		//----- IF、WHILE處理 -----//
		bool result = expression(true);
		
		int loop = 0;
		if(opcode == "WHILE") loop = i;
		
		GETLINE(++i);
		if(result) //條件成立時  
		{
			while(opcode!="ENDIF"&&opcode!="ELSE"&&opcode!="ENDW")
			{
				instr_proc(var, arg);
				if(opcode!="SET") out << instr << endl;
				GETLINE(++i);
			}
			if(opcode=="ELSE") while(opcode!="ENDIF") GETLINE(++i);
			if(loop) i = loop-1; //WHILE迴圈  
		}
		else //條件不成立時  
		{
			while(opcode!="ENDIF"&&opcode!="ELSE"&&opcode!="ENDW")
				GETLINE(++i);
			if(opcode!="ELSE") continue;
			while(opcode!="ENDIF")	
			{
				instr_proc(var, arg);
				if(opcode!="SET") out << instr << endl;
				GETLINE(++i);
			}
		}		
	}

	//----- LABEL增加 -----//
	label[1] += 1;
	if(label[1] > 'Z')
	{
		label[1] = 'A';
		label[0] += 1;
	}
	
	EXPANDING = false;
	
	cout << "end expand" << endl;
}


int str2int(string str)
{
	int sum=0;
	for(int i=0; i<str.length(); i++)
	{
		sum *= 10;
		sum += str[i] - '0';
	}
	return sum;
}

template <class T> 
T expression(T type)
{
	//----- 分別儲存運算子、運算元 -----//
	string op[3] = {"", "", ""}; //分別儲存{運算元1,運算子,運算元2} 
	int cnt = 0;
	for(int i=0; i<operand.length(); i++)
	{
		if(operand[i]=='\''||operand[i]=='\"'||operand[i]=='('||operand[i]==')')
			continue;
		if(operand[i] == ' ')
		{
			cnt++;
			continue;
		} 
		if(operand[i]=='+'||operand[i]=='-')
		{
			op[++cnt] = operand[i];
			cnt++;
			continue;
		}
		op[cnt] += operand[i];
	}
	
	//----- 參數賦值 SET -----//
	if(op[1] == "") return str2int(op[0]);
	
	//----- 運算式處理 -----//
	switch(op_map[op[1]])
	{
		case 1: //EQ
			return (op[0] == op[2]);
		case 2: //NE
			return (op[0] != op[2]);
		case 3: //GE
			return (op[0] >= op[2]);
		case 4: //GT
			return (op[0] > op[2]);
		case 5: //LE
			return (op[0] <= op[2]);
		case 6: //LT
			return (op[0] < op[2]);
		case 7: //+
			return str2int(op[0]) + str2int(op[2]);
		case 8: //-
			return str2int(op[0]) - str2int(op[2]);
		default:
			cout << "error : \'" + op[1] + "\' not a operator";
			return 0;
	}
}

void instr_proc(map<string, int>& var, string arg[])
{
	cout << " instr_proc" << endl;
	//----- 引數處理、參數處理、增加LABEL -----//
	string tmp = ""; //暫存更改後的OPERAND 
	for(int i=0; i<operand.length(); i++)
	{
		if(operand[i]=='?') //引數處理
		{
			int index = operand[++i] - '0';
			tmp += arg[index];
			if(operand[i+1] == '-' && operand[i+2] == '>') i+=2;
		}
		else if(operand[i]=='&') //參數處理
		{
			i++;
			string v = "&";
			while(i<operand.length())
			{
				if(operand[i]>='A'&&operand[i]<='Z') v+=operand[i++];
				else break; 
			}
			i--;
			if(var.find(v) != var.end()) tmp+=to_string(var[v]);
			else
			{
				tmp+="0";
				cout << "error : not var \'" << v << "\'" << endl;
			}
		}
		else if(operand[i]=='$') tmp += "$" + label; //增加LABEL
		else tmp += operand[i];
	}
	operand = tmp;
	
	//----- 增加LABEL -----//
	if(symbol[0] == '$')
	{
		tmp = "$" + label;
		for(int i=1; i<symbol.length(); i++) tmp+=symbol[i];
		symbol = tmp;
	}
	
	instr = symbol + '\t' + opcode + '\t' + operand;
	
	//----- 參數初始化 -----//
	if(opcode == "SET") var[symbol] = expression(0);
	cout << " end instr_proc" << endl;
}

