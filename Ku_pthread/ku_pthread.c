#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

pthread_mutex_t mutex;

struct info{
	int howmany;
	int process;
	int total;
	char* file_name;
	int value;
	int interval;
};
int* finalresult;

void* *thread_function(void *v){ //thread 내에서 사용할 howmany, process, total, file_name, value,interval이 들어있는 구조체
	// 서브 thread에서 돌아가는 함수로, 파일에서 value부터 howmany만큼 숫자를 읽어오고, 이를 interval 별로 나누어서 숫자를 센 뒤
	// mutex를 걸고 결과값을 전역변수에 저장
	
	struct info in = *((struct info *)v);
	int howmany = in.howmany;
	int process = in.process;
	int total = in.total;
	char* file_name = in.file_name;
	int value = in.value;
	int interval = in.interval;
	
	
	
	int compare1 =(process-1)*howmany*5;

	int compare2 =process*howmany*5;

	if(total - process*howmany >0 && value >compare1 && value <compare2){

			howmany = total - howmany*(process-1);

	}

	char buf[howmany][5];
	int buffer[howmany];
	int list = open(file_name, O_RDONLY);


	pread(list, buf, howmany*5, value+1);

	for(int i = 0; i<howmany; i++){
		buffer[i]=atoi(buf[i]);
	}

	close(list);

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
	pthread_mutex_trylock(&mutex);
	
	for(int i = 0; i<range; i++){
		finalresult[i] += result[i];
		
	}
	
	pthread_mutex_unlock(&mutex);
	
}



int main(int arc, char* argv[]){ //thread의 개수, interval, file_name을 입력받음
	int process = atoi(argv[1]);
	int interval = atoi(argv[2]);
	char* file_name = argv[3];
	
	pthread_t p_thread[process];
	
	FILE* file = fopen(file_name, "r");

	int numbers; // 파일에 입력되어있는 숫자의 

	fscanf(file, "%d", &numbers);

	fclose(file);

	int place = 1;

	int c = 10;
	while(numbers/c){
		place++;
		c*=10;
	}

	int howmany = numbers/process;

	int range;

	if(10000%interval == 0){

		range = 10000/interval;

	}else{

		range = 10000/interval +1;

	}

	int r[range];
	finalresult = r;
	for(int i = 0; i<range; i++){
		finalresult[i] = 0;
	}

	struct info value[process];

	for(int i = 0; i<process; i++){
		value[i].value = place;
		value[i].howmany = howmany;
		value[i].process = process;
		value[i].total = numbers;
		value[i].file_name = file_name;
		value[i].interval = interval;
		place += howmany*5;
	} //서브 thread가 사용할 값이 변동되는 것을 막기 위해, 구조체를 통해 thread의 인자 값으로 전달함

	for(int i = 0; i<process; i++){
		pthread_create(&p_thread[i], NULL, thread_function,(void*)&value[i]); //서브 thread 
	}
	
	for(int i = 0; i<process; i++){
		pthread_join(p_thread[i],NULL);
	}

	for(int i = 0; i<range;i++){
		printf("%d\n", finalresult[i]);
	}
		

}
