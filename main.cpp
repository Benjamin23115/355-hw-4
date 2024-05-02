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
    int id;
public:
    Chopstick(){
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
    // command to grab both chopsticks
    void getChopsticks(int id)
    {
        pickUpChopstick(id);
        pickUpChopstick((id + 1) % NUMBER_OF_PHILOSPHERS);
    }
    // command to release both chopsticks
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
    
    string name;
    int state;// 0:thinking, 1:hungry, 2:eating, 3:starved 
    int id;
    Chopstick *left, *right;
    thread philThread;
    mutex outputMutex;
    Syncro &syncro;
    double hungerTime;

public:
    // inititalize name, monitor, ID, leftChopstick, rightChopstick, philThread
    Philosopher(string name, Syncro &t, int id, Chopstick &leftChopstick,
                Chopstick &rightChopstick) : philThread(&Philosopher::run, this), syncro(t)
    {
        this->id = id;
        this->name = name;
        this->left = &leftChopstick;
        this->right = &rightChopstick;
        this->state = 0;
    }

    ~Philosopher()
    {
        philThread.join();
    }
    // Threaded operation for Philosopher
    void run()
    {
        do
        {
            cout << "waiting for philosopher initialization." << endl;
        } while (syncro.getStatus() == false);
        
        // Run while initialized
        while (syncro.getStatus() == true)
        {
            usleep(50000);
            if(this->state == 0){
                thinking();
            } else if(this->state == 1){
                time_t start = time(0);
                hungry(start);
            } else if(this->state == 2){
                eating();
            } else if(this->state == 3){
                cout << this->name << " HAS STARVED MFERS" << endl;
            }
        }
    }
    
    void thinking()
    {
        // Allow philosopher think for a random amount of time
        cout << this->name << " " << this->id << ": is thinking." << endl;
        usleep(500000);
        if (this->coinToss() == 1)
        {
            // Philosopher is now hungry
            this->state = 1;
        }
    }
    // Start and track hunger timer;
    void hungry(time_t start)
    {
        cout << this->name << " " << this->id << ": is hungry." << endl;
        if(coinToss()){
            syncro.getChopsticks(this->id);
            this->state = 2;
            return;
        }
        if (this->hungerTime > 1.0){
            this->state = 3;
        }
        this->hungerTime += this->hungerTime + difftime(time(0), start);
    }
    // Randomizer to determine if Philosopher drops chopstick
    int coinToss()
    {
        return rand() % 2 == 0 ? 0 : 1;
    }
    // Philosopher has access to both chopsticks and will eat
    void eating()
    {
        cout << this->name << " " << this->id << " is eating." << endl;
        usleep(500000);
        if (this->coinToss() == 1)
        {
            syncro.releaseChopsticks(this->id);
            cout << this->name << " " << this->id << " has finished eating." << endl;
            this->state = 0;
        }
    }
    int test(int state){
        return this->state;
    }
};

// Array used to inititalize Philosopher names
const string nameArray[] =
    {"Yoda", "Obi-Wan", "Rey", "Kanan", "Leia", "Luke", "Ahsoka",
     "Mace Windu", "Ezra", "Palpatine", "Anakin", "Kylo Ren", "Dooku",
     "Kit Fitso", "Luminara", "Plo Koon", "Revan", "Thrawn", "Zeb", "Sabine"};

int main() {
    // Syncro is thread monitor
    Syncro *syncro = new Syncro();
    // Set equal number of Chopsticks and Philosophers
    Chopstick *chopsticks[NUMBER_OF_PHILOSPHERS];
    Philosopher *philosophers[NUMBER_OF_PHILOSPHERS];
    int lastPhilosopher = NUMBER_OF_PHILOSPHERS - 1;
    
    // Declare Chopsticks and Philosophers
    for (int i = 0; i < NUMBER_OF_PHILOSPHERS; i++) {
        chopsticks[i] = new Chopstick(i);
        philosophers[i] = new Philosopher(nameArray[i], *syncro, i, *chopsticks[i], *chopsticks[(i + 1) % NUMBER_OF_PHILOSPHERS]);
    }

    // Set syncro to true to start all philosophers
    syncro->setStatus(true);
    usleep(1000000000000000000000000);

    // Set syncro to false to stop all philosophers
    syncro->setStatus(false);
    
    // Free the memory
    for(int i = 0; i < NUMBER_OF_PHILOSPHERS; i++){
        delete(chopsticks[i]);
        delete(philosophers[i]);
    }
    return 0;
}