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

const char number = '8'; //t.kind == number means that t is a number Token
const char quit = 'q'; //t.kind == quit means that t is a quit Token
const char print = ';'; //t.kind == print means that t is a print Token 
const char name = 'a'; //name Token
const char let = 'L'; //declaration Token
const string declkey = "let"; //declaration keyword
const string prompt = "> ";
const string result = "="; //used to indicate that what follows is a result

class Token
{
public:
    char kind; //what kind of token
    double value; //for numbers: a value
	string name;
	Token(char ch)             :kind(ch), value(0) { } //initialize kind with ch
	Token(char ch, double val) :kind(ch), value(val) { } //initialize kind and value
	Token(char ch, string n)   :kind(ch), name(n) { } //initialize kind and name
};


//stream of tokens
class Token_stream
{
public:
	Token_stream(); //make a Token_stream that reads from cin
	Token get(); //get a Token (get() is defined elsewhere)
	void putback(Token t); //put a Token back 
	void ignore(char c); //discard characters up to and include a c
private:
	bool full; // is there a Token in the buffer?
	Token buffer; //here is where we keep a Token put back using putback()
};

//constructor
Token_stream::Token_stream()
:full(false), buffer(0)
{
} //no Token in buffer

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
		case '=':
			return Token{ch}; //let each character represent itself
		case '.':			  //a floating-point-literal can start with a dot
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': //numeric literal
		{
			cin.putback(ch); // put digit back into the input stream 
			double val;
			cin >> val; //read a floating-point number
			return Token(number, val); 
		}
		  default:
        if (isalpha(ch)) {
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
            cin.putback(ch);
            if (s == declkey) return Token(let); // keyword "let"
            return Token(name,s);
        }
        error("Bad token");
        return Token(' '); //Line missing
	}
}

//ignore Tokens up to a certain kind
void Token_stream::ignore(char c) //c represents the kind of Token
{
	//first look in buffer:
	if (full && c == buffer.kind)
	{
		full = false;
		return;
	}
	full = false;
	
	//now search input:
	char ch;
	while (cin >> ch)
	{
		if (ch == c)
		{
			return;
		}
	}
}

//--------------------------------------------------------------------

Token_stream ts; //provides get() and putback()

//--------------------------------------------------------------------

//type for (name, value pairs)
class Variable
{
public:
	string name;
	double value;
	Variable(string n, double v) :name(n), value(v) { }
};

//--------------------------------------------------------------------

vector<Variable>var_table; //vector of variables 

//--------------------------------------------------------------------

//return the value of the Variable named s
double get_value(string s)	
{
	for (int i = 0; i<var_table.size(); i++)
	{
		if(var_table[i].name == s)
		{
			return var_table[i].value;
		}
	}
	error("get: undefined variable ", s);
	return 0.0; //line missing 
}

//--------------------------------------------------------------------

//set the Variable named s to d
void set_value(string s, double d)
{
	for(int i = 0; i<var_table.size(); i++)
	{
		if(var_table[i].name == s)
		{
			var_table[i].value = d;
			return; 
		}
	}
	error("set: undefined variable ", s);
}

//--------------------------------------------------------------------

//is var already in var_table?
bool is_declared(string var)	
{
	for (int i = 0; i<var_table.size(); i++)
	{
		if(var_table[i].name == var) 
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------

//add (var, val) to var_table
double define_name(string var, double val)
{
	if (is_declared(var)) error(var, " declared twice ");
	var_table.push_back(Variable(var, val));
	return val;
}

//--------------------------------------------------------------------

double expression(); //declaration so that primary() can call expression()

//--------------------------------------------------------------------

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
	case name:
		return get_value(t.name); //return the variable's val
		//THIS WAS THE BUG THAT I SPENT 6 HOURS DEBUGGING
	case '-':
		return - primary();
	case '+':
		return primary();
default:
	error("primary expected");
	}
}

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

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

double define_name(string var, double val);
//--------------------------------------------------------------------

//assume we have seen "let"
//handle: name = expression
//declare a variable called "name" with the initial value "expression"
double declaration()
{
	Token t = ts.get();
	if(t.kind != name) error("name expected in declaration");
	string var_name = t.name;

	Token t2 = ts.get();
	if(t2.kind != '=') error("= missing in declaration of ", var_name);

	double d = expression();
	define_name(var_name, d);
	return d; 
}

//--------------------------------------------------------------------

//handles declarations and expressions 
double statement()
{
	Token t = ts.get();
	switch (t.kind)
	{
		case let:
			return declaration();
		default:
			ts.putback(t);
			return expression();
	}
}

//--------------------------------------------------------------------

//clean input after error
void clean_up_mess() 
{
	ts.ignore(print);
}

//--------------------------------------------------------------------

void calculate() //expression evaluation loop
{
	while (cin) try
	{
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get(); //first discard all "prints"
		if (t.kind == quit) return; //quit
		ts.putback(t);
		cout << result << statement() << '\n';
	}
	catch (exception& e)
	{
		cerr << e.what() << '\n'; //write error message
		clean_up_mess();
	}
}

//--------------------------------------------------------------------

int main()
try
{	
	cout << "Welcome to our simple calculator.\n";
	cout << "Please enter expressions using floating-point numbers.\n";
	cout << "Operators available are '+', '-', '*', '/', and '%'.\n";
	cout << "Type ';' to get a value and 'q' to quit.\n";

	//predefine names:
	define_name("pi", 3.1415926535);
	define_name("e", 2.7182818284);
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
