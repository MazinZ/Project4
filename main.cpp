#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

struct Token{
public:
	Token(string t1, string t2)
	{
		type = t1;
		token = t2;
	}
	string get_token()
	{
		return token;
	}
	string get_type(){
		return type;
	}
private:
	string type;
	string token;
};

string typeGet(string token);
vector<Token> scanner(string s);
string upper(string s);



int main(){
	string command;
	getline(cin, command);
	vector<Token> commandLine;

	commandLine = scanner(command);
	for (int i = 0; i < commandLine.size(); i++)
		cout << commandLine[i].get_token() << " " << commandLine[i].get_type() << endl;
	system("PAUSE");
	return 0;
}


vector<Token> scanner(string s) {
	vector<Token> parsed;
	bool stringFound = false;
	s+=" ";
	replace(s.begin(),s.end(), '\t', ' ');
	string token = s.substr(0, s.find(" "));
	size_t pos = 0;
	string delimiter = " ";
	while ((pos = s.find(delimiter)) != string::npos ) {
		token = s.substr(0, pos);
		size_t stringCheck = s.find("\"");
			if (stringCheck < pos && stringCheck!=string::npos){
				s.erase(0,stringCheck+1);
				size_t endString = s.find("\"");
				parsed.push_back(Token(s.substr(stringCheck,endString),"string"));
				s.erase(0,endString+1);
				stringFound = true;
			}
			if (token != "" && !stringFound){
				parsed.push_back(Token(token, typeGet(token)));
				s.erase(0, pos + (delimiter).length());
			}
			stringFound = false;
			if (token == "")
				s.erase(0,1);
	}
	return parsed;
}

string typeGet(string token){
	bool validVariable = true;
	if (upper(token) == "DEFPROMPT" || upper(token) == "CD" || upper(token) == "LISTJOBS" || upper(token) ==
		"BYE" || upper(token) == "RUN" || upper(token) == "ASSIGNTO" || upper(token) == "<BG>")
		return "keyword";
	else if (token.length() == 1 && token.c_str()[0] == '#' || token.c_str()[0] == '=')
		return "metaChar";
	else if (token.c_str()[0] == '$' && token.length() > 1)
	{
		for (int i = 1; i < token.length(); i++)
		{
			if (!isalnum(token[i]))
			{
				validVariable = false;
				return "word";
			}
		}
		if (validVariable)
			return "variable";
	}
	else
		return "word";
}

string upper(string s){
   string uppercase = "";

   for(int i = 0; i < s.length(); i++)
       uppercase += toupper(s[i]);

   return uppercase;
}


////////////////////// Parsing ///////////////////////////

bool parse(vector<Token> scanned){
	bool founderror = true;
	int scannedLength = scanned.size();
	if (scannedLength == 0)
		return founderror;

	if (scanned[0].get_token() == "#")
		return !founderror;
	if (scanned[0].get_type()== "variable") {
		if (scannedLength > 3)
			return founderror;
		if (scanned[1]!="=")
			return founderror;
	}
	if (scanned[0]=="cd"){
		if (scannedLength==1)
			return founderror;
		if (scanned[1].get_type()!= "word" || scanned[1].get_type()!="string")
			return founderror;
		if ((scanned[0] = "listprocs" || scanned[0] == "bye") && scannedLength>1)
			return founderror;
	}
	if (scanned[0] == "run"){
		if (scannedLength < 2)
			return founderror;
		if (scanned[1].get_type()!="string" || scanned[1].get_type()!="variable" || scanned[1].get_type()!="word")
			return founderror;
		if (scanned[0] == "assignto"){
			if (scannedLength<3 || vector.back() == "<bg>")
				return founderror;
		}

	}

}


