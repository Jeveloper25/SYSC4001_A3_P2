#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <random>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

using namespace std;

//This struct contains all of the variables that will be shared amongst TAs
struct shared_vars{
	char rubric[5];
	int cur_exam;
	int student_id;
	int num_ta; //Total number of TAs
	int ta_marking; //Number of TAs actively marking
	int rubric_line = 0;
	int exam_line = 0;
};

int load_exam(int& exam_num);

unsigned int getRandom(unsigned int max);

int main(int argc, char **argv) {


	//Create and attach shared memory
	int mem_id = shmget(IPC_PRIVATE, sizeof(shared_vars), IPC_CREAT | IPC_EXCL | 0666); 
	if (mem_id == -1) {
		cerr << "SHMGET failed" << endl;
	}
	shared_vars *mem = (shared_vars *)shmat(mem_id,(char *)0, 0);
	if (mem == (void *)-1){
		cerr << "SHMAT failed" << endl;
	}
	
	mem->num_ta = atoi(argv[1]);
	mem->ta_marking = mem->num_ta;
	mem->cur_exam = 1;

	/*
	 * This section reads from the rubric file and stores the answers in an array.
	 * When the TA's 'read' and 'write' to/from the rubric they will work with this array instead of the file itself
	 */
	fstream rub("./rubric.txt");
	if (rub.is_open()) {
		string exercise;
		int ex_index = 0;
		while (getline(rub, exercise)) {
			mem->rubric[ex_index] = exercise[2];
			ex_index++;
		}
		rub.close();
	} else {
		cerr << "Rubric couldn't be opened" << endl;
	}
	mem->student_id = load_exam(mem->cur_exam);

	//Create desired amount of child processes
	for (int i = 1; i < mem->num_ta; i++) {
		int pid = fork();
		if (pid == -1) {
			cerr << "FORK failed" << endl;
		} else if (pid == 0) {
			break;
		}
	}

	int ta_id = getpid();
	//cout << ta_id << endl;

	while (true){
		//Print if a ta has begun checking rubric
		cout << endl << "TA: " << ta_id << " Reviewing rubric" << endl;
			
		//Loop for reviewing rubric, uses shared variable denoting next line for coordination between processes.
		while (mem->rubric_line < 5) {
			usleep((getRandom(5) + 5) * 100000);
			if (7 < getRandom(10)) {
				mem->rubric[mem->rubric_line] = mem->rubric[mem->rubric_line] % 65 + 66;
				char answer = mem->rubric[mem->rubric_line];
				cout << "TA: " << ta_id << " Corrected: " 
					<< mem->rubric_line + 1 << " " << char(answer - 1) 
					<< " -> " << answer << endl;
			}
			mem->rubric_line++;
		}
		
		//Loop for marking exams, uses shared variable denoting next line for coordination between the processes.
		while (mem->exam_line < 5) {
			usleep((getRandom(1) + 1) * 1000000);
			cout << "TA: " << ta_id << " Marking Question: " 
				<< mem->exam_line + 1 << " Student: " 
				<< mem->student_id << endl;
			mem->exam_line++;
		}
		
		//cout << "MARKING: " << ta_id << ", " << mem->ta_marking << endl;
		//TAs finish execution if student id is 9999, loads the next exam otherwise.
		if (mem->student_id == 9999) {
			break;
		}
		//Prepare next exam if all TAs have finished
		mem->ta_marking--;
		if (mem->ta_marking <= 0) {
			mem->student_id = load_exam(mem->cur_exam);
			mem->rubric_line = 0;
			mem->exam_line = 0;
			mem->ta_marking = mem->num_ta;
			//cout << "LOADED: " << ta_id << endl;
		} else {
			while (mem->ta_marking > 0){}; //Wait for all TAs to finish marking
		}
	}

	shmctl(mem_id, IPC_RMID, (shmid_ds *)0);
	return EXIT_SUCCESS;
}

/*
* This function reads and returns the student id from the requested exam.
* It takes in the shared exam_num variable which is used to keep track
* of the top of the exam pile.
* The function increments the exam_num before returning.
*/
int load_exam(int& exam_num) {
	fstream exam("./exams/s" + to_string(exam_num) + ".txt");
	string student_id;
	if (exam.is_open()) {
		getline(exam, student_id);
		exam.close();
	} else {
		cerr << "Exam" << to_string(exam_num) << "couldn't be opened" << endl;
		student_id = "-1";
	}
	exam_num++;
	return atoi(student_id.c_str());
}

/**
 * RANDOM NUMBER GENERATOR
 * *INCLUDES MAX*
 */
unsigned int getRandom(unsigned int max)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, max);
    return dist(gen);
}