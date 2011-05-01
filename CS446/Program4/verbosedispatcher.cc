#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <vector>

using namespace std;

class Proc
{
public:
  Proc(int _id, int _s, int _t, int _p) :
    id(_id), start(_s), ticks(_t), sticks(_t), priority(_p) {}
  Proc() {}

  int id;
  int start;
  int ticks;
  int sticks;
  int priority;
  int stop;
};

// class that dispatches procs
class Dispatcher
{
public:
  void add(Proc process)
  {
    cout << "\tJob ID#" << process.id << " has arrived in the job queue at t" << time << endl
         << "\tJob ID#" << process.id << " has priority #" << process.priority << " -> ";
      
    if ( process.priority == 0 )
      cout << "forwarded to real time queue." << endl;
    else
      cout << "forwarded to p" << process.priority << '.' << endl;

    q[process.priority].push_back(process);
  }

  Dispatcher( string fname ) : time(0)
  {
    ifstream fin(fname.c_str());

    // read input file
    int id = 1;
    char gar;
    while ( fin.good() )
    {
      Proc t;
      t.id = id;
      fin >> t.start >> gar
          >> t.priority >> gar
          >> t.ticks;


      if ( fin.good() )
      {
        t.sticks = t.ticks;
        procs.push_back(t);
      }

      id++;
    }
    fin.close();

    // print time 0 information
    cout << "t" << time << ":\r";
    bool done = true, added = false;;
    for ( int i = 0; i < procs.size(); i++ )
    {
      if ( procs[i].start >= time )
        done = false;
      if ( procs[i].start == time )
      {
        add(procs[i]);
        added = true;
      }
    }
    if ( !added )
      cout << "\tWaiting for processes..." << endl;

    time++;

  }

  // step 1 time unit
  bool step()
  {
    // return false when no more processes in procs
    cout << "t" << time << ":\r";

    bool proced = false;
    if ( !q[0].empty() )
    {
      Proc t = q[0].front();
      cout << "\tProcessing job ID#" << t.id << " in RT Queue (" << --t.ticks << " process time left)" << endl;
      if ( t.ticks == 0 )
      {
        cout << "\tDone with job ID#" << t.id << endl;
        q[0].pop_front();
        procs[t.id-1].stop = time;
      }
      else
        q[0].front().ticks--;

      proced = true;
    }
    else
      for ( int i = 1; i < 4; i++ )
        if ( !q[i].empty() )
        {
          Proc t = q[i].front();
          q[i].pop_front();
          cout << "\tProcessing job ID#" << t.id << " in P" << i
               << " (" << --t.ticks << " process time left)" << endl;

          if ( t.ticks == 0 )
          {
            cout << "\tDone with job ID#" << t.id << endl;
            procs[t.id-1].stop = time;
          }
          else
          {
            cout << "\tMoving job ID#" << t.id << " to P" << min(i+1,3) << "..." << endl;
            q[min(i+1,3)].push_back(t);
          }
          proced = true;
          break;
        }
    
    // add anything new to queues
    bool done = true, added = false;;
    for ( int i = 0; i < procs.size(); i++ )
    {
      if ( procs[i].start >= time )
        done = false;
      if ( procs[i].start == time )
      {
        add(procs[i]);
        added = true;
      }
    }
    
    if ( (done && !proced) )
    {
      cout << "\tAll jobs complete" << endl;
      return false;
    }

    if ( !proced && !added )
      cout << "\tWaiting for process..." << endl;

    time++;

    return true;

  }

  float avg_ttime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += procs[i].stop - procs[i].start;
    return tot / procs.size();
  }

  float avg_nttime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += (float)(procs[i].stop - procs[i].start) / procs[i].sticks;
    return tot / procs.size();
  }

  float avg_wtime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += procs[i].stop - procs[i].start - procs[i].sticks;
    return tot / procs.size();
  }

private:
  vector<Proc> procs;   // contains all processes
  deque<Proc> q[4];
  int time;
};

int main(int argc, char *argv[])
{
  if ( argc < 2 )
  {
    cout << "Please enter input file." << endl;
    return -1;
  }

  Dispatcher disp(argv[1]);

  while ( disp.step() );

  cout << endl << setprecision(2) << fixed
       << "Average turnaround time = " << disp.avg_ttime() << endl
       << "Average normalized turnaround time = " << disp.avg_nttime() << endl
       << "Average waiting time = " << disp.avg_wtime() << endl << endl;

  return 0;
}

