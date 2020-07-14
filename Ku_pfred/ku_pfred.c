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
	// value : 파일을 읽기 시작할 부분의 버퍼 값
	// howmany : 파일에 입력된 숫자 갯수/프로세스 갯수 => 각 프로세스가 읽을 숫자의 갯수
	// interval : main에서 입력받은 interval 값
	// total : 파일에 입력된 총 숫자 개수
	// process : main에서 입력받은 process 값
	// file_name : main에서 입력받은 파일 이름
	
	//파일에서 숫자를 읽어오고, 이를 구간 별로 나누어 결과 값을 저장하고
	//이를 message queue를 이용해서 parent process에 결과값을 보내는 
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
	//interval : main에서 입력받은 interval 값
	//process : main에서 입력받은 process 값
	
	// message queue를 통해 받은 결과 값을 
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



int main(int arc, char* argv[]){  //process의 갯수, interval, 그리고 file_name을 입력받음
	int process = atoi(argv[1]); //process의 갯수
	int interval = atoi(argv[2]); //interval
	char* file_name = argv[3]; //파일 이름
	FILE* file = fopen(file_name, "r");

	int numbers;
	fscanf(file, "%d", &numbers); //파일 맨 앞의 입력된 숫자의 갯수를 읽어옴
	fclose(file);
	
	int value;
	int s = 1;
	int c = 10;


	while(numbers/c){
		s++;
		c*=10;
	}

	int howmany = numbers/process; //한 개의 프로세스마다 읽어야 하는 숫자의 갯수 정해주기
	pid_t pid;
	for(int i = 0; i<process ;i++){
		pid = fork();  // child process 생성
		if(pid == 0){  // child process일 때, 각각 담당한 구간을 나눔
			value = s + howmany*5*i; //*5인 이유 : 공백 + 네자리 자연수이기 때문
			break;
		}
	}

	if(pid==0){ //child process일 경우 child_counting 실행
		child_counting(value,howmany,interval,numbers,process,file_name);
	}


	if(pid!=0){ //parent process일 경우, parent_receive 
		parent_receive(interval,process);
	}
}  
