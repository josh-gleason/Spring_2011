#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/wait.h>
#include <string>

using namespace std;

// used to store information on a line of the file
struct Line
{
  string parent[2], child[300];
  int children, index;
};

// returns the number of lines in the file and saves information to array
int readFile( char* fname, Line lines[])
{
  ifstream fin(fname);
  if ( !fin.good() )
    return 0;

  int lCount = 0;
  string line;
  istringstream sin;

  // traverse the file
  do {
    getline(fin, line);
    if ( fin.good() && line.length() > 0 )
    {
      sin.clear();    // clear stream flags (eof)
      sin.str(line);  // load stream

      // read parents
      sin >> lines[lCount].parent[0] >> lines[lCount].parent[1];

      // read children
      lines[lCount].children = 0;
      while ( !sin.eof() )
      {
        sin >> lines[lCount].child[lines[lCount].children];
        lines[lCount].children++;
      }

      lCount++;
    }
  } while ( fin.good() );

  fin.close();

  return lCount;
}

// prints tabs
void printTabs(int tabs)
{
  for ( int i = 0; i < tabs; i++ )
    cout << '\t';
}

void breed( Line lines[], int lnum, int tabs, int totlines )
{
  int pid, i, j;
  string name;

  printTabs(tabs);

  cout << lines[lnum].parent[0] << '(' << getpid() << ")-"
       << lines[lnum].parent[1] << endl;

  // increase number of tabs so children are 1 more indented
  tabs++;

  for ( i = 0; i < lines[lnum].children; i++ )
  {
    // spawn new process
    pid = fork();

    if ( pid )  // parent
    {
      waitpid( pid, NULL, 0 );  // wait for child
      exit(0);    // prevent backtracking
    }
    else        // child
    {
      name = lines[lnum].child[i];
      bool babies = false;  // search flag

      // search for partner
      for ( j = 0; j < totlines; j++ )
        if ( lines[j].parent[0] == name || lines[j].parent[1] == name )
        {
          // ensure that childs name is printed first
          if ( lines[j].parent[1] == name )
            swap(lines[j].parent[0], lines[j].parent[1]);

          // breed more children
          breed( lines, j, tabs, totlines );
          babies = true;
        }
      if ( !babies )
      {
        // if no babies just print yourself
        printTabs(tabs);
        cout << name << '(' << getpid() << ')' << endl;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  if ( argc < 2 )
  {
    cout << "Enter a valid input file" << endl;
    return -1;
  }

  Line lines[1000];
  int lCount;
  if ( (lCount = readFile(argv[1], lines)) <= 0 )
  {
    cout << "Input file does not contain information" << endl;
    return -1;
  }

  // begin breeding
  breed( lines, 0, 0, lCount );

  return 0;
}
