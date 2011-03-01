#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/wait.h>
#include <string>

using namespace std;

const int MAXLINES = 1000;
const int MAXCHILD = 300;

// on input line
class Line
{
  public: 
    Line() : _index(0) {}
    Line( string _parent[], int _children ) : _index(0), children(_children)
    {
      parent[0] = _parent[0];
      parent[1] = _parent[1];
    }

    void set( string _parent[], int _children ) 
    {
      _index = 0;
      children = _children;
      parent[0] = _parent[0];
      parent[1] = _parent[1];
    }

    bool addChild( const string& name )
    {
      if ( _index >= children )
        return false;
      child[_index] = name;
      _index++;
      return true;
    }

    bool getChild( string& name, int i )
    {
      if ( children >= i || i < 0 )
        return false;
      name = child[i];
      return true;
    }

    Line& operator=(Line &rhs)
    {
      parent[0] = rhs.parent[0];
      parent[1] = rhs.parent[1];
      children = rhs.children;
      _index = rhs._index;
      for ( int i = 0; i < rhs.children; i++ )
        child[i] = rhs.child[i];
    }

    string parent[2];
    int children;
    string child[MAXCHILD];
private:
    int _index;
};

// valididates the input file, returns error message if invalid
bool initFile( ifstream& fin, const int& argc, char *argv[] );

// read file into array and return number of lines
int readFile( ifstream& fin, Line lines[] );

// main breeding function (recursive)
void breed( Line[], int, int, int );

int main(int argc, char *argv[])
{
  // open file
  ifstream fin;
  if ( !initFile( fin, argc, argv ) )
    return -1;

  // read file
  Line lines[MAXLINES];
  int lCount = readFile(fin, lines);

  // start running
  breed( lines, 0, 0, lCount );  // start at line 1 and 0 tabs

  return 0;
}

// main recursive call
void breed( Line lines[], int lnum ,int tabs, int totlines )
{
  // counter and pid value
  int pid, i;

  Line next;  // 
  string name;

  for ( i = 0; i < tabs; i++ )
    cout << '\t';

  cout << lines[lnum].parent[0] << '-' << lines[lnum].parent[1] << endl;
  
  tabs++;

  // go through and fork for each child
  for ( i = 0; i < lines[lnum].children; i++ )
  {
    pid = fork();
    if ( pid ) // parent
    {
      waitpid( pid, NULL, 0 );  // wait for child to die   
    }
    else       // child
    {
      // get the name of this child
      name = lines[lnum].child[i];
      
      // does not currently know of any babies
      bool babies = false;

      // make babies
      for ( int j = 0; j < totlines; j++ )
        if ( lines[j].parent[0] == name || lines[j].parent[1] == name ) // if you are the parent
        {
          if ( lines[j].parent[1] == name )
          {
            // swap names so they are printed in correct order
            string temp = lines[j].parent[0];
            lines[j].parent[0] = lines[j].parent[1];
            lines[j].parent[1] = temp;
          }

          // make some child processes'
          breed( lines, j, tabs, totlines );
          babies = true;
        }
      if ( !babies ) // if had no children, print along and exit
      {
        for ( int j = 0; j < tabs; j++ )
          cout << '\t';
        cout << name << endl;
      }
      exit(0);    // exit
    }
  }
  exit(0);  // children have all been checked and run, now exit

}

// returns number of lines in file
int readFile( ifstream& fin, Line lines[] )
{
  string parent[2], child;
  int children, l = 0;
  Line current;

  // read through every line of the file
  do {
    fin >> parent[0] >> parent[1] >> children;
    if ( fin.good() )
    {
      current.set(parent, children);

      for ( int i = 0; i < children; i++ )
      {
        fin >> child;
        current.addChild(child);
      }

      lines[l] = current;
      l++;
    }
  } while ( fin.good() );

  // close the file
  fin.close();
  return l;
}

bool initFile( ifstream& fin, const int& argc, char *argv[] )
{
  // make sure argument is passed
  if ( argc < 2 )
  {
    cout << "Error: Please Enter an input file" << endl;
    return false;
  }

  fin.open(argv[1]);

  // check if file opened
  if ( !fin.good() )
  {
    cout << "Error: Could not open " << argv[1] << endl;
    return false;
  }

  // verify format is correct
  
  istringstream sin, numTest;
  string line, temp;
  int children, lineCount = 1, tempInt;

  // obtain the first line
  getline(fin, line);

  if ( !fin.good() )
  {
    cout << "Error: File is empty" << endl;
    return false;
  }

  do {
    if ( line.length() > 0 )
    {
      sin.clear();
      sin.str(line);

      sin >> temp;
      if ( sin.eof() )
      {
        cout << "Error: Line " << lineCount 
             << ": There must be EXACTLY 2 parents per line." << endl
             << "File line format is as follows..." << endl
             << "<Parent 1> <Parent 2> <n> <Child 1> <Child 2> ... <Child n>"
             << endl;
        return false;
      }
      sin >> temp;
      if ( sin.eof() )
      {
        cout << "Error: Line " << lineCount
             << ": There must be EXACTLY 2 parents per line." << endl
             << "File line format is as follows..." << endl
             << "<Parent 1> <Parent 2> <n> <Child 1> <Child 2> ... <Child n>"
             << endl;
        return false;
      }
      // attempt to read an integer
      if ((sin >> children) &&
          children >= 0 &&
          (sin.peek() == ' ' || sin.peek() == '\t' || sin.eof() ))
      {
        for ( int i = 0; i < children; i++ )
        {
          if ( !sin.eof() )
            sin >> temp;
          else
          {
            cout << "Error: Line " << lineCount
                 << ": Should be " << children << " children, not all accounted for."
                 << endl << "File line format is as follows..." << endl
                 << "<Parent 1> <Parent 2> <n> <Child 1> <Child 2> ... <Child n>"
                 << endl;
            return false;
          }
        }
        if ( !sin.eof() )
        {
          cout << "Error: Line " << lineCount
               << ": More children than specified exist" << endl
               << "File line format is as follows..." << endl
               << "<Parent 1> <Parent 2> <n> <Child 1> <Child 2> ... <Child n>"
               << endl;
          return false;
        }
      }
      else
      {
        cout << "Error: Line " << lineCount
             << ": Number of children <n> not specified" << endl
             << "File line format is as follows..." << endl
             << "<Parent 1> <Parent 2> <n> <Child 1> <Child 2> ... <Child n>"
             << endl;
        return false;
      }
    }
    lineCount++;
    getline(fin, line);
  } while ( fin.good() );

  fin.clear();  // clear flags
  fin.seekg(0, ios::beg); // reset file pos
  return true;
}

