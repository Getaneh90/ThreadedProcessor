/* Assignment Description: In this assignment, you'll write a program that will get you
** familiar with the use of threads, mutual exclusion and condition variables.
** Author: Getaneh Kudna
** Program: line_processor.c
**

*/
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <dirent.h>

// BUFFER SIZE
#define Buf_size 50
#define CHAR_LENGTH 1024
#define NUM_THREADS 4 // num of items that are going to be produced.

char buffer_1[Buf_size][CHAR_LENGTH];

// where input thread will put next data
int prod_index_1 = 0;
int index_cont = 0;
// num of items in the buff
int count1 = 0;

// same for buf 2
char buffer_2[Buf_size][CHAR_LENGTH];
int prod_index_2 = 0;
int index_cont_2 = 0;
int count2 = 0;

// same as buff 1 for buf 3
char buffer_3[Buf_size][CHAR_LENGTH];
int count_3 = 0;
int prod_index3 = 0;
int cont_index = 0;

// Initalize buff
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;
// Initalize buff
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;

// Initialize buff
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;

// get the input from user.
char *user_input()
{

  char *cur_lin = NULL;

  char *User_Input;

  size_t MAX_1 = CHAR_LENGTH;
  User_Input = calloc(CHAR_LENGTH + 1, sizeof(char));

  // Allocating space for the user input
  getline(&cur_lin, &MAX_1, stdin);

  strcpy(User_Input, cur_lin);

  if (strcmp(User_Input, "\n") == 0)
  {
    printf("Error: No input has been provided.\n");
    fflush(stdout);
  }
  return User_Input;
}

// get the data to buf 1
void buff_1_inPut(char *data)
{
  // lock mutex before putting the data in the buff
  pthread_mutex_lock(&mutex1);

  strcpy(buffer_1[prod_index_1], data);

  // Increment for next line
  prod_index_1 = prod_index_1 + 1;
  count1++;
  pthread_cond_signal(&full1);
  // then unlock the mutex
  pthread_mutex_unlock(&mutex1);
}

// get input from user
void *User_input(void *args)
{

  int forInput = 1;
  while (forInput == 1)
  {

    char *data = user_input();
    buff_1_inPut(data);
    if (strncmp(data, "STOP\n", strlen("STOP\n")) == 0)
    {
      break;
    }
  }
  return NULL;
}

/*
get data from buff 1
*/
char *buff_1()
{

  pthread_mutex_lock(&mutex1); // lock mutex
  while (count1 == 0)

    pthread_cond_wait(&full1, &mutex1);
  char *data = buffer_1[index_cont];

  index_cont = index_cont + 1; // incremenat
  count1--;
  pthread_mutex_unlock(&mutex1);
  return data;
}

// now put data in buff 2
void buff_2(char *data)
{

  pthread_mutex_lock(&mutex2); // lock mutex first

  strcpy(buffer_2[prod_index_2], data); // put data in buffer
  prod_index_2 = prod_index_2 + 1;      // Increment
  count2++;

  pthread_cond_signal(&full2); // not empty no more

  pthread_mutex_unlock(&mutex2); // unlock
}

// replace every line separator in input by space
void *line_separator(void *args)
{

  int forInput = 1;
  char *data;

  while (forInput == 1)
  {
    data = buff_1();
    int sizeofbuffer1 = strlen(data);

    int temp = 0;
    while (temp != sizeofbuffer1)
    {
      if (data[temp] == '\n')
      {
        data[temp] = ' ';
      }
      temp++;
    }
    buff_2(data);

    if (strncmp(data, "STOP ", strlen("STOP ")) == 0)
    {
      break;
    }
  }

  return NULL;
}

// get data from buff 2
char *get_buff2()
{

  pthread_mutex_lock(&mutex2); // lock mutex

  while (count2 == 0)
    pthread_cond_wait(&full2, &mutex2);

  char *data = buffer_2[index_cont_2];

  index_cont_2 = index_cont_2 + 1; // increment
  count2--;
  pthread_mutex_unlock(&mutex2); // unlock

  return data;
}

// put data to buff 3
void get_buff3(char *data)
{

  pthread_mutex_lock(&mutex3);         // lock mutex
  strcpy(buffer_3[prod_index3], data); // put data in buff
  prod_index3 = prod_index3 + 1;       // increment the inx where next data will be
  count_3++;
  pthread_cond_signal(&full3);   // sig that the buff is not empty
  pthread_mutex_unlock(&mutex3); // unlock
}

// this function replaces ++ with ^
void *replace_plus_sign()
{

  int forInput = 1;

  char *data;
  int i;
  int j;
  while (forInput == 1)
  {
    data = get_buff2();
    i = 0;

    int item_length = strlen(data);
    char temp;
    while (i != item_length)
    {
      if (data[i] == '+' && data[i + 1] == '+')
      {
        data[i] = '^';
        j = i + 1;

        while (j != item_length)
        {
          temp = data[j + 1];
          data[j] = temp;
          j++;
        }
        data[j + 1] = '\0';
      }
      i++;
    }
    get_buff3(data);
    if (strncmp(data, "STOP ", strlen("STOP ")) == 0)
    {
      break;
    }
  }

  return NULL;
}

// get data from buff
char *buff_3()
{
  pthread_mutex_lock(&mutex3); // lock mutex

  while (count_3 == 0)
    pthread_cond_wait(&full3, &mutex3); // buf empty
  char *data = buffer_3[cont_index];
  cont_index = cont_index + 1; // increament inx where item will be received
  count_3--;
  pthread_mutex_unlock(&mutex3); // unlock mutex

  return data; // return data
}

// output function
void *proccess_output(void *args)
{

  int forInput = 1;
  char *Tot_item = calloc(sizeof(CHAR_LENGTH + 1), sizeof(char));
  int temp;
  char *data;

  while (forInput == 1)
  {
    data = buff_3();
    int itemLen = strlen(data);

    for (int j = 0; j < itemLen; j++)
    {
      Tot_item[temp] = data[j]; // copy chars to total of item string
      temp++;

      if (temp == 80)
      { // make sure it's 80 chars

        for (int i = 0; i < 80; i++)
        { // if its 80 then print
          printf("%c", Tot_item[i]);
        }
        printf("\n");
        temp = 0;
      }
    }

    if (strncmp(data, "STOP ", strlen("STOP ")) == 0)
    {
      break;
    }
  }
  return NULL;
}
int main()
{

  srand(time(0));
  // creaete varibales for the thread ids
  pthread_t thread_1, thread_2, thread_3, thread_4;

  // Create pipeline threads
  pthread_create(&thread_1, NULL, User_input, NULL);
  pthread_create(&thread_2, NULL, line_separator, NULL);
  pthread_create(&thread_3, NULL, replace_plus_sign, NULL);
  pthread_create(&thread_4, NULL, proccess_output, NULL);

  // Wait for the threads to finish executing
  pthread_join(thread_1, NULL);
  pthread_join(thread_2, NULL);
  pthread_join(thread_3, NULL);
  pthread_join(thread_4, NULL);

  return EXIT_SUCCESS;
}

