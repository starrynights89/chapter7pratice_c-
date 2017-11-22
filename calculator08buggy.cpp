
/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.

    Fix in progress
*/

#include "std_lib_facilities.h"

/* kind if the key field, and has the following values and meanings:
	number - value contains a value
	name - varname [ NONFIX ] contains a name
	let - it is a let statement
	quit - it is a quit statement
	print - it is a print statement
	other - it is an operator character
*/

struct Token {
	char kind;
	double value;
	string varname;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { }
	Token(char ch, string val) :kind(ch), varname(val) { } // fix added token 
};

/* This is a token stream, with buffer being the lookahead character
if full is true. buffer is initialised more cleanly [ NONFIX ].
*/

class Token_stream {
	bool full;
	Token buffer;
public:
	Token_stream() :full(0), buffer(' ') { }

	Token get();
	void unget(Token t) { buffer=t; full=true; }

	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
	if (full) { full=false; return buffer; }
	char ch;
	cin >> ch;
	if (! cin) return(Token(quit));
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();
		double val;
		cin >> val;
		if (! cin) error("Bad token");
		return Token(number,val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s=ch;
			cin.unget();
			if (! cin) error("Bad token");
			if (s == "let") return Token(let);	
			if (s == "quit") return Token(quit); //FIX 1
			return Token(name,s);
		}
		error("Bad token");
		return Token(' ');
	}
}

//Skip characters until its argument is matches, and throw that
//character away.

void Token_stream::ignore(char c)
{
	if (full && c==buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin>>ch)
		if (ch==c) return;
}

// This is a named variable

struct Variable {
	string name;
	double value;
	Variable(string n, double v) :name(n), value(v) { }
};

//The active variables

vector<Variable> names;	

//Get the value of a variable, or fail if no match

double get_value(string s)
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ",s);
	return 0.0;
}

//Set the value of a variable, or fail if no match. 

void set_value(string s, double d)
{
	for (int i = 0; i<=names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ",s);
}

// Check if a variable exists. 

bool is_declared(string s)
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

//The token stream. Might be better to a class

Token_stream ts;

double expression();

// Read a primary(value, operator or compound expression)

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	double d = expression();
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
	}
	case '-':
		return - primary();
	case number:
		return t.value;
	case name:
		return get_value(t.varname);
	default:
		error("primary expected");
		return 0.0;
	}
}

// Read a mupltiplcation or division statement

double term()
{
	double left = primary();
	while(true) {
		Token t = ts.get();
		switch(t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

//Read an addition or subtraction statement

double expression()
{
	double left = term();
	while(true) {
		Token t = ts.get();
		switch(t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

//Read a variable declaration. 'a' has been changed to name [ NONFIX ]

double declaration()
{
	Token t = ts.get();
	if (t.kind != name) error ("name expected in declaration");
	string name = t.varname;
	if (is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of " ,name);
	double d = expression();
	names.push_back(Variable(name,d));
	return d;
}

//Read a statement. Missing print detection added [ NONFIX ]

double statement()
{
	Token t = ts.get();
	double d;
	switch(t.kind) {
	case let:
		return declaration();
	default:
		ts.unget(t);
		return expression();
	}
}

//Skip to the next print character

void clean_up_mess()
{
	ts.ignore(print);
}

//Output strings

const string prompt = "> ";
const string result = "= ";

//Calculation loop is a total mess. Cleaned up case handling and indentation

void calculate()
{
	while(cin)
	{
		try
		{
			cout << prompt;
			Token t = ts.get();
			if (t.kind == print)
			{
				t = ts.get();
			}
			else if (t.kind == quit)
			{
				return;
			}
			else
			{
				ts.unget(t);
				cout << result << statement() << endl;
			}
		}
		catch(runtime_error& e)
		{
			cerr << e.what() << endl;
			clean_up_mess();
		}
	}
}

int main()
{
	calculate();
	return 0;
}