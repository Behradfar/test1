// FUS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <iterator>
#include <time.h>
#include <thread>
#include <string_view>
using namespace std;


// function prototypes
void convolve(const vector<vector<int>>&, vector<vector<int>>&);
void energy(const vector<vector<int>>&, unsigned long long&  );
void parser(const vector<string>& par_strings, vector <vector <int>>& v);



int main() {

	clock_t t = clock();
	//Open file in binary mode
	std::fstream is("Phantom.txt", std::ios::in | std::ios::binary);
	//read the size
	is.seekg(0, std::ios::end);
	size_t data_size = is.tellg();
	is.seekg(0, std::ios::beg);
	//read into memory
	std::unique_ptr<char[]> data(new char[data_size]);
	is.read(data.get(), data_size);
			
	// Write input as vector of strings
	std::vector<string> strings;
	strings.reserve(data_size/10); 
	for (size_t i = 0, start = 0; i < data_size; ++i)
	{
		if (data[i] == '\n') 
		{
			strings.emplace_back(data.get() + start, i - start);
			start = i + 1;
		}
	}
	
	
	// Divide vector of strings to 4 vectors
	int thread_size(4);
	int substrings_size = strings.size() / thread_size;
	std::vector<std::vector<string>> sub_strings;
	for (std::size_t i(0); i < thread_size; ++i) {
		sub_strings.emplace_back(strings.begin() + i * substrings_size, strings.begin() + (i + 1) * substrings_size);
	}


	int subvec_size = strings.size() / thread_size;
	std::vector<std::vector<std::vector <int>>> sub_vectors(subvec_size);

	// Parse input string and write them in vector of int using 4 threads
	std::thread thread1(parser, std::ref(sub_strings[0]), std::ref(sub_vectors[0]));
	std::thread thread2(parser, std::ref(sub_strings[1]), std::ref(sub_vectors[1]));
	std::thread thread3(parser, std::ref(sub_strings[2]), std::ref(sub_vectors[2]));
	std::thread thread4(parser, std::ref(sub_strings[3]), std::ref(sub_vectors[3]));

	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	
	// Concatinate sub matrices to form the image
	vector<vector<int>> image;
	image.reserve(sub_vectors[0].size() + sub_vectors[1].size() + sub_vectors[2].size() + sub_vectors[3].size());
	image.insert(image.end(), sub_vectors[0].begin(), sub_vectors[0].end());
	image.insert(image.end(), sub_vectors[1].begin(), sub_vectors[1].end());
	image.insert(image.end(), sub_vectors[2].begin(), sub_vectors[2].end());
	image.insert(image.end(), sub_vectors[3].begin(), sub_vectors[3].end());

	// Initialize a matrix for filtered image and filter the image
	vector<vector<int>> output_image(image.size(), vector<int>(image[0].size(),0));
	convolve(image, output_image);
	
	// Calculate energy of image pixels using 4 threads
	unsigned long long sum1 = 0;
	unsigned long long sum2 = 0;
	unsigned long long sum3 = 0;
	unsigned long long sum4 = 0;

	std::thread th1(energy, std::ref(sub_vectors[0]), std::ref(sum1));
	std::thread th2(energy, std::ref(sub_vectors[1]), std::ref(sum2));
	std::thread th3(energy, std::ref(sub_vectors[2]), std::ref(sum3));
	std::thread th4(energy, std::ref(sub_vectors[3]), std::ref(sum4));

	th1.join();
	th2.join();
	th3.join();
	th4.join();

	cout << "\nsum1 + summ2 + sum3 + sum4: " << sum1 + sum2 +sum3 + sum4 << endl;

	t = clock() - t;
	std::cout << "\n\nCalculation time  " << (((float)t) / CLOCKS_PER_SEC) << " ms\n";

	return 0;
}

// High-pass filter function
void convolve(const vector<vector<int>>& matrix, vector<vector<int>>& filtered) {
	int n_row = matrix.size();
	int n_col = matrix[0].size();
	
	// zero padding the matrix
	vector <int> zeros(n_col,0);
	vector<vector<int>> pmatrix = matrix;
	pmatrix.push_back(zeros);
	pmatrix.insert(pmatrix.begin(), zeros);
	
	vector<vector<float>> kernel = {
							{-1 / 9.0, -1 / 9.0, -1 / 9.0},
							{-1 / 9.0, 8 / 9.0, -1 / 9.0},
							{-1 / 9.0, -1 / 9.0, -1 / 9.0}
	};
	for (int i = 1; i < n_row  ; ++i) {
		for (int j = 1; j < n_col - 1; ++j) {
			filtered[i][j] = 
				(int)(kernel[0][0] * pmatrix[i - 1][j - 1] +
				kernel[0][1] * pmatrix[i - 1][j] +
				kernel[0][2] * pmatrix[i - 1][j + 1] +
				kernel[1][0] * pmatrix[i][j - 1] +
				kernel[1][1] * pmatrix[i][j] +
				kernel[1][2] * pmatrix[i][j + 1] +
				kernel[2][0] * pmatrix[i + 1][j - 1] +
				kernel[2][1] * pmatrix[i + 1][j] +
				kernel[2][2] * pmatrix[i + 1][j + 1]);
		}
	}
}

// Energy calculator function
void energy(const vector<vector<int>>& matrix, unsigned long long & sum) {
	for (auto row : matrix) {
		for (auto el : row) {
			sum = sum + el * el;
		}
	}
}
// Parse the string and write it to vector of vector of int
void parser(const vector <string> & par_strings, vector <vector <int>> & v) {
	for (size_t i = 0; i < par_strings.size(); ++i)
	{
		std::istringstream iss(par_strings[i]);
		std::vector<int> tempv(std::istream_iterator<int>{iss}, std::istream_iterator<int>());
		v.push_back(tempv);
		
	}
}
