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
    }
    void unlockChopstick()
    {
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
        // maybe use these functions vvv themselves rather than `putDownChopstick` and `pickUpChopstick`
        //       chopsticks(id).unlockChopstick();
    }
    void pickUpChopstick(int id)
    {
        // work with mutex in chopstick
        // chopsticks(id).unlockChopstick();
    }

    void getChopsticks(int id)
    {
        // random selection? (left or right first?
        if (1)
            pickUpChopstick(id);
        pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }

    void releaseChopsticks(int id)
    {
        // random selection? (left or right first?
        if (1)
            putDownChopstick(id);
        putDownChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    bool setStatus(bool status)
    {
        this->status = status;
    }
    bool getStatus()
    {
        return this->status;
    }
};

class Philosopher : thread
{
private:
    string name;
    double thinkTime;
    int id;
    thread mainThread;
    mutex outputMutex;
    Syncro &syncro;

public:
    Philosopher(string name, Syncro &t, int id) : mainThread(&Philosopher::run, this), syncro(t)
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

            cout << " start eating." << endl;
            usleep(50000);
            cout << " finished eating." << endl;
            ;
            thinking();
            eating();
        }
    };

    void thinking()
    {
        cout << this->id << " thinking." << endl;
        usleep(50000);
        // time hungry
        syncro.getChopsticks(this->id);
        // time hungry
    }

    void eating()
    {
        cout << this->id << " is eating." << endl;
        // time eating
        usleep(50000);
        // time eating
    }
};

const string nameArray[] = {"Yoda", "Obi-Wan", "Rey", "Kanan", "Leia", "Luke", "Ahsoka",
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
    usleep(100000);
    syncro.setStatus(false);
}

int main()
{
    dine();
    return 0;
}