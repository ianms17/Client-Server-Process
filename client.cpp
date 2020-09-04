/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;


int main(int argc, char *argv[]){
    int pid = fork();
    if (pid == 0) {
        // char* arg [] = {"-m", "1024", NULL};
        char* arg [] = {"", NULL};
        execvp("./server", arg);
        delete arg;
    }
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);


    /*
     *   TASK 1: Requesting a single data message
     */
    datamsg* d = new datamsg(0, 0, 0);
    filemsg* f = new filemsg(0, 0);
    string filename;
    int opt;
    ofstream outfile;
    MESSAGE_TYPE opener;
    MESSAGE_TYPE closer;
    struct timeval start, end;
    
    if (argc <= 3) {
        while ((opt = getopt(argc, argv, "p:f:c")) != -1) {
            switch(opt) {
                /*
                 * Single Command to get all values for single person, non-specified ecg or time
                 *
                 * DIDNT END UP USING, REMOVE BEFORE SUBMITTING
                 * 
                case 'p': {
                    outfile.open("x1.csv");
                    // store the command line argument as the person
                    d->person = atoi(optarg);
                    // iterate through all the possible time values
                    for (double time = 0; time <= 500; time += 1) {
                        outfile << time * 0.004 << ",";
                        // iterate through all the possible ecg nums
                        for (int eno = 1; eno <= 2; eno++) {
                            d->seconds = time * 0.004;
                            d->ecgno = eno;
                            chan.cwrite(d, sizeof(datamsg));
                            double result;
                            chan.cread(&result, sizeof(double));
                            if (eno == 1) {
                                outfile << result << ",";
                            } else {
                                outfile << result << endl;
                            }
                        }
                    }
                    outfile.close();
                    break;
                }
                */

                /*
                 * Single command to get a file
                 */ 
                case 'f': {
                    // calculate the length of the file for the filename passed
                    filename = optarg;
                    char* buffer = new char[sizeof(filemsg) + sizeof(filename) + 1];
                    memcpy(buffer, f, sizeof(filemsg));
                    strcpy(buffer + sizeof(filemsg), filename.c_str());
                    chan.cwrite(buffer, sizeof(filemsg) + filename.size() + 1);
                    __int64_t filelen;
                    chan.cread(&filelen, sizeof(__int64_t));

                    // open a file for writing to
                    // string write_file = "home/osboxes/Classwork/CSCE313/PA1/received/" + filename;
                    outfile.open(filename, ios::out | ios::binary);

                    // iterate through file, taking MAX_MESSAGE bytes each time and writing to file
                    __int64_t remaining = filelen;
                    __int64_t currByte = 0;
                    /*
                    * LAST WORKING HERE
                    */
                    int size = 0;
                    while (remaining > 0) {
                        if (remaining < MAX_MESSAGE) {
                            f->length = remaining;
                            f->offset = currByte;
                            size = remaining;
                        } else {
                            f->length = MAX_MESSAGE;
                            f->offset = currByte;
                            size = MAX_MESSAGE;
                        }
                        // cout << remaining << endl;
                        char receive [size];
                        memcpy(buffer, f, sizeof(filemsg));
                        strcpy(buffer + sizeof(filemsg), filename.c_str());
                        chan.cwrite(buffer, sizeof(filemsg) + filename.length() + 1);
                        chan.cread(receive, sizeof(receive));
                        // cout << receive << endl;
                        currByte += MAX_MESSAGE;
                        remaining -= MAX_MESSAGE;
                        outfile.write(receive, sizeof(receive));
                    }
                    outfile.close();
                    break;
                }
                /*
                 * Single command line flag, no argument, create new channel
                 */ 
                case 'c': {
                    char chan_name [30];
                    opener = NEWCHANNEL_MSG;
                    chan.cwrite(&opener, sizeof(MESSAGE_TYPE));
                    chan.cread(&chan_name, sizeof(chan_name));
                    cout << "Requesting Data on new Pipe" << endl;
                    FIFORequestChannel newChan (chan_name, FIFORequestChannel::CLIENT_SIDE);
                    d->person = 15; d->ecgno = 1;
                    for (int i = 0; i < 6; i++) {
                        d->seconds = 0.004 * i;
                        newChan.cwrite(d, sizeof(datamsg));
                        double result;
                        newChan.cread(&result, sizeof(double));
                        cout << result << endl;
                    }

                    closer = QUIT_MSG;
                    chan.cwrite(&closer, sizeof(MESSAGE_TYPE));
                    break;
                }
            }
        }
    } 
    else if (argc > 3 && argc < 7) {
        while ((opt = getopt(argc, argv, "p:e:")) != -1) {
            switch(opt) {
                case 'p':
                    d->person = atoi(optarg);
                    break;
                case 'e':
                    d->ecgno = atoi(optarg);
            }
        }
        outfile.open("x1.csv");
        gettimeofday(&start, NULL);
        ios_base::sync_with_stdio(false);
        // iterate through all the possible time values
        for (double time = 0; time < 1000; time += 1) {
            // outfile << time * 0.004 << ",";
            // iterate through all the possible ecg nums
            d->seconds = time * 0.004;
            chan.cwrite(d, sizeof(datamsg));
            double result;
            chan.cread(&result, sizeof(double));
            outfile << result << endl;
        }
        gettimeofday(&end, NULL);
        outfile.close();

        double time_taken;
        time_taken = (end.tv_sec - start.tv_sec) * 1e6;
        time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
        cout << "Time Taken for 1000 Data Points: " << fixed << time_taken << setprecision(6) << " seconds" << endl;
    }
    else {
        // gather response from command line args with getopt
        while ((opt = getopt(argc, argv, "p:t:e:")) != -1) {
            switch(opt) {
                case 'p':
                    d->person = atoi(optarg);
                    break;
                case 't':
                    d->seconds = atof(optarg);
                    break;
                case 'e':
                    d->ecgno = atoi(optarg);
                    break;
            } // end switch
        }
        gettimeofday(&start, NULL);
        chan.cwrite(d, sizeof(datamsg));
        double result;
        chan.cread(&result, sizeof(double));
        gettimeofday(&end, NULL);
        cout << "Result: " << result << endl;
        cout << "Time Taken for 1 Data Point: " << end.tv_usec - start.tv_usec << " microseconds" << endl;
    }

    delete d;
    delete f;
	
    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));

    
}
