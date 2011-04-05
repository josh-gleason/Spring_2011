// Joshua Gleason
// Programming assignment 3

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 5

using namespace std;

typedef int buffer_item;

buffer_item buffer[BUFFER_SIZE];
int back = 0, front = 0, p = 0, c = 0;

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

void insert_item(buffer_item item)
{
  // wait if full
  sem_wait(&full);
  
  // lock area
  pthread_mutex_lock(&mutex);
  buffer[front] = item;
  front = (front + 1) % BUFFER_SIZE;
  p++;
  // signal not empty
  sem_post(&empty);
}

void remove_item(buffer_item *item)
{
  // wait if empty
  sem_wait(&empty);

  // lock area
  pthread_mutex_lock(&mutex);
  *item = buffer[back];
  back = (back + 1) % BUFFER_SIZE;
  c++;
  // signal not full
  sem_post(&full);
}

void *producer(void *param)
{
  buffer_item item;
  int id = *(int*)(param);

  while (true)
  {
    // sleep random time 0.5-5.5 seconds
    usleep(rand()%5500000+500000);
    // generate random number
    item = rand();
    insert_item(item);
    cout << "Producer " << id << " produced " << item << endl;
    pthread_mutex_unlock(&mutex); // don't unlock until print
  }
}

void *consumer(void *param)
{
  buffer_item item;
  int id = *(int*)(param);

  while (true)
  {
    // sleep random time 0.5-5.5 seconds
    usleep(rand()%5500000+500000);
    remove_item(&item);
    cout << "\tConsumer " << id << " consumed " << item << endl;
    pthread_mutex_unlock(&mutex); // don't unlock until print
  }
}

int main(int argc, char *argv[])
{
  if ( argc < 4 )
  {
    cout << "Please input sleeptime, producers, and consumers" << endl;
    return -1;
  }

  srand(time(0));

  // get command line arguments
  int sleeptime = atoi(argv[1]),
      producers = atoi(argv[2]),
      consumers = atoi(argv[3]);

  // initialize buffer
  pthread_mutex_init(&mutex, NULL);
  sem_init(&empty, 0, 0);
  sem_init(&full, 0, BUFFER_SIZE);
  
  vector<pthread_t> prodThreads;
  vector<int> prodIds(producers,0);
  vector<pthread_t> consThreads;
  vector<int> consIds(consumers,0);

  // create producer thread(s)
  for ( int i = 0; i < producers; i++ )
  {
    prodIds[i] = i+1;
    prodThreads.push_back(pthread_t());
    pthread_create( &prodThreads[i], 0, producer, (void*)(&prodIds[i]));
  }

  // create producer thread(s)
  for ( int i = 0; i < consumers; i++ )
  {
    consIds[i] = i+1;
    consThreads.push_back(pthread_t());
    pthread_create( &consThreads[i], 0, consumer, (void*)(&consIds[i]));
  }

  // sleep
  sleep(sleeptime);

  // lock the threads
  pthread_mutex_lock(&mutex);

  cout << "Items produced: " << p << endl
       << "Items consumed: " << c << endl;

  // exit
  return 0;
}
