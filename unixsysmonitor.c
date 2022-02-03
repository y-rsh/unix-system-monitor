#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
//#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <utmp.h>

void breaker(){
    printf("------------------------------------------------\n");
}

//returns array populated with data collected over sampleNumber increments.
//masterData[0] contains CPU % utilization in each sample.
//masterData[1] contains mem % utilization in each sample. 
void fetchSysData(int sampleNumber, double inputArray[][sampleNumber+1], bool system, int sampleDelta){

    //initializing constatns and old times to 0 for later comparison.
    char *singleOpTime;

    char dataString[75];
        
    int oldTime = 0;

    int oldIdle = 0;

    int idle;

    //iterating N+1 times, since first iteration will have total CPU util times not relative ones.
    for(int i = 0; i < sampleNumber + 1; i++){
        int totalTime = 0; 
        int n = 0;
        FILE *cpudata = fopen("/proc/stat", "r");
        //reads line 1 of /proc/stat and inititalizes into dataString for processing.
        fgets(dataString, 75, cpudata);
        //printf("fgets called \n");
        //populates singleOpTime with first "word" in /proc/stat, initializes reading of CPU times.
        singleOpTime = strtok(dataString, " ");


        //skips "cpu " from start of string.
        singleOpTime = strtok(NULL, " ");

        //printf("strtok called \n");

        while(singleOpTime != NULL){
            totalTime += atoi(singleOpTime);

            //need to keep track of time spent idling to find utlization %.
            if(n==3){
                idle = atoi(singleOpTime);
                //printf("fetched idle is %d\n", idle);
            }
            
            singleOpTime = strtok(NULL, " ");
            n++;

            

            //printf("token loop called \n");
        }

        //totalTime now contains total CPU runtime, idle contains time idling
        //deltaX = (deltaIdle / deltaTotalTime) gives percentage of time spent idling, so (1 - deltaX) will give percentage time not idling (since last sample).
        double percentage;
        if(i == 0){
            double percentage = 0;
            inputArray[0][i] = percentage;
        }
        else{
            double percentage = (1-((double)(idle - oldIdle) / (double)(totalTime - oldTime))) * 100;
            inputArray[0][i] = percentage;
        }

        //update old idle and old time to current values, so next iteration can compare to previous for relative cpu use.
        oldIdle = idle;
        oldTime = totalTime;

        //reading system information from sysinfo struct type, populating with sysinfo().
        struct sysinfo *info = (struct sysinfo*) malloc (sizeof(struct sysinfo));
        sysinfo(info);

        //want total used physical ram in gigabytes, so divide by number of bytes in gigabyte (2^30)
        double usedPhysRam = ((double)(info->totalram - info->freeram)) / (double)1073741824;

        //want total used virtual ram in gigabytes, so adding physical ram to swap space should give us this.
        double usedVirtualRam = ((double)(info->totalswap - info->freeswap) / (double)1073741824) + (double)usedPhysRam;


        //printf("%d\n", usedPhysRam);

        inputArray[1][i] = usedPhysRam;

        inputArray[2][i] = usedVirtualRam;

        fclose(cpudata);

        //wait sampleDelta seconds before repeating loop.
        sleep(sampleDelta);

    }

}

//evaluate whether or not command is valid, update constants to command specification.
void updateconstants(char *command, bool *user, bool *system, bool *graphics, int *sampleNumber, int *sampleDelta, bool *correctinput){

    //check for --system input.
    if(strcmp(command, "--system") == 0){
        //check for case that --user has been called.
        if(*user == 1 && *system == 0){
            printf("Cannot accept --user and --system simultaneously.\n");
            *correctinput = 0;
        }
        //prevents displaying user info.
        else{
            *user = 0;
        }
    }

    //check for --user input.
    else if(strcmp(command, "--user") == 0){
        //check for case that --system has been called.
        if(*user == 0 && *system == 1){
            printf("Cannot accept --user and --system simultaneously.\n");
            *correctinput = 0;
        }
        //prevents displaying system info
        else{
            *system = 0;
        }
    }

    //check for --graphics input.
    else if(strcmp(command, "--graphics") == 0){
        *graphics = 1;
    }

    //check for --samples=N input.
    else if(strncmp(command, "--samples=", 10) == 0){
        //check if N is empty, i.e. command[11] == \0
        if(command[10]=='\0'){
            printf("--samples=N command requires integer N input.\n");
            *correctinput = 0;
        }

        //accumulator variable for reading value from N
        int counter = 0;

        //check if N is correct integer and if so, read integer from command.
        for(int i=(strlen(command)-1); i >= 10; i--){
            //valid number case
            if(isdigit(command[i])){
                int x = command[i] - '0';
                //adding digits in integer in ascending powers of 10.
                counter += x * pow(10, (strlen(command)-1)-i);
            }
            //invalid number case 
            else{
                printf("--samples=N requires N to be an integer.\n");
                *correctinput = 0;
                break;
            }
        }
        //update samplenumber to input N value.
        *sampleNumber = counter;
    }

    //check for --tdelay=T input.
    else if(strncmp(command, "--tdelay=", 9) == 0){
        //check if T is empty, i.e. command[10] == \0
        if(command[10]=='\0'){
            printf("--tdelay=T command requires integer T input.\n");
            *correctinput = 0;
        }

        //accumulator variable for reading value from T
        int counter = 0;

        //check if T is correct integer and if so, read integer from command.
        for(int i=(strlen(command)-1); i > 9; i--){
            //valid number case
            if(isdigit(command[i])){
                int x = command[i] - '0';
                //adding digits in integer in ascending powers of 10.
                counter += (x * pow(10, (strlen(command)-1)-i));
            }
            //invalid number case 
            else{
                printf("--tdelay=T requires T to be an integer.\n");
                *correctinput = 0;
                break;
            }
        }
        //update sampleDelta to input T value.
        *sampleDelta = counter;
    }

    //command is not valid.
    else{
        printf("Please enter valid command line arguments.\n");
        *correctinput = 0;
    }
}

void display(bool user, bool system, bool graphics, int sampleNumber, int sampleDelta){
    struct sysinfo info;
    sysinfo(&info);

    struct utsname nameInfo;
    uname(&nameInfo);

    //printing basic system information -- always included.
    breaker();
    printf("::: System Information :::\n");
    printf("System Name: %s\n", nameInfo.sysname);
    printf("Version: %s\n", nameInfo.version);
    printf("Release: %s\n", nameInfo.release);
    printf("Machine Name: %s\n", nameInfo.machine);

    breaker();

    //handling case of displaying system data for graphic and nongraphic case.
    if(system){
        printf("%d samples : %ds between samples\n", sampleNumber, sampleDelta);
        breaker();
        printf("::: Memory :::\n");
        printf("Phys. Used / Tot. -- Virtual Used / Tot.\n");
        //double masterData[3][sampleNumber+1];
        double masterData[3][sampleNumber+1];
        //double masterData[3][200];

        fetchSysData(sampleNumber, masterData, system, sampleDelta);

        //double **masterData = fetchSysData(system, sampleNumber, sampleDelta);
        //printf("did system data break?\n");
        struct sysinfo info;

        sysinfo(&info);

        double totalPhysRam = (double)info.totalram / (double)1073741824;

        double totalVirtualRam = (double)(info.totalram + info.totalswap) / (double)1073741824;

        //setting up array of 2 arrays of strings that will be displayed as graphic representation of system info.
        double physMemSum = 0;
        double virtualMemSum = 0;
        char graphicsArray[sampleNumber][102];

        //graphic case
        if(graphics){
            char maxString1[102] = "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
            char maxString2[66] = "################################################################*";
            char maxString3[66] = "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::@";
            int delta = 0;
            //initialize array for storing strings of graphic representations of memory change and cpu util.


            for(int i = 1; i < sampleNumber+1; i++){
                //this inserts CPU percentage under integer division + 1 * '|' into our graphicsArray.
                int numLines = (div(masterData[0][i], 1).quot)+1;
                sprintf(graphicsArray[i-1], "%.*s", numLines, maxString1);
                
                //define delta as the integer division of "previous" memory used - "current" memory used. (first mem will always have 0 change)
                if(i != 1){ 
                    delta = round(10 * (masterData[1][i]-masterData[1][i-1]));
                }

                char graphicsString[66];

                strcpy(graphicsString, maxString3);

                //positive relative change displayed as ####*
                if(delta > 63){
                    delta = 63;
                }
                if(delta < -63){
                    delta = -63;
                }

                if(delta > 0){
                    strcpy(graphicsString, maxString3 + (63 - delta));
                    //sprintf(graphicsString, "%s", maxString2 + (63 - delta));
                    //graphicsString = strrev(graphicsString);  
                }
                //negative relative change displayed as :::::@
                else if(delta < 0){
                    strcpy(graphicsString, maxString3 + (63 + delta));
                    //sprintf(graphicsString, "%s", maxString3 + (63 + delta));
                    //graphicsString = strrev(graphicsString));
                }
                else{
                    strcpy(graphicsString, "|");
                    //printf("graphics string is empty \n");
                }
                //at this point graphicsArray should be populated with strings corresponding to those that need to be printed
                //for graphical representations of CPU%.

                printf("%.2fgb / %.2fgb -- %.2fgb / %.2fgb   %s\n", masterData[1][i], totalPhysRam, masterData[2][i], totalVirtualRam, graphicsString);
                physMemSum += masterData[1][i];
                virtualMemSum += masterData[2][i];
            }
        }

        //nongraphics case
        else{
            for(int i=1; i<sampleNumber + 1; i++){
                printf("%.2fgb / %.2fgb -- %.2fgb / %.2fgb\n", masterData[1][i], totalPhysRam, masterData[2][i], totalVirtualRam);
                physMemSum += masterData[1][i];
                virtualMemSum += masterData[2][i];
            }
        }

        printf("----------\n");
        printf("Avg. over %d trials:\n", sampleNumber);
        printf("%.2fgb / %.2fgb -- %.2fgb / %.2fgb\n", (physMemSum / (double)sampleNumber), totalPhysRam, (virtualMemSum / (double)sampleNumber), totalVirtualRam);
        breaker();

        //reading CPU info from /proc/cpuinfo file to include name and num of cores
        printf("::: CPU Info :::\n");
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        char line[150];
        for(int i = 0; i < 13; i++){
            fgets(line, sizeof(line), cpuinfo);
            if(i == 4 || i == 12){
                printf("%s\n", line);
            }
        }
        fclose(cpuinfo);
        printf("Total Utilization: %.2f%% \n", masterData[0][sampleNumber]);
        printf("----------\n");
        //graphics case for displaying CPU information.
        if(graphics){
            for(int i = 1; i < sampleNumber+1; i++){
                printf("Trial %03d : %.2f%% : %s \n", i, masterData[0][i], graphicsArray[i-1]);
            }
        }
        breaker();
    }

    //handling displaying user information
    if(user){
        struct utmp *userinfo;
        printf("::: Users :::\n");
        setutent();
        int numUsers = 1;
        //iterating through utmp database to read all user information and displaying it
        userinfo = getutent();
        while(userinfo != NULL){
            if(userinfo->ut_type == USER_PROCESS){
                printf("%s : %s, %s\n", userinfo->ut_user, userinfo->ut_line, userinfo->ut_host);
                numUsers++;
            }
        
            //numUsers++;
            userinfo = getutent();
        } 
        printf("----------\n");
        printf("Total Users: %d\n", numUsers);
        breaker();
    }


}



int main(int argc, char **argv){
    //initialize values for information to display
    bool user = 1;
    bool system = 1;
    bool graphics = 0;
    int sampleNumber = 10;
    int sampleDelta = 1;
    bool correctinput = 1;

    //printf("argc is: %d\n", argc);
    //check for valid number of arguments
    //since --user and --system are exclusive we have max. 4 CLAs at once.
    if(argc >= 5){
        printf("Too many arguments!\n");
    }
    //check for positional integer arguments for samples=N and tdelay=T (will only happen in case argc == 3).
    if(argc == 3){
        char *cPointer = argv[1];
        bool areIntegers = 1;
        //check if first input is strictly an integer.
        while(*cPointer != '\0'){
            if(!isdigit(*cPointer)){
                areIntegers = 0;
            }
            cPointer++;
        }
        cPointer = argv[2];
        //check if second input is strictly an integer.
        while(*cPointer != '\0'){
            if(!isdigit(*cPointer)){
                areIntegers = 0;
            }
            cPointer++;
        }
        //update information if correct formatting.
        if(areIntegers){
            sampleNumber = atoi(argv[1]);
            sampleDelta = atoi(argv[2]);
        }
        //check if they are other valid inputs.
        else{
            for(int i = 1; i < argc; i++){
                updateconstants(argv[i], &user, &system, &graphics, &sampleNumber, &sampleDelta, &correctinput);
            }
        }
    }
    //check for other CLAs
    else if(2 <= argc < 5){
        for(int i = 1; i < argc; i++){
            //update data for single command argv[i].
            updateconstants(argv[i], &user, &system, &graphics, &sampleNumber, &sampleDelta, &correctinput);
            if(correctinput == 0) return 0;
        }
    }
    display(user, system, graphics, sampleNumber, sampleDelta);
    return 0;
}



