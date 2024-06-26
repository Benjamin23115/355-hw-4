#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <unistd.h>
#include <ctime>
#include <time.h>

using namespace std;
using namespace std::chrono;

const int UNLOCKED = 0;
const int LOCKED = 1;
const int NUMBER_OF_PHILOSPHERS = 5;

const unsigned int twntyMin = 20 * 60 * 1000000;
const unsigned int fiveMin = 5 * 60 * 1000000;
const unsigned int oneMin = 1 * 60 * 1000000;

class Chopstick
{
private:
    mutex chopTex;
    int status;
    int id;

public:
    Chopstick()
    {
    }
    Chopstick(int id)
    {
        this->id = id;
    }
    void lockChopstick()
    {
        chopTex.lock();
        this->status = LOCKED;
    }
    void unlockChopstick()
    {
        chopTex.unlock();
        this->status = UNLOCKED;
    }
};

class Syncro
{
private:
    bool dining;
    Chopstick chopsticks[NUMBER_OF_PHILOSPHERS]; // Array of Chopsticks
    bool status = false;
    mutex statusMutex;

public:
    Syncro()
    {
        this->dining = false;
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
        // Pick up chopsticks in ascending order
        if (id == 0)
        {
            pickUpChopstick(id);
            pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
        }
        else
        {
            pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
            pickUpChopstick(id);
        }
    }
    void releaseChopsticks(int id)
    {
        putDownChopstick(id);
        putDownChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    void setStatus(bool status)
    {
        statusMutex.lock();
        this->status = status;
        statusMutex.unlock();
    }
    bool getStatus()
    {
        statusMutex.lock();
        bool currentStatus = this->status;
        statusMutex.unlock();
        return currentStatus;
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
    int state;
    int id;
    Chopstick *left, *right;
    thread philThread;
    mutex outputMutex;
    Syncro &syncro;
    double thinkTime;
    double hungerTime;
    double eatTime;

public:
    Philosopher(string name, Syncro &t, int id, Chopstick &leftChopstick,
                Chopstick &rightChopstick) : philThread(&Philosopher::run, this), syncro(t)
    {
        this->id = id;
        this->name = name;
        this->left = &leftChopstick;
        this->right = &rightChopstick;
        this->state = 0;
        this->thinkTime = 0.0;
        this->hungerTime = 0.0;
        this->eatTime = 0.0;
    }

    ~Philosopher()
    {
        philThread.join();
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
            if (this->state == 0)
            {
                thinking();
            }
            else if (this->state == 1)
            {
                hungry();
            }
            else if (this->state == 2)
            {
                eating();
            }
            else if (this->state == 3)
            {
                cout << this->name << " has starved." << endl;
                syncro.setStatus(false);
            }
        }
    }

    void thinking()
    {
        auto start = high_resolution_clock::now(); // cout << this->name << " " << this->id << ": is thinking." << endl;
        usleep(500000);
        this->state = 1;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        this->thinkTime += duration.count();
    }

    void hungry()
    {
        auto start = high_resolution_clock::now(); // cout << this->name << " " << this->id << ": is hungry." << endl;
        syncro.getChopsticks(this->id);
        this->state = 2;
        if (this->hungerTime > oneMin)
        {
            this->state = 3;
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        this->hungerTime += duration.count();
    }

    int coinToss()
    {
        return rand() % 2 == 0 ? 0 : 1;
    }

    void eating()
    {
        auto start = high_resolution_clock::now(); // cout << this->name << " " << this->id << " is eating." << endl;
        usleep(500000);
        syncro.releaseChopsticks(this->id);
        // cout << this->name << " " << this->id << " has finished eating." << endl;
        this->state = 0;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        this->eatTime += duration.count();
    }
    void getStats()
    {
        cout << this->name << " " << this->id << " thought for: " << this->thinkTime << " milli-seconds" << endl;
        cout << this->name << " " << this->id << " was hungry for: " << this->hungerTime << " milli-Seconds" << endl;
        cout << this->name << " " << this->id << " Ate for: " << this->eatTime << " milli-Seconds" << endl;
    }
};

const string nameArray[] =
    {"Yoda", "Obi-Wan", "Rey", "Kanan", "Leia", "Luke", "Ahsoka",
     "Mace Windu", "Ezra", "Palpatine", "Anakin", "Kylo Ren", "Dooku",
     "Kit Fitso", "Luminara", "Plo Koon", "Revan", "Thrawn", "Zeb", "Sabine"};

int main()
{
    Syncro *syncro = new Syncro();
    Chopstick *chopsticks[NUMBER_OF_PHILOSPHERS];
    Philosopher *philosophers[NUMBER_OF_PHILOSPHERS];

    for (int i = 0; i < NUMBER_OF_PHILOSPHERS; i++)
    {
        chopsticks[i] = new Chopstick(i);
        philosophers[i] = new Philosopher(nameArray[i], *syncro, i, *chopsticks[i], *chopsticks[(i + 1) % NUMBER_OF_PHILOSPHERS]);
    }

    syncro->setStatus(true);
    usleep(fiveMin);

    syncro->setStatus(false);

    // go through the list of philosophers
    for (int i = 0; i < NUMBER_OF_PHILOSPHERS; i++)
    {
        philosophers[i]->getStats();
        delete (chopsticks[i]);
        delete (philosophers[i]);
    }
    delete syncro;
    return 0;
}

