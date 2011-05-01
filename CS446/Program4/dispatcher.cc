#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <vector>

using namespace std;

// single process class
struct Proc
{
  int id, start, stop, ticks, sticks, priority;
};

// class that dispatches procs
class Dispatcher
{
public:
  // read in input file
  Dispatcher( string fname ) : time(0)
  {
    ifstream fin(fname.c_str());

    // read input file
    int id = 0;
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

    // add any processes at time 0
    for ( int i = 0; i < procs.size(); i++ )
      if ( procs[i].start == time )
        q[procs[i].priority].push_back(procs[i]);

    time++;
  }

  // step the cpu by 1 (return false when no more processes to process)
  bool step()
  {
    // process anything from queues
    bool proced = false;
    if ( !q[0].empty() )    // real time queue
    {
      Proc t = q[0].front();
      q[0].front().ticks--;
      if ( --t.ticks == 0 )
      {
        q[0].pop_front();
        procs[t.id].stop = time;
      }
      
      proced = true;
    }
    else    // user level queues
      for ( int i = 1; i < 4; i++ )
        if ( !q[i].empty() )
        {
          Proc t = q[i].front();
          q[i].pop_front();

          if ( --t.ticks == 0 )
            procs[t.id].stop = time;
          else
            q[min(i+1,3)].push_back(t);

          proced = true;
          break;
        }
    
    // add anything new to queues
    bool doneadding = true;
    for ( int i = 0; i < procs.size(); i++ )
    {
      if ( procs[i].start >= time )
        doneadding = false;
      if ( procs[i].start == time )
        q[procs[i].priority].push_back(procs[i]);
    }
    
    // increment time
    time++;

    return !(doneadding && !proced);
  }
  
  // average turnaround time
  float avg_ttime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += procs[i].stop - procs[i].start;
    return tot / procs.size();
  }

  // average normalized turnaround time
  float avg_nttime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += (float)(procs[i].stop - procs[i].start) / procs[i].sticks;
    return tot / procs.size();
  }

  // average wait time
  float avg_wtime()
  {
    float tot = 0;
    for ( int i = 0; i < procs.size(); i++ )
      tot += procs[i].stop - procs[i].start - procs[i].sticks;
    return tot / procs.size();
  }

private:
  vector<Proc> procs;   // contains all processes
  deque<Proc> q[4];     // priority queue
  int time;             // the time
};

int main(int argc, char *argv[])
{
  if ( argc < 2 )
  {
    cout << "Please enter input file." << endl;
    return -1;
  }

  // init dispatcher
  Dispatcher disp(argv[1]);

  // run dispatcher
  while ( disp.step() );

  // print results
  cout << endl << setprecision(2) << fixed
       << "Average turnaround time = " << disp.avg_ttime() << endl
       << "Average normalized turnaround time = " << disp.avg_nttime() << endl
       << "Average waiting time = " << disp.avg_wtime() << endl << endl;

  return 0;
}

