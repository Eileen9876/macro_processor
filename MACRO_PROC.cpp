#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include <vector>

#define filename "input/input05.txt"

using namespace std;

//----- ��l�ȳ]�w -----//
bool EXPANDING;
int def_point = 0;
string instr, symbol, opcode, operand;
string label = "AA";

//----- �إ�TAB -----//
vector <string> DEFTAB;
map <string, pair<int, int> > NAMTAB;
map <string, vector<string> > ARGTAB;
map <string, int> op_map;

//----- �}�� -----// 
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
	//----- �B��l�]�w -----//	
	op_map["EQ"] = 1;
	op_map["NE"] = 2;
	op_map["GE"] = 3;
	op_map["GT"] = 4;
	op_map["LE"] = 5;
	op_map["LT"] = 6;
	op_map["+"] = 7;
	op_map["-"] = 8;
	
	//----- ��J�ɳB�z -----//
	EXPANDING = false;
	do{
		GETLINE();
		PROCESSLINE();
	}while(opcode != "END");
	
	//----- ���� -----//
	in.close();
	out.close();
	
	//----- �g�JDEFTAB FILE -----//
	out.open("DEFTAB.txt");
	for(int i=0; i<DEFTAB.size(); i++) out << DEFTAB[i] << endl;
	out.close();
	
	//----- �g�JNAMTAB FILE -----//
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
	//----- ���o�U�@����O -----// 
	if(EXPANDING) instr = DEFTAB[line];
	else getline(in, instr);
	
	//----- ���O�B�z -----// 
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
	//----- �x�s������T -----//
	string macro_name = symbol;
	int start_point = def_point;
	
	//----- �x�s�޼ƦW�٩M�q�{�� -----//
	vector <string> arg_nam; //�x�s�޼ƦW�� 
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
	if(set_arg) //�x�s�̫�@�ӭ� 
	{
		arg_nam.push_back(tmp);
		ARGTAB[macro_name].push_back("");
	}
	else ARGTAB[macro_name].push_back(tmp);
	
	//----- �g�JDEFTAB -----//
	string def = symbol + '\t' + arg_nam[0];
	for(int i=1; i<arg_nam.size(); i++) def += "," + arg_nam[i];
	DEFTAB.push_back(def);
	def_point++; //��ƼW�[ 
	
	//----- �w�q�B�z -----//
	int level = 1;
	while(level > 0)
	{
		GETLINE();
		
		//----- ���L���� -----//
		if(instr[0] == '.') continue;
		
		//----- ���޴��N�޼� -----//
		string opd_tmp = "";
		for(int i=0; i<operand.length(); i++)
		{
			if(operand[i]!='&')
			{
				opd_tmp += operand[i];
				continue;
			}
			//----- �޼ƦW -----// 
			tmp = "&";
			i++;
			while(i<operand.length())
			{
				if(operand[i]>='A'&&operand[i]<='Z') tmp+=operand[i++];
				else break; 
			}
			i--;
			//----- ���N -----// 
			bool find = false;
			for(int j=0; j<arg_nam.size(); j++)
				if(tmp == arg_nam[j]) 
				{
					opd_tmp += "?" + to_string(j+1);
					find = true;
				}
			if(!find) opd_tmp += tmp;
		}
		
		//----- �g�JDEFTAB -----//
		def = symbol + '\t' + opcode + '\t' + opd_tmp;
		DEFTAB.push_back(def);
		def_point++; //��ƼW�[ 
		
		if(opcode == "MACRO") level+=1;
		else if(opcode == "MEND") level-=1;
	}
	
	//----- �N������T�g�JNAMTAB -----//
	NAMTAB[macro_name] = make_pair(start_point, def_point-1);
	
	cout << "end define" << endl;
}

void EXPAND()
{
	cout << "expand" << endl;
	EXPANDING = true;
	
	//----- ��l�޼ƭ� -----//
	int arg_size = ARGTAB[opcode].size();
	string arg[arg_size+1]; //�q1�}�l�x�s
	for(int i=0; i<arg_size; i++) arg[i+1] = ARGTAB[opcode][i];
	
	//----- ��J�޼ƭ� -----//
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
	
	//----- �g�JARGTAB FILE -----//
	ofstream f("ARGTAB.txt");
	for(int i=1; i<arg_size+1; i++) f << arg[i] << endl;
	f.close();
	
	//----- �N�����եΥH���Ѥ覡�g�Joutput file -----//
	out << "." << instr << endl;
	
	//----- �����B�z -----//
	map <string, int> var; //�x�s�����Ѽ� 
	int beg=NAMTAB[opcode].first, end=NAMTAB[opcode].second;
	for(int i=beg+1; i<end; i++)
	{
		GETLINE(i);
		
		//----- ���O�B�z(�޼ơB�ѼƵ�) -----//	
		instr_proc(var, arg);
		
		 
		if(opcode == "SET") continue; //���������y�k ���L
		
		//----- �DIF�BWHILE�B�z -----//
		if(opcode != "IF" && opcode != "WHILE")
		{
			out << instr << endl;
			continue;
		}
		
		//----- IF�BWHILE�B�z -----//
		bool result = expression(true);
		
		int loop = 0;
		if(opcode == "WHILE") loop = i;
		
		GETLINE(++i);
		if(result) //���󦨥߮�  
		{
			while(opcode!="ENDIF"&&opcode!="ELSE"&&opcode!="ENDW")
			{
				instr_proc(var, arg);
				if(opcode!="SET") out << instr << endl;
				GETLINE(++i);
			}
			if(opcode=="ELSE") while(opcode!="ENDIF") GETLINE(++i);
			if(loop) i = loop-1; //WHILE�j��  
		}
		else //���󤣦��߮�  
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

	//----- LABEL�W�[ -----//
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
	//----- ���O�x�s�B��l�B�B�⤸ -----//
	string op[3] = {"", "", ""}; //���O�x�s{�B�⤸1,�B��l,�B�⤸2} 
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
	
	//----- �Ѽƽ�� SET -----//
	if(op[1] == "") return str2int(op[0]);
	
	//----- �B�⦡�B�z -----//
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
	//----- �޼ƳB�z�B�ѼƳB�z�B�W�[LABEL -----//
	string tmp = ""; //�Ȧs���᪺OPERAND 
	for(int i=0; i<operand.length(); i++)
	{
		if(operand[i]=='?') //�޼ƳB�z
		{
			int index = operand[++i] - '0';
			tmp += arg[index];
			if(operand[i+1] == '-' && operand[i+2] == '>') i+=2;
		}
		else if(operand[i]=='&') //�ѼƳB�z
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
		else if(operand[i]=='$') tmp += "$" + label; //�W�[LABEL
		else tmp += operand[i];
	}
	operand = tmp;
	
	//----- �W�[LABEL -----//
	if(symbol[0] == '$')
	{
		tmp = "$" + label;
		for(int i=1; i<symbol.length(); i++) tmp+=symbol[i];
		symbol = tmp;
	}
	
	instr = symbol + '\t' + opcode + '\t' + operand;
	
	//----- �Ѽƪ�l�� -----//
	if(opcode == "SET") var[symbol] = expression(0);
	cout << " end instr_proc" << endl;
}

