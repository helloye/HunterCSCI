#include "Globaldata.h"
#include "Process.h"
#include "Cpu.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <queue>
#include <list>

using namespace std;


int g_PCount=0;
int g_DCount=0;
int g_DCCount=0;
int g_CCount=0;

int g_PID=-1;
int g_ProcessCounter=0;

float g_SystemAvg=0;
float g_CompletedProcess=0;

float g_Tau=0;
float g_Alpha=0;

int g_DiskHeadLocation=0;
int g_CylinderMax=0; //Max bound for LOOK Algorithm
int g_CylinderMin=0; //Min bound for LOOK Algorithm
char direction='o'; //Out by default, change to 'i' for in once max is reached

int lineCount=0;

void sysGen();
void helpMenu();
void screenShot(queue<Process> printQ);
void screenShot(list<Process> printQ);

int main()
{
    string repeatGen="n";
    do
    {
        sysGen();
        cout << "<SysGen#>"
             << "\n----------------------------"
             << "\nCount Type             Count"
             << "\n----------------------------"
             << "\nPrinter                  " << g_PCount
             << "\nDisk                     " << g_DCount
             << "\nCylinders per Disk       " << g_DCCount
             << "\nCD/RW                    " << g_CCount
             << "\nTau                      " << g_Tau
             << "\nAlpha                    " << g_Alpha
             << "\n----------------------------"
             << "\nIs this configuration correct?(y/n):";

        do
        {
            cin >> repeatGen;
            if(repeatGen.length()>1 || (repeatGen[0]!='y' && repeatGen[0]!= 'Y'
               && repeatGen[0]!='n' && repeatGen[0]!='N'))
               {
                    cout << "SysGen#:Error input, please enter \"y\" or \"n\"!!" << endl;
                    cout << "#:";
               }

        }while(repeatGen.length()>1 || (repeatGen[0]!='y' && repeatGen[0]!= 'Y'
               && repeatGen[0]!='n' && repeatGen[0]!='N'));

    }while(repeatGen[0]=='n' || repeatGen[0]=='N');

    //SYSGEN ENDS HERE

    //START RUNNING
    //Create all the Queues and CPI
    queue<Process> Printers[g_PCount];
    list <Process> Disk[g_DCount];
    queue<Process> Cdrw[g_CCount];
    queue<Process> ReadyQueue;
    Cpu cpu0;

    cout << "\nRunning OS..." << endl;
    cout << "Type \"help\" for help menu." << endl;
    cout << "Press enter to continue..." << endl;
    cin.get();
    cin.ignore();

    Process tempBuffer;
    tempBuffer.setPID(g_PID);
    cpu0.loadProc(&tempBuffer);
    cpu0.getProc();
    g_PID++;



////////////////////
//RUNNING OS BLOCK//
////////////////////
    string command;
    int deviceNum;
    do
    {
        //Before each command line is received
        //Do ReadyQueue/CPU Proc pushes here

        //SJF Sorting here to ensure whatever being loaded in the next if block is the Shortest Job


        //If there is something in RQueue (After first run)
        //And cpu is idle.
        if(!ReadyQueue.empty() && cpu0.isEmpty())
        {
            //Initially shortest is the first in queue.
            Process shortest;
            shortest=ReadyQueue.front();
            ReadyQueue.pop();

            //Then compare with rest to make sure it's first
            for(int i=0; i<ReadyQueue.size(); i++)
            {
                //If tau of front is less than shortest, swap.
                if(ReadyQueue.front().getTau() < shortest.getTau())
                {
                    ReadyQueue.push(shortest);
                    shortest=ReadyQueue.front();
                    ReadyQueue.pop();
                }
            }
            //Load with shortest tau after comparing with all.
            cpu0.loadProc(&shortest);
        }
        cout << "#:";
        do
        {
            cin >> command;
            if(commandChecker(command)==false)
            {
                cout << "Error input. Type \"help\" for help menu."
                     << "\n#:";
            }
        }while(commandChecker(command)==false);

        if(command=="help")
        {
            helpMenu();
        }

        else if(command=="clear")
        {
            #ifdef _WIN32
            system("cls");
            #else
            system("clear");
            #endif
        }

        else if(command=="A")
        {
            //Process *temp=new Process;
            Process *temp=new Process;
            //temp->setPID(g_PID);
            temp->setPID(g_PID);
            temp->setTau(g_Tau);
            //cout << "Queuing temp:" << temp->getPID() << endl;
            //cout << "TEMP'S PID-->" << temp->getPID() << endl; //Ok this works.

            if(cpu0.isEmpty())
            {
                cout << "Queuing process..." << endl;
                cout << "Process queued into CPU!" << endl;
                cpu0.loadProc(temp);
            }
            else if(!cpu0.isEmpty())
            {
                cout << "Queuing process..." << endl;
                cpu0.interruptBurst(); //CPU BURST UPDATE!!
                                       //Should only be done if process is in CPU to get time.
                cout << "Process queued into READYQUEUE!" << endl;
                ReadyQueue.push(*temp);
            }
            //Write Queue Process here.
            g_PID++;
            g_ProcessCounter++;
//            delete temp;
            //cout << "Current Process ID in CPU " << cpu0.cpudebug() << endl;
        }

        //Else if to check if there are Process Queued in System.
        //YOU SHALL NOT PASS...until you've queued a process.... :D
        else if(g_ProcessCounter==0 && command!="exit")
        {
            cout << "Error, No processes in the System!"
                    << "\nQueue processes with the \"A\" command." << endl;
        }

        else if(command=="t")
        {
            cout << "Terminating CPU Process." << endl;
            /*while(!ReadyQueue.empty())
            {
                cout << "Front:" << ReadyQueue.front().getPID() << endl;
                ReadyQueue.pop();
            }*/

            if(!cpu0.isEmpty()) //Means there's a process loaded
            {
                //cout << "ECHO SEG FAULT!!" << endl;
                cout << "Terminating PID:" << cpu0.getProc()->getPID() << endl;
                /*Process *temp=new Process;
                temp=cpu0.getProc();
                cout << "THIS IS THE PID>>>: " << temp->getPID() << endl;
                ReadyQueue.push(*temp);
                delete temp;*/
                g_SystemAvg+=cpu0.getProc()->getCpuTime();
                g_ProcessCounter--;
                g_CompletedProcess++;
            }
            else if(cpu0.isEmpty())
                cout << "Error termination. No process found in CPU." << endl;
            //cout << "PID CHECK-->" << cpu0.getProc()->getPID() << endl; Ok this works.
        }

        else if(command=="S")
        {
            if(!cpu0.isEmpty())
                cpu0.interruptBurst(); //CPU BURST UPDATE!!

            string input;
            cout << "Taking Snapshot." << endl;
            do
            {
                cout << "Select r,p,d,c:";
                cin >> input;
                if(input.length()>1 || (input[0]!='r' && input[0]!='p'
                                        && input[0]!='d' && input[0]!='c'))
                    cout << "Error input please try again..." << endl;

            }while(input.length()>1 || (input[0]!='r' && input[0]!='p'
                                        && input[0]!='d' && input[0]!='c'));
            //Write Snapshot here.
            switch (input[0])
            {
                //r -Ready Queue
                case 114:
                    //cout << "Echo r" << endl;

                    cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
                    screenShot(ReadyQueue);
                    break;

                //p -Printer Queue
                case 112:
                    //cout << "Echo p" << endl;

                    cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
                    for(int i=0; i<g_PCount; i++)
                    {
                        cout << "p" << i+1 << "----" << endl;
                        screenShot(Printers[i]);
                    }
                    break;

                //d -Disk Queue
                case 100:
                    //cout << "Echo d" << endl;

                    cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg" << left << setw(9) << "Cylinder"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
                    for(int i=0; i<g_DCount; i++)
                    {
                        cout << "d" << i+1 << "----" << endl;
                        screenShot(Disk[i]);
                    }
                    break;

                //c -CD/RW Queue
                case 99:
                    //cout << "Echo c" << endl;

                   cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
                    for(int i=0; i<g_CCount; i++)
                    {
                        cout << "cdrw" << i+1 << "----" << endl;
                        screenShot(Cdrw[i]);
                    }
                    break;

                default:
                    cout << "Hello Professor, if you are reading this please take points off." << endl;
                    break;
            }

        }

        //If the check gets to here, there is at least one PCB in the system.
        //Device Queue/Dequeue block. Error check to make sure process are there.
        else if(command[0]=='P' || command[0]=='p')
        {
            deviceNum=stringConvertInt(command.substr(1));
            //cout << "Calling Printer Device #:" << deviceNum << endl;
            if(deviceNum>g_PCount || deviceNum<=0)
            {
                cout << "Error Printer Device #..."
                     << "\nPlease enter a number from 1-" << g_PCount << endl;
            }
            else if(command[0]=='p' && cpu0.isEmpty())
            {
                cout << "Error no available process..."
                     << "\nPlease load a process with [A]" << endl;
            }
            else if(command[0]=='P' && Printers[deviceNum-1].empty())
            {
                cout << "No processes in Printer" << deviceNum << " queue to terminate." << endl;
            }
            else
            {
                if(command[0]=='P')
                {
                    cpu0.interruptBurst(); //CPU BURST UPDATE!!
                    cout << "Terminating Printer process." << endl;
                    //Maybe use front to access the set functions?? Instead of using temp
                    //Process *temp=&Printers[deviceNum-1].front();
                    Printers[deviceNum-1].front().setFileName("");
                    Printers[deviceNum-1].front().setProcName("");
                    Printers[deviceNum-1].front().setStartMem(0);
                    Printers[deviceNum-1].front().setRW(' ');
                    ReadyQueue.push(Printers[deviceNum-1].front());
                    Printers[deviceNum-1].pop();
                    //delete temp;
                }
                else if(command[0]=='p')
                {
                    cout << "Queuing process into printer[" << deviceNum << "]" << endl;
                    cout << "Enter all strings without spaces." << endl;

                    //DEFINES THE PROCESS HERE for 'p' printer
                    cpu0.defineProc('p');
                    Printers[deviceNum-1].push(*cpu0.getProc());
                    //cout << "ECHO SEG FAULT" << endl;
                    //cout << Printers[deviceNum-1].size() << endl;
                }
            }
        }

        else if(command[0]=='D' || command[0]=='d')
        {
            deviceNum=stringConvertInt(command.substr(1));
            cout << "Calling Disk Device #:" << deviceNum << endl;
            if(deviceNum>g_DCount || deviceNum<=0)
            {
                cout << "Error Disk Device #..."
                     << "\nPlease enter a number from 1-" << g_DCount << endl;
            }
            else if(command[0]=='d' && Disk[deviceNum-1].size()==g_DCCount)
            {
                cout << "**DISK[" << deviceNum << "] CYLINDERS ARE FULL**" << endl;
                cout << "Please select another disk or remove processes from device!" << endl;
            }
            else if(command[0]=='d' && cpu0.isEmpty())
            {
                cout << "Error no available process..."
                     << "\nPlease load a process with [A]" << endl;
            }
            else if(command[0]=='D' && Disk[deviceNum-1].empty())
            {
                cout << "No processes in Disk" << deviceNum << " queue to terminate." << endl;
            }
            else
            {
                if(command[0]=='D')
                {
                    cpu0.interruptBurst();
                    int nextDeleteCylinder=-1;
                    //DO ALGORITHM HERE
                    //FIND NEXT DELETE HERE
                    list <Process> temp=Disk[deviceNum-1];
                    //Copy the list in question and look for the next delete item between the disk head and max.
                    //If direction is out, look for max cylinder
                    if(direction=='o')
                    {
                        while(!temp.empty())
                        {
                            //Check if current cylinder is between head and next delete
                            //Next delete is initially -1 so
                            if(temp.front().getDiskCylinder()<nextDeleteCylinder && temp.front().getDiskCylinder()>=g_DiskHeadLocation)
                                nextDeleteCylinder=temp.front().getDiskCylinder();

                            //Else check for the next delete between head and max
                            else if(temp.front().getDiskCylinder()<=g_CylinderMax && temp.front().getDiskCylinder()>=g_DiskHeadLocation)
                                nextDeleteCylinder=temp.front().getDiskCylinder();

                            temp.pop_front();
                        }
                        if(g_DiskHeadLocation==g_CylinderMax)
                            direction=='i';
                        else
                            g_DiskHeadLocation=nextDeleteCylinder;
                    }

                    else if(direction=='i')
                    {
                        while(!temp.empty())
                        {
                            //Check if current cylinder is between head and 0
                            //Next delete is initially -1 so
                            if(temp.front().getDiskCylinder()<=g_DiskHeadLocation && temp.front().getDiskCylinder()>nextDeleteCylinder)
                                nextDeleteCylinder=temp.front().getDiskCylinder();

                            temp.pop_front();
                        }
                        if(g_DiskHeadLocation==g_CylinderMin)
                            direction=='o';
                        else
                            g_DiskHeadLocation=nextDeleteCylinder;
                    }


                    //USE LOOP HERE TO LOOK FOR DELETE ELEMENT
                    //while ! element or something....
                    for(int i=0; i<Disk[deviceNum-1].size(); i++)
                    {
                        if(Disk[deviceNum-1].front().getDiskCylinder()==nextDeleteCylinder)
                        {
                            cout << "Terminating Disk process using LOOK algorithm." << endl;
                            Disk[deviceNum-1].front().setFileName("");
                            Disk[deviceNum-1].front().setProcName("");
                            Disk[deviceNum-1].front().setStartMem(0);
                            Disk[deviceNum-1].front().setRW(' ');
                            //cout << "OUTBOUND PROC TAU:" << Disk[deviceNum-1].front().getTau() << endl;
                            ReadyQueue.push(Disk[deviceNum-1].front());
                            Disk[deviceNum-1].pop_front();
			    break;
                        }
                        //If the front not the deleted item, delete
                        else if(Disk[deviceNum-1].front().getDiskCylinder()!=nextDeleteCylinder)
                        {
                            Disk[deviceNum-1].push_back(Disk[deviceNum-1].front());
                            Disk[deviceNum-1].pop_front();
                        }
                    }
                }
                else if(command[0]=='d')
                {
                    int cylinder=0;
                    string input;
                    do
                    {
                        cout << "Enter disk cylinder #(int):";
                        cin >> input;
                        if(integerChecker(input)==false || stringConvertInt(input)<0)
                            cout << "Please enter an INTEGER  >= 0!!" << endl;
                        else if(stringConvertInt(input)>g_DCCount-1)
                        {
                            cout << "Error cylinder #, Enter cylinder # 0-" << g_DCCount-1 << endl;
                        }
                        else
                            cylinder=stringConvertInt(input);
                    }while((integerChecker(input)==false || stringConvertInt(input)<0) ||
                           (stringConvertInt(input)>g_DCCount-1));
                    //cout << "Process requesting Disk." << endl;

                    //If cylinder list is empty, max=min
                    if(Disk[deviceNum-1].size()==0)
                        g_CylinderMax=g_CylinderMin=cylinder;
                    else
                    {
                        if(cylinder>g_CylinderMax)
                            g_CylinderMax=cylinder;
                        if(cylinder<g_CylinderMin)
                            g_CylinderMin=cylinder;
                    }

                    cout << "Queuing process into disk[" << deviceNum << "] cylinder indexed at[" << cylinder << "]" << endl;
                    cout << "Enter all strings without spaces." << endl;
                    cpu0.defineProc('d', cylinder);
                    Disk[deviceNum-1].push_back(*cpu0.getProc());
                }
            }
        }


        else if(command[0]=='C' || command[0]=='c')
        {
            deviceNum=stringConvertInt(command.substr(1));
            cout << "Calling CD/RW Device #:" << deviceNum << endl;
            if(deviceNum>g_CCount || deviceNum<=0)
            {
                cout << "Error CD/RW Device #..."
                     << "\nPlease enter a number from 1-" << g_CCount << endl;
            }
            else if(command[0]=='c' && cpu0.isEmpty())
            {
                cout << "Error no available process..."
                     << "\nPlease load a process with [A]" << endl;
            }
            else if(command[0]=='C' && Cdrw[deviceNum-1].empty())
            {
                cout << "No processes in CD/RW" << deviceNum << " queue to terminate." << endl;
            }
            else
            {
                if(command[0]=='C')
                {
                    cpu0.interruptBurst(); //CPU BURST UPDATE!!
                    cout << "Terminating CD/RW process." << endl;
                    Cdrw[deviceNum-1].front().setFileName("");
                    Cdrw[deviceNum-1].front().setProcName("");
                    Cdrw[deviceNum-1].front().setStartMem(0);
                    Cdrw[deviceNum-1].front().setRW(' ');
                    ReadyQueue.push(Cdrw[deviceNum-1].front());
                    Cdrw[deviceNum-1].pop();
                }
                else if(command[0]=='c')
                {
                    //cout << "Process requesting CD/RW." << endl;
                    cout << "Queuing process into cd/rw[" << deviceNum << "]" << endl;
                    cout << "Enter all strings without spaces." << endl;
                    cpu0.defineProc('c');
                    Cdrw[deviceNum-1].push(*cpu0.getProc());
                }
            }
        }

    }while(command!="exit");

}

//Function Def

void sysGen()
{
    string input;
    cout << "-Running SysGen..." << endl;
    cout << "<SysGen#>" << endl;

    do
    {
        cout << "\nNumber of Printers<int>:";
        cin >> input;

        if(integerChecker(input)==false || stringConvertInt(input)<=0)
            cout << "SysGen#:Error input. Please enter non-zero positive INTEGERS only!!" << endl;
        else
            g_PCount=stringConvertInt(input);

    }while(integerChecker(input)==false || stringConvertInt(input)<=0);

    do
    {
        cout << "\nNumber of Disks<int>:";
        cin >> input;

        if(integerChecker(input)==false || stringConvertInt(input)<=0)
            cout << "SysGen#:Error input. Please enter non-zero positive INTEGERS only!!" << endl;
        else
            g_DCount=stringConvertInt(input);

    }while(integerChecker(input)==false || stringConvertInt(input)<=0);

    do
    {
        cout << "\nNumber of Disk Cylinders<int>:";
        cin >> input;

        if(integerChecker(input)==false || stringConvertInt(input)<=0)
            cout << "SysGen#:Error input. Please enter non-zero positive INTEGERS only!!" << endl;
        else
            g_DCCount=stringConvertInt(input);

    }while(integerChecker(input)==false || stringConvertInt(input)<=0);

    do
    {
        cout << "\nNumber of CD/RW<int>:";
        cin >> input;

        if(integerChecker(input)==false || stringConvertInt(input)<=0)
            cout << "SysGen#:Error input. Please enter non-zero positive INTEGERS only!!" << endl;
        else
            g_CCount=stringConvertInt(input);

    }while(integerChecker(input)==false || stringConvertInt(input)<=0);

    do
    {
        cout << "\nEnter initial burst Tau<float>:";
        cin >> input;

        if(atof(input.c_str())<=0)
            cout << "SysGen#:Error input. Please enter non-zero positive FLOATING POINT NUMBERS only!!" << endl;
        else
            g_Tau= ::atof(input.c_str());

    }while(atof(input.c_str())<=0);

    do
    {
        cout << "\nEnter value for Alpha<float>:";
        cin >> input;

        if(atof(input.c_str())<0 || atof(input.c_str())>1)
            cout << "SysGen#:Error input. Please enter number between 0 and 1" << endl;
        else
            g_Alpha= ::atof(input.c_str());

    }while(atof(input.c_str())<0 || atof(input.c_str())>1);
    cout << endl;
};

void helpMenu()
{

    cout << "------------"
         << "\nHelp Menu"
         << "\n------------"
         << "\nCommand List:"
         << "\n     [A]: Queue process"
         << "\n\n     [S]: Creates a snapshot of desired device."
         << "\n\n     [p(number)]: Sends current process in CPU to Printer (number)"
         << "\n     [d(number)]: Sends current process in CPU to Disk (number)"
         << "\n     [c(number)]: Send current process in CPU to CD/RW (number)"
         << "\n     *Upper case P/C/D will terminate process,"
         << "\n     *And returns the PCB to the ready queue."
         << "\n\n     -All other commands will be displayed when needed."
         << "\n     [clear]: Clears the screen."
         << "\n     [help]:  Prints this help menu"
         << "\n     [exit]:  Exits the OS."
         << "\n\n     Press enter to return to # command line..." << endl;
        cin.get();
        cin.ignore();
}

void screenShot(queue<Process> printQ)
{
    queue<Process> temp = printQ;
    while(!temp.empty())
    {
        cout << left << setw(9) << temp.front().getPID() << left << setw(10) << temp.front().getFileName()
                 << left << setw(10) << temp.front().getCpuTime() << left << setw(7) << temp.front().getTau()
                 << left << setw(11) << g_SystemAvg/g_CompletedProcess
                 << left << setw(9) << temp.front().getStartMem() << " " << temp.front().getRW() << endl;
        temp.pop();
        lineCount++;
        if(lineCount==22)
        {
            lineCount=0;
            cout << "\nPress enter to continue..." << endl;
            cin.get();
            cin.ignore();

            cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
        }
    }
}

//Overloaded screenshot for list processes for disk.
void screenShot(list<Process> printQ)
{
    list<Process> temp = printQ;
    while(!temp.empty())
    {
        cout << left << setw(9) << temp.front().getPID() << left << setw(10) << temp.front().getFileName()
                 << left << setw(10) << temp.front().getCpuTime() << left << setw(7) << temp.front().getTau()
                 << left << setw(11) << g_SystemAvg/g_CompletedProcess << left << setw(9) << temp.front().getDiskCylinder()
                 << left << setw(9) << temp.front().getStartMem() << " " << temp.front().getRW() << endl;
        temp.pop_front();
        lineCount++;
        if(lineCount==22)
        {
            lineCount=0;
            cout << "\nPress enter to continue..." << endl;
            cin.get();
            cin.ignore();

            cout << left << setw(9) << "PID" << left << setw(10) << "Filename"
                 << left << setw(10) << "CPUTime" << left << setw(7) << "Tau"
                 << left << setw(11) << "SystemAvg" << left << setw(9) << "Cylinder"
                 << left << setw(9) << "Memstart" << " R/W" << endl;
        }
    }
}
