#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <unistd.h>
#include <ctime>
#include <time.h>

using namespace std;

const int UNLOCKED = 0;
const int LOCKED = 1;
const int NUMBER_OF_PHILOSPHERS = 5;

class Chopstick
{
private:
    mutex chopTex;
    int status;

public:
    Chopstick()
    {
    }
    void lockChopstick()
    {
        lock_guard<mutex> lock(chopTex);
    }
    void unlockChopstick()
    {
        lock_guard<mutex> unlock(chopTex);
    }
};

// static semaphore is optional. up to implementation

class Syncro
{
private:
    bool dining;
    Chopstick chopsticks[NUMBER_OF_PHILOSPHERS];
    bool status = false;

public:
    Syncro()
    {
    }
    void putDownChopstick(int id)
    {
        chopsticks[id].unlockChopstick();
    }
    void pickUpChopstick(int id)
    {
        chopsticks[id].lockChopstick();
    }

    void getChopsticks(int id)
    {
        pickUpChopstick(id);
        pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }

    void releaseChopsticks(int id)
    {
        putDownChopstick(id);
        putDownChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    void setStatus(bool status)
    {
        this->status = status;
    }
    bool getStatus()
    {
        return this->status;
    }
    void setDining(bool dining)
    {
        this->dining = dining;
    }
    bool getDining()
    {
        return this->dining;
    }
};

class Philosopher
{
private:
    string name;
    double thinkTime;
    int id;
    thread mainThread;
    mutex outputMutex;
    Syncro &syncro;

public:
    Philosopher(string name, Syncro &t,
                int id) : mainThread(&Philosopher::run, this), syncro(t)
    {
        this->name = name;
        this->id = id;
        this->thinkTime = 0.0;
    }

    ~Philosopher()
    {
        mainThread.join();
    }

    void run()
    {
        do
        {
            cout << "waiting for philosopher initialization." << endl;
        } while (syncro.getStatus() == false);

        while (syncro.getStatus() == true)
        {

            usleep(50000);

            thinking();
            cout << "Philosopher " << this->id << " is going to start eating." << endl;
            eating();
            cout << "Philosopher " << this->id << " has finished eating." << endl;
        }
    };

    void thinking()
    {
        time_t start = time(0);
        cout << "Philosopher " << this->id << " is thinking." << endl;
        usleep(50000);
        if (this->coinToss() == 1)
        {
            syncro.getChopsticks(this->id);
        }
        this->thinkTime += this->thinkTime + difftime(time(0), start);
    }
    int coinToss()
    {
        return rand() % 2 + 1;
    }

    void eating()
    {
        cout << "Philosopher " << this->id << " is eating." << endl;
        syncro.setDining(true);
        usleep(50000);
        if (this->coinToss() == 1)
        {
            syncro.releaseChopsticks(this->id);
        }
        syncro.setDining(false);
    }
};

const string nameArray[] =
    {"Yoda", "Obi-Wan", "Rey", "Kanan", "Leia", "Luke", "Ahsoka",
     "Mace Windu", "Ezra", "Palpatine", "Anakin", "Kylo Ren", "Dooku",
     "Kit Fitso", "Luminara", "Plo Koon", "Revan", "Thrawn", "Zeb", "Sabine"};

void dine()
{
    Syncro syncro;
    Philosopher *philosophers[NUMBER_OF_PHILOSPHERS];

    for (int i = 0; i < NUMBER_OF_PHILOSPHERS; i++)
    {
        philosophers[i] = new Philosopher(nameArray[i], syncro, i);
    }
    syncro.setStatus(true);
    usleep(10000000);
    syncro.setStatus(false);
}

int main()
{
    dine();
    return 0;
}
