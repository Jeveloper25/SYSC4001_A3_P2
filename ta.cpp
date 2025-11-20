#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

string load_exam(int& exam_num);

unsigned int getRandom(unsigned int max);

int main() {

	int cur_exam = 1;
	char rubric[5];
	/*
	 * This section reads from the rubric file and stores the answers in an array.
	 * When the TA's 'read' and 'write' to/from the rubric they will work with this array instead of the file itself
	 */
	fstream rub("./rubric.txt");
	if (rub.is_open()) {
		string exercise;
		int ex_index = 0;
		while (getline(rub, exercise)) {
			rubric[ex_index] = exercise[2];
			ex_index++;
		}
		rub.close();
	} else {
		cerr << "Rubric couldn't be opened" << endl;
	}
	string student_id = load_exam(cur_exam);

	int rubric_line = 0;
	int exam_line = 0;
	bool loaded = true;
	int ta_marking = 1;
	while (true){
		
		while (!loaded){};
		loaded = false;
		//Loop for reviewing rubric, uses shared variable denoting next line for coordination between processes.
		while (rubric_line < 5) {
			if (7 < getRandom(10)) {
				rubric[rubric_line]++;
				char answer = rubric[rubric_line];
				cout << "TA: " << 1 << " Exercise: " 
					<< rubric_line + 1 << " " << char(answer - 1) 
					<< " -> " << answer << endl;
			}
			rubric_line++;
		}
		
		//Loop for marking exams, uses shared variable denoting next line for coordination between the processes.
		while (exam_line < 5) {
			cout << "TA: " << 1 << " Question: " 
				<< exam_line + 1<< " Student: " 
				<< student_id << endl;
			exam_line++;
		}
		ta_marking--;

		//TA finishes execution if student id is 9999, loads the next exam otherwise.
		if (student_id == "9999") {
			break;
		}
		if (!loaded && ta_marking == 0) {
			student_id = load_exam(cur_exam);
			loaded = true;
			rubric_line = 0;
			exam_line = 0;
			ta_marking = 1;
		}
		cout << endl;
	}

	return EXIT_SUCCESS;
}

/*
* This function reads and returns the student id from the requested exam.
* It takes in the shared exam_num variable which is used to keep track
* of the top of the exam pile.
* The function increments the exam_num before returning.
*/
string load_exam(int& exam_num) {
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
	return student_id;
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

