#include "header.h"

//takes a line and returns an array with items seperated by space
//each value is unique up until there is a space, then new item in array
//along with null at the end
char** split(char *line){

    int holder, current, c;
    char *temp, **arr;

    //number of arguments we get from this line
    c = 0;
    //start of the current string we are on before we get a space
    holder = 0;
    //the current spot of where we're at in the line
    current = 0;
    //our line we are splitting up
    temp = line;

    //while we haven't reached the end of the line/string
    while(*temp){

        //if theres a space, we got some work to do
        if (isspace(*temp)){
            //make sure its valid by its length being greater than 0
            if (current - holder > 0)
                //then we have a valid argument, so add to our count
                c++;
            //now we store the place of where our next argument is going to start
            holder = current + 1;
        }
        //move to the next spot in the string and store that index
        current++;
        //move further in the string (not stored necessarily)
        temp++;
    }

    //here we now know how many items we are going to have in our array
    //add one more item in array because of null
    c++;
    //allocate memory to the array
    arr = (char**)malloc(sizeof(char*) * c);

    //same as above
    c = 0;
    holder = 0;
    current = 0;
    //take our string from the top again
    temp = line;

    //same as above
    while(*temp){

        //same as above
        if (isspace(*temp)){
            //if its a valid string
            if (current - holder > 0){
                //allocate memory for the current string
                arr[c] = malloc(sizeof(char) * (current - holder + 1));
                //take the indexes of the current string and add it into our array
                strncpy(arr[c], line + holder, current - holder);
                //null terminator for this entry
                arr[c][current - holder] = 0;
                //we added an item into our array, increment the count
                c++;
            }
            //go to the index of the next string after the space
            holder = current + 1;
        }
        //same as above
        current++;
        temp++;
    }
    //for our execution we need the last item in the array to be null, so here
    arr[c] = NULL;

    //return the array we've made for this particular line
    return arr;
}

//frees up the array memory
void freeArray(char **array){

    char **temp = array;
    //iterate over the array and free each string
    while(*temp){

        free(*temp);
        temp++; //go to next item in array
    }
    //free the array itself
    free(array);
}

//runs the items we split up and put into an array from earlier
pid_t execute(char *program, char **array){

    int sig;
    pid_t pid;
    sigset_t sigset;

    //initalize the signal
    sigemptyset(&sigset);
    //add SIGUSR1 into the signal set
    sigaddset(&sigset, SIGUSR1);
    //tells the signal to block
    sigprocmask(SIG_BLOCK, &sigset, NULL);


    pid = fork();
    //if we weren't able to create the child
    if (pid < 0){
        perror("fork failed");
        exit(-1);
    }

    //if we were able to create the child process
    if (pid == 0){
        //wait for signal before executing
        if (sigwait(&sigset, &sig) == 0)
        //then execute with our command and array using execvp
            execvp(program, array);
        //if for some reason it does finish, then we give back an error
        exit(-1);
    }

    //return the ID
    return pid;
}


int main(int argc, char *argv[]){
    FILE *f;
    char *line;
    pid_t *pids;
    size_t i, linesz, count;

    //we atleast need the part and the workload file we're running off of
    if (argc != 2){
        printf("you need to add the workload file you want to run, please try again\n");
        exit(-1);
    }

    //now lets see if we can open the file
    f = fopen(argv[1], "r");
    //if for some reason we weren't able to open the file, give error
    if (f == NULL){
        perror("failed to open workload_file");
        exit(-1);
    }

    count = 0;
    linesz = 1000;
    //allocate memory for lines and IDs
    line = malloc(sizeof(char) * linesz);
    pids = malloc(sizeof(pid_t) * 1000);

    //store the current line into our line variable
    //continue running so long as there are lines, not end of file (>0)
    while(getline(&line, &linesz, f) > 0){
        //flush our file stream when we do a new line
        fflush(f);

        //split our line up into an array seperated by spaces
        char **array = split(line);

        //store the IDs to be checked on later on
        //pass the command and array associated with it
        pids[count] = execute(array[0], array);
        //prepare for the next ID
        count++;

        //free the array we created from split
        freeArray(array);
    }

    //close file
    fclose(f);

    //send start signal to all the processess
    for (i = 0; i < count; i++){
        printf("started pid: %d\n", pids[i]);
        kill(pids[i], SIGUSR1);
    }

    ///send stop signal to all the processess
    for (i = 0; i < count; i++){
        printf("stopped pid: %d\n", pids[i]);
        kill(pids[i], SIGSTOP);
    }

    //send continue signal to all the processess
    for (i = 0; i < count; i++){
        printf("continued pid: %d\n", pids[i]);
        kill(pids[i], SIGCONT);
    }
    
    //check on the child processess from above
    for (i = 0; i < count; i++)
        //wait until the i process is finished before moving on to check the next
        waitpid(pids[i], NULL, 0);

    //free up memory from earlier
    free(line);
    free(pids);

    return 0;
}
