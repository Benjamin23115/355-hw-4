#include <iostream>
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
static int TOTAL_CHOPSTICKS = 0;

// Hi Luis, this is the dead lock version
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
        pickUpChopstick(id);
        pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    // command to
    void releaseChopsticks(int id)
    {
        putDownChopstick(id);
        putDownChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    // Sends Activation signal to all philosophers
    void setStatus(bool status)
    {
        statusMutex.lock();
        this->status = status;
        statusMutex.unlock();
    }
    // Check condition of Status
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

    int state;
    int id;
    string name;
    double thinkTime;
    thread mainThread;
    mutex outputMutex;
    Chopstick *left, *right;
    Syncro &syncro;
    int hunger;

public:
    Philosopher(string name, Syncro &t, int id, Chopstick &leftChopstick,
                Chopstick &rightChopstick) : mainThread(&Philosopher::run, this), syncro(t)
    {
        this->name = name;
        this->id = id;
        this->thinkTime = 0.0;
        this->left = &leftChopstick;
        this->right = &rightChopstick;
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
            checkHunger();
            eating();
        }
    };

    void checkHunger()
    {
        if (this->thinkTime > 5.0)
        {
            this->hunger = 1;
            cout << "Philosopher " << this->id << " is hungry." << endl;
        }
        else
        {
            this->hunger = 0;
        }
    }

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
        return rand() % 2 == 0 ? 0 : 1;
    }

    void eating()
    {
        syncro.setDining(true);
        cout << "Philosopher " << this->id << " is eating." << endl;
        usleep(50000);
        if (this->coinToss() == 1)
        {
            syncro.releaseChopsticks(this->id);
        }
        syncro.setDining(false);
        cout << "Philosopher " << this->id << " has finished eating." << endl;
    }
};

const string nameArray[] =
    {"Yoda", "Obi-Wan", "Rey", "Kanan", "Leia", "Luke", "Ahsoka",
     "Mace Windu", "Ezra", "Palpatine", "Anakin", "Kylo Ren", "Dooku",
     "Kit Fitso", "Luminara", "Plo Koon", "Revan", "Thrawn", "Zeb", "Sabine"};

int main()
{
    Syncro syncro;
    Chopstick *chopsticks[NUMBER_OF_PHILOSPHERS];
    Philosopher *philosophers[NUMBER_OF_PHILOSPHERS];
    int lastPhilosopher = NUMBER_OF_PHILOSPHERS - 1;

    // Declare Chopsticks and Philosophers
    for (int i = 0; i < NUMBER_OF_PHILOSPHERS; i++)
    {
        chopsticks[i] = new Chopstick(i);
        philosophers[i] = new Philosopher(nameArray[i], syncro, i, *chopsticks[i],
                            *chopsticks[(i + 1) % NUMBER_OF_PHILOSPHERS]);
    }

    // Set syncro to true to start all philosophers
    syncro.setStatus(true);
    usleep(10000000);

    // Set syncro to false to stop all philosophers
    syncro.setStatus(false);

    return 0;
}
