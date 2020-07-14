#include <stdio.h>



#include <stdlib.h>



#include <fcntl.h>



#include <mqueue.h>



#include <unistd.h>



#include <sys/wait.h>







#define MSG_SIZE	4



#define NAME	"/m_queue"



#define MAX_PRIO	32



void child_counting(int value, int howmany, int interval, int total, int process,char* file_name){



	



	int list = open(file_name, O_RDONLY);



	char buf[howmany][5];



	int compare1 =(process-1)*howmany*5;



	int compare2 =process*howmany*5;



	if(total - process*howmany >0 && value >compare1 && value <compare2){



			howmany = total - howmany*(process-1);



	}







	pread(list, buf, howmany*5, value);



	



	close(list);



	



	int buffer[howmany];





	for(int i = 0; i<howmany; i++){



		buffer[i] = atoi(buf[i]);



	}







  	int range;



	if(10000%interval == 0){



		range = 10000/interval;



	}else{



		range = 10000/interval +1;



	}



	





	int result[range];



	for(int i = 0; i< range; i++){



		int counting = 0;



		for(int a= 0; a<howmany; a++){



			if(buffer[a] >= i*interval && buffer[a]<i*interval+interval){



				counting++;



			}



		}



		result[i] = counting;





	}	





	



	struct mq_attr attr;







	unsigned int prio = 0;



	mqd_t mqdes;







	



	attr.mq_maxmsg = 10;



	attr.mq_msgsize = range;







	mqdes = mq_open(NAME, O_CREAT|O_WRONLY|O_NONBLOCK, 0666, &attr);	







	



	if(mqdes<0){



		perror("mq_open()");



		exit(0);



	}



	

	for(int i = 0; i<range; i++){

		mq_send(mqdes, (char*)&result[i], sizeof(int), i);

	}

	



	mq_close(mqdes);	



	



}



void parent_receive(int interval,int process){



	struct mq_attr attr;



	unsigned int prio = 0;



	mqd_t mqdes;



	



	int range;	



	if(10000%interval == 0){



		range = 10000/interval;



	}else{



		range = 10000/interval +1;



	}



	

	int value;

	int result[range];

	for(int i = 0; i<range;i++){

		result[i] = 0;

	}

	attr.mq_maxmsg = 10;



	attr.mq_msgsize = range;







	mqdes = mq_open(NAME, O_RDWR | O_CREAT |O_NONBLOCK, 0666, &attr);



		

	

	for(int a = 0; a<process;a++){

		wait(NULL);

	for(int i =0 ;i<range;i++){

		mq_receive(mqdes, (char*)&value, range*sizeof(int), &prio);

		result[prio] += value;

	}

	}







	



	for(int i = 0; i<range; i++){



		printf("%d\n",result[i]);



	}



	mq_close(mqdes);



}



int main(int arc, char* argv[]){



	int process = atoi(argv[1]);



	int interval = atoi(argv[2]);



	char* file_name = argv[3];





	FILE* file = fopen(file_name, "r");







	int numbers;



	fscanf(file, "%d", &numbers);



	fclose(file);



	





	int value;

	int s = 1;



	int c = 10;



	while(numbers/c){



		s++;



		c*=10;



	}



	int howmany = numbers/process;







	pid_t pid;



	for(int i = 0; i<process ;i++){



		pid = fork();



		if(pid == 0){

			value = s + howmany*5*i;



			break;



		}



	}







	if(pid==0){



		child_counting(value,howmany,interval,numbers,process,file_name);



	}







	if(pid!=0){

	

		parent_receive(interval,process);



	}



}  