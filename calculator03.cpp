/*
		Simple calculator

		Revision history:

			Revised by Alexander Hartson Nov 2017
		
		This program implements a basic expression calculator.
		Input from cin; output to cout.
		The grammar for input is:

		Statement:
			Expression
			Print 
			Quit

		Print:
			;
		Quit:
			q
		
		Expression:
			Term
			Expression + Term
			Expression - Term
		Term:
			Primary 
			Term * Primary
			Term / Primary 
			Term % Primary
		Primary:
*/

#include "std_lib_facilities.h"

class Token
{
public:
    char kind; //what kind of token
    double value; //for numbers: a value
	Token(char ch) //make a Token from char
		:kind(ch), value(0) {}
	Token(char ch, double val) //make a Token from a char and a double
		:kind(ch), value(val) {}
};

class Token_stream
{
public:
	Token get(); //get a Token (get() is defined elswhere)
	void putback(Token t); //put a Token back 
	void ignore(char c); //discard characters up to and include a c
	//The constructor just sets full to indicate that the buffer is empty:
	Token_stream() :full(false), buffer(0) { } //no Token in buffer
	
private:
	bool full; // is there a Token in the buffer?
	Token buffer; //here is where we keep a Token put back using putback()
};

class Variable
{
public:
	string name;
	double value;
};

vector<Variable>var_table;

const char number = '8'; //t.kind == number means that t is a number Token
const char quit = 'q'; //t.kind == quit means that t is a quit Token
const char print = ';'; //t.kind == print means that t is a print Token 

void Token_stream::ignore(char c) //c represent the kind of Token
{
	//first look in buffer:
	if (full && c == buffer.kind)
	{
		full = false;
		return;
	}
	full = false;
	
	//now search input:
	char ch = 0;
	while (cin >> ch)
	{
		if (ch == c)
		{
			return;
		}
	}
}

double get_value(string s)
	//return the value of the Variable named s
{
	for (const Variable& v : var_table)
	{
		if(v.name == s)
		{
			return v.value;
		}
	}
	error("get: undefined variable ", s);
}

void set_value(string s, double d)
	//set the Variable named s to d
{
	for(Variable& v : var_table)
	{
		if(v.name == s)
		{
			v.value = d;
			return; 
		}
	}
	error("set: undefined variable ", s);
}

// The putback() member function puts its argument back into the Token_stream's buffer:
void Token_stream::putback(Token t)
{
	if (full) error("putback() into the full buffer");
	buffer = t; //copy t to buffer
	full = true; // buffer is now full 
}

Token Token_stream::get() //read a token from cin and compose a Token
{
	if (full) //check if we already have a Token ready
	{
		full = false;
		return buffer; 
	}
	char ch;
	cin >> ch; //note that >> skips whitespace (space, newline, tab, etc.)
	
	switch (ch)
	{
		case quit:
		case print:
		case '(':
		case ')':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			return Token{ch}; //let each character represent itself
		case '.':			  //a floating-point-literal can start with a dot
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':		//numeric literal
		{
			cin.putback(ch); // put digit back into the input stream 
			double val;
			cin >> val; //read a floating-point number
			return Token(number, val); 
		}
		default: 
			error("Bad token"); 
	}
}

Token_stream ts; //provides get() and putback()

double expression(); //declaration so that primary() can call expression()

//deal with numbers and parentheses
double primary()  
{
	Token t = ts.get();
	switch (t.kind)
	{
	case'(': //handle '('expression')'
	{
		double d = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		{
			return d;
		}
	}
	case number: 
		return t.value; //return the number's value
	case '-':
		return - primary();
	case '+':
		return primary();
default:
	error("primary expected");
	}
}

// deal with *, /, %
double term()
{
	double left = primary(); //read and evaluate a Term
	Token t = ts.get();      //get the next Token from the Token stream
	while (true)
	{
		switch (t.kind)
		{
		case '*':
			left *= primary();
			t = ts.get();
			break;
		case '/':
		{
			double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
			t = ts.get();
			break;
		}
		case '%':
		{
			double d = primary();
			if (d == 0) error("divide by zero");
			left = fmod(left, d);
			t = ts.get();
			break;
		}
		default:
			ts.putback(t); //put t back into the Token stream
			return left;
		}
	}
}

// deal with + and -
double expression()
{
	double left = term(); //read and evaluate a Term
	Token t = ts.get(); //get the next Token from the Token stream
	while (true)
	{
		switch (t.kind)
		{
		case '+':
			left += term(); //evaluate a Term and add
			t = ts.get();
			break;
		case '-':
			left -= term(); //evaluate a Term and subtract
			t = ts.get();
			break;
		default:
			ts.putback(t); //put t back into the token stream
			return left; //finally: no more + or -; return the answer
		}
	}
}

const string prompt = ">";
const string result = "="; //used to indicate that what follows is a result

void clean_up_mess() 
{
	ts.ignore(print);
}

void calculate() //expression evaluation loop
{
	while (cin)
	try
	{
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get(); //first discard all "prints"
		if (t.kind == quit) return;
		ts.putback(t);
		cout << result << expression() << '\n';
	}
	catch (exception& e)
	{
		cerr << e.what() << '\n'; //write error message
		clean_up_mess();
	}
}

int main()
try
{	
	cout << "Welcome to our simple calculator.\n";
	cout << "Please enter expressions using floating-point numbers.\n";
	cout << "Operators available are '+', '-', '*', '/', and '%'.\n";
	cout << "Type ';' to get a value and 'q' to quit.\n";

	calculate();
	keep_window_open(); //cope with Windows console mode
	return 0;
}
catch(runtime_error& e)
{
    cerr << e.what() << '\n';
    keep_window_open("~~");
    return 1;
}
catch (...)
{
    cerr << "exception \n";
    keep_window_open("~~");
    return 2; 
}
