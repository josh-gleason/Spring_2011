////////////////////////////////////////////////////////////////////
// Author: Joshua Gleason                                         //
// Class : CS 446                                                 //
//                                                                //
// This program reads in files and determines if they are in the  //
// graduate or undergraduate format.  This is done by checking    //
// the 3rd member of the first line to see if it begins with a    //
// digit [0-9].  This does impose a slight restriction on the     //
// graduate part as the 1st born child of the first parents can   //
// not have a name that begins with a digit.                      //
//                                                                //
// Ex:                                                            //
//    John Jane 1337baby Jeff                                     //
// would cause some trouble, However                              //
//    John Jane Jeff 1337baby                                     //
// is just fine.                                                  //
////////////////////////////////////////////////////////////////////

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
int readFile( char* fname, Line lines[], bool &grad )
{
  ifstream fin(fname);
  if ( !fin.good() )
    return 0;

  int lCount = 0;
  string line;
  istringstream sin;
  
  // check if this is grad input file or undergrad input file
  fin >> line >> line >> line;
  if ( !fin.good() || line[0] < '0' || line[0] > '9' )
    grad = true;
  else
    grad = false;

  // reset the file to start
  fin.clear();
  fin.seekg(0, ios::beg);
  
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
      if ( grad )
      {
        lines[lCount].children = 0;
        while ( !sin.eof() )
          sin >> lines[lCount].child[lines[lCount].children++];
      }
      else
      {
        sin >> lines[lCount].children;
        for ( int i = 0; i < lines[lCount].children; i++ )
          sin >> lines[lCount].child[i];
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

void breed( Line lines[], int lnum, int tabs, int totlines, bool grad )
{
  int pid, i, j;
  string name;

  printTabs(tabs);

  // print parents
  cout << lines[lnum].parent[0];
  if ( grad ) // dont print pid if in undergrad format
    cout << '(' << getpid() << ')';
  cout << '-' << lines[lnum].parent[1] << endl;

  // increase number of tabs so children are 1 more indented
  tabs++;

  for ( i = 0; i < lines[lnum].children; i++ )
  {
    // spawn new process
    pid = fork();

    if ( pid )  // parent
      waitpid( pid, NULL, 0 );  // wait for child
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
          breed( lines, j, tabs, totlines, grad );
          babies = true;
        }
      if ( !babies )
      {
        // if no babies just print yourself
        printTabs(tabs);
        cout << name;
        if ( grad ) // dont print pid if in undergrad format
          cout << '(' << getpid() << ')';
        cout << endl;
      }
      exit(0);    // murder child (why is this program so violent?)
    }
  }
  exit(0);    // murder parent
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
  bool grad;
  if ( (lCount = readFile(argv[1], lines, grad)) <= 0 )
  {
    cout << "Input file does not contain information" << endl;
    return -1;
  }

  // begin breeding
  breed( lines, 0, 0, lCount, grad );

  return 0;
}
